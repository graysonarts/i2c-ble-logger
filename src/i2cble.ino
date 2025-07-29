#include "BLESerial.h"
#include "I2CListener.h"
#include "I2CFormatter.h"

#define LED_1 12
#define LED_2 13

BLESerial bleSerial;
I2CListener i2cListener;
I2CFormatter formatter;

void onI2CData(const I2CTransaction &transaction)
{
  String formattedData = formatter.formatTransaction(transaction);

  Serial.print(formattedData);

  if (bleSerial.isConnected())
  {
    bleSerial.write(formattedData.c_str());
  }
}

void setup()
{
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, HIGH);
  Serial.begin(115200);
  digitalWrite(LED_2, LOW);
  while (!Serial)
    ;
  digitalWrite(LED_1, LOW);

  Serial.println("\n=== I2C BLE Logger Starting ===");
  Serial.println("ESP32-C3 USB CDC Serial Active");

  // Initialize I2C first (simpler initialization)
  Serial.println("Initializing I2C...");
  if (!i2cListener.begin())
  {
    Serial.println("ERROR: Failed to initialize I2C");
    return;
  }
  Serial.println("✓ I2C initialized successfully");

  // Add delay before BLE initialization
  delay(1000);

  // Initialize BLE with error handling
  Serial.println("Initializing BLE...");
  if (!bleSerial.begin("I2C-BLE-Logger"))
  {
    Serial.println("⚠ Failed to initialize BLE - continuing without BLE");
    // Continue without BLE rather than stopping
  }
  else
  {
    Serial.println("✓ BLE initialized successfully");
  }

  i2cListener.setDataCallback(onI2CData);

  Serial.println("=== I2C BLE Logger Ready ===");
  Serial.println("Device name: I2C-BLE-Logger");
  Serial.println("I2C pins - SDA: GPIO4, SCL: GPIO5");
  Serial.println("Monitoring I2C bus...\n");
}

void loop()
{
  bleSerial.handleConnection();

  static unsigned long lastScan = 0;
  if (millis() - lastScan > 1000)
  {
    i2cListener.scanBus();
    lastScan = millis();
  }

  if (bleSerial.isConnected())
  {
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 10000)
    {
      bleSerial.write("I2C Logger Active\n");
      lastHeartbeat = millis();
    }
  }

  delay(50);
}
