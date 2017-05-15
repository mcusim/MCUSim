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
#define AVR_IO_PORT_MSGTYP		3

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
