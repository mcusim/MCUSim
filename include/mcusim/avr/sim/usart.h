/*
 * This file is part of MCUSim, an XSPICE library with microcontrollers.
 *
 * Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.
 *
 * MCUSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCUSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MSIM_AVR_USART_H_
#define MSIM_AVR_USART_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MSIM_AVR_USART {
	uint32_t baud;		/* Current baud rate value */
	uint8_t txb;		/* Transmit Buffer */
	uint32_t rx_ticks;	/* USART ticks passed since last Rx */
	uint32_t tx_ticks;	/* USART ticks passed since last Tx */
	uint32_t rx_presc;	/* Rx clock prescaler, (UBRR+1) */
	uint32_t tx_presc;	/* Tx clock prescaler, m*(UBRR+1) */
} MSIM_AVR_USART;

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_USART_H_ */
