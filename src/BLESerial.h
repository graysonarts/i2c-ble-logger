#ifndef BLE_SERIAL_H
#define BLE_SERIAL_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <functional>

typedef std::function<void(const String&)> ConfigCallback;

class BLESerial {
private:
    BLEServer* server;
    BLEService* serialService;
    BLEService* configService;
    BLECharacteristic* txCharacteristic;
    BLECharacteristic* rxCharacteristic;
    BLECharacteristic* configCharacteristic;
    BLECharacteristic* statusCharacteristic;
    bool deviceConnected;
    bool oldDeviceConnected;
    ConfigCallback configCallback;
    
    static const char* SERIAL_SERVICE_UUID;
    static const char* CONFIG_SERVICE_UUID;
    static const char* CHARACTERISTIC_UUID_RX;
    static const char* CHARACTERISTIC_UUID_TX;
    static const char* CHARACTERISTIC_UUID_CONFIG;
    static const char* CHARACTERISTIC_UUID_STATUS;
    
    class ServerCallbacks;
    class CharacteristicCallbacks;
    
public:
    BLESerial();
    bool begin(const char* deviceName);
    void write(const char* data);
    void write(uint8_t* data, size_t length);
    bool isConnected();
    void handleConnection();
    void setConfigCallback(ConfigCallback callback);
    void writeStatus(const String& status);
    
private:
    void setupSerialService();
    void setupConfigService();
    void startAdvertising();
};

#endif