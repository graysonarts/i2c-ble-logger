mod ble;
mod tui;

use anyhow::Result;
use clap::Parser;
use tokio::sync::mpsc;
use tui::{App, AppMessage};

#[derive(Parser)]
#[command(name = "i2c-ble-client")]
#[command(about = "A TUI client for I2C BLE Logger")]
struct Args {
    #[arg(short, long, default_value = "I2C-BLE-Logger")]
    device_name: String,
    
    #[arg(short, long)]
    scan_only: bool,
}

#[tokio::main]
async fn main() -> Result<()> {
    let args = Args::parse();

    if args.scan_only {
        println!("Scanning for BLE devices...");
        // TODO: Implement device scanning
        return Ok(());
    }

    println!("Connecting to {}...", args.device_name);
    
    // Connect to BLE device
    let ble_client = match ble::BleClient::connect(&args.device_name).await {
        Ok(client) => client,
        Err(e) => {
            eprintln!("Failed to connect: {}", e);
            eprintln!("Make sure the I2C BLE Logger is powered on and advertising.");
            return Err(e);
        }
    };

    // Create channels for communication
    let (message_sender, message_receiver) = mpsc::unbounded_channel::<AppMessage>();
    let (command_sender, mut command_receiver) = mpsc::unbounded_channel::<String>();

    // Clone sender for BLE notifications
    let ble_message_sender = message_sender.clone();

    // Set up BLE data subscription
    ble_client
        .subscribe_to_data({
            let sender = ble_message_sender.clone();
            move |data| {
                // Determine if this is I2C data or status based on content
                let message = if data.contains("[") && (data.contains("READ:") || data.contains("WRITE:")) {
                    AppMessage::I2CData(data)
                } else {
                    AppMessage::StatusUpdate(data)
                };
                let _ = sender.send(message);
            }
        })
        .await?;

    // Handle commands from TUI
    let ble_client_clone = ble_client.clone();
    let command_message_sender = message_sender.clone();
    tokio::spawn(async move {
        while let Some(command) = command_receiver.recv().await {
            match ble_client_clone.send_config_command(&command).await {
                Ok(_) => {
                    let response = format!("Sent: {}", command);
                    let _ = command_message_sender.send(AppMessage::CommandResponse(response));
                }
                Err(e) => {
                    let error = format!("Error sending command: {}", e);
                    let _ = command_message_sender.send(AppMessage::CommandResponse(error));
                }
            }
        }
    });

    // Create and run TUI
    let app = App::new(command_sender);
    
    println!("Starting TUI interface...");
    println!("Press 'q' to quit, 'c' to enter commands");
    
    // Small delay to let user see the messages
    tokio::time::sleep(tokio::time::Duration::from_millis(1000)).await;
    
    tui::run_tui(app, message_receiver).await?;

    // Cleanup
    println!("Disconnecting...");
    ble_client.disconnect().await?;
    println!("Goodbye!");

    Ok(())
}
