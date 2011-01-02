#ifdef _ARM_
#include "board.h"
TMyApplication *const MyApplication= (TMyApplication*)0x300000;

// restart the watchdog
inline void Watchdog() {
	AT91C_BASE_WDTC->WDTC_WDCR= 0xA5000001;
}

void watchdog(); // same function as above, but not inlined...
#else
extern TMyApplication MYAPPLICATION;
TMyApplication *const MyApplication= &MYAPPLICATION;
inline void Watchdog() { }
#endif
