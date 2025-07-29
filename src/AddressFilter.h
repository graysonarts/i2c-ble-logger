#ifndef ADDRESS_FILTER_H
#define ADDRESS_FILTER_H

#include <stdint.h>

struct AddressRange {
    uint8_t minAddress;
    uint8_t maxAddress;
    bool enabled;
};

class AddressFilter {
private:
    static const int MAX_RANGES = 4;
    AddressRange ranges[MAX_RANGES];
    int activeRanges;
    
public:
    AddressFilter();
    bool addRange(uint8_t minAddr, uint8_t maxAddr);
    bool removeRange(int index);
    void clearRanges();
    bool isAddressAllowed(uint8_t address);
    int getRangeCount();
    AddressRange getRange(int index);
    void setRangeEnabled(int index, bool enabled);
    
private:
    bool isValidAddress(uint8_t address);
    bool isValidRange(uint8_t minAddr, uint8_t maxAddr);
};

#endif