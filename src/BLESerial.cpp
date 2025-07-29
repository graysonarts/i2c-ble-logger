#include "BLESerial.h"
#include <Arduino.h>

const char* BLESerial::SERIAL_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const char* BLESerial::CONFIG_SERVICE_UUID = "12345678-1234-1234-1234-123456789ABC";
const char* BLESerial::CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
const char* BLESerial::CHARACTERISTIC_UUID_TX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
const char* BLESerial::CHARACTERISTIC_UUID_CONFIG = "12345678-1234-1234-1234-123456789ABD";
const char* BLESerial::CHARACTERISTIC_UUID_STATUS = "12345678-1234-1234-1234-123456789ABE";

class BLESerial::ServerCallbacks : public BLEServerCallbacks {
private:
    BLESerial* bleSerial;
    
public:
    ServerCallbacks(BLESerial* serial) : bleSerial(serial) {}
    
    void onConnect(BLEServer* server) {
        bleSerial->deviceConnected = true;
        Serial.println("[BLE] Client connected");
        Serial.print("[BLE] Connection count: ");
        Serial.println(server->getConnectedCount());
    }
    
    void onDisconnect(BLEServer* server) {
        bleSerial->deviceConnected = false;
        Serial.println("[BLE] Client disconnected");
        Serial.print("[BLE] Connection count: ");
        Serial.println(server->getConnectedCount());
    }
};

class BLESerial::CharacteristicCallbacks : public BLECharacteristicCallbacks {
private:
    BLESerial* bleSerial;
    
public:
    CharacteristicCallbacks(BLESerial* serial) : bleSerial(serial) {}
    
    void onWrite(BLECharacteristic* characteristic) {
        std::string value = characteristic->getValue();
        String characteristicUUID = String(characteristic->getUUID().toString().c_str());
        
        Serial.print("[BLE] Write to characteristic: ");
        Serial.println(characteristicUUID);
        Serial.print("[BLE] Data length: ");
        Serial.println(value.length());
        Serial.print("[BLE] Data: \"");
        Serial.print(value.c_str());
        Serial.println("\"");
        Serial.print("[BLE] Raw bytes: ");
        for (size_t i = 0; i < value.length(); i++) {
            Serial.print("0x");
            Serial.print((uint8_t)value[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        
        if (value.length() > 0 && bleSerial->configCallback) {
            String command = String(value.c_str());
            Serial.println("[BLE] Calling config callback...");
            bleSerial->configCallback(command);
        } else if (value.length() == 0) {
            Serial.println("[BLE] Warning: Empty command received");
        } else if (!bleSerial->configCallback) {
            Serial.println("[BLE] Warning: No config callback set");
        }
    }
};

BLESerial::BLESerial() : 
    server(nullptr),
    serialService(nullptr),
    configService(nullptr),
    txCharacteristic(nullptr),
    rxCharacteristic(nullptr),
    configCharacteristic(nullptr),
    statusCharacteristic(nullptr),
    deviceConnected(false),
    oldDeviceConnected(false),
    configCallback(nullptr) {
}

bool BLESerial::begin(const char* deviceName) {
    BLEDevice::init(deviceName);
    
    server = BLEDevice::createServer();
    if (!server) {
        return false;
    }
    
    server->setCallbacks(new ServerCallbacks(this));
    
    setupSerialService();
    setupConfigService();
    startAdvertising();
    
    return true;
}

void BLESerial::setupSerialService() {
    serialService = server->createService(SERIAL_SERVICE_UUID);
    
    txCharacteristic = serialService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    txCharacteristic->addDescriptor(new BLE2902());
    
    rxCharacteristic = serialService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE
    );
    rxCharacteristic->setCallbacks(new CharacteristicCallbacks(this));
    
    serialService->start();
}

void BLESerial::setupConfigService() {
    configService = server->createService(CONFIG_SERVICE_UUID);
    
    configCharacteristic = configService->createCharacteristic(
        CHARACTERISTIC_UUID_CONFIG,
        BLECharacteristic::PROPERTY_WRITE
    );
    configCharacteristic->setCallbacks(new CharacteristicCallbacks(this));
    
    statusCharacteristic = configService->createCharacteristic(
        CHARACTERISTIC_UUID_STATUS,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    statusCharacteristic->addDescriptor(new BLE2902());
    statusCharacteristic->setValue("I2C Logger Ready");
    
    configService->start();
}

void BLESerial::startAdvertising() {
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERIAL_SERVICE_UUID);
    advertising->addServiceUUID(CONFIG_SERVICE_UUID);
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    Serial.println("[BLE] Advertising started");
    Serial.println("[BLE] Services advertised:");
    Serial.println("[BLE]   - Serial: " + String(SERIAL_SERVICE_UUID));
    Serial.println("[BLE]   - Config: " + String(CONFIG_SERVICE_UUID));
}

void BLESerial::write(const char* data) {
    if (deviceConnected && txCharacteristic) {
        txCharacteristic->setValue(data);
        txCharacteristic->notify();
        Serial.print("[BLE] TX notify sent: \"");
        Serial.print(data);
        Serial.println("\"");
    } else if (!deviceConnected) {
        Serial.println("[BLE] Warning: Attempted to write but no client connected");
    }
}

void BLESerial::write(uint8_t* data, size_t length) {
    if (deviceConnected && txCharacteristic) {
        txCharacteristic->setValue(data, length);
        txCharacteristic->notify();
    }
}

bool BLESerial::isConnected() {
    return deviceConnected;
}

void BLESerial::handleConnection() {
    if (!deviceConnected && oldDeviceConnected) {
        Serial.println("[BLE] Restarting advertising after disconnect");
        delay(500);
        server->startAdvertising();
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        Serial.println("[BLE] Connection state changed to connected");
        oldDeviceConnected = deviceConnected;
    }
}

void BLESerial::setConfigCallback(ConfigCallback callback) {
    configCallback = callback;
}

void BLESerial::writeStatus(const String& status) {
    if (deviceConnected && statusCharacteristic) {
        statusCharacteristic->setValue(status.c_str());
        statusCharacteristic->notify();
        Serial.print("[BLE] Status notify sent: \"");
        Serial.print(status);
        Serial.println("\"");
    } else if (!deviceConnected) {
        Serial.println("[BLE] Warning: Attempted to send status but no client connected");
    }
}