#include "I2CListener.h"

I2CListener::I2CListener() : 
    addressFilter(),
    dataCallback(nullptr),
    isInitialized(false) {
}

bool I2CListener::begin() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);
    
    isInitialized = true;
    return true;
}

void I2CListener::setDataCallback(I2CDataCallback callback) {
    dataCallback = callback;
}

AddressFilter& I2CListener::getAddressFilter() {
    return addressFilter;
}

void I2CListener::scanBus() {
    if (!isInitialized) {
        return;
    }
    
    for (uint8_t address = 1; address < 127; address++) {
        if (addressFilter.isAddressAllowed(address)) {
            scanAddress(address);
        }
        delay(1);
    }
}

void I2CListener::scanAddress(uint8_t address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        handleTransaction(address, false, nullptr, 0, false);
        
        Wire.requestFrom(address, (uint8_t)1);
        if (Wire.available()) {
            uint8_t receivedByte = Wire.read();
            handleTransaction(address, true, &receivedByte, 1, false);
        }
    } else if (error != 2) {
        handleTransaction(address, false, nullptr, 0, true);
    }
}

void I2CListener::handleTransaction(uint8_t address, bool isRead, uint8_t* data, size_t length, bool hasError) {
    if (dataCallback) {
        I2CTransaction transaction;
        transaction.address = address;
        transaction.isRead = isRead;
        transaction.data = data;
        transaction.dataLength = length;
        transaction.timestamp = millis();
        transaction.hasError = hasError;
        
        dataCallback(transaction);
    }
}