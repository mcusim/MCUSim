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

/* Utility macro for bits and bytes. */
#ifndef MSIM_BIT_MACRO_H_
#define MSIM_BIT_MACRO_H_ 1

/* Helps to test individual bits. */
#define IS_SET(b, bit)		(((b)>>(bit))&1)
#define IS_CLEAR(b, bit)	((~(((b)>>(bit))&1))&1)

/* Helps to test change of an individual bit. */
#define IS_RISE(init, val, bit)	((!((init>>bit)&1)) & ((val>>bit)&1))
#define IS_FALL(init, val, bit)	(((init>>bit)&1) & (!((val>>bit)&1)))

/* Helps to manipulate individual bits. */
#define CLEAR(b, bit)		((b)=(uint8_t)((b)&(uint8_t)(~(1<<(bit)))))
#define SET(b, bit)		((b)=(uint8_t)((b)|(uint8_t)(1<<(bit))))

#define UPDATE_BIT(val, i, b) do {	\
	if (b == 0U) {			\
		*val &= ~(1<<i);	\
	} else {			\
		*val |= (1<<i);		\
	}				\
} while (0)				\

#endif /* MSIM_BIT_MACRO_H_ */
