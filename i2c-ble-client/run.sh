#!/bin/bash

# I2C BLE Client Runner
# Make sure your ESP32 I2C BLE Logger is powered on and advertising

echo "🔍 I2C BLE Client for ESP32 Logger"
echo "=================================="
echo ""
echo "Prerequisites:"
echo "  ✓ ESP32 I2C BLE Logger powered on"
echo "  ✓ Bluetooth enabled on this system"
echo "  ✓ Device advertising as 'I2C-BLE-Logger'"
echo ""

# Check if Bluetooth is available (macOS/Linux)
if command -v bluetoothctl &> /dev/null; then
    echo "🔵 Checking Bluetooth status..."
    if bluetoothctl show | grep -q "Powered: yes"; then
        echo "  ✓ Bluetooth is powered on"
    else
        echo "  ⚠️  Bluetooth may be off - please enable it"
    fi
elif system_profiler SPBluetoothDataType &> /dev/null 2>&1; then
    echo "🔵 macOS Bluetooth detected"
else
    echo "⚠️  Could not detect Bluetooth status"
fi

echo ""
echo "🚀 Starting I2C BLE Client..."
echo ""
echo "TUI Controls:"
echo "  q         - Quit"
echo "  c         - Enter command mode"
echo "  ↑/↓       - Scroll I2C data"
echo "  PgUp/PgDn - Fast scroll"
echo ""
echo "Commands to try:"
echo "  HELP              - Show available commands"
echo "  ADD 0x08-0x77     - Add I2C address range"
echo "  ADD 0x50          - Add single address"
echo "  LIST              - List current ranges"
echo "  CLEAR             - Clear all ranges"
echo ""
echo "Press Enter to continue..."
read -r

# Run the client
cargo run --release

echo ""
echo "👋 Thanks for using I2C BLE Client!"