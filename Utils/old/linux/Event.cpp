#include "../stdafx.h"
#include "Event.h"


Event::Event(bool bManualSet, bool bInitialStat) {
}

Event::~Event() {
}

bool Event::set() {
    return true;
}

bool Event::reset() {
    return true;
}

bool Event::acquire(uint32_t nTimeOut) {
    return false;
}
