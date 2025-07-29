#ifndef I2C_LISTENER_H
#define I2C_LISTENER_H

#include <functional>
#include <Arduino.h>
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

enum I2CState {
    IDLE,
    START_DETECTED,
    ADDRESS_BITS,
    ADDRESS_ACK,
    DATA_BITS,
    DATA_ACK,
    STOP_DETECTED
};

class I2CListener {
private:
    static const int SDA_PIN = 4;
    static const int SCL_PIN = 5;
    static const int MAX_DATA_SIZE = 32;
    static const int MAX_TRANSACTIONS = 16;
    
    AddressFilter addressFilter;
    I2CDataCallback dataCallback;
    bool isInitialized;
    
    // I2C Protocol State
    volatile I2CState currentState;
    volatile bool lastSDA;
    volatile bool lastSCL;
    volatile uint8_t currentByte;
    volatile uint8_t bitCount;
    volatile uint8_t currentAddress;
    volatile bool isReadTransaction;
    volatile unsigned long transactionStart;
    
    // Data buffers
    uint8_t dataBuffer[MAX_DATA_SIZE];
    volatile size_t dataIndex;
    volatile bool hasError;
    
    // Timing for debouncing
    volatile unsigned long lastEdgeTime;
    static const unsigned long DEBOUNCE_MICROS = 2;
    
public:
    I2CListener();
    bool begin();
    void setDataCallback(I2CDataCallback callback);
    void processI2C();  // Call this regularly from main loop instead of scanBus
    AddressFilter& getAddressFilter();
    
private:
    static void IRAM_ATTR sclInterrupt();
    static void IRAM_ATTR sdaInterrupt();
    static I2CListener* instance;
    
    void IRAM_ATTR handleSCLEdge();
    void IRAM_ATTR handleSDAEdge();
    void IRAM_ATTR resetState();
    void IRAM_ATTR processBit(bool bit);
    void IRAM_ATTR processAck(bool ack);
    void handleTransaction(uint8_t address, bool isRead, uint8_t* data, size_t length, bool hasError);
    
    inline bool readSDA() { return digitalRead(SDA_PIN); }
    inline bool readSCL() { return digitalRead(SCL_PIN); }
};

#endif