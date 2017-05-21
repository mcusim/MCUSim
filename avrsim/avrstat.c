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
#define __STDC_FORMAT_MACROS 1
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "mcusim/avr/ipc/message.h"

int main(int argc, char *argv[])
{
	int status_qid;
	struct MSIM_RawMsg raw_msg;
	struct MSIM_SimMsg *sim_msg;
	struct MSIM_InstMsg *inst_msg;

	/* Get AVR status queue */
	if ((status_qid = msgget(AVR_SQ_KEY, 0644)) < 0) {
		fprintf(stderr, "AVR status queue cannot be opened!\n");
		return -1;
	}

	while (1) {
		if (msgrcv(status_qid, (void *) &raw_msg, MSGSZ, 0, 0) < 0) {
			fprintf(stderr, "Cannot receive AVR status!\n");
			continue;
		}

		sim_msg = (struct MSIM_SimMsg *) &raw_msg;
		if (raw_msg.type == AVR_START_SIM_MSGTYP) {
			printf("Start of AVR simulation, mcuid: %" PRIu32 "\n",
			       sim_msg->mcuid);
		} else if (raw_msg.type == AVR_END_SIM_MSGTYP) {
			printf("End of AVR simulation, mcuid: %" PRIu32 "\n",
			       sim_msg->mcuid);
		} else if (raw_msg.type == AVR_INST_MSGTYP) {
			inst_msg = (struct MSIM_InstMsg *) sim_msg;
			printf("mcuid=%" PRIu32 ":\t%x: %x %x\n",
			       inst_msg->mcuid, inst_msg->pc,
			       inst_msg->inst[0], inst_msg->inst[1]);
		} else {
			printf("Unknown message, mcuid: %" PRIu32 ", type: %ld\n",
			       sim_msg->mcuid, sim_msg->type);
		}
	}
	return 0;
}
