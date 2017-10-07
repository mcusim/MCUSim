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
#ifndef MSIM_AVR_GDB_RSP_H_
#define MSIM_AVR_GDB_RSP_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

void MSIM_RSPInit(struct MSIM_AVR *mcu, int portn);

int MSIM_RSPHandle(void);

void MSIM_RSPClose(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_GDB_RSP_H_ */
