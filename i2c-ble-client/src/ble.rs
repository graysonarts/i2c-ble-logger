use anyhow::{Context, Result};
use btleplug::api::{
    Central, Manager as _, Peripheral as _, ScanFilter, WriteType, CharPropFlags
};
use btleplug::platform::{Manager, Peripheral};
use futures::StreamExt;
use std::time::Duration;
use tokio::time;
use uuid::Uuid;

// I2C BLE Logger Service UUIDs (from ESP32)
pub const SERIAL_SERVICE_UUID: Uuid = Uuid::from_u128(0x6E400001_B5A3_F393_E0A9_E50E24DCCA9E);
pub const CONFIG_SERVICE_UUID: Uuid = Uuid::from_u128(0x12345678_1234_1234_1234_123456789ABC);

pub const CHARACTERISTIC_UUID_RX: Uuid = Uuid::from_u128(0x6E400002_B5A3_F393_E0A9_E50E24DCCA9E);
pub const CHARACTERISTIC_UUID_TX: Uuid = Uuid::from_u128(0x6E400003_B5A3_F393_E0A9_E50E24DCCA9E);
pub const CHARACTERISTIC_UUID_CONFIG: Uuid = Uuid::from_u128(0x12345678_1234_1234_1234_123456789ABD);
pub const CHARACTERISTIC_UUID_STATUS: Uuid = Uuid::from_u128(0x12345678_1234_1234_1234_123456789ABE);

#[derive(Clone)]
pub struct BleClient {
    peripheral: Peripheral,
}

impl BleClient {
    pub async fn connect(device_name: &str) -> Result<Self> {
        let manager = Manager::new().await?;
        let adapters = manager.adapters().await?;
        let central = adapters.into_iter().nth(0).context("No BLE adapter found")?;

        central.start_scan(ScanFilter::default()).await?;
        
        // Scan for devices
        time::sleep(Duration::from_secs(3)).await;
        
        let peripherals = central.peripherals().await?;
        
        let mut target_peripheral = None;
        for p in peripherals {
            if let Ok(Some(props)) = p.properties().await {
                if let Some(name) = props.local_name {
                    if name == device_name {
                        target_peripheral = Some(p);
                        break;
                    }
                }
            }
        }
        
        let peripheral = target_peripheral
            .context(format!("Device '{}' not found", device_name))?;

        central.stop_scan().await?;
        
        peripheral.connect().await?;
        peripheral.discover_services().await?;
        
        // Verify services are available
        let services = peripheral.services();
        let has_serial = services.iter().any(|s| s.uuid == SERIAL_SERVICE_UUID);
        let has_config = services.iter().any(|s| s.uuid == CONFIG_SERVICE_UUID);
        
        if !has_serial || !has_config {
            return Err(anyhow::anyhow!("Required services not found on device"));
        }
        
        println!("Connected to I2C BLE Logger");
        println!("Services found: Serial={}, Config={}", has_serial, has_config);
        
        Ok(Self { peripheral })
    }

    pub async fn send_config_command(&self, command: &str) -> Result<()> {
        let chars = self.peripheral.characteristics();
        let config_char = chars
            .iter()
            .find(|c| c.uuid == CHARACTERISTIC_UUID_CONFIG)
            .context("Config characteristic not found")?;

        // Trim whitespace and ensure clean command
        let clean_command = command.trim();
        
        self.peripheral
            .write(config_char, clean_command.as_bytes(), WriteType::WithoutResponse)
            .await?;
        
        println!("Sent config command: {}", clean_command);
        Ok(())
    }

    pub async fn subscribe_to_data<F>(&self, callback: F) -> Result<()>
    where
        F: Fn(String) + Send + Sync + 'static,
    {
        let chars = self.peripheral.characteristics();
        
        // Subscribe to I2C data (TX characteristic)
        let tx_char = chars
            .iter()
            .find(|c| c.uuid == CHARACTERISTIC_UUID_TX)
            .context("TX characteristic not found")?;

        if tx_char.properties.contains(CharPropFlags::NOTIFY) {
            self.peripheral.subscribe(tx_char).await?;
            println!("Subscribed to I2C data stream");
        }

        // Subscribe to status updates
        let status_char = chars
            .iter()
            .find(|c| c.uuid == CHARACTERISTIC_UUID_STATUS)
            .context("Status characteristic not found")?;

        if status_char.properties.contains(CharPropFlags::NOTIFY) {
            self.peripheral.subscribe(status_char).await?;
            println!("Subscribed to status updates");
        }

        // Handle notifications
        let mut notification_stream = self.peripheral.notifications().await?;
        
        tokio::spawn(async move {
            while let Some(data) = notification_stream.next().await {
                if let Ok(text) = String::from_utf8(data.value.clone()) {
                    callback(text);
                }
            }
        });

        Ok(())
    }

    pub async fn disconnect(&self) -> Result<()> {
        self.peripheral.disconnect().await?;
        println!("Disconnected from I2C BLE Logger");
        Ok(())
    }

    pub async fn is_connected(&self) -> bool {
        self.peripheral.is_connected().await.unwrap_or(false)
    }
}