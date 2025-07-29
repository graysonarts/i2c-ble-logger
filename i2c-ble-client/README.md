# I2C BLE Client

A Rust TUI application for connecting to and controlling the I2C BLE Logger ESP32 device.

## Features

- 🔗 **BLE Connection**: Automatically connects to I2C-BLE-Logger device
- 📊 **Real-time I2C Data**: Live stream of I2C bus transactions with timestamps
- ⚙️ **Configuration Control**: Send commands to configure I2C address filtering
- 🖥️ **Terminal UI**: Clean, responsive interface built with ratatui
- 📜 **Scrollable Logs**: Navigate through I2C data and status messages

## Installation

```bash
# Build the application
cargo build --release

# Run directly
cargo run
```

## Usage

### Basic Connection
```bash
# Connect to default device name "I2C-BLE-Logger"
cargo run

# Connect to custom device name
cargo run -- --device-name "My-I2C-Logger"

# Scan for devices only
cargo run -- --scan-only
```

### TUI Controls

| Key | Action |
|-----|--------|
| `q` | Quit application |
| `c` | Enter command mode |
| `↑/↓` | Scroll I2C data |
| `PgUp/PgDn` | Fast scroll I2C data |
| `Enter` | Send command (in command mode) |
| `Esc` | Exit command mode |

### Configuration Commands

Send these commands using the command input (`c` key):

- `HELP` - Show available commands
- `ADD 0x08-0x77` - Add I2C address range to monitor
- `ADD 0x50` - Add single I2C address to monitor
- `LIST` - List current address ranges
- `CLEAR` - Clear all address ranges

## TUI Layout

```
┌─────────────────────────────────────────────────────────┐
│                    I2C Data Stream                      │
│ 14:30:15.123 | 1234567 [0x50] READ: 0x42              │
│ 14:30:15.456 | 1234568 [0x51] WRITE: ACK               │
│                                                         │
├─────────────────────────────────────────────────────────┤
│                Status & Config Responses                │
│ 14:30:10 | I2C Logger Active - 120s uptime             │
│ 14:30:12 | Added address range: 0x08-0x77              │
├─────────────────────────────────────────────────────────┤
│ Command Input (c to edit, Enter to send, Esc to cancel)│
│ ADD 0x20-0x2F                                          │
├─────────────────────────────────────────────────────────┤
│ Commands: HELP, ADD 0x08-0x77, LIST, CLEAR             │
│ Controls: q=quit, c=command, ↑↓=scroll, PgUp/PgDn=fast │
└─────────────────────────────────────────────────────────┘
```

## Dependencies

- **ratatui**: Terminal user interface framework
- **btleplug**: Cross-platform Bluetooth Low Energy library
- **tokio**: Async runtime
- **crossterm**: Cross-platform terminal manipulation
- **clap**: Command line argument parsing

## Troubleshooting

### Connection Issues
- Ensure the ESP32 I2C BLE Logger is powered on and advertising
- Check that Bluetooth is enabled on your system
- Verify the device name matches (default: "I2C-BLE-Logger")

### Permission Issues (Linux)
```bash
# Add user to bluetooth group
sudo usermod -a -G bluetooth $USER
# Logout and login again
```

### macOS Permissions
- Grant Bluetooth permissions to Terminal/iTerm in System Preferences > Security & Privacy

## Development

### Build
```bash
cargo build
```

### Run with debug output
```bash
RUST_LOG=debug cargo run
```

### Format code
```bash
cargo fmt
```

### Lint
```bash
cargo clippy
```