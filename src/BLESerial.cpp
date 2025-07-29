#include "BLESerial.h"
#include <Arduino.h>

const char* BLESerial::SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const char* BLESerial::CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
const char* BLESerial::CHARACTERISTIC_UUID_TX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

class BLESerial::ServerCallbacks : public BLEServerCallbacks {
private:
    BLESerial* bleSerial;
    
public:
    ServerCallbacks(BLESerial* serial) : bleSerial(serial) {}
    
    void onConnect(BLEServer* server) {
        bleSerial->deviceConnected = true;
    }
    
    void onDisconnect(BLEServer* server) {
        bleSerial->deviceConnected = false;
    }
};

class BLESerial::CharacteristicCallbacks : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic* characteristic) {
        // Handle incoming data if needed
    }
};

BLESerial::BLESerial() : 
    server(nullptr),
    service(nullptr),
    txCharacteristic(nullptr),
    rxCharacteristic(nullptr),
    deviceConnected(false),
    oldDeviceConnected(false) {
}

bool BLESerial::begin(const char* deviceName) {
    BLEDevice::init(deviceName);
    
    server = BLEDevice::createServer();
    if (!server) {
        return false;
    }
    
    server->setCallbacks(new ServerCallbacks(this));
    
    setupService();
    startAdvertising();
    
    return true;
}

void BLESerial::setupService() {
    service = server->createService(SERVICE_UUID);
    
    txCharacteristic = service->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    txCharacteristic->addDescriptor(new BLE2902());
    
    rxCharacteristic = service->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE
    );
    rxCharacteristic->setCallbacks(new CharacteristicCallbacks());
    
    service->start();
}

void BLESerial::startAdvertising() {
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
}

void BLESerial::write(const char* data) {
    if (deviceConnected && txCharacteristic) {
        txCharacteristic->setValue(data);
        txCharacteristic->notify();
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
        delay(500);
        server->startAdvertising();
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}