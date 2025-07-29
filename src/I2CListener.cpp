#include "I2CListener.h"

// Static instance pointer for interrupt handlers
I2CListener* I2CListener::instance = nullptr;

I2CListener::I2CListener() : 
    addressFilter(),
    dataCallback(nullptr),
    isInitialized(false),
    currentState(IDLE),
    lastSDA(true),
    lastSCL(true),
    currentByte(0),
    bitCount(0),
    currentAddress(0),
    isReadTransaction(false),
    transactionStart(0),
    dataIndex(0),
    hasError(false),
    lastEdgeTime(0) {
    instance = this;
}

bool I2CListener::begin() {
    // Configure pins as inputs with pull-ups (passive listening only)
    pinMode(SDA_PIN, INPUT_PULLUP);
    pinMode(SCL_PIN, INPUT_PULLUP);
    
    // Initialize state
    resetState();
    
    // Attach interrupts for both edges on both pins
    attachInterrupt(digitalPinToInterrupt(SCL_PIN), sclInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SDA_PIN), sdaInterrupt, CHANGE);
    
    isInitialized = true;
    Serial.println("[I2C] Passive sniffer initialized - listening only, never writes to bus");
    Serial.printf("[I2C] SDA: GPIO%d, SCL: GPIO%d\n", SDA_PIN, SCL_PIN);
    
    return true;
}

void I2CListener::setDataCallback(I2CDataCallback callback) {
    dataCallback = callback;
}

AddressFilter& I2CListener::getAddressFilter() {
    return addressFilter;
}

void I2CListener::processI2C() {
    // Non-interrupt processing can be done here if needed
    // For now, everything is handled in interrupts
}

// Static interrupt handlers
void IRAM_ATTR I2CListener::sclInterrupt() {
    if (instance) {
        instance->handleSCLEdge();
    }
}

void IRAM_ATTR I2CListener::sdaInterrupt() {
    if (instance) {
        instance->handleSDAEdge();
    }
}

void IRAM_ATTR I2CListener::handleSCLEdge() {
    unsigned long currentTime = micros();
    if (currentTime - lastEdgeTime < DEBOUNCE_MICROS) {
        return;
    }
    lastEdgeTime = currentTime;
    
    bool sclState = readSCL();
    bool sdaState = readSDA();
    
    // SCL rising edge - data is stable, read the bit
    if (sclState && !lastSCL) {
        switch (currentState) {
            case ADDRESS_BITS:
                processBit(sdaState);
                break;
            case DATA_BITS:
                processBit(sdaState);
                break;
            case ADDRESS_ACK:
            case DATA_ACK:
                processAck(sdaState);
                break;
        }
    }
    
    lastSCL = sclState;
    lastSDA = sdaState;
}

void IRAM_ATTR I2CListener::handleSDAEdge() {
    unsigned long currentTime = micros();
    if (currentTime - lastEdgeTime < DEBOUNCE_MICROS) {
        return;
    }
    lastEdgeTime = currentTime;
    
    bool sclState = readSCL();
    bool sdaState = readSDA();
    
    // Only check for START/STOP when SCL is high
    if (sclState) {
        // START condition: SDA falls while SCL is high
        if (!sdaState && lastSDA) {
            currentState = START_DETECTED;
            transactionStart = millis();
            currentByte = 0;
            bitCount = 0;
            dataIndex = 0;
            hasError = false;
            currentState = ADDRESS_BITS;
        }
        // STOP condition: SDA rises while SCL is high
        else if (sdaState && !lastSDA) {
            if (currentState != IDLE) {
                // Complete transaction
                if (dataIndex > 0 && addressFilter.isAddressAllowed(currentAddress)) {
                    handleTransaction(currentAddress, isReadTransaction, 
                                   const_cast<uint8_t*>(dataBuffer), dataIndex, hasError);
                }
            }
            resetState();
        }
    }
    
    lastSCL = sclState;
    lastSDA = sdaState;
}

void IRAM_ATTR I2CListener::resetState() {
    currentState = IDLE;
    currentByte = 0;
    bitCount = 0;
    currentAddress = 0;
    isReadTransaction = false;
    dataIndex = 0;
    hasError = false;
    lastSDA = true;
    lastSCL = true;
}

void IRAM_ATTR I2CListener::processBit(bool bit) {
    currentByte = (currentByte << 1) | (bit ? 1 : 0);
    bitCount++;
    
    if (bitCount == 8) {
        if (currentState == ADDRESS_BITS) {
            currentAddress = currentByte >> 1;  // Address is upper 7 bits
            isReadTransaction = currentByte & 1; // R/W bit is LSB
            currentState = ADDRESS_ACK;
        } else if (currentState == DATA_BITS) {
            if (dataIndex < MAX_DATA_SIZE) {
                dataBuffer[dataIndex++] = currentByte;
            }
            currentState = DATA_ACK;
        }
        currentByte = 0;
        bitCount = 0;
    }
}

void IRAM_ATTR I2CListener::processAck(bool ack) {
    if (!ack) {  // ACK is low
        if (currentState == ADDRESS_ACK) {
            currentState = DATA_BITS;
        } else if (currentState == DATA_ACK) {
            currentState = DATA_BITS;  // Continue reading data
        }
    } else {  // NACK is high
        if (currentState == DATA_ACK) {
            // NACK received, transaction ending
            hasError = false;  // NACK is normal end of read transaction
        } else {
            hasError = true;   // Unexpected NACK
        }
    }
}

void I2CListener::handleTransaction(uint8_t address, bool isRead, uint8_t* data, size_t length, bool hasError) {
    if (dataCallback && addressFilter.isAddressAllowed(address)) {
        I2CTransaction transaction;
        transaction.address = address;
        transaction.isRead = isRead;
        transaction.data = data;
        transaction.dataLength = length;
        transaction.timestamp = transactionStart;
        transaction.hasError = hasError;
        
        dataCallback(transaction);
    }
}