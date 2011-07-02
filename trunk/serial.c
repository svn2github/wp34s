#include "serial.h"
#include "xeq.h"
#include "storage.h"

/* Output a single byte to the serial
 */
static void put_byte(unsigned char byte) {
	err(ERR_PROG_BAD);
}

/* Get a single byte from the serial port
 */
static unsigned int get_byte(void) {
	err(ERR_PROG_BAD);
	return 65;
}


/* Transmit a number of bytes and a checksum to the serial port.
 */
static void put_bytes(void *bytes, unsigned int n) {
	unsigned int i;
	const unsigned short int checksum = crc16(bytes, n);

	for (i=0; i<n; i++)
		put_byte(((unsigned char *)bytes)[i]);
	put_byte((checksum >> 8) & 0xff);
	put_byte(checksum & 0xff);
}

/* Receive a number of bytes from the serial port and validate the checksum.
 * If the checksum doesn't match, nothing happens.
 */
static void get_bytes(void *bytes, unsigned int n) {
	unsigned int i;
	unsigned char buffer[2048];
	unsigned short crc;
	unsigned int byte;

	for (i=0; i<n; i++)
		buffer[i] = get_byte();

	byte = get_byte();
	crc = (byte & 0xff) << 8;
	byte = get_byte();
	crc |= (byte & 0xff);
	if (crc != crc16(buffer, n)) {
		err(ERR_IO);
		return;
	}
	xcopy(bytes, buffer, n);
}

/* Open the serial port
 */
void serial_open(decimal64 *nul1, decimal64 *nul2) {
}

/* Close the serial port
 */
void serial_close(decimal64 *nul1, decimal64 *nul2) {
}


/* Transmit the program space from RAM to the serial port.
 * 506 * 2 = 1012 bytes plus checksums.
 */
void send_program(decimal64 *nul1, decimal64 *nul2) {
	put_bytes(Prog, sizeof(s_opcode) * NUMPROG);
}

/* Load the RAM program space from the serial port.
 */
void recv_program(decimal64 *nul1, decimal64 *nul2) {
	if (check_return_stack_segment(-1)) {
		err( ERR_INVALID );
		return;
	}
	get_bytes(Prog, sizeof(s_opcode) * NUMPROG);
}


/* Send registers 00 through 99 to the serial port.
 * 100 registers at 8 bytes each is 800 bytes in total plus checksums.
 */
void send_registers(decimal64 *nul1, decimal64 *nul2) {
	put_bytes(Regs, sizeof(decimal64) * TOPREALREG);
}

/* Load registers 00 through 99 from the serial port.
 */
void recv_registers(decimal64 *nul1, decimal64 *nul2) {
	get_bytes(Regs, sizeof(decimal64) * TOPREALREG);
}


/* Send all of RAM to the serial port.  2kb in total.
 */
void send_all(decimal64 *nul1, decimal64 *nul2) {
	put_bytes(&PersistentRam, sizeof(TPersistentRam));
}

/* Load all of RAM from the serial port.
 */
void recv_all(decimal64 *nul1, decimal64 *nul2) {
	if (check_return_stack_segment(-1)) {
		err( ERR_INVALID );
		return;
	}
	get_bytes(&PersistentRam, sizeof(TPersistentRam));
}


/* Send a single byte as specified in X to the serial port.
 */
void send_byte(decimal64 *nul1, decimal64 *nul2) {
	int sgn;
	const unsigned char byte = get_int(&regX, &sgn) & 0xff;

	put_byte(byte);
}

/* Receive a single byte from the serial port and return the
 * value in X.
 * If the transfer times out, ???.
 */
void recv_byte(decimal64 *reg, decimal64 *nul2) {
	put_int(get_byte(), 0, reg);
}
