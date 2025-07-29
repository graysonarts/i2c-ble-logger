# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based I2C logger that outputs over BLE serial whatever data is coming in across the I2C bus. The project is implemented as an Arduino sketch using PlatformIO.

## Architecture

- **Main file**: `i2cble.ino` - Contains the Arduino sketch setup() and loop() functions
- **Core functionality**: 
  - Passive I2C bus monitoring without interfering with existing connections
  - BLE serial output for captured I2C data
  - Configurable address range monitoring

## Development Environment

This project uses PlatformIO with Arduino framework for ESP32 development.

### Common Commands

- `pio run` - Build the project
- `pio run --target upload` - Upload to device
- `pio run --target monitor` - Open serial monitor
- `pio run --target uploadfs` - Upload filesystem (if SPIFFS/LittleFS used)
- `pio device monitor` - Monitor serial output
- `pio run --target clean` - Clean build files

## Key Requirements

- Must listen passively to I2C data without interfering with existing bus connections
- Should allow user configuration of monitored address ranges
- Outputs captured I2C traffic via BLE serial connection

## Current State

The project is in initial state with empty setup() and loop() functions that need implementation.