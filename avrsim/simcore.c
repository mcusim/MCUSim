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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/hex/ihex.h"

#define DATA_MEMORY		65536

/* To temporarily store data memory and test changes after execution of
 * an instruction. */
static uint8_t data_mem[DATA_MEMORY];

static int decode_inst(struct MSIM_AVR *mcu, unsigned int inst);
static int is_inst32(unsigned int inst);
static void before_inst(struct MSIM_AVR *mcu);
static void after_inst(struct MSIM_AVR *mcu);
static int load_progmem(struct MSIM_AVR *mcu, FILE *fp);

/* AVR opcodes executors. */
static void exec_in_out(struct MSIM_AVR *mcu, unsigned int inst,
			unsigned char reg, unsigned char io_loc);
static void exec_cp(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_cpi(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_cpc(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_eor_clr(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_load_immediate(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_rjmp(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brne(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brlt(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brge(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brcs_brlo(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_rcall(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_sts(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_ret(struct MSIM_AVR *mcu);
static void exec_ori(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_sbi_cbi(struct MSIM_AVR *mcu, unsigned int inst, unsigned char set_bit);
static void exec_sbis_sbic(struct MSIM_AVR *mcu, unsigned int inst,
			   unsigned char set_bit);
static void exec_push_pop(struct MSIM_AVR *mcu, unsigned int inst, unsigned char push);
static void exec_movw(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_mov(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_sbci(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_sbiw(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_andi_cbr(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_and(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_subi(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_cli(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_adiw(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_adc(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_add(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_asr(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_bclr(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_bld(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brbc(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brbs(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brcc_brsh(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_break(struct MSIM_AVR *mcu);
static void exec_breq(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brhc(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brhs(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brid(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brie(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brmi(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brpl(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brtc(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brts(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brvc(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_brvs(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_bset(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_bst(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_call(struct MSIM_AVR *mcu, unsigned int inst_lsb);
static void exec_clc(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_clh(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_cln(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_cls(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_clt(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_clv(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_clz(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_com(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_cpse(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_dec(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_fmul(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_fmuls(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_fmulsu(struct MSIM_AVR *mcu, unsigned int inst);

static void exec_st_x(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_st_y(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_st_ydisp(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_st_z(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_st_zdisp(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_st(struct MSIM_AVR *mcu, unsigned int inst,
		    unsigned char *addr_low, unsigned char *addr_high,
		    unsigned char regr);

static void exec_ld_x(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_ld_y(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_ld_ydisp(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_ld_z(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_ld_zdisp(struct MSIM_AVR *mcu, unsigned int inst);
static void exec_ld(struct MSIM_AVR *mcu, unsigned int inst,
		    unsigned char *addr_low, unsigned char *addr_high,
		    unsigned char regd);

int MSIM_SimulateAVR(struct MSIM_AVR *mcu, unsigned long steps,
		     unsigned long addr)
{
	unsigned short inst, msb, lsb;
	char inf_loop, stop;

	stop = 0;
	inf_loop = !steps ? 1 : 0;
	steps = !steps ? 1 : steps;
	while (steps > 0) {
		lsb = (unsigned short) mcu->prog_mem[mcu->pc];
		msb = (unsigned short) mcu->prog_mem[mcu->pc+1];
		inst = (unsigned short) (lsb | (msb << 8));

		printf("%lu:\t%lx: %x %x\n", mcu->id, mcu->pc, lsb, msb);
		if (addr >= mcu->flashstart && addr <= mcu->flashend &&
		    addr == mcu->pc)
			stop = 1;

		before_inst(mcu);
		if (decode_inst(mcu, inst)) {
			fprintf(stderr, "Unknown instruction: 0x%X\n", inst);
			return 1;
		}
		after_inst(mcu);

		if (stop)
			break;
		if (!inf_loop)
			steps--;
	}
	return 0;
}

int MSIM_PrintInstructions(struct MSIM_AVR *mcu, unsigned long start_addr,
			   unsigned long end_addr, unsigned long steps)
{
	unsigned short inst, msb, lsb;
	unsigned long loc_pc;

	loc_pc = mcu->pc;
	if (start_addr > mcu->flashend || start_addr < mcu->flashstart)
		return 0;
	if (end_addr > mcu->flashend || end_addr < mcu->flashstart)
		end_addr = start_addr + steps;
	if (end_addr < start_addr)
		return 0;

	while (loc_pc <= end_addr) {
		lsb = (unsigned short) mcu->prog_mem[loc_pc];
		msb = (unsigned short) mcu->prog_mem[loc_pc+1];
		inst = (unsigned short) (lsb | (msb << 8));

		printf("%lu:\t%lx: %x %x\n", mcu->id, loc_pc, lsb, msb);

		loc_pc += is_inst32(inst) ? 4 : 2;
	}
	return 0;
}

int MSIM_InitAVR(struct MSIM_AVR *mcu, const char *mcu_name,
		 unsigned char *pm, unsigned long pm_size,
		 unsigned char *dm, unsigned long dm_size,
		 FILE *fp)
{
	if (!strcmp("atmega8a", mcu_name)) {
		if (MSIM_M8AInit(mcu, pm, pm_size, dm, dm_size))
			return -1;
	} else if (!strcmp("atmega328p", mcu_name)) {
		if (MSIM_M328PInit(mcu, pm, pm_size, dm, dm_size))
			return -1;
	} else if (!strcmp("atmega328", mcu_name)) {
		if (MSIM_M328Init(mcu, pm, pm_size, dm, dm_size))
			return -1;
	} else {
		fprintf(stderr, "Microcontroller AVR %s is unsupported!\n",
				mcu_name);
		return -1;
	}

	if (load_progmem(mcu, fp)) {
		fprintf(stderr, "Program memory cannot be loaded from a "
				"file!\n");
		return -1;
	}
	return 0;
}

int MSIM_SetProgmem(struct MSIM_AVR *mcu, unsigned char *mem,
		    unsigned long memsz)
{
	unsigned long flash_size;

	/* Size of the program memory, in bytes */
	flash_size = (mcu->flashend - mcu->flashstart) + 1;

	if (memsz < flash_size) {
		fprintf(stderr, "Program memory is limited by %lu bytes,"
				" %lu bytes isn't enough\n",
				flash_size, memsz);
		return -1;
	}
	mcu->prog_mem = mem;
	mcu->pm_size = memsz;
	return 0;
}

int MSIM_SetDatamem(struct MSIM_AVR *mcu, unsigned char *mem,
		    unsigned long memsz)
{
	unsigned long dm_size;

	/* Size of the data memory, in bytes */
	dm_size = mcu->regs + mcu->io_regs + mcu->ramsize;

	if (memsz < dm_size) {
		fprintf(stderr, "Data memory is limited by %lu bytes,"
				" %lu bytes isn't enough\n",
				dm_size, memsz);
		return -1;
	}
	mcu->data_mem = mem;
	mcu->dm_size = memsz;
	return 0;
}

static int load_progmem(struct MSIM_AVR *mcu, FILE *fp)
{
	IHexRecord rec, mem_rec;

	if (!fp) {
		fprintf(stderr, "Cannot read from the filestream\n");
		return -1;
	}

	/* Copy HEX data to program memory of the MCU */
	while (Read_IHexRecord(&rec, fp) == IHEX_OK) {
		switch (rec.type) {
		case IHEX_TYPE_00:	/* Data */
			memcpy(mcu->prog_mem + rec.address,
			       rec.data, (uint16_t) rec.dataLen);
			break;
		case IHEX_TYPE_01:	/* End of File */
		default:		/* Other types, unlikely occured */
			continue;
		}
	}

	/* Verify checksum of the loaded data */
	rewind(fp);
	while (Read_IHexRecord(&rec, fp) == IHEX_OK) {
		if (rec.type != IHEX_TYPE_00)
			continue;

		memcpy(mem_rec.data, mcu->prog_mem + rec.address,
		       (uint16_t) rec.dataLen);
		mem_rec.address = rec.address;
		mem_rec.dataLen = rec.dataLen;
		mem_rec.type = rec.type;
		mem_rec.checksum = 0;

		mem_rec.checksum = Checksum_IHexRecord(&mem_rec);
		if (mem_rec.checksum != rec.checksum) {
			printf("Checksum is not correct: 0x%X (memory) != "
			       "0x%X (file)\nFile record:\n",
			       mem_rec.checksum, rec.checksum);
			Print_IHexRecord(&rec);
			printf("Memory record:\n");
			Print_IHexRecord(&mem_rec);
			return -1;
		}
	}
	return 0;
}

void MSIM_UpdateSREGFlag(struct MSIM_AVR *mcu, enum MSIM_AVRSREGFlag flag,
			 unsigned char set_f)
{
	unsigned char v;

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
	case AVR_SREG_T_BIT:
		v = 0x40;
		break;
	case AVR_SREG_GLOB_INT:
		v = 0x80;
		break;
	}

	if (set_f)
		*mcu->sreg |= v;
	else
		*mcu->sreg &= (unsigned char)~v;
}

unsigned char MSIM_ReadSREGFlag(struct MSIM_AVR *mcu,
				enum MSIM_AVRSREGFlag flag)
{
	unsigned char pos;

	if (!mcu) {
		fprintf(stderr, "MCU is null");
		return UINT8_MAX;
	}

	switch (flag) {
	case AVR_SREG_CARRY:
		pos = 0;
		break;
	case AVR_SREG_ZERO:
		pos = 1;
		break;
	case AVR_SREG_NEGATIVE:
		pos = 2;
		break;
	case AVR_SREG_TWOSCOM_OF:
		pos = 3;
		break;
	case AVR_SREG_SIGN:
		pos = 4;
		break;
	case AVR_SREG_HALF_CARRY:
		pos = 5;
		break;
	case AVR_SREG_T_BIT:
		pos = 6;
		break;
	case AVR_SREG_GLOB_INT:
		pos = 7;
		break;
	}

	return (unsigned char)((*mcu->sreg >> pos) & 0x01);
}

void MSIM_StackPush(struct MSIM_AVR *mcu, uint8_t val)
{
	uint16_t sp;

	sp = (uint16_t) ((*mcu->spl) | (*mcu->sph << 8));
	mcu->data_mem[sp--] = val;
	*mcu->spl = (uint8_t) (sp & 0xFF);
	*mcu->sph = (uint8_t) (sp >> 8);
}

uint8_t MSIM_StackPop(struct MSIM_AVR *mcu)
{
	uint16_t sp;
	uint8_t v;

	sp = (uint16_t) ((*mcu->spl) | (*mcu->sph << 8));
	v = mcu->data_mem[++sp];
	*mcu->spl = (uint8_t) (sp & 0xFF);
	*mcu->sph = (uint8_t) (sp >> 8);

	return v;
}

static void before_inst(struct MSIM_AVR *mcu)
{
	memcpy(data_mem, mcu->data_mem, mcu->dm_size);
}

static void after_inst(struct MSIM_AVR *mcu)
{
	uint16_t i;

	for (i = 0; i < mcu->io_regs; i++) {
		/* Has I/O register value been changed? */
		if (mcu->data_mem[i+mcu->sfr_off] == data_mem[i+mcu->sfr_off])
			continue;

		#ifdef MSIM_TEXT_MODE
		printf("%" PRIu32 ":\tIOREG=0x%x, VALUE=0x%x\n",
		       mcu->id, i, mcu->data_mem[i+mcu->sfr_off]);
		#endif
	}
}

static int decode_inst(struct MSIM_AVR *mcu, unsigned int inst)
{
	switch (inst & 0xF000) {
	case 0x0000:
		if ((inst & 0x77) == 0x308) {
			exec_fmul(mcu, inst);
			break;
		} else if ((inst & 0x77) == 0x380) {
			exec_fmuls(mcu, inst);
			break;
		} else if ((inst & 0x77) == 0x388) {
			exec_fmulsu(mcu, inst);
			break;
		}

		switch (inst) {
		case 0x0000:			/* NOP – No Operation */
			mcu->pc += 2;
			break;
		default:
			switch (inst & 0xFC00) {
			case 0x0400:
				exec_cpc(mcu, inst);
				goto exit;
			case 0x0C00:
				exec_add(mcu, inst);
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
		case 0x1000:
			exec_cpse(mcu, inst);
			break;
		case 0x1400:
			exec_cp(mcu, inst);
			break;
		case 0x1C00:
			exec_adc(mcu, inst);
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
			exec_eor_clr(mcu, inst);
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
		exec_andi_cbr(mcu, inst);
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
		if ((inst & 0xFF00) == 0x9600) {
			exec_adiw(mcu, inst);
			break;
		} else if ((inst & 0xFF8F) == 0x9488) {
			exec_bclr(mcu, inst);
			break;
		} else if ((inst & 0xFF8F) == 0x9408) {
			exec_bset(mcu, inst);
			break;
		} else if ((inst & 0xFE0E) == 0x940E) {
			exec_call(mcu, inst);
			break;
		}

		switch (inst) {
		case 0x9488:
			exec_clc(mcu, inst);
			break;
		case 0x9498:
			exec_clz(mcu, inst);
			break;
		case 0x94A8:
			exec_cln(mcu, inst);
			break;
		case 0x94B8:
			exec_clv(mcu, inst);
			break;
		case 0x94C8:
			exec_cls(mcu, inst);
			break;
		case 0x94D8:
			exec_clh(mcu, inst);
			break;
		case 0x94E8:
			exec_clt(mcu, inst);
			break;
		case 0x94F8:
			exec_cli(mcu, inst);
			break;
		case 0x9508:
			exec_ret(mcu);
			break;
		case 0x9598:
			exec_break(mcu);
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
			case 0x9400:
				exec_com(mcu, inst);
				break;
			case 0x9405:
				exec_asr(mcu, inst);
				break;
			case 0x940A:
				exec_dec(mcu, inst);
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
			    (unsigned char)((inst & 0x01F0) >> 4),
			    (unsigned char)((inst & 0x0F) |
				    	    ((inst & 0x0600) >> 5)));
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
		if ((inst & 0xFE08) == 0xF800) {
			exec_bld(mcu, inst);
			break;
		} else if ((inst & 0xFE08) == 0xFA00) {
			exec_bst(mcu, inst);
			break;
		} else if ((inst & 0xFC00) == 0xF400) {
			exec_brbc(mcu, inst);
			break;
		} else if ((inst & 0xFC00) == 0xF000) {
			exec_brbs(mcu, inst);
			break;
		}

		switch (inst & 0xFC07) {
		case 0xF000:
			exec_brcs_brlo(mcu, inst);
			break;
		case 0xF001:
			exec_breq(mcu, inst);
			break;
		case 0xF002:
			exec_brmi(mcu, inst);
			break;
		case 0xF003:
			exec_brvs(mcu, inst);
			break;
		case 0xF004:
			exec_brlt(mcu, inst);
			break;
		case 0xF005:
			exec_brhs(mcu, inst);
			break;
		case 0xF006:
			exec_brts(mcu, inst);
			break;
		case 0xF007:
			exec_brie(mcu, inst);
			break;
		case 0xF400:
			exec_brcc_brsh(mcu, inst);
			break;
		case 0xF401:
			exec_brne(mcu, inst);
			break;
		case 0xF402:
			exec_brpl(mcu, inst);
			break;
		case 0xF403:
			exec_brvc(mcu, inst);
			break;
		case 0xF404:
			exec_brge(mcu, inst);
			break;
		case 0xF405:
			exec_brhc(mcu, inst);
			break;
		case 0xF406:
			exec_brtc(mcu, inst);
			break;
		case 0xF407:
			exec_brid(mcu, inst);
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

static void exec_eor_clr(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* EOR - Exclusive OR */
	unsigned char rd, rr;

	rd = (unsigned char)((inst & 0x01F0) >> 4);
	rr = (unsigned char)((inst & 0x0F) | ((inst & 0x0200) >> 5));

	mcu->data_mem[rd] = mcu->data_mem[rd] ^ mcu->data_mem[rr];
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !mcu->data_mem[rd]);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, mcu->data_mem[rd] & 0x80);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN, (mcu->data_mem[rd] & 0x80) ^ 0);
}

static void exec_in_out(struct MSIM_AVR *mcu, unsigned int inst,
			unsigned char reg, unsigned char io_loc)
{
	switch (inst & 0xF800) {
	/* IN - Load an I/O Location to Register */
	case 0xB000:
		mcu->data_mem[reg] = mcu->data_mem[io_loc + mcu->sfr_off];
		break;
	/* OUT – Store Register to I/O Location */
	case 0xB800:
		mcu->data_mem[io_loc + mcu->sfr_off] = mcu->data_mem[reg];
		break;
	}
	mcu->pc += 2;
}

static void exec_cpi(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CPI – Compare with Immediate */
	uint8_t rd, rd_addr, c;
	int r, buf;

	rd_addr = (unsigned char)(((inst & 0xF0) >> 4) + 16);
	c = (unsigned char)((inst & 0x0F) | ((inst & 0x0F00) >> 4));

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
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_cpc(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CPC – Compare with Carry */
	unsigned char rd, rd_addr;
	unsigned char rr, rr_addr;
	int r, buf;

	rd_addr = (unsigned char)((inst & 0x01F0) >> 4);
	rr_addr = (unsigned char)((inst & 0x0F) | ((inst & 0x0200) >> 5));

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

static void exec_cp(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CP - Compare */
	unsigned char rd, rd_addr;
	unsigned char rr, rr_addr;
	int buf, r;

	rd_addr = (unsigned char)((inst & 0x01F0) >> 4);
	rr_addr = (unsigned char)((inst & 0x0F) | ((inst & 0x0200) >> 5));

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
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_load_immediate(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* LDI – Load Immediate */
	unsigned char rd_off, c;

	rd_off = (unsigned char)((inst & 0xF0) >> 4);
	c = (unsigned char)((inst & 0x0F) | ((inst & 0x0F00) >> 4));

	mcu->data_mem[0x10 + rd_off] = c;
	mcu->pc += 2;
}

static void exec_rjmp(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* RJMP - Relative Jump */
	int c;

	c = inst & 0x0FFF;
	if (c >= 2048)
		c -= 4096;
	mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
}

static void exec_brne(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRNE – Branch if Not Equal */
	int c;

	if (!MSIM_ReadSREGFlag(mcu, AVR_SREG_ZERO)) {
		/* Z == 0, i.e. Rd != Rr */
		c = (int) ((int) (inst << 6)) >> 9;
		mcu->pc = (unsigned long) (((long) mcu->pc) + (c + 1) * 2);
	} else {
		/* Z == 1, i.e. Rd == Rr */
		mcu->pc += 2;
	}
}

static void exec_st_x(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ST – Store Indirect From Register to Data Space using Index X */
	uint8_t regr, *x_low, *x_high;

	x_low = &mcu->data_mem[26];
	x_high = &mcu->data_mem[27];
	regr = (unsigned char)((inst & 0x01F0) >> 4);
	exec_st(mcu, inst, x_low, x_high, regr);
}

static void exec_st_y(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ST – Store Indirect From Register to Data Space using Index Y */
	unsigned char regr, *y_low, *y_high;

	y_low = &mcu->data_mem[28];
	y_high = &mcu->data_mem[29];
	regr = (unsigned char)((inst & 0x01F0) >> 4);
	exec_st(mcu, inst, y_low, y_high, regr);
}

static void exec_st_z(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ST – Store Indirect From Register to Data Space using Index Z */
	unsigned char regr, *z_low, *z_high;

	z_low = &mcu->data_mem[30];
	z_high = &mcu->data_mem[31];
	regr = (unsigned char)((inst & 0x01F0) >> 4);
	exec_st(mcu, inst, z_low, z_high, regr);
}

static void exec_st(struct MSIM_AVR *mcu, unsigned int inst,
		    unsigned char *addr_low, unsigned char *addr_high,
		    unsigned char regr)
{
	/* ST – Store Indirect From Register to Data Space
	 *	using Index X, Y or Z */
	unsigned int addr = (unsigned int)(*addr_low | (*addr_high << 8));
	regr = (unsigned char)((inst & 0x01F0) >> 4);

	switch (inst & 0x03) {
	case 0x02:	/*	X ← X-1, (X) ← Rr	X: Pre decremented */
		addr--;
		*addr_low = (unsigned char) (addr & 0xFF);
		*addr_high = (unsigned char) (addr >> 8);
	case 0x00:	/*	(X) ← Rr		X: Unchanged */
		mcu->data_mem[addr] = mcu->data_mem[regr];
		break;
	case 0x01:	/*	(X) ← Rr, X ← X+1	X: Post incremented */
		mcu->data_mem[addr] = mcu->data_mem[regr];
		addr++;
		*addr_low = (unsigned char) (addr & 0xFF);
		*addr_high = (unsigned char) (addr >> 8);
		break;
	}
	mcu->pc += 2;
}

static void exec_st_ydisp(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ST (STD) – Store Indirect using Index Y */
	unsigned int addr;
	unsigned char regr, *y_low, *y_high, disp;

	y_low = &mcu->data_mem[28];
	y_high = &mcu->data_mem[29];
	addr = (unsigned int) *y_low | (unsigned int) (*y_high << 8);
	regr = (unsigned char)((inst & 0x01F0) >> 4);
	disp = (unsigned char)((inst & 0x07) |
			       ((inst & 0x0C00) >> 7) |
			       ((inst & 0x2000) >> 8));

	mcu->data_mem[addr + disp] = mcu->data_mem[regr];
	mcu->pc += 2;
}

static void exec_st_zdisp(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ST (STD) – Store Indirect using Index Z*/
	unsigned int addr;
	unsigned char regr, *z_low, *z_high, disp;

	z_low = &mcu->data_mem[30];
	z_high = &mcu->data_mem[31];
	addr = (unsigned int) *z_low | (unsigned int) (*z_high << 8);
	regr = (unsigned char)((inst & 0x01F0) >> 4);
	disp = (unsigned char)((inst & 0x07) |
			       ((inst & 0x0C00) >> 7) |
			       ((inst & 0x2000) >> 8));

	mcu->data_mem[addr + disp] = mcu->data_mem[regr];
	mcu->pc += 2;
}

static void exec_rcall(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* RCALL – Relative Call to Subroutine */
	int c;
	unsigned long pc;

	pc = mcu->pc + 2;
	c = inst & 0x0FFF;
	if (c >= 2048)
		c -= 4096;
	MSIM_StackPush(mcu, (unsigned char) (pc & 0xFF));
	MSIM_StackPush(mcu, (unsigned char) ((pc >> 8) & 0xFF));
	mcu->pc = (unsigned long) (((long) mcu->pc) + (c + 1) * 2);
}

static void exec_sts(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* STS – Store Direct to Data Space */
	unsigned int addr;
	unsigned char rr, addr_msb, addr_lsb;

	addr_lsb = mcu->prog_mem[mcu->pc + 2];
	addr_msb = mcu->prog_mem[mcu->pc + 3];
	addr = (unsigned int) (addr_lsb | (addr_msb << 8));

	rr = (unsigned char)((inst & 0x01F0) >> 4);
	mcu->data_mem[addr] = mcu->data_mem[rr];
	mcu->pc += 4;
}

static void exec_ret(struct MSIM_AVR *mcu)
{
	/* RET – Return from Subroutine */
	unsigned char ah, al;

	ah = MSIM_StackPop(mcu);
	al = MSIM_StackPop(mcu);
	mcu->pc = (unsigned int)((ah << 8) | al);
}

static void exec_ori(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ORI – Logical OR with Immediate */
	unsigned char rd_addr, c, r;

	rd_addr = (unsigned char)(((inst & 0xF0) >> 4) + 16);
	c = (unsigned char)((inst & 0x0F) | ((inst & 0x0F00) >> 4));
	r = mcu->data_mem[rd_addr] |= c;
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_sbi_cbi(struct MSIM_AVR *mcu, unsigned int inst,
			 unsigned char set_bit)
{
	/* SBI – Set Bit in I/O Register
	 * CBI – Clear Bit in I/O Register */
	unsigned char reg, b;

	reg = (unsigned char)((inst & 0x00F8) >> 3);
	b = inst & 0x07;
	if (set_bit)
		mcu->data_mem[reg] |= (unsigned char)(1 << b);
	else
		mcu->data_mem[reg] &= (unsigned char)(~(1 << b));

	mcu->pc += 2;
}

static void exec_sbis_sbic(struct MSIM_AVR *mcu, unsigned int inst,
			   unsigned char set_bit)
{
	/* SBIS – Skip if Bit in I/O Register is Set
	 * SBIC – Skip if Bit in I/O Register is Cleared */
	unsigned char reg, b, pc_delta;
	unsigned char msb, lsb;
	unsigned int ni;

	reg = (unsigned char)((inst & 0x00F8) >> 3);
	b = inst & 0x07;
	pc_delta = 1;
	if (set_bit) {
		if (mcu->data_mem[reg] & (1 << b)) {
			lsb = mcu->prog_mem[mcu->pc+2];
			msb = mcu->prog_mem[mcu->pc+3];
			ni = (unsigned int) (lsb | (msb << 8));
			if (is_inst32(ni))
				pc_delta = 6;
			else
				pc_delta = 4;
		}
	} else {
		if (mcu->data_mem[reg] ^ (1 << b)) {
			lsb = mcu->prog_mem[mcu->pc+2];
			msb = mcu->prog_mem[mcu->pc+3];
			ni = (unsigned int) (lsb | (msb << 8));
			if (is_inst32(ni))
				pc_delta = 6;
			else
				pc_delta = 4;
		}
	}
	mcu->pc += pc_delta;
}

static int is_inst32(unsigned int inst)
{
	unsigned int i = inst & 0xfc0f;
	return	/* STS */ i == 0x9200 ||
		/* LDS */ i == 0x9000 ||
		/* JMP */ i == 0x940c ||
		/* JMP */ i == 0x940d ||
		/* CALL */i == 0x940e ||
		/* CALL */i == 0x940f;
}

static void exec_push_pop(struct MSIM_AVR *mcu, unsigned int inst,
			  unsigned char push)
{
	/*
	 * PUSH – Push Register on Stack
	 * POP – Pop Register from Stack
	 */
	unsigned char reg;

	reg = (inst >> 4) & 0x1F;
	if (push)
		MSIM_StackPush(mcu, mcu->data_mem[reg]);
	else
		mcu->data_mem[reg] = MSIM_StackPop(mcu);

	mcu->pc += 2;
}

static void exec_movw(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* MOVW – Copy Register Word */
	unsigned char regd, regr;

	regr = inst & 0x0F;
	regd = (inst >> 4) & 0x0F;
	mcu->data_mem[regd+1] = mcu->data_mem[regr+1];
	mcu->data_mem[regd] = mcu->data_mem[regr];
	mcu->pc += 2;
}

static void exec_mov(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* MOV - Copy register */
	unsigned char rd, rr;

	rr = (unsigned char)(((inst & 0x200) >> 5) | (inst & 0x0F));
	rd = (unsigned char)((inst & 0x1F0) >> 4);
	mcu->data_mem[rd] = mcu->data_mem[rr];
	mcu->pc += 2;
}

static void exec_ld_x(struct MSIM_AVR *mcu, unsigned int inst)
{
	/*
	 * LD – Load Indirect from Data Space to Register using Index X
	 */
	unsigned char regd, *x_low, *x_high;

	x_low = &mcu->data_mem[26];
	x_high = &mcu->data_mem[27];
	regd = (unsigned char)((inst & 0x01F0) >> 4);
	exec_ld(mcu, inst, x_low, x_high, regd);
}

static void exec_ld_y(struct MSIM_AVR *mcu, unsigned int inst)
{
	/*
	 * LD – Load Indirect from Data Space to Register using Index Y
	 */
	unsigned char regd, *y_low, *y_high;

	y_low = &mcu->data_mem[28];
	y_high = &mcu->data_mem[29];
	regd = (unsigned char)((inst & 0x01F0) >> 4);
	exec_ld(mcu, inst, y_low, y_high, regd);
}

static void exec_ld_z(struct MSIM_AVR *mcu, unsigned int inst)
{
	/*
	 * LD – Load Indirect from Data Space to Register using Index Z
	 */
	unsigned char regd, *z_low, *z_high;

	z_low = &mcu->data_mem[30];
	z_high = &mcu->data_mem[31];
	regd = (unsigned char)((inst & 0x01F0) >> 4);
	exec_ld(mcu, inst, z_low, z_high, regd);
}

static void exec_ld(struct MSIM_AVR *mcu, unsigned int inst,
		    unsigned char *addr_low, unsigned char *addr_high,
		    unsigned char regd)
{
	/* LD – Load Indirect from Data Space to Register
	 *	using Index X, Y or Z */
	unsigned int addr = (unsigned int) (*addr_low | (*addr_high << 8));

	switch (inst & 0x03) {
	case 0x02:	/*	X ← X-1, Rd ← (X)	X: Pre decremented */
		addr--;
		*addr_low = (unsigned char) (addr & 0xFF);
		*addr_high = (unsigned char) (addr >> 8);
	case 0x00:	/*	Rd ← (X)		X: Unchanged */
		mcu->data_mem[regd] = mcu->data_mem[addr];
		break;
	case 0x01:	/*	Rd ← (X), X ← X+1	X: Post incremented */
		mcu->data_mem[regd] = mcu->data_mem[addr];
		addr++;
		*addr_low = (unsigned char) (addr & 0xFF);
		*addr_high = (unsigned char) (addr >> 8);
		break;
	}
	mcu->pc += 2;
}

static void exec_ld_ydisp(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* LD – Load Indirect from Data Space to Register using Index Y */
	unsigned int addr;
	unsigned char regd, *y_low, *y_high, disp;

	y_low = &mcu->data_mem[28];
	y_high = &mcu->data_mem[29];
	addr = (unsigned int) *y_low | (unsigned int) (*y_high << 8);
	regd = (unsigned char)((inst & 0x01F0) >> 4);
	disp = (unsigned char)((inst & 0x07) |
			       ((inst & 0x0C00) >> 7) |
			       ((inst & 0x2000) >> 8));

	mcu->data_mem[regd] = mcu->data_mem[addr + disp];
	mcu->pc += 2;
}

static void exec_ld_zdisp(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* LD – Load Indirect from Data Space to Register using Index Z */
	unsigned int addr;
	unsigned char regd, *z_low, *z_high, disp;

	z_low = &mcu->data_mem[30];
	z_high = &mcu->data_mem[31];
	addr = (unsigned int) *z_low | (unsigned int) (*z_high << 8);
	regd = (unsigned char)((inst & 0x01F0) >> 4);
	disp = (unsigned char)((inst & 0x07) |
			       ((inst & 0x0C00) >> 7) |
			       ((inst & 0x2000) >> 8));

	mcu->data_mem[regd] = mcu->data_mem[addr + disp];
	mcu->pc += 2;
}

static void exec_sbci(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* SBCI – Subtract Immediate with Carry */
	unsigned char rd, rd_addr, c, r;
	int buf;

	rd_addr = (unsigned char)(((inst & 0xF0) >> 4) + 16);
	c = (unsigned char)(((inst & 0xF00) >> 4) | (inst & 0x0F));

	rd = mcu->data_mem[rd_addr];
	r = mcu->data_mem[rd_addr] = (unsigned char)(mcu->data_mem[rd_addr] -
			c - MSIM_ReadSREGFlag(mcu, AVR_SREG_CARRY));
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
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_brlt(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRLT – Branch if Less Than (Signed) */
	unsigned char cond;
	int c;

	cond = MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
	       MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF);
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (!cond)
		mcu->pc += 2;
	else
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
}

static void exec_brge(struct MSIM_AVR *mcu, unsigned int inst)
{
	/*
	 * BRGE – Branch if Greater or Equal (Signed)
	 */
	unsigned char cond;
	int c;

	cond = MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
	       MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF);
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (!cond)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_andi_cbr(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ANDI – Logical AND with Immediate */
	unsigned char rd_addr;
	unsigned char c, r;

	rd_addr = (unsigned char)(((inst >> 4) & 0x0F) + 16);
	c = (unsigned char)(((inst >> 4) & 0xF0) | (inst & 0x0F));
	r = mcu->data_mem[rd_addr] = mcu->data_mem[rd_addr] & c;
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_and(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* AND - Logical AND */
	unsigned char rd_addr, rr_addr, r;

	rd_addr = (unsigned char)((inst & 0x1F0) >> 4);
	rr_addr = (unsigned char)(((inst & 0x200) >> 5) | (inst & 0x0F));
	r = mcu->data_mem[rd_addr] = mcu->data_mem[rd_addr] &
				     mcu->data_mem[rr_addr];
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_sbiw(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* SBIW – Subtract Immediate from Word */
	const unsigned char regs[] = { 24, 26, 28, 30 };
	unsigned char rdh_addr, rdl_addr;
	unsigned int c, r, buf;

	rdl_addr = regs[(inst >> 4) & 0x03];
	rdh_addr = (unsigned char)(rdl_addr + 1);
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
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);

	mcu->data_mem[rdh_addr] = (r >> 8) & 0x0F;
	mcu->data_mem[rdl_addr] = r & 0x0F;
	mcu->pc += 2;
}

static void exec_brcc_brsh(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRCC – Branch if Carry Cleared */
	unsigned char cond;
	int c;

	cond = MSIM_ReadSREGFlag(mcu, AVR_SREG_CARRY);
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (!cond)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brcs_brlo(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRCS - Branch if Carry Set */
	unsigned char cond;
	int c;

	cond = MSIM_ReadSREGFlag(mcu, AVR_SREG_CARRY);
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (cond)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_subi(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* SUBI - Subtract Immediate */
	unsigned char rd, c, r;
	unsigned char rd_addr;
	int buf;

	rd_addr = (unsigned char)((inst & 0xF0) >> 4);
	c = (unsigned char)(((inst & 0xF00) >> 4) | (inst & 0xF));

	buf = rd = mcu->data_mem[rd_addr+16];
	buf -= c;
	r = mcu->data_mem[rd_addr+16] = (unsigned char) buf;
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
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_adiw(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ADIW – Add Immediate to Word */
	const unsigned char regs[] = { 24, 26, 28, 30 };
	unsigned char rdh_addr, rdl_addr;
	unsigned int c, r, rd;

	rdl_addr = regs[(inst >> 4) & 3];
	rdh_addr = (unsigned char)(rdl_addr + 1);
	c = ((inst >> 2) & 0x30) | (inst & 0x0F);

	rd = (uint16_t) ((mcu->data_mem[rdh_addr] << 8) |
			 mcu->data_mem[rdl_addr]);
	r = rd + c;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, ((~r & rd) >> 15) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (uint8_t) ((r >> 15) & 1));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, ((r & ~rd) >> 15) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);

	mcu->data_mem[rdh_addr] = (r >> 8) & 0x0F;
	mcu->data_mem[rdl_addr] = r & 0x0F;
	mcu->pc += 2;
}

static void exec_adc(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ADC - Add with Carry */
	unsigned char rd_addr, rr_addr;
	unsigned char rd, rr, r;
	int buf;

	rd_addr = (unsigned char)((inst & 0x1F0) >> 4);
	rr_addr = (unsigned char)(((inst & 0x200) >> 5) | (inst & 0x0F));

	rd = mcu->data_mem[rd_addr];
	rr = mcu->data_mem[rr_addr];
	mcu->data_mem[rd_addr] = r = (unsigned char)
		(rd + rr + MSIM_ReadSREGFlag(mcu, AVR_SREG_CARRY));

	buf = (rd & rr) | (rr & ~r) | (~r & rd);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (buf >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF,
			(((rd & rr & ~r) | (~rd & ~rr & r)) >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 1);
	mcu->pc += 2;
}

static void exec_add(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ADD - Add without Carry */
	unsigned char rd_addr, rr_addr;
	unsigned char rd, rr, r;
	int buf;

	rd_addr = (unsigned char)((inst & 0x1F0) >> 4);
	rr_addr = (unsigned char)(((inst & 0x200) >> 5) | (inst & 0x0F));

	rd = mcu->data_mem[rd_addr];
	rr = mcu->data_mem[rr_addr];
	mcu->data_mem[rd_addr] = r = (unsigned char)(rd + rr);

	buf = (rd & rr) | (rr & ~r) | (~r & rd);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (buf >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF,
			(((rd & rr & ~r) | (~rd & ~rr & r)) >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 1);
	mcu->pc += 2;
}

static void exec_asr(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* ASR – Arithmetic Shift Right */
	unsigned char rd_addr, rd, r;
	unsigned char msb_orig, lsb_orig;

	rd_addr = (unsigned char)((inst & 0x1F0) >> 4);
	rd = mcu->data_mem[rd_addr];
	msb_orig = (rd >> 7) & 1;
	lsb_orig = rd & 1;

	r = rd >> 1;
	if (msb_orig)
		r |= 1<<7;
	else
		r &= (unsigned char)(~(1<<7));
	mcu->data_mem[rd_addr] = r;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, lsb_orig);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, msb_orig);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_CARRY));
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
	mcu->pc += 2;
}

static void exec_bclr(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BCLR – Bit Clear in SREG */
	unsigned char bit;

	bit = (unsigned char)((inst & 0x70) >> 4);
	*mcu->sreg &= (unsigned char)(~((1<<bit)&0xFF));
	mcu->pc += 2;
}

static void exec_bld(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BLD - Bit Load from the T Flag in SREG to a Bit in Register */
	unsigned char bit, rd_addr, t;

	rd_addr = (unsigned char)((inst & 0x1F0) >> 4);
	bit = inst & 0x07;
	t = MSIM_ReadSREGFlag(mcu, AVR_SREG_T_BIT);
	if (t)
		mcu->data_mem[rd_addr] |= (unsigned char)((1<<bit)&0xFF);
	else
		mcu->data_mem[rd_addr] &= (unsigned char)(~((1<<bit)&0xFF));
	mcu->pc += 2;
}

static void exec_brbc(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRBC – Branch if Bit in SREG is Cleared */
	unsigned char cond;
	int c;

	cond = (*mcu->sreg >> (inst & 0x07)) & 1;
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (!cond)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brbs(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRBS – Branch if Bit in SREG is Set */
	unsigned char cond;
	int c;

	cond = (*mcu->sreg >> (inst & 0x07)) & 1;
	c = (inst >> 3) & 0x7F;
	if (c > 63)
		c -= 128;

	if (cond)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_break(struct MSIM_AVR *mcu)
{
	/* BREAK – Break (the AVR CPU is set in the Stopped Mode). */
	/* Treat it as NOP before actual support is implemented. */
	mcu->pc += 2;
}

static void exec_breq(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BREQ – Branch if Equal */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_ZERO);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (f)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brhc(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRHC – Branch if Half Carry Flag is Cleared */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_HALF_CARRY);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (!f)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brhs(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRHS – Branch if Half Carry Flag is Set */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_HALF_CARRY);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (f)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brid(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRID – Branch if Global Interrupt is Disabled */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_GLOB_INT);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (!f)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brie(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRIE – Branch if Global Interrupt is Enabled */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_GLOB_INT);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (f)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brmi(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRMI – Branch if Minus */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (f)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;

}

static void exec_brpl(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRPL – Branch if Plus */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (!f)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brtc(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRTC – Branch if the T Flag is Cleared */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_T_BIT);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (!f)
		mcu->pc = (unsigned long)(((long) mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brts(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRTS – Branch if the T Flag is Set */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_T_BIT);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (f)
		mcu->pc = (MSIM_AVRFlashAddr_t)
			  (((long)mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brvc(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRVC – Branch if Overflow Cleared */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (!f)
		mcu->pc = (MSIM_AVRFlashAddr_t)
			  (((long)mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_brvs(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BRVS – Branch if Overflow Set */
	unsigned char f;
	int c;

	f = MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;

	if (f)
		mcu->pc = (MSIM_AVRFlashAddr_t)
			  (((long)mcu->pc) + (c + 1) * 2);
	else
		mcu->pc += 2;
}

static void exec_bset(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BSET – Bit Set in SREG */
	unsigned int bit;

	bit = (inst & 0x70) >> 4;
	*mcu->sreg |= (unsigned char)((1<<bit)&0xFF);
	mcu->pc += 2;
}

static void exec_bst(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* BST – Bit Store from Bit in Register to T Flag in SREG */
	uint8_t b, rd_addr;

	b = inst & 0x07;
	rd_addr = (inst >> 4) & 0x1F;
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_T_BIT,
			    (mcu->data_mem[rd_addr] >> b) & 1);
	mcu->pc += 2;
}

static void exec_call(struct MSIM_AVR *mcu, unsigned int inst_lsb)
{
	/* CALL – Long Call to a Subroutine */
	uint8_t lsb, msb;
	uint16_t inst_msb;
	MSIM_AVRFlashAddr_t pc, c;

	/* prepare the whole 32-bit instruction */
	lsb = mcu->prog_mem[mcu->pc+2];
	msb = mcu->prog_mem[mcu->pc+3];
	inst_msb = (uint16_t) (lsb | (msb << 8));

	pc = mcu->pc + 4;
	c = (MSIM_AVRFlashAddr_t)( ((inst_msb<<6)&0xFF) |
				   ((inst_lsb>>3)&0x3E) |
				   (inst_lsb&1) );

	MSIM_StackPush(mcu, (uint8_t) (pc & 0xFF));
	MSIM_StackPush(mcu, (uint8_t) ((pc >> 8) & 0xFF));
	mcu->pc = c;
}

static void exec_clc(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CLC – Clear Carry Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, 0);
	mcu->pc += 2;
}

static void exec_clh(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CLH – Clear Half Carry Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_HALF_CARRY, 0);
	mcu->pc += 2;
}

static void exec_cli(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CLI - Clear Global Interrupt Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_GLOB_INT, 0);
	mcu->pc += 2;
}

static void exec_cln(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CLN – Clear Negative Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, 0);
	mcu->pc += 2;
}

static void exec_cls(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CLS – Clear Signed Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN, 0);
	mcu->pc += 2;
}

static void exec_clt(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CLT – Clear T Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_T_BIT, 0);
	mcu->pc += 2;
}

static void exec_clv(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CLV – Clear Overflow Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	mcu->pc += 2;
}

static void exec_clz(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CLZ – Clear Zero Flag */
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, 0);
	mcu->pc += 2;
}

static void exec_com(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* COM – One’s Complement */
	unsigned char rd_addr, r;

	rd_addr = (inst >> 4) & 0x1F;
	r = mcu->data_mem[rd_addr] = (unsigned char)(~mcu->data_mem[rd_addr]);
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
}

static void exec_cpse(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* CPSE – Compare Skip if Equal */
	unsigned char rd_addr, rr_addr;

	rd_addr = (inst >> 4) & 0x1F;
	rr_addr = (unsigned char)(((inst >> 5) & 0x10) | (inst & 0x0F));
	if (mcu->data_mem[rd_addr] == mcu->data_mem[rr_addr])
		mcu->pc += is_inst32(mcu->prog_mem[mcu->pc+2]) ? 6 : 4;
	else
		mcu->pc += 2;
}

static void exec_dec(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* DEC - Decrement */
	unsigned short rd_addr, r, rd;
	unsigned int val;

	rd_addr = (inst >> 4) & 0x1F;
	rd = mcu->data_mem[rd_addr];
	val = mcu->data_mem[rd_addr];
	val -= 1U;
	r = mcu->data_mem[rd_addr] = (unsigned char)val;
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_NEGATIVE, (r >> 7) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_TWOSCOM_OF, rd == 0x80 ? 1 : 0);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_SIGN,
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_NEGATIVE) ^
			 MSIM_ReadSREGFlag(mcu, AVR_SREG_TWOSCOM_OF));
}

static void exec_fmul(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* FMUL – Fractional Multiply Unsigned */
	unsigned short rd_addr, rr_addr;
	unsigned short r;

	rd_addr = (unsigned short)(0x10 + ((inst >> 4) & 7));
	rr_addr = (unsigned short)(0x10 + (inst & 7));
	r = (unsigned short)((unsigned char)(mcu->data_mem[rd_addr]) *
			     (unsigned char)(mcu->data_mem[rr_addr]));
	mcu->data_mem[0] = (r << 1) & 0x0F;
	mcu->data_mem[1] = (r >> 7) & 0x0F;
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (r >> 15) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_fmuls(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* FMULS – Fractional Multiply Signed */
	unsigned short rd_addr, rr_addr;
	short r;

	rd_addr = (unsigned short)(0x10 + ((inst >> 4) & 7));
	rr_addr = (unsigned short)(0x10 + (inst & 7));
	r = (short)((signed char)(mcu->data_mem[rd_addr]) *
		    (signed char)(mcu->data_mem[rr_addr]));
	mcu->data_mem[0] = (r << 1) & 0x0F;
	mcu->data_mem[1] = (r >> 7) & 0x0F;
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (r >> 15) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}

static void exec_fmulsu(struct MSIM_AVR *mcu, unsigned int inst)
{
	/* FMULSU – Fractional Multiply Signed with Unsigned */
	unsigned short rd_addr, rr_addr;
	short r;

	rd_addr = (unsigned short)(0x10 + ((inst >> 4) & 7));
	rr_addr = (unsigned short)(0x10 + (inst & 7));
	r = (short)((signed char)(mcu->data_mem[rd_addr]) *
		    (unsigned char)(mcu->data_mem[rr_addr]));
	mcu->data_mem[0] = (r << 1) & 0x0F;
	mcu->data_mem[1] = (r >> 7) & 0x0F;
	mcu->pc += 2;

	MSIM_UpdateSREGFlag(mcu, AVR_SREG_CARRY, (r >> 15) & 1);
	MSIM_UpdateSREGFlag(mcu, AVR_SREG_ZERO, !r ? 1 : 0);
}
