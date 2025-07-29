#ifndef BLE_SERIAL_H
#define BLE_SERIAL_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BLESerial {
private:
    BLEServer* server;
    BLEService* service;
    BLECharacteristic* txCharacteristic;
    BLECharacteristic* rxCharacteristic;
    bool deviceConnected;
    bool oldDeviceConnected;
    
    static const char* SERVICE_UUID;
    static const char* CHARACTERISTIC_UUID_RX;
    static const char* CHARACTERISTIC_UUID_TX;
    
    class ServerCallbacks;
    class CharacteristicCallbacks;
    
public:
    BLESerial();
    bool begin(const char* deviceName);
    void write(const char* data);
    void write(uint8_t* data, size_t length);
    bool isConnected();
    void handleConnection();
    
private:
    void setupService();
    void startAdvertising();
};

#endif