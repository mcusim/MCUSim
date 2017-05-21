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
#include "mcusim/avr/ipc/message.h"

void MSIM_SendSimMsg(const int sqid,
		     struct MSIM_AVR *mcu,
		     long sim_type)
{
	if (sqid < 0) {
		fprintf(stderr, "Illegal queue ID: %d\n", sqid);
		return;
	}
	struct MSIM_SimMsg sim_msg;

	sim_msg.type = sim_type;
	sim_msg.mcuid = mcu->id;
	msgsnd(sqid, (void *) &sim_msg, sizeof sim_msg.mcuid, 0);
}

void MSIM_SendIOPortMsg(const int sqid,
			struct MSIM_AVR *mcu,
			uint8_t port_data,
			uint8_t data_dir,
			uint8_t input_pins,
			long io_type)
{
	if (sqid < 0) {
		fprintf(stderr, "Illegal queue ID: %d\n", sqid);
		return;
	}
	struct MSIM_IOPortMsg msg;

	msg.type = io_type;
	msg.mcuid = mcu->id;
	msg.port_data = port_data;
	msg.data_dir = data_dir;
	msg.input_pins = input_pins;
	msgsnd(sqid, (void *) &msg, sizeof msg.mcuid +
				    sizeof msg.port_data +
				    sizeof msg.data_dir +
				    sizeof msg.input_pins, 0);
}

void MSIM_SendInstMsg(const int sqid,
		      struct MSIM_AVR *mcu,
		      uint8_t inst[4])
{
	if (sqid < 0) {
		fprintf(stderr, "Illegal queue ID: %d\n", sqid);
		return;
	}
	struct MSIM_InstMsg inst_msg;
	uint8_t i;

	inst_msg.type = AVR_INST_MSGTYP;
	inst_msg.mcuid = mcu->id;
	inst_msg.pc = mcu->pc;
	for (i = 0; i < 4; i++)
		inst_msg.inst[i] = inst[i];
	msgsnd(sqid, (void *) &inst_msg, sizeof inst_msg.mcuid +
					 sizeof inst_msg.pc +
					 sizeof inst_msg.inst, 0);
}
