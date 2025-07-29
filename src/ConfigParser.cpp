#include "ConfigParser.h"

ConfigParser::CommandResult ConfigParser::parseCommand(const String& command, AddressFilter& filter, String& response) {
    String cmd = command;
    cmd.trim();
    cmd.toUpperCase();
    
    if (cmd.startsWith("ADD ")) {
        return parseAddressRange(cmd.substring(4), filter, response);
    } else if (cmd == "LIST") {
        return parseListRanges(filter, response);
    } else if (cmd == "CLEAR") {
        return parseClearRanges(filter, response);
    } else if (cmd == "HELP") {
        return parseHelp(response);
    } else {
        response = "ERROR: Unknown command. Send 'HELP' for usage.";
        return INVALID_COMMAND;
    }
}

ConfigParser::CommandResult ConfigParser::parseAddressRange(const String& params, AddressFilter& filter, String& response) {
    String trimmed = params;
    trimmed.trim();
    
    int dashIndex = trimmed.indexOf('-');
    if (dashIndex == -1) {
        // Single address
        if (!isValidHex(trimmed)) {
            response = "ERROR: Invalid hex address format. Use 0x08-0x77 format.";
            return INVALID_PARAMETERS;
        }
        
        uint8_t addr = parseHexByte(trimmed);
        if (!filter.addRange(addr, addr)) {
            response = "ERROR: Could not add range. Check address validity or range limit.";
            return OUT_OF_RANGE;
        }
        
        response = "Added address range: 0x" + String(addr, HEX) + "-0x" + String(addr, HEX);
        return SUCCESS;
    } else {
        // Address range
        String startStr = trimmed.substring(0, dashIndex);
        String endStr = trimmed.substring(dashIndex + 1);
        startStr.trim();
        endStr.trim();
        
        if (!isValidHex(startStr) || !isValidHex(endStr)) {
            response = "ERROR: Invalid hex format. Use 0x08-0x77 format.";
            return INVALID_PARAMETERS;
        }
        
        uint8_t startAddr = parseHexByte(startStr);
        uint8_t endAddr = parseHexByte(endStr);
        
        if (!filter.addRange(startAddr, endAddr)) {
            response = "ERROR: Could not add range. Check addresses are valid (0x08-0x77) and start <= end.";
            return OUT_OF_RANGE;
        }
        
        response = "Added address range: 0x" + String(startAddr, HEX) + "-0x" + String(endAddr, HEX);
        return SUCCESS;
    }
}

ConfigParser::CommandResult ConfigParser::parseListRanges(AddressFilter& filter, String& response) {
    response = "Active address ranges:\n";
    int count = filter.getRangeCount();
    
    if (count == 0) {
        response += "No ranges configured.";
    } else {
        for (int i = 0; i < count; i++) {
            AddressRange range = filter.getRange(i);
            response += String(i) + ": 0x" + String(range.minAddress, HEX) + 
                       "-0x" + String(range.maxAddress, HEX) + 
                       (range.enabled ? " (enabled)" : " (disabled)") + "\n";
        }
    }
    
    return SUCCESS;
}

ConfigParser::CommandResult ConfigParser::parseClearRanges(AddressFilter& filter, String& response) {
    filter.clearRanges();
    response = "All address ranges cleared.";
    return SUCCESS;
}

ConfigParser::CommandResult ConfigParser::parseHelp(String& response) {
    response = "I2C Address Filter Commands:\n";
    response += "ADD 0x08-0x77  - Add address range\n";
    response += "ADD 0x50       - Add single address\n";
    response += "LIST           - List current ranges\n";
    response += "CLEAR          - Clear all ranges\n";
    response += "HELP           - Show this help\n";
    response += "\nExample: ADD 0x08-0x0F";
    return SUCCESS;
}

uint8_t ConfigParser::parseHexByte(const String& hexStr) {
    String str = hexStr;
    if (str.startsWith("0x") || str.startsWith("0X")) {
        str = str.substring(2);
    }
    
    return (uint8_t)strtol(str.c_str(), NULL, 16);
}

bool ConfigParser::isValidHex(const String& str) {
    String trimmed = str;
    trimmed.trim();
    
    if (trimmed.startsWith("0x") || trimmed.startsWith("0X")) {
        trimmed = trimmed.substring(2);
    }
    
    if (trimmed.length() == 0 || trimmed.length() > 2) {
        return false;
    }
    
    for (int i = 0; i < trimmed.length(); i++) {
        char c = trimmed.charAt(i);
        if (!isDigit(c) && c != 'A' && c != 'B' && c != 'C' && 
            c != 'D' && c != 'E' && c != 'F') {
            return false;
        }
    }
    
    return true;
}