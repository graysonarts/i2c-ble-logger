use anyhow::Result;
use chrono::{DateTime, Local};
use crossterm::{
    event::{self, DisableMouseCapture, EnableMouseCapture, Event, KeyCode, KeyEventKind, KeyModifiers},
    execute,
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
};
use ratatui::{
    backend::{Backend, CrosstermBackend},
    layout::{Constraint, Direction, Layout, Rect},
    style::{Color, Modifier, Style},
    text::{Line, Span, Text},
    widgets::{Block, Borders, List, ListItem, ListState, Paragraph, Wrap},
    Frame, Terminal,
};
use std::io;
use tokio::sync::mpsc;

#[derive(Debug, Clone)]
pub enum AppMessage {
    I2CData(String),
    StatusUpdate(String),
    CommandResponse(String),
}

#[derive(Debug, Clone)]
pub struct I2CLogEntry {
    pub timestamp: DateTime<Local>,
    pub data: String,
}

#[derive(Debug, Clone)]
pub struct StatusEntry {
    pub timestamp: DateTime<Local>,
    pub message: String,
}

pub struct App {
    pub should_quit: bool,
    pub i2c_logs: Vec<I2CLogEntry>,
    pub status_logs: Vec<StatusEntry>,
    pub command_input: String,
    pub input_mode: InputMode,
    pub scroll_offset: usize,
    pub status_scroll_offset: usize,
    pub command_sender: mpsc::UnboundedSender<String>,
    pub i2c_list_state: ListState,
    pub status_list_state: ListState,
}

#[derive(Debug, PartialEq)]
pub enum InputMode {
    Normal,
    Editing,
}

impl App {
    pub fn new(command_sender: mpsc::UnboundedSender<String>) -> Self {
        Self {
            should_quit: false,
            i2c_logs: Vec::new(),
            status_logs: Vec::new(),
            command_input: String::new(),
            input_mode: InputMode::Normal,
            scroll_offset: 0,
            status_scroll_offset: 0,
            command_sender,
            i2c_list_state: ListState::default(),
            status_list_state: ListState::default(),
        }
    }

    pub fn handle_message(&mut self, message: AppMessage) {
        match message {
            AppMessage::I2CData(data) => {
                // Split on newlines and add each line as separate entry
                for line in data.lines() {
                    if !line.trim().is_empty() {
                        self.i2c_logs.push(I2CLogEntry {
                            timestamp: Local::now(),
                            data: line.to_string(),
                        });
                    }
                }
                // Keep only last 1000 entries
                while self.i2c_logs.len() > 1000 {
                    self.i2c_logs.remove(0);
                }
                // Auto-scroll to bottom
                self.scroll_offset = self.i2c_logs.len().saturating_sub(1);
                if !self.i2c_logs.is_empty() {
                    self.i2c_list_state.select(Some(self.i2c_logs.len() - 1));
                }
            }
            AppMessage::StatusUpdate(message) | AppMessage::CommandResponse(message) => {
                // Replace literal \n with actual newlines, then split
                let processed_message = message.replace("\\n", "\n");
                for line in processed_message.lines() {
                    if !line.trim().is_empty() {
                        self.status_logs.push(StatusEntry {
                            timestamp: Local::now(),
                            message: line.to_string(),
                        });
                    }
                }
                // Keep only last 100 status entries
                while self.status_logs.len() > 100 {
                    self.status_logs.remove(0);
                }
                // Auto-scroll to bottom
                self.status_scroll_offset = self.status_logs.len().saturating_sub(1);
                if !self.status_logs.is_empty() {
                    self.status_list_state.select(Some(self.status_logs.len() - 1));
                }
            }
        }
    }

    pub fn handle_key_event(&mut self, key: KeyCode) {
        match self.input_mode {
            InputMode::Normal => match key {
                KeyCode::Char('q') => self.should_quit = true,
                KeyCode::Char('c') => self.input_mode = InputMode::Editing,
                KeyCode::Up => {
                    self.scroll_offset = self.scroll_offset.saturating_sub(1);
                }
                KeyCode::Down => {
                    if self.scroll_offset < self.i2c_logs.len().saturating_sub(1) {
                        self.scroll_offset += 1;
                    }
                }
                KeyCode::PageUp => {
                    self.scroll_offset = self.scroll_offset.saturating_sub(10);
                }
                KeyCode::PageDown => {
                    self.scroll_offset = (self.scroll_offset + 10).min(self.i2c_logs.len().saturating_sub(1));
                }
                _ => {}
            },
            InputMode::Editing => match key {
                KeyCode::Enter => {
                    let trimmed_command = self.command_input.trim();
                    if !trimmed_command.is_empty() {
                        let _ = self.command_sender.send(trimmed_command.to_string());
                        self.command_input.clear();
                    }
                    self.input_mode = InputMode::Normal;
                }
                KeyCode::Char(c) => {
                    self.command_input.push(c);
                }
                KeyCode::Backspace => {
                    self.command_input.pop();
                }
                KeyCode::Esc => {
                    self.command_input.clear();
                    self.input_mode = InputMode::Normal;
                }
                KeyCode::F(12) => {
                    // Emergency quit from editing mode
                    self.should_quit = true;
                }
                _ => {}
            },
        }
    }
}

pub async fn run_tui(
    mut app: App,
    mut message_receiver: mpsc::UnboundedReceiver<AppMessage>,
) -> Result<()> {
    // Setup terminal
    enable_raw_mode()?;
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen, EnableMouseCapture)?;
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;

    let result = run_app(&mut terminal, &mut app, &mut message_receiver).await;

    // Restore terminal
    disable_raw_mode()?;
    execute!(
        terminal.backend_mut(),
        LeaveAlternateScreen,
        DisableMouseCapture
    )?;
    terminal.show_cursor()?;

    result
}

async fn run_app<B: Backend>(
    terminal: &mut Terminal<B>,
    app: &mut App,
    message_receiver: &mut mpsc::UnboundedReceiver<AppMessage>,
) -> Result<()> {
    loop {
        terminal.draw(|f| ui(f, app))?;

        // Handle events with timeout
        let timeout = std::time::Duration::from_millis(100);
        
        // Check for messages first
        if let Ok(message) = message_receiver.try_recv() {
            app.handle_message(message);
            continue;
        }

        // Check for keyboard events
        if event::poll(timeout)? {
            if let Event::Key(key) = event::read()? {
                if key.kind == KeyEventKind::Press {
                    // Handle Ctrl+C for emergency quit
                    if key.modifiers.contains(KeyModifiers::CONTROL) && key.code == KeyCode::Char('c') {
                        app.should_quit = true;
                    } else {
                        app.handle_key_event(key.code);
                    }
                }
            }
        }

        if app.should_quit {
            break;
        }
    }
    Ok(())
}

fn ui(f: &mut Frame, app: &mut App) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Min(10),    // I2C Data panel
            Constraint::Length(8),  // Status panel
            Constraint::Length(3),  // Command input
            Constraint::Length(3),  // Help
        ])
        .split(f.size());

    // I2C Data Panel
    render_i2c_panel(f, app, chunks[0]);
    
    // Status Panel
    render_status_panel(f, app, chunks[1]);
    
    // Command Input
    render_command_input(f, app, chunks[2]);
    
    // Help Panel
    render_help_panel(f, chunks[3]);
}

fn render_i2c_panel(f: &mut Frame, app: &mut App, area: Rect) {
    let items: Vec<ListItem> = app
        .i2c_logs
        .iter()
        .map(|entry| {
            let time_str = entry.timestamp.format("%H:%M:%S%.3f").to_string();
            let content = format!("{} | {}", time_str, entry.data.trim());
            ListItem::new(content)
        })
        .collect();

    let list = List::new(items)
        .block(
            Block::default()
                .title("I2C Data Stream")
                .borders(Borders::ALL)
                .border_style(Style::default().fg(Color::Cyan)),
        )
        .style(Style::default().fg(Color::White))
        .highlight_style(Style::default().bg(Color::Blue));

    f.render_stateful_widget(list, area, &mut app.i2c_list_state);
}

fn render_status_panel(f: &mut Frame, app: &mut App, area: Rect) {
    let items: Vec<ListItem> = app
        .status_logs
        .iter()
        .map(|entry| {
            let time_str = entry.timestamp.format("%H:%M:%S").to_string();
            let content = format!("{} | {}", time_str, entry.message.trim());
            ListItem::new(content)
        })
        .collect();

    let list = List::new(items)
        .block(
            Block::default()
                .title("Status & Config Responses")
                .borders(Borders::ALL)
                .border_style(Style::default().fg(Color::Green)),
        )
        .style(Style::default().fg(Color::White));

    f.render_stateful_widget(list, area, &mut app.status_list_state);
}

fn render_command_input(f: &mut Frame, app: &App, area: Rect) {
    let input_style = match app.input_mode {
        InputMode::Normal => Style::default().fg(Color::Gray),
        InputMode::Editing => Style::default().fg(Color::Yellow),
    };

    let input = Paragraph::new(app.command_input.as_str())
        .style(input_style)
        .block(
            Block::default()
                .borders(Borders::ALL)
                .title("Command Input (c to edit, Enter to send, Esc to cancel)")
                .border_style(input_style),
        );

    f.render_widget(input, area);

    // Show cursor in edit mode
    if app.input_mode == InputMode::Editing {
        f.set_cursor(
            area.x + app.command_input.len() as u16 + 1,
            area.y + 1,
        );
    }
}

fn render_help_panel(f: &mut Frame, area: Rect) {
    let help_text = Text::from(vec![
        Line::from(vec![
            Span::styled("Commands: ", Style::default().fg(Color::Cyan).add_modifier(Modifier::BOLD)),
            Span::raw("HELP, ADD 0x08-0x77, LIST, CLEAR"),
        ]),
        Line::from(vec![
            Span::styled("Controls: ", Style::default().fg(Color::Cyan).add_modifier(Modifier::BOLD)),
            Span::raw("q=quit, Ctrl+C=force quit, c=command, ↑↓=scroll, Esc=exit edit"),
        ]),
    ]);

    let help = Paragraph::new(help_text)
        .block(
            Block::default()
                .title("Help")
                .borders(Borders::ALL)
                .border_style(Style::default().fg(Color::Yellow)),
        )
        .wrap(Wrap { trim: true });

    f.render_widget(help, area);
}