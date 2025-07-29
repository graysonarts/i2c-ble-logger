# I2C BLE Logger

A passive I2C sniffer for ESP32-C3 that monitors I2C bus traffic and streams data over Bluetooth Low Energy (BLE). Perfect for debugging I2C communications without interfering with the bus.

## 🔧 Hardware Requirements

- **ESP32-C3 Development Board** (tested on [LuatOS ESP32-C3 board](https://wiki.luatos.org/chips/esp32c3/board.html))
- **I2C Bus Connection:**
  - SDA: GPIO4
  - SCL: GPIO5
  - Ground connection to target I2C bus

## ✨ Key Features

### 🎯 **Passive I2C Monitoring**
- **Zero Bus Interference**: Uses GPIO interrupts only - never writes to the I2C bus
- **Real-time Protocol Decoding**: Captures START/STOP conditions, addresses, data, and ACK/NACK
- **High-Speed Capture**: Interrupt-driven with microsecond timing and debouncing
- **Complete Transaction Logging**: Records full I2C transactions with timestamps

### 📡 **Dual BLE Services**
- **Serial Service** (`6E400001-B5A3-F393-E0A9-E50E24DCCA9E`): I2C data stream
- **Config Service** (`12345678-1234-1234-1234-123456789ABC`): Configuration commands

### ⚙️ **Address Filtering**
- Configure up to 4 address ranges to monitor
- Commands: `ADD 0x08-0x77`, `LIST`, `CLEAR`, `HELP`
- Only captures traffic for specified addresses

### 💻 **Rust TUI Client**
- Real-time terminal interface using `ratatui`
- Separate panels for I2C data and status messages
- Interactive command input for configuration
- Auto-scrolling with proper newline handling

## 🚀 Quick Start

### 1. Flash ESP32-C3
```bash
# Build and upload firmware
pio run --target upload

# Monitor serial output
pio device monitor
```

### 2. Connect I2C Bus
- Connect SDA (GPIO4) and SCL (GPIO5) to your target I2C bus
- **Important**: Only connect as passive listener - do not use pull-up resistors

### 3. Use BLE Client
```bash
# Using the Rust TUI client
cd i2c-ble-client
./run.sh

# Or use any BLE app (iOS: BLE Hero, Android: nRF Connect)
# Device name: "I2C-BLE-Logger"
```

## 📱 BLE Configuration Commands

Send commands to the Config service characteristic:

| Command | Description | Example |
|---------|-------------|---------|
| `HELP` | Show available commands | `HELP` |
| `ADD` | Add address range | `ADD 0x08-0x77` |
| `LIST` | List active ranges | `LIST` |
| `CLEAR` | Clear all ranges | `CLEAR` |

## 🖥️ Rust TUI Client Controls

| Key | Action |
|-----|--------|
| `q` | Quit application |
| `c` | Enter command mode |
| `Ctrl+C` | Emergency quit |
| `↑/↓` | Scroll I2C data |
| `PgUp/PgDn` | Fast scroll |
| `Esc` | Exit command mode |

## 🔍 I2C Data Format

```
timestamp [0xADDRESS] READ/WRITE: data bytes
```

Example output:
```
1234567 [0x48] W: 0b10000001 0b11110000
1234568 [0x48] R: 0b00110011
```

## 🏗️ Architecture

### ESP32-C3 Firmware
- **I2CListener**: GPIO interrupt-based passive sniffer
- **BLESerial**: Dual GATT service implementation
- **ConfigParser**: Command parsing and address management
- **I2CFormatter**: Data formatting with binary/hex/decimal support
- **AddressFilter**: Up to 4 configurable address ranges

### Rust Client
- **BLE Connection**: `btleplug` for cross-platform BLE support
- **TUI Interface**: `ratatui` for rich terminal interface
- **Real-time Display**: Separate panels for data and status
- **Command Interface**: Interactive configuration

## 🛠️ Development

### ESP32 Development
```bash
# Check code
pio check

# Build
pio run

# Upload and monitor
pio run --target upload --target monitor
```

### Rust Client Development
```bash
cd i2c-ble-client

# Build
cargo build --release

# Run with logging
RUST_LOG=debug cargo run
```

## 🔒 Safety Features

- **Passive Operation**: Never drives I2C bus lines
- **Debouncing**: 2μs debounce prevents noise-induced errors
- **IRAM Optimization**: Interrupt handlers in fast memory
- **Address Filtering**: Only processes configured address ranges
- **Error Handling**: Detects and reports I2C protocol errors

## 🤝 Contributing

This project follows conventional commit standards and uses clean architecture principles with single-responsibility classes.

## 📄 License

Open source - feel free to modify and distribute.