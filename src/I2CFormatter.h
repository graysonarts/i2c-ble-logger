#ifndef I2C_FORMATTER_H
#define I2C_FORMATTER_H

#include "I2CListener.h"
#include <String.h>

class I2CFormatter {
private:
    static const size_t MAX_OUTPUT_SIZE = 256;
    char outputBuffer[MAX_OUTPUT_SIZE];
    
public:
    I2CFormatter();
    String formatTransaction(const I2CTransaction& transaction);
    String formatAsHex(const I2CTransaction& transaction);
    String formatAsDecimal(const I2CTransaction& transaction);
    String formatTimestamp(unsigned long timestamp);
    
private:
    String byteToHex(uint8_t value);
    void appendDataToString(String& str, const I2CTransaction& transaction, bool useHex = true);
};

#endif