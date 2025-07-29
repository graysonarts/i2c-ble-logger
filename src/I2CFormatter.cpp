#include "I2CFormatter.h"

I2CFormatter::I2CFormatter() {
    memset(outputBuffer, 0, MAX_OUTPUT_SIZE);
}

String I2CFormatter::formatTransaction(const I2CTransaction& transaction, I2CFormatterType kind) {
        String output = "";

        output += formatTimestamp(transaction.timestamp);
        output += " [0x";
        output += byteToHex(transaction.address);
        output += "] ";

        if (transaction.hasError) {
            output += "ERROR";
        } else {
            output += transaction.isRead ? "READ: " : "WRITE: ";
            appendDataToString(output, transaction, kind);
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

String I2CFormatter::byteToBinary(uint8_t value) {
    String bin = String(value, BIN);
    if (bin.length() == 1) {
        bin = "0" + bin;
    }
    return bin;
}

void I2CFormatter::appendDataToString(String& str, const I2CTransaction& transaction, I2CFormatterType kind) {
    if (transaction.data == nullptr || transaction.dataLength == 0) {
        str += "ACK";
        return;
    }

    for (size_t i = 0; i < transaction.dataLength; i++) {
        if (i > 0) {
            str += " ";
        }

        switch (kind) {
            case I2CFormatterType::Hex:
                str += "0x";
                str += byteToHex(transaction.data[i]);
                break;
            case I2CFormatterType::Binary:
                str += "0b";
                str += byteToBinary(transaction.data[i]);
                break;
            case I2CFormatterType::Decimal:
                str += String(transaction.data[i]);
                break;
        }
    }
}
