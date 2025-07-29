#ifndef I2C_LISTENER_H
#define I2C_LISTENER_H

#include <Wire.h>
#include <functional>
#include "AddressFilter.h"

struct I2CTransaction {
    uint8_t address;
    bool isRead;
    uint8_t* data;
    size_t dataLength;
    unsigned long timestamp;
    bool hasError;
};

typedef std::function<void(const I2CTransaction&)> I2CDataCallback;

class I2CListener {
private:
    static const int SDA_PIN = 4;
    static const int SCL_PIN = 5;
    static const int MAX_DATA_SIZE = 32;
    
    AddressFilter addressFilter;
    I2CDataCallback dataCallback;
    bool isInitialized;
    
    uint8_t dataBuffer[MAX_DATA_SIZE];
    
public:
    I2CListener();
    bool begin();
    void setDataCallback(I2CDataCallback callback);
    void scanBus();
    AddressFilter& getAddressFilter();
    
private:
    void scanAddress(uint8_t address);
    void handleTransaction(uint8_t address, bool isRead, uint8_t* data, size_t length, bool hasError);
};

#endif