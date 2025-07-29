#include "I2CFormatter.h"

I2CFormatter::I2CFormatter() {
    memset(outputBuffer, 0, MAX_OUTPUT_SIZE);
}

String I2CFormatter::formatTransaction(const I2CTransaction& transaction) {
    return formatAsHex(transaction);
}

String I2CFormatter::formatAsHex(const I2CTransaction& transaction) {
    String output = "";
    
    output += formatTimestamp(transaction.timestamp);
    output += " [0x";
    output += byteToHex(transaction.address);
    output += "] ";
    
    if (transaction.hasError) {
        output += "ERROR";
    } else {
        output += transaction.isRead ? "READ: " : "WRITE: ";
        appendDataToString(output, transaction, true);
    }
    
    output += "\n";
    return output;
}

String I2CFormatter::formatAsDecimal(const I2CTransaction& transaction) {
    String output = "";
    
    output += formatTimestamp(transaction.timestamp);
    output += " [";
    output += String(transaction.address);
    output += "] ";
    
    if (transaction.hasError) {
        output += "ERROR";
    } else {
        output += transaction.isRead ? "READ: " : "WRITE: ";
        appendDataToString(output, transaction, false);
    }
    
    output += "\n";
    return output;
}

String I2CFormatter::formatTimestamp(unsigned long timestamp) {
    return String(timestamp);
}

String I2CFormatter::byteToHex(uint8_t value) {
    String hex = String(value, HEX);
    if (hex.length() == 1) {
        hex = "0" + hex;
    }
    hex.toUpperCase();
    return hex;
}

void I2CFormatter::appendDataToString(String& str, const I2CTransaction& transaction, bool useHex) {
    if (transaction.data == nullptr || transaction.dataLength == 0) {
        str += "ACK";
        return;
    }
    
    for (size_t i = 0; i < transaction.dataLength; i++) {
        if (i > 0) {
            str += " ";
        }
        
        if (useHex) {
            str += "0x";
            str += byteToHex(transaction.data[i]);
        } else {
            str += String(transaction.data[i]);
        }
    }
}