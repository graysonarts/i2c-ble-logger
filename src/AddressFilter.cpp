#include "AddressFilter.h"

AddressFilter::AddressFilter() : activeRanges(0) {
    for (int i = 0; i < MAX_RANGES; i++) {
        ranges[i] = {0, 0, false};
    }
    
    addRange(0x08, 0x77);
}

bool AddressFilter::addRange(uint8_t minAddr, uint8_t maxAddr) {
    if (activeRanges >= MAX_RANGES || !isValidRange(minAddr, maxAddr)) {
        return false;
    }
    
    ranges[activeRanges] = {minAddr, maxAddr, true};
    activeRanges++;
    return true;
}

bool AddressFilter::removeRange(int index) {
    if (index < 0 || index >= activeRanges) {
        return false;
    }
    
    for (int i = index; i < activeRanges - 1; i++) {
        ranges[i] = ranges[i + 1];
    }
    
    activeRanges--;
    ranges[activeRanges] = {0, 0, false};
    return true;
}

void AddressFilter::clearRanges() {
    activeRanges = 0;
    for (int i = 0; i < MAX_RANGES; i++) {
        ranges[i] = {0, 0, false};
    }
}

bool AddressFilter::isAddressAllowed(uint8_t address) {
    if (!isValidAddress(address)) {
        return false;
    }
    
    for (int i = 0; i < activeRanges; i++) {
        if (ranges[i].enabled && 
            address >= ranges[i].minAddress && 
            address <= ranges[i].maxAddress) {
            return true;
        }
    }
    
    return false;
}

int AddressFilter::getRangeCount() {
    return activeRanges;
}

AddressRange AddressFilter::getRange(int index) {
    if (index >= 0 && index < activeRanges) {
        return ranges[index];
    }
    return {0, 0, false};
}

void AddressFilter::setRangeEnabled(int index, bool enabled) {
    if (index >= 0 && index < activeRanges) {
        ranges[index].enabled = enabled;
    }
}

bool AddressFilter::isValidAddress(uint8_t address) {
    return address >= 0x08 && address <= 0x77;
}

bool AddressFilter::isValidRange(uint8_t minAddr, uint8_t maxAddr) {
    return isValidAddress(minAddr) && 
           isValidAddress(maxAddr) && 
           minAddr <= maxAddr;
}