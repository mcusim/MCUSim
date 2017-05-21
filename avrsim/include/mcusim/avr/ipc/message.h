/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, interactive simulator for
 * microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>
#include <sys/msg.h>

#include "mcusim/avr/sim/sim.h"

#define QKEY_BASE			755700
#define MSGSZ				64

/* AVR status queue */
#define AVR_SQ_KEY			(QKEY_BASE+1)
#define AVR_SQ_FLAGS			(IPC_CREAT | 0644)

/* AVR control queue */
#define AVR_CQ_KEY			(QKEY_BASE+2)
#define AVR_CQ_FLAGS			(IPC_CREAT | 0644)

#define AVR_START_SIM_MSGTYP		1
#define AVR_END_SIM_MSGTYP		2

#define AVR_IO_PORTB_MSGTYP		3
#define AVR_IO_PORTC_MSGTYP		4
#define AVR_IO_PORTD_MSGTYP		5
#define AVR_IO_SREG_MSGTYP		6

#define AVR_INST_MSGTYP			7

/* The raw message to cast into specific one based on a type. */
struct MSIM_RawMsg {
	long type;
	uint8_t data[MSGSZ];
};

/* The messages shows simulation process started or stopped. */
struct MSIM_SimMsg {
	long type;
	uint32_t mcuid;			/* ID of a simulated AVR MCU */
};

/*
 * The message allows to provide a status or manipulate
 * I/O port of the AVR microcontroller.
 */
struct MSIM_IOPortMsg {
	long type;
	uint32_t mcuid;			/* ID of a simulated AVR MCU */
	uint8_t port_data;		/* PORTx MCU register */
	uint8_t data_dir;		/* DDRx MCU register */
	uint8_t input_pins;		/* PINx MCU register */
};

/* The status message tells what instruction will be executed next. */
struct MSIM_InstMsg {
	long type;
	uint32_t mcuid;
	MSIM_AVRFlashAddr_t pc;		/* Program counter */
	uint8_t inst[4];		/* Most of the AVR instructions are
					   16-bits wide, but there
					   are 32-bits wide, too.
					       MSB  1   LSB  0
					   |--------|--------| 1st word
					   |--------|--------| 2nd word */
};

void MSIM_SendSimMsg(const int sqid,
		     struct MSIM_AVR *mcu,
		     long sim_type);
void MSIM_SendIOPortMsg(const int sqid,
			struct MSIM_AVR *mcu,
			uint8_t port_data,
			uint8_t data_dir,
			uint8_t input_pins,
			long io_type);
void MSIM_SendInstMsg(const int sqid,
		      struct MSIM_AVR *mcu,
		      uint8_t inst[4]);
