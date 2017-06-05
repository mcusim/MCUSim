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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <inttypes.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/ipc/message.h"

#ifdef MSIM_IPC_MODE_QUEUE
static int status_qid = -1;		/* Status queue ID */
static int ctrl_qid = -1;		/* Control queue ID */

/*
 * Open/close IPC queues to let external programs to interact with
 * the simulator.
 */
static int open_queues(void);
static void close_queues(void);
#endif

#define DATA_MEMORY		1120

/*
 * To temporarily store data memory and test changes after execution of
 * an instruction.
 */
static uint8_t data_mem[DATA_MEMORY];

static int decode_inst(struct MSIM_AVR *mcu, uint16_t inst);
static int is_inst32(uint16_t inst);
static void before_inst(struct MSIM_AVR *mcu);
static void after_inst(struct MSIM_AVR *mcu);

/* AVR opcodes executors. */
static void exec_in_out(struct MSIM_AVR *mcu, uint16_t inst,
			uint8_t reg, uint8_t io_loc);
static void exec_cp(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_cpi(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_cpc(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_eor(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_load_immediate(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_rjmp(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_brne(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_brlt(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_brge(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_brcs(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_rcall(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_sts(struct MSIM_AVR *mcu, uint16_t inst);
/* static void exec_sts16(struct MSIM_AVR *mcu, uint16_t inst); */
static void exec_ret(struct MSIM_AVR *mcu);
static void exec_ori(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_sbi_cbi(struct MSIM_AVR *mcu, uint16_t inst, uint8_t set_bit);
static void exec_sbis_sbic(struct MSIM_AVR *mcu, uint16_t inst,
			   uint8_t set_bit);
static void exec_push_pop(struct MSIM_AVR *mcu, uint16_t inst, uint8_t push);
static void exec_movw(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_mov(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_sbci(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_sbiw(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_andi(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_and(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_subi(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_cli(struct MSIM_AVR *mcu, uint16_t inst);

static void exec_st_x(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_st_y(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_st_ydisp(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_st_z(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_st_zdisp(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_st(struct MSIM_AVR *mcu, uint16_t inst,
		    uint8_t *addr_low, uint8_t *addr_high, uint8_t regr);

static void exec_ld_x(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_ld_y(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_ld_ydisp(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_ld_z(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_ld_zdisp(struct MSIM_AVR *mcu, uint16_t inst);
static void exec_ld(struct MSIM_AVR *mcu, uint16_t inst,
		    uint8_t *addr_low, uint8_t *addr_high, uint8_t regd);

void MSIM_SimulateAVR(struct MSIM_AVR *mcu)
{
	uint16_t inst;
	uint8_t msb, lsb;

	#ifdef MSIM_TEXT_MODE
	printf("%" PRIu32 ":\tStart of AVR simulation\n", mcu->id);
	#endif
	#ifdef MSIM_IPC_MODE_QUEUE
	if (open_queues()) {
		close_queues();
		return;
	}
	MSIM_SendSimMsg(status_qid, mcu, AVR_START_SIM_MSGTYP);
	#endif

	while (1) {
		lsb = mcu->prog_mem[mcu->pc];
		msb = mcu->prog_mem[mcu->pc+1];
		inst = (uint16_t) (lsb | (msb << 8));

		#if defined MSIM_TEXT_MODE && defined MSIM_PRINT_INST
		printf("%" PRIu32 ":\t%x: %x %x\n",
		    	mcu->id, mcu->pc, lsb, msb);
		#endif
		#if defined MSIM_IPC_MODE_QUEUE && defined MSIM_PRINT_INST
		uint8_t i[4];
		i[0] = lsb;
		i[1] = msb;
		i[2] = i[3] = 0;
		MSIM_SendInstMsg(status_qid, mcu, i);
		#endif

		before_inst(mcu);
		if (decode_inst(mcu, inst)) {
			fprintf(stderr, "Unknown instruction: 0x%X\n", inst);
			exit(1);
		}
		after_inst(mcu);
	}

	#ifdef MSIM_TEXT_MODE
	printf("%" PRIu32 ":\tEnd of AVR simulation\n", mcu->id);
	#endif
	#ifdef MSIM_IPC_MODE_QUEUE
	MSIM_SendSimMsg(status_qid, mcu, AVR_END_SIM_MSGTYP);
	close_queues();
	#endif
}

int MSIM_InitAVR(struct MSIM_AVR *mcu, const char *mcu_name,
		 uint8_t *pm, uint32_t pm_size,
		 uint8_t *dm, uint32_t dm_size)
{
	if (!strcmp("atmega8a", mcu_name)) {
		return MSIM_M8AInit(mcu, pm, pm_size, dm, dm_size);
	} else {
		fprintf(stderr, "Microcontroller AVR %s is unsupported!\n",
				mcu_name);
	}
	return -1;
}

int MSIM_LoadProgmem(struct MSIM_AVR *mcu, FILE *fp)
{
	if (!strcmp("atmega8a", mcu->name)) {
		return MSIM_M8ALoadProgmem(mcu, fp);
	} else {
		fprintf(stderr, "Microcontroller AVR %s is unsupported!\n",
				mcu->name);
	}
	return -1;
}

void MSIM_UpdateSREGFlag(struct MSIM_AVR *mcu,
			 enum MSIM_AVRSREGFlag flag,
			 uint8_t set_f)
{
	uint8_t v;

	if (!mcu) {
		fprintf(stderr, "MCU is null");
		return;
	}

	switch (flag) {
	case AVR_SREG_CARRY:
		v = 0x01;
		break;
	case AVR_SREG_ZERO:
		v = 0x02;
		break;
	case AVR_SREG_NEGATIVE:
		v = 0x04;
		break;
	case AVR_SREG_TWOSCOM_OF:
		v = 0x08;
		break;
	case AVR_SREG_SIGN:
		v = 0x10;
		break;
	case AVR_SREG_HALF_CARRY:
		v = 0x20;
		break;
	case AVR_SREG_BITCOPY_ST:
		v = 0x40;
		break;
	case AVR_SREG_GLOB_INT:
		v = 0x80;
		break;
	}

	if (set_f)
		*mcu->sreg |= v;
	else
		*mcu->sreg &= ~v;
}

uint8_t MSIM_ReadSREGFlag(struct MSIM_AVR *mcu,
			  enum MSIM_AVRSREGFlag flag)
{
	uint8_t v, pos;

	if (!mcu) {
		fprintf(stderr, "MCU is null");
		return UINT8_MAX;
	}

	switch (flag) {
	case AVR_SREG_CARRY:
		v = 0x01;
		pos = 0;
		break;
	case AVR_SREG_ZERO:
		v = 0x02;
		pos = 1;
		break;
	case AVR_SREG_NEGATIVE:
		v = 0x04;
		pos = 2;
		break;
	case AVR_SREG_TWOSCOM_OF:
		v = 0x08;
		pos = 3;
		break;
	case AVR_SREG_SIGN:
		v = 0x10;
		pos = 4;
		break;
	case AVR_SREG_HALF_CARRY:
		v = 0x20;
		pos = 5;
		break;
	case AVR_SREG_BITCOPY_ST:
		v = 0x40;
		pos = 6;
		break;
	case AVR_SREG_GLOB_INT:
		v = 0x80;
		pos = 7;
		break;
	}

	return (*mcu->sreg & v) >> pos;
}

void MSIM_StackPush(struct MSIM_AVR *mcu, uint8_t val)
{
	uint16_t sp;

	sp = (uint16_t) ((*mcu->sp_low) | (*mcu->sp_high << 8));
	mcu->data_mem[sp--] = val;
	*mcu->sp_low = (uint8_t) (sp & 0xFF);
	*mcu->sp_high = (uint8_t) (sp >> 8);
}

uint8_t MSIM_StackPop(struct MSIM_AVR *mcu)
{
	uint16_t sp;
	uint8_t v;

	sp = (uint16_t) ((*mcu->sp_low) | (*mcu->sp_high << 8));
	v = mcu->data_mem[++sp];
	*mcu->sp_low = (uint8_t) (sp & 0xFF);
	*mcu->sp_high = (uint8_t) (sp >> 8);

	return v;
}

#ifdef MSIM_IPC_MODE_QUEUE
static int open_queues(void)
{
	if ((status_qid = msgget(AVR_SQ_KEY, AVR_SQ_FLAGS)) < 0) {
		fprintf(stderr, "AVR status queue cannot be opened!\n");
		return -1;
	}
	if ((ctrl_qid = msgget(AVR_CQ_KEY, AVR_CQ_FLAGS)) < 0) {
		fprintf(stderr, "AVR control queue cannot be opened!\n");
		return -1;
	}

	return 0;
}

static void close_queues(void)
{
	struct msqid_ds desc;

	if (status_qid >= 0) {
		msgctl(status_qid, IPC_RMID, &desc);
		status_qid = -1;
	}
	if (ctrl_qid >= 0) {
		msgctl(ctrl_qid, IPC_RMID, &desc);
		ctrl_qid = -1;
	}
}
#endif

static void before_inst(struct MSIM_AVR *mcu)
{
	memcpy(data_mem, mcu->data_mem, mcu->dm_size);
}

static void after_inst(struct MSIM_AVR *mcu)
{
	int16_t r;
	uint16_t i;

	for (i = 0; i < sizeof(mcu->io_addr)/sizeof(mcu->io_addr[0]); i++) {
		if ((r = mcu->io_addr[i]) < 0)
			continue;
		r += mcu->sfr_off;

		/* Has I/O register value been changed? */
		if (mcu->data_mem[r] == data_mem[r])
			continue;

		#ifdef MSIM_TEXT_MODE
		printf("%" PRIu32 ":\tIOREG=0x%x, VALUE=0x%x\n",
			mcu->id,
			mcu->io_addr[i],
			mcu->data_mem[r]);
		#endif
		#ifdef MSIM_IPC_MODE_QUEUE
		#endif
	}
}

static int decode_inst(struct MSIM_AVR *mcu, uint16_t inst)
{
	switch (inst & 0xF000) {
	case 0x0000:
		switch (inst) {
		case 0x0000:			/* NOP – No Operation */
			mcu->pc += 2;
			break;
		default:
			switch (inst & 0xFC00) {
			case 0x0400:
				exec_cpc(mcu, inst);
				goto exit;
			default:
				break;
			}

			switch (inst & 0xFF00) {
			case 0x0100:
				exec_movw(mcu, inst);
				break;
			default:
				return -1;
			}
			break;
		}
		break;
	case 0x1000:
		switch (inst & 0xFC00) {
		case 0x1400:
			exec_cp(mcu, inst);
			break;
		default:
			return -1;
		}
		break;
	case 0x2000:
		switch (inst & 0xFC00) {
		case 0x2000:
			exec_and(mcu, inst);
			break;
		case 0x2400:
			exec_eor(mcu, inst);
			break;
		case 0x2C00:
			exec_mov(mcu, inst);
			break;
		default:
			return -1;
		}
		break;
	case 0x3000:
		exec_cpi(mcu, inst);
		break;
	case 0x4000:
		exec_sbci(mcu, inst);
		break;
	case 0x5000:
		exec_subi(mcu, inst);
		break;
	case 0x6000:
		exec_ori(mcu, inst);
		break;
	case 0x7000:
		exec_andi(mcu, inst);
		break;
	case 0x8000:
		switch (inst & 0xD208) {
		case 0x8000:
			exec_ld_zdisp(mcu, inst);
			goto exit;
		case 0x8008:
			exec_ld_ydisp(mcu, inst);
			goto exit;
		case 0x8200:
			exec_st_zdisp(mcu, inst);
			goto exit;
		case 0x8208:
			exec_st_ydisp(mcu, inst);
			goto exit;
		}

		switch (inst & 0xFE0F) {
		case 0x8000:
			exec_ld_z(mcu, inst);
			break;
		case 0x8008:
			exec_ld_y(mcu, inst);
			break;
		case 0x8200:
			exec_st_z(mcu, inst);
			break;
		case 0x8208:
			exec_st_y(mcu, inst);
			break;
		default:
			return -1;
		}
		break;
	case 0x9000:
		switch (inst) {
		case 0x94F8:
			exec_cli(mcu, inst);
			break;
		case 0x9508:
			exec_ret(mcu);
			break;
		default:
			switch (inst & 0xFE0F) {
			case 0x9001:
			case 0x9002:
				exec_ld_z(mcu, inst);
				break;
			case 0x9009:
			case 0x900A:
				exec_ld_y(mcu, inst);
				break;
			case 0x900C:
			case 0x900D:
			case 0x900E:
				exec_ld_x(mcu, inst);
				break;
			case 0x900F:
				exec_push_pop(mcu, inst, 0);
				break;
			case 0x9200:
				exec_sts(mcu, inst);
				break;
			case 0x9201:
			case 0x9202:
				exec_st_z(mcu, inst);
				break;
			case 0x9209:
			case 0x920A:
				exec_st_y(mcu, inst);
				break;
			case 0x920C:
			case 0x920D:
			case 0x920E:
				exec_st_x(mcu, inst);
				break;
			case 0x920F:
				exec_push_pop(mcu, inst, 1);
				break;
			default:
				switch (inst & 0xFF00) {
				case 0x9700:
					exec_sbiw(mcu, inst);
					break;
				case 0x9800:
					exec_sbi_cbi(mcu, inst, 0);
					break;
				case 0x9900:
					exec_sbis_sbic(mcu, inst, 0);
					break;
				case 0x9A00:
					exec_sbi_cbi(mcu, inst, 1);
					break;
				case 0x9B00:
					exec_sbis_sbic(mcu, inst, 1);
					break;
				default:
					return -1;
				}
			}
			break;
		}
		break;
	case 0xA000:
		switch (inst & 0xD208) {
		case 0x8000:
			exec_ld_zdisp(mcu, inst);
			break;
		case 0x8008:
			exec_ld_ydisp(mcu, inst);
			break;
		case 0x8200:
			exec_st_zdisp(mcu, inst);
			break;
		case 0x8208:
			exec_st_ydisp(mcu, inst);
			break;
		default:
			return -1;
		}
		break;
	case 0xB000:
		exec_in_out(mcu, inst,
			    (inst & 0x01F0) >> 4,
			    (inst & 0x0F) | ((inst & 0x0600) >> 5));
		break;
	case 0xC000:
		exec_rjmp(mcu, inst);
		break;
	case 0xD000:
		exec_rcall(mcu, inst);
		break;
	case 0xE000:
		exec_load_immediate(mcu, inst);
		break;
	case 0xF000:
		switch (inst & 0xFC07) {
		case 0xF000:
			exec_brcs(mcu, inst);
			break;
		case 0xF004:
			exec_brlt(mcu, inst);
			break;
		case 0xF401:
			exec_brne(mcu, inst);
			break;
		case 0xF404:
			exec_brge(mcu, inst);
			break;
		default:
			return -1;
		}
		break;
	default:
		return -1;
	}
exit:
	return 0;
}

static void exec_eor(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * EOR - Exclusive OR
	 */
	uint8_t rd, rr;

	rd = (inst & 0x01F0) >> 4;
	rr = (inst & 0x0F) | ((inst & 0x0200) >> 5);

	mcu->data_mem[rd] = mcu->data_mem[rd] ^ mcu->data_mem[rr];
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !mcu->data_mem[rd]);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, mcu->data_mem[rd] & 0x80);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN, (mcu->data_mem[rd] & 0x80) ^ 0);
}

static void exec_in_out(struct MSIM_AVR *mcu, uint16_t inst,
			uint8_t reg, uint8_t io_loc)
{
	switch (inst & 0xF800) {
	/*
	 * IN - Load an I/O Location to Register
	 */
	case 0xB000:
		mcu->data_mem[reg] = mcu->data_mem[io_loc + mcu->sfr_off];
		break;
	/*
	 * OUT – Store Register to I/O Location
	 */
	case 0xB800:
		mcu->data_mem[io_loc + mcu->sfr_off] = mcu->data_mem[reg];
		break;
	}
	mcu->pc += 2;
}

static void exec_cpi(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* CPI – Compare with Immediate */
	uint8_t rd, rd_addr, c, r, buf;

	rd_addr = ((inst & 0xF0) >> 4) + 16;
	c = (inst & 0x0F) | ((inst & 0x0F00) >> 4);

	rd = mcu->data_mem[rd_addr];
	r = mcu->data_mem[rd_addr] - c;
	buf = (~rd & c) | (c & r) | (r & ~rd);
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (buf >> 7) & 0x01);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF,
			 (((rd & ~c & ~r) | (~rd & c & r)) >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 1);
	if (!r)
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 1);
	else
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
}

static void exec_cpc(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* CPC – Compare with Carry */
	uint8_t rd, rd_addr;
	uint8_t rr, rr_addr;
	uint8_t r, buf;

	rd_addr = (inst & 0x01F0) >> 4;
	rr_addr = (inst & 0x0F) | ((inst & 0x0200) >> 5);

	rd = mcu->data_mem[rd_addr];
	rr = mcu->data_mem[rr_addr];
	r = mcu->data_mem[rd_addr] -
	    mcu->data_mem[rr_addr] -
	    MSIM_ReadSREGFlag(mcu, AVR_SREG_CARRY);
	mcu->pc += 2;

	buf = (~rd & rr) | (rr & r) | (r & ~rd);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (buf >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 1);

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF,
			 (((rd & ~rr & ~r) | (~rd & rr & r)) >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	if (r)
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
}

static void exec_cp(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* CP - Compare */
	uint8_t rd, rd_addr;
	uint8_t rr, rr_addr;
	uint8_t r, buf;

	rd_addr = (inst & 0x01F0) >> 4;
	rr_addr = (inst & 0x0F) | ((inst & 0x0200) >> 5);

	rd = mcu->data_mem[rd_addr];
	rr = mcu->data_mem[rr_addr];
	r = mcu->data_mem[rd_addr] - mcu->data_mem[rr_addr];
	mcu->pc += 2;

	buf = (~rd & rr) | (rr & r) | (r & ~rd);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (buf >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF,
			 (((rd & ~rr & ~r) | (~rd & rr & r)) >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	if (!r) {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 1);
	} else {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
	}
}

static void exec_load_immediate(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * LDI – Load Immediate
	 */
	uint8_t rd_off, c;

	rd_off = (inst & 0xF0) >> 4;
	c = (inst & 0x0F) | ((inst & 0x0F00) >> 4);

	mcu->data_mem[0x10 + rd_off] = c;
	mcu->pc += 2;
}

static void exec_rjmp(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * RJMP - Relative Jump
	 */
	int16_t c;

	c = inst & 0x0FFF;
	if (c >= 2048)
		c -= 4096;
	mcu->pc = (uint32_t) (((int32_t) mcu->pc) + (c + 1) * 2);
}

static void exec_brne(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * BRNE – Branch if Not Equal
	 */
	int16_t c;

	if (!MSIM_ReadSREGFlag(mcu, AVR_SREG_ZERO)) {
		/* Z == 0, i.e. Rd != Rr */
		c = (int16_t) ((int16_t) (inst << 6)) >> 9;
		mcu->pc = (uint32_t) (((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		/* Z == 1, i.e. Rd == Rr */
		mcu->pc += 2;
	}
}

static void exec_st_x(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * ST – Store Indirect From Register to Data Space using Index X
	 */
	uint8_t regr, *x_low, *x_high;

	x_low = &mcu->data_mem[26];
	x_high = &mcu->data_mem[27];
	regr = (inst & 0x01F0) >> 4;
	exec_st(mcu, inst, x_low, x_high, regr);
}

static void exec_st_y(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * ST – Store Indirect From Register to Data Space using Index Y
	 */
	uint8_t regr, *y_low, *y_high;

	y_low = &mcu->data_mem[28];
	y_high = &mcu->data_mem[29];
	regr = (inst & 0x01F0) >> 4;
	exec_st(mcu, inst, y_low, y_high, regr);
}

static void exec_st_z(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * ST – Store Indirect From Register to Data Space using Index Z
	 */
	uint8_t regr, *z_low, *z_high;

	z_low = &mcu->data_mem[30];
	z_high = &mcu->data_mem[31];
	regr = (inst & 0x01F0) >> 4;
	exec_st(mcu, inst, z_low, z_high, regr);
}

static void exec_st(struct MSIM_AVR *mcu, uint16_t inst,
		    uint8_t *addr_low, uint8_t *addr_high, uint8_t regr)
{
	/*
	 * ST – Store Indirect From Register to Data Space
	 *	using Index X, Y or Z
	 */
	uint16_t addr = (uint16_t) *addr_low | (uint16_t) (*addr_high << 8);
	regr = (inst & 0x01F0) >> 4;

	switch (inst & 0x03) {
	case 0x02:	/*	X ← X-1, (X) ← Rr	X: Pre decremented */
		addr--;
		*addr_low = (uint8_t) (addr & 0xFF);
		*addr_high = (uint8_t) (addr >> 8);
	case 0x00:	/*	(X) ← Rr		X: Unchanged */
		mcu->data_mem[addr] = mcu->data_mem[regr];
		break;
	case 0x01:	/*	(X) ← Rr, X ← X+1	X: Post incremented */
		mcu->data_mem[addr] = mcu->data_mem[regr];
		addr++;
		*addr_low = (uint8_t) (addr & 0xFF);
		*addr_high = (uint8_t) (addr >> 8);
		break;
	}
	mcu->pc += 2;
}

static void exec_st_ydisp(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * ST (STD) – Store Indirect From Register to Data Space using Index Y
	 */
	uint16_t addr;
	uint8_t regr, *y_low, *y_high, disp;

	y_low = &mcu->data_mem[28];
	y_high = &mcu->data_mem[29];
	addr = (uint16_t) *y_low | (uint16_t) (*y_high << 8);
	regr = (inst & 0x01F0) >> 4;
	disp = (inst & 0x07) |
	       ((inst & 0x0C00) >> 7) |
	       ((inst & 0x2000) >> 8);

	mcu->data_mem[addr + disp] = mcu->data_mem[regr];
	mcu->pc += 2;
}

static void exec_st_zdisp(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * ST (STD) – Store Indirect From Register to Data Space using Index Z
	 */
	uint16_t addr;
	uint8_t regr, *z_low, *z_high, disp;

	z_low = &mcu->data_mem[30];
	z_high = &mcu->data_mem[31];
	addr = (uint16_t) *z_low | (uint16_t) (*z_high << 8);
	regr = (inst & 0x01F0) >> 4;
	disp = (inst & 0x07) |
	       ((inst & 0x0C00) >> 7) |
	       ((inst & 0x2000) >> 8);

	mcu->data_mem[addr + disp] = mcu->data_mem[regr];
	mcu->pc += 2;
}

static void exec_rcall(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * RCALL – Relative Call to Subroutine
	 */
	int16_t c;
	uint32_t pc;

	pc = mcu->pc + 2;
	c = inst & 0x0FFF;
	if (c >= 2048)
		c -= 4096;
	MSIM_StackPush(mcu, (uint8_t) (pc & 0xFF));
	MSIM_StackPush(mcu, (uint8_t) ((pc >> 8) & 0xFF));
	mcu->pc = (uint32_t) (((int32_t) mcu->pc) + (c + 1) * 2);
}

static void exec_sts(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * STS – Store Direct to Data Space
	 */
	uint16_t addr;
	uint8_t rr, addr_msb, addr_lsb;

	addr_lsb = mcu->prog_mem[mcu->pc + 2];
	addr_msb = mcu->prog_mem[mcu->pc + 3];
	addr = (uint16_t) (addr_lsb | (addr_msb << 8));

	rr = (inst & 0x01F0) >> 4;
	mcu->data_mem[addr] = mcu->data_mem[rr];
	mcu->pc += 4;
}

static void exec_ret(struct MSIM_AVR *mcu)
{
	/*
	 * RET – Return from Subroutine
	 */
	uint8_t ah, al;

	ah = MSIM_StackPop(mcu);
	al = MSIM_StackPop(mcu);
	mcu->pc = (uint16_t) ((ah << 8) | al);
}

static void exec_ori(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* ORI – Logical OR with Immediate */
	uint8_t rd, rd_addr, c, r;

	rd_addr = ((uint8_t) ((inst & 0xF0) >> 4)) + 16;
	c = (inst & 0x0F) | ((inst & 0x0F00) >> 4);

	rd = mcu->data_mem[rd_addr];
	r = mcu->data_mem[rd_addr] |= c;
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	if (!r)
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 1);
	else
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
}

static void exec_sbi_cbi(struct MSIM_AVR *mcu, uint16_t inst, uint8_t set_bit)
{
	/*
	 * SBI – Set Bit in I/O Register
	 * CBI – Clear Bit in I/O Register
	 */
	uint8_t reg, b;

	reg = (inst & 0x00F8) >> 3;
	b = inst & 0x07;
	if (set_bit)
		mcu->data_mem[reg] |= (1 << b);
	else
		mcu->data_mem[reg] &= ~(1 << b);

	mcu->pc += 2;
}

static void exec_sbis_sbic(struct MSIM_AVR *mcu, uint16_t inst,
			   uint8_t set_bit)
{
	/*
	 * SBIS – Skip if Bit in I/O Register is Set
	 * SBIC – Skip if Bit in I/O Register is Cleared
	 */
	uint8_t reg, b, pc_delta;
	uint8_t msb, lsb;
	uint16_t ni;

	reg = (inst & 0x00F8) >> 3;
	b = inst & 0x07;
	pc_delta = 1;
	if (set_bit) {
		if (mcu->data_mem[reg] & (1 << b)) {
			lsb = mcu->prog_mem[mcu->pc+2];
			msb = mcu->prog_mem[mcu->pc+3];
			ni = (uint16_t) (lsb | (msb << 8));
			if (is_inst32(ni))
				pc_delta = 6;
			else
				pc_delta = 4;
		}
	} else {
		if (mcu->data_mem[reg] ^ (1 << b)) {
			lsb = mcu->prog_mem[mcu->pc+2];
			msb = mcu->prog_mem[mcu->pc+3];
			ni = (uint16_t) (lsb | (msb << 8));
			if (is_inst32(ni))
				pc_delta = 6;
			else
				pc_delta = 4;
		}
	}
	mcu->pc += pc_delta;
}

static int is_inst32(uint16_t inst)
{
	uint16_t i = inst & 0xfc0f;
	return	/* STS */ i == 0x9200 ||
		/* LDS */ i == 0x9000 ||
		/* JMP */ i == 0x940c ||
		/* JMP */ i == 0x940d ||
		/* CALL */i == 0x940e ||
		/* CALL */i == 0x940f;
}

static void exec_push_pop(struct MSIM_AVR *mcu, uint16_t inst, uint8_t push)
{
	/*
	 * PUSH – Push Register on Stack
	 * POP – Pop Register from Stack
	 */
	uint8_t reg;

	reg = (inst >> 4) & 0x1F;
	if (push)
		MSIM_StackPush(mcu, mcu->data_mem[reg]);
	else
		mcu->data_mem[reg] = MSIM_StackPop(mcu);

	mcu->pc += 2;
}

static void exec_movw(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* MOVW – Copy Register Word */
	uint8_t regd, regr;

	regr = inst & 0x0F;
	regd = (inst >> 4) & 0x0F;
	mcu->data_mem[regd+1] = mcu->data_mem[regr+1];
	mcu->data_mem[regd] = mcu->data_mem[regr];
	mcu->pc += 2;
}

static void exec_mov(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* MOV - Copy register */
	uint8_t rd, rr;

	rr = ((inst & 0x200) >> 5) | (inst & 0x0F);
	rd = (inst & 0x1F0) >> 4;
	mcu->data_mem[rd] = mcu->data_mem[rr];
	mcu->pc += 2;
}

static void exec_ld_x(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * LD – Load Indirect from Data Space to Register using Index X
	 */
	uint8_t regd, *x_low, *x_high;

	x_low = &mcu->data_mem[26];
	x_high = &mcu->data_mem[27];
	regd = (inst & 0x01F0) >> 4;
	exec_ld(mcu, inst, x_low, x_high, regd);
}

static void exec_ld_y(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * LD – Load Indirect from Data Space to Register using Index Y
	 */
	uint8_t regd, *y_low, *y_high;

	y_low = &mcu->data_mem[28];
	y_high = &mcu->data_mem[29];
	regd = (inst & 0x01F0) >> 4;
	exec_ld(mcu, inst, y_low, y_high, regd);
}

static void exec_ld_z(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * LD – Load Indirect from Data Space to Register using Index Z
	 */
	uint8_t regd, *z_low, *z_high;

	z_low = &mcu->data_mem[30];
	z_high = &mcu->data_mem[31];
	regd = (inst & 0x01F0) >> 4;
	exec_ld(mcu, inst, z_low, z_high, regd);
}

static void exec_ld(struct MSIM_AVR *mcu, uint16_t inst,
		    uint8_t *addr_low, uint8_t *addr_high, uint8_t regd)
{
	/*
	 * LD – Load Indirect from Data Space to Register
	 *	using Index X, Y or Z
	 */
	uint16_t addr = (uint16_t) *addr_low | (uint16_t) (*addr_high << 8);

	switch (inst & 0x03) {
	case 0x02:	/*	X ← X-1, Rd ← (X)	X: Pre decremented */
		addr--;
		*addr_low = (uint8_t) (addr & 0xFF);
		*addr_high = (uint8_t) (addr >> 8);
	case 0x00:	/*	Rd ← (X)		X: Unchanged */
		mcu->data_mem[regd] = mcu->data_mem[addr];
		break;
	case 0x01:	/*	Rd ← (X), X ← X+1	X: Post incremented */
		mcu->data_mem[regd] = mcu->data_mem[addr];
		addr++;
		*addr_low = (uint8_t) (addr & 0xFF);
		*addr_high = (uint8_t) (addr >> 8);
		break;
	}
	mcu->pc += 2;
}

static void exec_ld_ydisp(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * LD – Load Indirect from Data Space to Register using Index Y
	 */
	uint16_t addr;
	uint8_t regd, *y_low, *y_high, disp;

	y_low = &mcu->data_mem[28];
	y_high = &mcu->data_mem[29];
	addr = (uint16_t) *y_low | (uint16_t) (*y_high << 8);
	regd = (inst & 0x01F0) >> 4;
	disp = (inst & 0x07) |
	       ((inst & 0x0C00) >> 7) |
	       ((inst & 0x2000) >> 8);

	mcu->data_mem[regd] = mcu->data_mem[addr + disp];
	mcu->pc += 2;
}

static void exec_ld_zdisp(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * LD – Load Indirect from Data Space to Register using Index Z
	 */
	uint16_t addr;
	uint8_t regd, *z_low, *z_high, disp;

	z_low = &mcu->data_mem[30];
	z_high = &mcu->data_mem[31];
	addr = (uint16_t) *z_low | (uint16_t) (*z_high << 8);
	regd = (inst & 0x01F0) >> 4;
	disp = (inst & 0x07) |
	       ((inst & 0x0C00) >> 7) |
	       ((inst & 0x2000) >> 8);

	mcu->data_mem[regd] = mcu->data_mem[addr + disp];
	mcu->pc += 2;
}

static void exec_sbci(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* SBCI – Subtract Immediate with Carry */
	uint8_t rd, rd_addr, c, r, buf;

	rd_addr = ((inst & 0xF0) >> 4) + 16;
	c = ((inst & 0xF00) >> 4) | (inst & 0x0F);

	rd = mcu->data_mem[rd_addr];
	r = mcu->data_mem[rd_addr] = mcu->data_mem[rd_addr] - c -
		MSIM_ReadSREGFlag(mcu, AVR_SREG_CARRY);
	mcu->pc += 2;

	buf = (~rd & c) | (c & r) | (r & ~rd);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (buf >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF,
			 (((rd & ~c & ~r) | (~rd & c & r)) >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	if (!r) {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 1);
	} else {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
	}
}

static void exec_brlt(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * BRLT – Branch if Less Than (Signed)
	 */
	uint8_t cond;
	int8_t c;

	cond = MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
	       MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF);
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (!cond)
		mcu->pc += 2;
	else
		mcu->pc = (uint32_t) (((int32_t) mcu->pc) + (c + 1) * 2);
}

static void exec_brge(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * BRGE – Branch if Greater or Equal (Signed)
	 */
	uint8_t cond;
	int8_t c;

	cond = MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
	       MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF);
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (!cond)
		mcu->pc = (uint32_t) (((int32_t) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_andi(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* ANDI – Logical AND with Immediate */
	uint8_t rd, rd_addr;
	uint8_t c, r;

	rd_addr = ((inst >> 4) & 0x0F) + 16;
	c = ((inst >> 4) & 0xF0) | (inst & 0x0F);

	rd = mcu->data_mem[rd_addr];
	r = mcu->data_mem[rd_addr] = mcu->data_mem[rd_addr] & c;
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	if (!r) {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 1);
	} else {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
	}
}

static void exec_and(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* AND - Logical AND */
	uint8_t rd, rd_addr;
	uint8_t rr, rr_addr;
	uint8_t r;

	rd_addr = (inst & 0x1F0) >> 4;
	rr_addr = ((inst & 0x200) >> 5) | (inst & 0x0F);

	rd = mcu->data_mem[rd_addr];
	rr = mcu->data_mem[rr_addr];
	r = mcu->data_mem[rd_addr] = mcu->data_mem[rd_addr] &
				     mcu->data_mem[rr_addr];
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	if (!r) {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 1);
	} else {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
	}
}

static void exec_sbiw(struct MSIM_AVR *mcu, uint16_t inst)
{
	/*
	 * SBIW – Subtract Immediate from Word
	 */
	const uint8_t regs[] = { 24, 26, 28, 30 };
	uint8_t rdh_addr, rdl_addr;
	uint16_t c, r, buf;

	rdl_addr = regs[(inst >> 4) & 0x03];
	rdh_addr = rdl_addr + 1;
	c = ((inst >> 2) & 0x30) | (inst & 0x0F);

	buf = r = (uint16_t) ((mcu->data_mem[rdh_addr] << 8) |
			      mcu->data_mem[rdl_addr]);
	r -= c;
	buf = r & ~buf;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (buf >> 15) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (uint8_t) ((r >> 15) & 1));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, (buf >> 15) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	if (!r) {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 1);
	} else {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
	}

	mcu->data_mem[rdh_addr] = (r >> 8) & 0x0F;
	mcu->data_mem[rdl_addr] = r & 0x0F;
	mcu->pc += 2;
}

static void exec_brcs(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* BRCS - Branch if Carry Set */
	uint8_t cond;
	int8_t c;

	cond = MSIM_ReadSREGFlag(mcu, AVR_SREG_CARRY);
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (cond)
		mcu->pc = (uint32_t) (((int32_t) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;

}

static void exec_subi(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* SUBI - Subtract Immediate */
	uint8_t rd, c, r, buf;
	uint8_t rd_addr;

	rd_addr = (inst & 0xF0) >> 4;
	c = ((inst & 0xF00) >> 4) | (inst & 0xF);

	rd = mcu->data_mem[rd_addr+16];
	r = mcu->data_mem[rd_addr+16] -= c;
	mcu->pc += 2;

	buf = (~rd & c) | (c & r) | (r & ~rd);

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, buf >> 7);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 0x01);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, r >> 7);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF,
			    ((rd & ~c & ~r) | (~rd & c & r)) >> 7);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	if (!r) {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 1);
	} else {
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
	}
}

static void exec_cli(struct MSIM_AVR *mcu, uint16_t inst)
{
	/* CLI - Clear Global Interrupt Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_GLOB_INT, 0);
	mcu->pc += 2;
}
