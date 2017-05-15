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

		if (raw_msg.type == AVR_START_SIM_MSGTYP) {
			sim_msg = (struct MSIM_SimMsg *) &raw_msg;
			printf("Start of AVR simulation, mcuid: %" PRIu32 "\n",
			       sim_msg->mcuid);
		} else if (raw_msg.type == AVR_END_SIM_MSGTYP) {
			sim_msg = (struct MSIM_SimMsg *) &raw_msg;
			printf("End of AVR simulation, mcuid: %" PRIu32 "\n",
			       sim_msg->mcuid);
		} else {
			printf("Unknown message type: %ld!\n", raw_msg.type);
		}
	}
	return 0;
}
