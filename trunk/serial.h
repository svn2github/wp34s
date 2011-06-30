#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "xeq.h"

extern void send_program(decimal64 *nul1, decimal64 *nul2);
extern void recv_program(decimal64 *nul1, decimal64 *nul2);
extern void send_registers(decimal64 *nul1, decimal64 *nul2);
extern void recv_registers(decimal64 *nul1, decimal64 *nul2);
extern void send_all(decimal64 *nul1, decimal64 *nul2);
extern void recv_all(decimal64 *nul1, decimal64 *nul2);
extern void send_byte(decimal64 *nul1, decimal64 *nul2);
extern void recv_byte(decimal64 *byte, decimal64 *nul2);
extern void serial_open(decimal64 *byte, decimal64 *nul2);
extern void serial_close(decimal64 *byte, decimal64 *nul2);

#endif
