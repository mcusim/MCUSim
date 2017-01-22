#ifndef MSIM_AVR_SIM_H_
#define MSIM_AVR_SIM_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint32_t avr_flashaddr_t;

enum avr_state {
	AVR_RUNNING = INT16_MIN,
	AVR_STOPPED,
	AVR_SLEEPING
}

/*
 * Instance of the AVR microcontroller.
 */
struct avr {
	const char *mmcu;		/* Name of the MCU */

	uint16_t spm_pagesize;		/* For devices with bootloader support,
					   the flash pagesize (in bytes) to be
					   used for SPM instruction. */
	uint16_t flashstart;		/* The first byte address in flash
					   program space. */
	uint16_t flashend;		/* The last byte address in flash
					   program space. */
	uint16_t ramstart;
	uint16_t ramend;
	uint32_t ramsize;
	uint16_t e2start;		/* The first EEPROM address */
	uint16_t e2end;			/* The last EEPROM address */
	uint16_t e2size;
	uint16_t e2pagesize;		/* The size of the EEPROM page */
	uint8_t lockbits;
	uint8_t fuse[6];

	enum avr_state state;
	uint32_t freq;			/* Frequency we're currently
					   working at. */

	avr_flashaddr_t pc;		/* Current program counter register */
	avr_flashaddr_t reset_pc;	/* This is a value used to jump to
					   at reset time. It allows support
					   for bootloaders. */

	uint8_t *sreg;			/* Points directly to SREG placed
					   in data section. */

	uint8_t *flash;			/* Flash memory */
	uint8_t *data;			/* General purpose registers,
					   IO registers and SRAM */
};

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIM_H_ */
