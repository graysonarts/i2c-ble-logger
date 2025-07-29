#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <Arduino.h>
#include "AddressFilter.h"

class ConfigParser {
public:
    enum CommandResult {
        SUCCESS,
        INVALID_COMMAND,
        INVALID_PARAMETERS,
        OUT_OF_RANGE
    };
    
    static CommandResult parseCommand(const String& command, AddressFilter& filter, String& response);
    
private:
    static CommandResult parseAddressRange(const String& params, AddressFilter& filter, String& response);
    static CommandResult parseListRanges(AddressFilter& filter, String& response);
    static CommandResult parseClearRanges(AddressFilter& filter, String& response);
    static CommandResult parseHelp(String& response);
    static uint8_t parseHexByte(const String& hexStr);
    static bool isValidHex(const String& str);
};

#endif