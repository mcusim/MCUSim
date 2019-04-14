/*
 * Copyright 2017-2019 The MCUSim Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 600
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "mcusim/mcusim.h"
#include "mcusim/log.h"
#include "mcusim/bit/macro.h"
#include "mcusim/avr/sim/macro.h"

static int decode_inst(struct MSIM_AVR *mcu, uint32_t inst);

static void exec_in_out(struct MSIM_AVR *mcu, uint32_t inst, uint8_t reg,
                        uint8_t io_loc);
static void exec_cp(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_cpi(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_cpc(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_eor_clr(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ldi(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_rjmp(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brne(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brlt(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brge(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brcs_brlo(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_rcall(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sts(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ret(struct MSIM_AVR *mcu);
static void exec_ori_sbr(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sbi_cbi(struct MSIM_AVR *mcu, uint32_t inst, uint8_t set_bit);
static void exec_sbis_sbic(struct MSIM_AVR *mcu, uint32_t inst, uint8_t set_bit);
static void exec_push_pop(struct MSIM_AVR *mcu, uint32_t inst, uint8_t push);
static void exec_movw(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_mov(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sbci(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sbiw(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_andi_cbr(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_and(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sub(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_subi(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sbc(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_cli(struct MSIM_AVR *mcu);
static void exec_adiw(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_adc_rol(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_add_lsl(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_asr(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_bclr(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_bld(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brbc(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brbs(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brcc_brsh(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_break(struct MSIM_AVR *mcu);
static void exec_breq(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brhc(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brhs(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brid(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brie(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brmi(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brpl(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brtc(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brts(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brvc(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_brvs(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_bset(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_bst(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_call(struct MSIM_AVR *mcu, uint32_t inst_msb);
static void exec_clc(struct MSIM_AVR *mcu);
static void exec_clh(struct MSIM_AVR *mcu);
static void exec_cln(struct MSIM_AVR *mcu);
static void exec_cls(struct MSIM_AVR *mcu);
static void exec_clt(struct MSIM_AVR *mcu);
static void exec_clv(struct MSIM_AVR *mcu);
static void exec_clz(struct MSIM_AVR *mcu);
static void exec_com(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_cpse(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_dec(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_fmul(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_fmuls(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_fmulsu(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_icall(struct MSIM_AVR *mcu);
static void exec_ijmp(struct MSIM_AVR *mcu);
static void exec_inc(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_jmp(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_lac(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_las(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_lat(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_lds(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_lds16(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_lpm(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_lsr(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sbrc(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sbrs(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_eicall(struct MSIM_AVR *mcu);
static void exec_eijmp(struct MSIM_AVR *mcu);
static void exec_xch(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ror(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_swap(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_reti(struct MSIM_AVR *mcu);
static void exec_sev(struct MSIM_AVR *mcu);
static void exec_set(struct MSIM_AVR *mcu);
static void exec_ses(struct MSIM_AVR *mcu);
static void exec_sen(struct MSIM_AVR *mcu);
static void exec_sei(struct MSIM_AVR *mcu);
static void exec_seh(struct MSIM_AVR *mcu);
static void exec_sec(struct MSIM_AVR *mcu);
static void exec_or(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_neg(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ser(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_mul(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_muls(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_mulsu(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_elpm(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_spm(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_sez(struct MSIM_AVR *mcu);
static void exec_wdr(struct MSIM_AVR *mcu);
static void exec_st_x(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_st_y(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_st_ydisp(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_st_z(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_st_zdisp(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_st(struct MSIM_AVR *mcu, uint32_t inst,
                    uint8_t *addr_low, uint8_t *addr_high,
                    uint8_t regr);
static void exec_ld_x(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ld_y(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ld_ydisp(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ld_z(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ld_zdisp(struct MSIM_AVR *mcu, uint32_t inst);
static void exec_ld(struct MSIM_AVR *mcu, uint32_t inst,
                    uint8_t *addr_low, uint8_t *addr_high,
                    uint8_t regd);

int MSIM_AVR_Step(struct MSIM_AVR *mcu)
{
	uint8_t msb, lsb;
	uint16_t inst;

	/* Clean I/O regs read/written during the previous MCU tick. */
	for (uint32_t i = 0; i < 4; i++) {
		mcu->writ_io[i] = 0;
		mcu->read_io[i] = 0;
	}

	if (!mcu->read_from_mpm) {
		/* Decode instruction from program memory as usual */
		lsb = (uint8_t) mcu->pm[mcu->pc];
		msb = (uint8_t) mcu->pm[mcu->pc+1];
		inst = (uint16_t) (lsb | (msb << 8));
	} else {
		/* Decode instruction from match points memory */
		lsb = (uint8_t) mcu->mpm[mcu->pc];
		msb = (uint8_t) mcu->mpm[mcu->pc+1];
		inst = (uint16_t) (lsb | (msb << 8));
		mcu->read_from_mpm = 0;
	}

	if (decode_inst(mcu, inst)) {
		snprintf(mcu->log, sizeof mcu->log, "unknown instruction: "
		         "0x%04" PRIX16 ", pc=0x%06" PRIX32, inst, mcu->pc);
		MSIM_LOG_FATAL(mcu->log);
		return -1;
	}
	return 0;
}

int MSIM_AVR_Is32(uint32_t inst)
{
	return ((inst&0xFE0F) == 0x9200) ||		/* STS */
	       ((inst&0xFE0F) == 0x9000) ||		/* LDS */
	       ((inst&0xFE0E) == 0x940C) ||		/* JMP */
	       ((inst&0xFE0E) == 0x940E);		/* CALL */
}

static int decode_inst(struct MSIM_AVR *mcu, uint32_t inst)
{
	uint8_t done = 0;

	switch (inst & 0xF000) {
	case 0x0000:
		if ((inst&0xFF00) == 0x0200) {
			exec_muls(mcu, inst);
			break;
		} else if ((inst&0xFF88) == 0x0300) {
			exec_mulsu(mcu, inst);
			break;
		} else if ((inst & 0xFF88) == 0x308) {
			exec_fmul(mcu, inst);
			break;
		} else if ((inst & 0xFF88) == 0x380) {
			exec_fmuls(mcu, inst);
			break;
		} else if ((inst & 0xFF88) == 0x388) {
			exec_fmulsu(mcu, inst);
			break;
		}

		switch (inst) {
		case 0x0000:			/* NOP – No Operation */
#ifdef DEBUG
			if (mcu->pc == 0U) {
				snprintf(mcu->log, sizeof mcu->log, "NOP at: "
				         "pc=0x%06" PRIX32, mcu->pc);
				MSIM_LOG_WARN(mcu->log);
			}
#endif
			mcu->pc += 2;
			break;
		default:
			switch (inst & 0xFC00) {
			case 0x0400:
				exec_cpc(mcu, inst);
				done = 1;
				break;
			case 0x0800:
				exec_sbc(mcu, inst);
				done = 1;
				break;
			case 0x0C00:
				exec_add_lsl(mcu, inst);
				done = 1;
				break;
			default:
				break;
			}

			if (done != 0) {
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
		case 0x1800:
			exec_sub(mcu, inst);
			break;
		case 0x1C00:
			exec_adc_rol(mcu, inst);
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
		case 0x2800:
			exec_or(mcu, inst);
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
		exec_ori_sbr(mcu, inst);
		break;
	case 0x7000:
		exec_andi_cbr(mcu, inst);
		break;
	case 0x8000:
		switch (inst & 0xD208) {
		case 0x8000:
			exec_ld_zdisp(mcu, inst);
			done = 1;
			break;
		case 0x8008:
			exec_ld_ydisp(mcu, inst);
			done = 1;
			break;
		case 0x8200:
			exec_st_zdisp(mcu, inst);
			done = 1;
			break;
		case 0x8208:
			exec_st_ydisp(mcu, inst);
			done = 1;
			break;
		}

		if (done != 0) {
			break;
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
		} else if ((inst & 0xFE0E) == 0x940C) {
			exec_jmp(mcu, inst);
			break;
		} else if ((inst & 0xFE0E) == 0x940E) {
			exec_call(mcu, inst);
			break;
		} else if ((inst&0xFC00) == 0x9C00) {
			exec_mul(mcu, inst);
			break;
		}

		switch (inst) {
		case 0x9408:
			exec_sec(mcu);
			break;
		case 0x9409:
			exec_ijmp(mcu);
			break;
		case 0x9418:
			exec_sez(mcu);
			break;
		case 0x9419:
			exec_eijmp(mcu);
			break;
		case 0x9428:
			exec_sen(mcu);
			break;
		case 0x9438:
			exec_sev(mcu);
			break;
		case 0x9448:
			exec_ses(mcu);
			break;
		case 0x9458:
			exec_seh(mcu);
			break;
		case 0x9468:
			exec_set(mcu);
			break;
		case 0x9478:
			exec_sei(mcu);
			break;
		case 0x9488:
			exec_clc(mcu);
			break;
		case 0x9498:
			exec_clz(mcu);
			break;
		case 0x94A8:
			exec_cln(mcu);
			break;
		case 0x94B8:
			exec_clv(mcu);
			break;
		case 0x94C8:
			exec_cls(mcu);
			break;
		case 0x94D8:
			exec_clh(mcu);
			break;
		case 0x94E8:
			exec_clt(mcu);
			break;
		case 0x94F8:
			exec_cli(mcu);
			break;
		case 0x9508:
			exec_ret(mcu);
			break;
		case 0x9509:
			exec_icall(mcu);
			break;
		case 0x9518:
			exec_reti(mcu);
			break;
		case 0x9519:
			exec_eicall(mcu);
			break;
		case 0x9598:
			exec_break(mcu);
			break;
		case 0x95A8:
			exec_wdr(mcu);
			break;
		case 0x95C8:
			exec_lpm(mcu, inst);
			break;
		case 0x95D8:
			exec_elpm(mcu, inst);
			break;
		case 0x95E8:
		case 0x95F8:
			exec_spm(mcu, inst);
			break;
		default:
			switch (inst & 0xFE0F) {
			case 0x9000:
				exec_lds(mcu, inst);
				break;
			case 0x9001:
			case 0x9002:
				exec_ld_z(mcu, inst);
				break;
			case 0x9004:
			case 0x9005:
				exec_lpm(mcu, inst);
				break;
			case 0x9006:
			case 0x9007:
				exec_elpm(mcu, inst);
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
			case 0x9204:
				exec_xch(mcu, inst);
				break;
			case 0x9205:
				exec_las(mcu, inst);
				break;
			case 0x9206:
				exec_lac(mcu, inst);
				break;
			case 0x9207:
				exec_lat(mcu, inst);
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
			case 0x9401:
				exec_neg(mcu, inst);
				break;
			case 0x9402:
				exec_swap(mcu, inst);
				break;
			case 0x9403:
				exec_inc(mcu, inst);
				break;
			case 0x9405:
				exec_asr(mcu, inst);
				break;
			case 0x9406:
				exec_lsr(mcu, inst);
				break;
			case 0x9407:
				exec_ror(mcu, inst);
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
		if ((inst & 0xF800) == 0xA000) {
			exec_lds16(mcu, inst);
			break;
		}

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
		            (uint8_t)((inst & 0x01F0) >> 4),
		            (uint8_t)((inst & 0x0F) |
		                            ((inst & 0x0600) >> 5)));
		break;
	case 0xC000:
		exec_rjmp(mcu, inst);
		break;
	case 0xD000:
		exec_rcall(mcu, inst);
		break;
	case 0xE000:
		if ((inst&0xFF0F) == 0xEF0F) {
			exec_ser(mcu, inst);
			break;
		}
		exec_ldi(mcu, inst);
		break;
	case 0xF000:
		if ((inst & 0xFE08) == 0xF800) {
			exec_bld(mcu, inst);
			break;
		} else if ((inst & 0xFE08) == 0xFA00) {
			exec_bst(mcu, inst);
			break;
		} else if ((inst & 0xFE08) == 0xFC00) {
			exec_sbrc(mcu, inst);
			break;
		} else if ((inst & 0xFE08) == 0xFE00) {
			exec_sbrs(mcu, inst);
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

	return 0;
}

static void exec_eor_clr(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* EOR - Exclusive OR */
	uint8_t rd, rr;

	rd = (uint8_t)((inst & 0x01F0) >> 4);
	rr = (uint8_t)((inst & 0x0F) | ((inst & 0x0200) >> 5));

	mcu->dm[rd] = mcu->dm[rd] ^ mcu->dm[rr];
	mcu->pc += 2;

	UPDSR(mcu, SR_ZERO, !mcu->dm[rd]);
	UPDSR(mcu, SR_NEG, mcu->dm[rd] & 0x80);
	UPDSR(mcu, SR_TCOF, 0);
	UPDSR(mcu, SR_SIGN, (mcu->dm[rd] & 0x80) ^ 0);
}

static void exec_in_out(struct MSIM_AVR *mcu, uint32_t inst,
                        uint8_t reg, uint8_t io_loc)
{
	switch (inst & 0xF800) {
	/* IN - Load an I/O Location to Register */
	case 0xB000:
		mcu->dm[reg] = mcu->dm[io_loc + mcu->sfr_off];
		mcu->read_io[0] = io_loc + mcu->sfr_off;
		break;
	/* OUT – Store Register to I/O Location */
	case 0xB800:
		WRITE_DS(io_loc+SFR, DM(reg));
		break;
	}
	mcu->pc += 2;
}

static void exec_cpi(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* CPI – Compare with Immediate */
	uint8_t rd, rd_addr, c;
	int r, buf;

	rd_addr = (uint8_t)(((inst & 0xF0) >> 4) + 16);
	c = (uint8_t)((inst & 0x0F) | ((inst & 0x0F00) >> 4));

	rd = mcu->dm[rd_addr];
	r = mcu->dm[rd_addr] - c;
	buf = (~rd & c) | (c & r) | (r & ~rd);
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, (buf >> 7) & 0x01);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, (((rd & ~c & ~r) | (~rd & c & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_HCARRY, (buf >> 3) & 1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_cpc(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* CPC – Compare with Carry */
	uint8_t rd, rd_addr;
	uint8_t rr, rr_addr;
	int r, buf;

	rd_addr = (uint8_t)((inst & 0x01F0) >> 4);
	rr_addr = (uint8_t)((inst & 0x0F) | ((inst & 0x0200) >> 5));
	rd = DM(rd_addr);
	rr = DM(rr_addr);
	r = DM(rd_addr)-DM(rr_addr)-SR(mcu, SR_CARRY);
	mcu->pc += 2;
	buf = (~rd & rr) | (rr & r) | (r & ~rd);

	UPDSR(mcu, SR_CARRY, (buf >> 7) & 1);
	UPDSR(mcu, SR_HCARRY, (buf >> 3) & 1);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, (((rd & ~rr & ~r) | (~rd & rr & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	if (r) {
		UPDSR(mcu, SR_ZERO, 0);
	}
}

static void exec_cp(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* CP - Compare */
	uint8_t rd, rd_addr;
	uint8_t rr, rr_addr;
	int buf, r;

	rd_addr = (uint8_t)((inst & 0x01F0) >> 4);
	rr_addr = (uint8_t)((inst & 0x0F) | ((inst & 0x0200) >> 5));
	rd = mcu->dm[rd_addr];
	rr = mcu->dm[rr_addr];
	r = mcu->dm[rd_addr] - mcu->dm[rr_addr];
	mcu->pc += 2;
	buf = (~rd & rr) | (rr & r) | (r & ~rd);

	UPDSR(mcu, SR_CARRY, (buf >> 7) & 1);
	UPDSR(mcu, SR_HCARRY, (buf >> 3) & 1);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, (((rd & ~rr & ~r) | (~rd & rr & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_ldi(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LDI – Load Immediate */
	uint8_t rd_off, c;

	rd_off = (uint8_t)((inst & 0xF0) >> 4);
	c = (uint8_t)((inst & 0x0F) | ((inst & 0x0F00) >> 4));
	mcu->dm[0x10 + rd_off] = c;
	mcu->pc += 2;
}

static void exec_rjmp(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* RJMP - Relative Jump */
	int c;

	SKIP_CYCLES(mcu, 1, 1);
	c = inst & 0x0FFF;
	if (c >= 2048) {
		c -= 4096;
	}
	mcu->pc = (uint32_t)((int32_t)mcu->pc + ((c+1)*2));
}

static void exec_brne(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRNE – Branch if Not Equal */
	uint8_t cond;

	cond = SR(mcu, SR_ZERO);
	SKIP_CYCLES(mcu, !cond, 1);
	if (!cond) {
		int c;
		/* Z == 0, i.e. Rd != Rr */
		c = (int) ((int) (inst << 6)) >> 9;
		mcu->pc = (uint32_t) (((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		/* Z == 1, i.e. Rd == Rr */
		mcu->pc += 2;
	}
}

static void exec_st_x(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ST – Store Indirect From Register to Data Space using Index X */
	uint8_t regr, *x_low, *x_high;

	x_low = &mcu->dm[26];
	x_high = &mcu->dm[27];
	regr = (uint8_t)((inst & 0x01F0) >> 4);
	exec_st(mcu, inst, x_low, x_high, regr);
}

static void exec_st_y(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ST – Store Indirect From Register to Data Space using Index Y */
	uint8_t regr, *y_low, *y_high;

	y_low = &mcu->dm[28];
	y_high = &mcu->dm[29];
	regr = (uint8_t)((inst & 0x01F0) >> 4);
	exec_st(mcu, inst, y_low, y_high, regr);
}

static void exec_st_z(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ST – Store Indirect From Register to Data Space using Index Z */
	uint8_t regr, *z_low, *z_high;

	z_low = &mcu->dm[30];
	z_high = &mcu->dm[31];
	regr = (uint8_t)((inst & 0x01F0) >> 4);
	exec_st(mcu, inst, z_low, z_high, regr);
}

static void exec_st(struct MSIM_AVR *mcu, uint32_t inst,
                    uint8_t *addr_low, uint8_t *addr_high,
                    uint8_t regr)
{
	/* ST – Store Indirect From Register to Data Space
	 *	using Index X, Y or Z */
	uint32_t addr = (uint32_t)(*addr_low | (*addr_high << 8));
	uint8_t r = (uint8_t)((inst & 0x01F0) >> 4);

	switch (inst & 0x03) {
	case 0x00:	/*	(X) ← Rr		X: Unchanged */
		if (!mcu->xmega && !mcu->reduced_core) {
			SKIP_CYCLES(mcu, 1, 1);
		}
		WRITE_DS(addr, DM(r));
		break;
	case 0x01:	/*	(X) ← Rr, X ← X+1	X: Post incremented */
		if (!mcu->xmega && !mcu->reduced_core) {
			SKIP_CYCLES(mcu, 1, 1);
		}
		WRITE_DS(addr, DM(r));
		addr++;
		*addr_low = (uint8_t) (addr & 0xFF);
		*addr_high = (uint8_t) (addr >> 8);
		break;
	case 0x02:	/*	X ← X-1, (X) ← Rr	X: Pre decremented */
		SKIP_CYCLES(mcu, 1, 1);
		addr--;
		*addr_low = (uint8_t) (addr & 0xFF);
		*addr_high = (uint8_t) (addr >> 8);
		WRITE_DS(addr, DM(r));
		break;
	}
	mcu->pc += 2;
}

static void exec_st_ydisp(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ST (STD) – Store Indirect using Index Y */
	uint32_t addr;
	uint8_t regr, *y_low, *y_high, disp;

	SKIP_CYCLES(mcu, 1, 1);
	y_low = &mcu->dm[28];
	y_high = &mcu->dm[29];
	addr = (uint32_t) *y_low | (uint32_t) (*y_high << 8);
	regr = (uint8_t)((inst & 0x01F0) >> 4);
	disp = (uint8_t)((inst & 0x07) |
	                       ((inst & 0x0C00) >> 7) |
	                       ((inst & 0x2000) >> 8));

	WRITE_DS(addr+disp, DM(regr));
	mcu->pc += 2;
}

static void exec_st_zdisp(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ST (STD) – Store Indirect using Index Z */
	uint32_t addr;
	uint8_t regr, *z_low, *z_high, disp;

	SKIP_CYCLES(mcu, 1, 1);
	z_low = &mcu->dm[30];
	z_high = &mcu->dm[31];
	addr = (uint32_t) *z_low | (uint32_t) (*z_high << 8);
	regr = (uint8_t)((inst & 0x01F0) >> 4);
	disp = (uint8_t)((inst & 0x07) |
	                       ((inst & 0x0C00) >> 7) |
	                       ((inst & 0x2000) >> 8));

	WRITE_DS(addr+disp, DM(regr));
	mcu->pc += 2;
}

static void exec_rcall(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* RCALL – Relative Call to Subroutine */
	int c;
	uint64_t pc;

	if (!mcu->reduced_core && mcu->xmega) {
		SKIP_CYCLES(mcu, 1, mcu->pc_bits > 16 ? 2 : 1);
	} else if (!mcu->reduced_core && !mcu->xmega) {
		SKIP_CYCLES(mcu, 1, mcu->pc_bits > 16 ? 3 : 2);
	} else {
		SKIP_CYCLES(mcu, 1, 3);
	}

	pc = mcu->pc+2;
	c = inst&0x0FFF;
	if (c >= 2048) {
		c -= 4096;
	}

	MSIM_AVR_StackPush(mcu, (uint8_t)(pc&0xFF));
	MSIM_AVR_StackPush(mcu, (uint8_t)((pc>>8)&0xFF));
	if (mcu->pc_bits > 16) {		/* for 22-bit PC or above */
		MSIM_AVR_StackPush(mcu, (uint8_t)((pc>>16)&0xFF));
	}
	mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
}

static void exec_sts(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* STS – Store Direct to Data Space */
	uint32_t addr;
	uint8_t rr, addr_msb, addr_lsb;

	SKIP_CYCLES(mcu, 1, 1);

	addr_lsb = mcu->pm[mcu->pc + 2];
	addr_msb = mcu->pm[mcu->pc + 3];
	addr = (uint32_t) (((addr_msb<<8)&0xFF00) | (addr_lsb&0xFF));
	rr = (uint8_t)((inst & 0x01F0) >> 4);
	WRITE_DS(addr, DM(rr));
	mcu->pc += 4;
}

static void exec_ret(struct MSIM_AVR *mcu)
{
	/* RET – Return from Subroutine */
	uint8_t ae, ah, al;

	SKIP_CYCLES(mcu, 1, mcu->pc_bits > 16 ? 4 : 3);

	ae = ah = al = 0;
	if (mcu->pc_bits > 16) {
		ae = MSIM_AVR_StackPop(mcu);
	}
	ah = MSIM_AVR_StackPop(mcu);
	al = MSIM_AVR_StackPop(mcu);
	mcu->pc = (uint32_t)((ae<<16) | (ah<<8) | al);
}

static void exec_ori_sbr(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ORI – Logical OR with Immediate */
	/* SBR – Set Bits in Register */
	uint8_t rd_addr, c, r;

	rd_addr = (uint8_t)(((inst & 0xF0) >> 4) + 16);
	c = (uint8_t)((inst & 0x0F) | ((inst & 0x0F00) >> 4));
	r = mcu->dm[rd_addr] |= c;
	mcu->pc += 2;

	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, 0);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_sbi_cbi(struct MSIM_AVR *mcu, uint32_t inst,
                         uint8_t set_bit)
{
	/* SBI – Set Bit in I/O Register
	 * CBI – Clear Bit in I/O Register */
	uint8_t reg, b;

	if (!mcu->reduced_core && !mcu->xmega) {
		SKIP_CYCLES(mcu, 1, 1);
	}
	reg = (uint8_t)((inst & 0x00F8) >> 3);
	b = inst & 0x07;
	if (set_bit) {
		WRITE_DS(reg+SFR, DM(reg+SFR) | (uint8_t)(1<<b));
	} else {
		WRITE_DS(reg+SFR, DM(reg+SFR) & (uint8_t)(~(1<<b)));
	}
	mcu->pc += 2;
}

static void exec_sbis_sbic(struct MSIM_AVR *mcu, uint32_t inst,
                           uint8_t set_bit)
{
	/* SBIS – Skip if Bit in I/O Register is Set
	 * SBIC – Skip if Bit in I/O Register is Cleared */
	uint8_t reg, b, pc_delta;
	uint8_t msb, lsb;
	uint32_t ni;
	int is32;

	reg = (uint8_t)(((inst&0x00F8)>>3)+0x20);
	b = inst & 0x07;
	pc_delta = 2;

	lsb = mcu->pm[mcu->pc+2];
	msb = mcu->pm[mcu->pc+3];
	ni = (uint32_t) (lsb | (msb << 8));
	is32 = MSIM_AVR_Is32(ni);

	if (set_bit && (mcu->dm[reg] & (1 << b))) {
		if (mcu->xmega) {
			SKIP_CYCLES(mcu, 1, is32 ? 3 : 2);
		} else {
			SKIP_CYCLES(mcu, 1, is32 ? 2 : 1);
		}
		pc_delta = is32 ? 6 : 4;
	} else if (!set_bit && (mcu->dm[reg] ^ (1 << b))) {
		if (mcu->xmega) {
			SKIP_CYCLES(mcu, 1, is32 ? 3 : 2);
		} else {
			SKIP_CYCLES(mcu, 1, is32 ? 2 : 1);
		}
		pc_delta = is32 ? 6 : 4;
	} else {
		if (mcu->xmega) {
			SKIP_CYCLES(mcu, 1, 1);
		}
	}

	mcu->read_io[0] = reg;
	mcu->pc += pc_delta;
}

static void exec_push_pop(struct MSIM_AVR *mcu, uint32_t inst,
                          uint8_t push)
{
	/*
	 * PUSH – Push Register on Stack
	 * POP – Pop Register from Stack
	 */
	uint8_t reg;

	reg = (inst >> 4) & 0x1F;
	if (push) {
		if (!mcu->xmega) {
			SKIP_CYCLES(mcu, 1, 1);
		}
		MSIM_AVR_StackPush(mcu, mcu->dm[reg]);
	} else {
		SKIP_CYCLES(mcu, 1, 1);
		mcu->dm[reg] = MSIM_AVR_StackPop(mcu);
	}

	mcu->pc += 2;
}

static void exec_movw(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* MOVW – Copy Register Word */
	uint8_t regd, regr;

	regr = (uint8_t)((inst&0x0F)<<1);
	regd = (uint8_t)(((inst>>4)&0x0F)<<1);
	mcu->dm[regd+1] = mcu->dm[regr+1];
	mcu->dm[regd] = mcu->dm[regr];
	mcu->pc += 2;
}

static void exec_mov(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* MOV - Copy register */
	uint8_t rd, rr;

	rr = (uint8_t)(((inst & 0x200) >> 5) | (inst & 0x0F));
	rd = (uint8_t)((inst & 0x1F0) >> 4);
	mcu->dm[rd] = mcu->dm[rr];
	mcu->pc += 2;
}

static void exec_ld_x(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LD – Load Indirect from Data Space to Register using Index X */
	uint8_t regd, *x_low, *x_high;

	x_low = &mcu->dm[26];
	x_high = &mcu->dm[27];
	regd = (uint8_t)((inst & 0x01F0) >> 4);
	exec_ld(mcu, inst, x_low, x_high, regd);
}

static void exec_ld_y(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LD – Load Indirect from Data Space to Register using Index Y */
	uint8_t regd, *y_low, *y_high;

	y_low = &mcu->dm[28];
	y_high = &mcu->dm[29];
	regd = (uint8_t)((inst & 0x01F0) >> 4);
	exec_ld(mcu, inst, y_low, y_high, regd);
}

static void exec_ld_z(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LD – Load Indirect from Data Space to Register using Index Z */
	uint8_t regd, *z_low, *z_high;

	z_low = &mcu->dm[30];
	z_high = &mcu->dm[31];
	regd = (uint8_t)((inst & 0x01F0) >> 4);
	exec_ld(mcu, inst, z_low, z_high, regd);
}

static void exec_ld(struct MSIM_AVR *mcu, uint32_t inst,
                    uint8_t *addr_low, uint8_t *addr_high,
                    uint8_t regd)
{
	/* LD – Load Indirect from Data Space to Register
	 *	using Index X, Y or Z */
	uint32_t addr = (uint32_t) (*addr_low | (*addr_high << 8));

	switch (inst & 0x03) {
	case 0x00:	/*	Rd ← (X)		X: Unchanged */
		if ((mcu->xmega) && (addr <= mcu->ramend) &&
		                (addr >= mcu->ramstart)) {
			SKIP_CYCLES(mcu, 1, 1);
		}
		mcu->dm[regd] = mcu->dm[addr];
		mcu->read_io[0] = addr;
		break;
	case 0x01:	/*	Rd ← (X), X ← X+1	X: Post incremented */
		if (!mcu->xmega) {
			SKIP_CYCLES(mcu, 1, 1);
		} else if (addr <= mcu->ramend && addr >= mcu->ramstart) {
			SKIP_CYCLES(mcu, 1, 1);
		} else {
			/* Do not skip any cycles */;
		}
		mcu->dm[regd] = mcu->dm[addr];
		mcu->read_io[0] = addr;
		addr++;
		*addr_low = (uint8_t) (addr & 0xFF);
		*addr_high = (uint8_t) (addr >> 8);
		break;
	case 0x02:	/*	X ← X-1, Rd ← (X)	X: Pre decremented */
		if (!mcu->xmega) {
			SKIP_CYCLES(mcu, 1, 2);
		} else if (addr <= mcu->ramend && addr >= mcu->ramstart) {
			SKIP_CYCLES(mcu, 1, 2);
		} else if (addr > mcu->ramend && addr < mcu->ramstart) {
			SKIP_CYCLES(mcu, 1, 1);
		} else {
			/* Do not skip any cycles */;
		}
		addr--;
		*addr_low = (uint8_t) (addr & 0xFF);
		*addr_high = (uint8_t) (addr >> 8);
		mcu->dm[regd] = mcu->dm[addr];
		mcu->read_io[0] = addr;
		break;
	}
	mcu->pc += 2;
}

static void exec_ld_ydisp(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LD – Load Indirect from Data Space to Register using Index Y */
	uint32_t addr;
	uint8_t regd, *y_low, *y_high, disp;

	y_low = &mcu->dm[28];
	y_high = &mcu->dm[29];
	addr = (uint32_t) *y_low | (uint32_t) (*y_high << 8);

	if (!mcu->xmega) {
		SKIP_CYCLES(mcu, 1, 1);
	} else if (addr <= mcu->ramend && addr >= mcu->ramstart) {
		SKIP_CYCLES(mcu, 1, 2);
	} else if (addr > mcu->ramend && addr < mcu->ramstart) {
		SKIP_CYCLES(mcu, 1, 1);
	} else {
		/* Do not skip any cycles */;
	}
	regd = (uint8_t)((inst & 0x01F0) >> 4);
	disp = (uint8_t)((inst & 0x07) |
	                       ((inst & 0x0C00) >> 7) |
	                       ((inst & 0x2000) >> 8));
	mcu->dm[regd] = mcu->dm[addr + disp];
	mcu->read_io[0] = addr + disp;
	mcu->pc += 2;
}

static void exec_ld_zdisp(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LD – Load Indirect from Data Space to Register using Index Z */
	uint32_t addr;
	uint8_t regd, *z_low, *z_high, disp;

	z_low = &mcu->dm[30];
	z_high = &mcu->dm[31];
	addr = (uint32_t) *z_low | (uint32_t) (*z_high << 8);

	if (!mcu->xmega) {
		SKIP_CYCLES(mcu, 1, 1);
	} else if (addr <= mcu->ramend && addr >= mcu->ramstart) {
		SKIP_CYCLES(mcu, 1, 2);
	} else if (addr > mcu->ramend && addr < mcu->ramstart) {
		SKIP_CYCLES(mcu, 1, 1);
	} else {
		/* Do not skip any cycles */;
	}

	regd = (uint8_t)((inst & 0x01F0) >> 4);
	disp = (uint8_t)((inst & 0x07) |
	                       ((inst & 0x0C00) >> 7) |
	                       ((inst & 0x2000) >> 8));

	mcu->dm[regd] = mcu->dm[addr + disp];
	mcu->read_io[0] = addr + disp;
	mcu->pc += 2;
}

static void exec_sbci(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SBCI – Subtract Immediate with Carry */
	uint8_t rd, rd_addr, c, r;
	int buf;

	rd_addr = (uint8_t)(((inst & 0xF0) >> 4) + 16);
	c = (uint8_t)(((inst & 0xF00) >> 4) | (inst & 0x0F));

	rd = mcu->dm[rd_addr];
	r = (uint8_t)(mcu->dm[rd_addr] - c -
	                    SR(mcu, SR_CARRY));
	mcu->dm[rd_addr] = r;
	mcu->pc += 2;

	buf = (~rd & c) | (c & r) | (r & ~rd);
	UPDSR(mcu, SR_CARRY, (buf >> 7) & 1);
	UPDSR(mcu, SR_HCARRY, (buf >> 3) & 1);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, (((rd & ~c & ~r) | (~rd & c & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_brlt(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRLT – Branch if Less Than (Signed) */
	uint8_t cond = SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF);
	int c = (inst>>3)&0x7F;

	if (c > 63) {
		c -= 128;
	}
	SKIP_CYCLES(mcu, cond, 1);
	if (cond) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brge(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRGE – Branch if Greater or Equal (Signed) */
	uint8_t cond = SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF);
	int c = (inst>>3)&0x7F;

	if (c > 63) {
		c -= 128;
	}
	SKIP_CYCLES(mcu, !cond, 1);
	if (!cond) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_andi_cbr(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ANDI – Logical AND with Immediate */
	uint8_t rd_addr;
	uint8_t c, r;

	rd_addr = (uint8_t)(((inst >> 4) & 0x0F) + 16);
	c = (uint8_t)(((inst >> 4) & 0xF0) | (inst & 0x0F));
	r = mcu->dm[rd_addr] = mcu->dm[rd_addr] & c;
	mcu->pc += 2;

	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, 0);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_and(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* AND - Logical AND */
	uint8_t rd_addr, rr_addr, r;

	rd_addr = (uint8_t)((inst & 0x1F0) >> 4);
	rr_addr = (uint8_t)(((inst & 0x200) >> 5) | (inst & 0x0F));
	r = mcu->dm[rd_addr] = mcu->dm[rd_addr] & mcu->dm[rr_addr];
	mcu->pc += 2;

	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, 0);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_sbiw(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SBIW – Subtract Immediate from Word */
	const uint8_t regs[] = { 24, 26, 28, 30 };
	uint8_t rdh, rdl;
	int16_t c, r, buf;

	SKIP_CYCLES(mcu, 1, 1);

	rdl= regs[((inst>>4) & 0x03)];
	rdh= (uint8_t)(rdl + 1);
	c = (int16_t)(((inst>>2)&0x30U) | (inst&0x0FU));
	buf = (int16_t)((DM(rdh)<<8) | (DM(rdl)));
	r = buf;
	r = (int16_t)(r-c);
	buf = (int16_t)(r & ((int16_t)~buf));

	UPDSR(mcu, SR_CARRY, (buf>>15) & 1);
	UPDSR(mcu, SR_NEG, (uint8_t)((r>>15)&1));
	UPDSR(mcu, SR_TCOF, ((DM(rdh)>>7)&1) & (~((r>>15)&1)));
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	DM(rdh) = (uint8_t)((r>>8)&0xFF);
	DM(rdl) = (uint8_t)(r&0xFF);
	mcu->pc += 2;
}

static void exec_brcc_brsh(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRCC – Branch if Carry Cleared */
	uint8_t cond = SR(mcu, SR_CARRY);
	int c = (inst>>3)&0x7F;

	if (c > 63) {
		c -= 128;
	}
	SKIP_CYCLES(mcu, !cond, 1);
	if (!cond) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brcs_brlo(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRCS - Branch if Carry Set */
	uint8_t cond = SR(mcu, SR_CARRY);
	int c = (inst>>3)&0x7F;

	if (c > 63) {
		c -= 128;
	}
	SKIP_CYCLES(mcu, cond, 1);
	if (cond) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_sub(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SUB - Subtract Without Carry */
	uint8_t rda, rra, rd, rr, r;
	int buf;

	rda = (uint8_t)((inst>>4)&0x1F);
	rra = (uint8_t)(((inst>>5)&0x10)|(inst&0xF));
	rd = mcu->dm[rda];
	rr = mcu->dm[rra];
	r = (uint8_t)(mcu->dm[rda] - mcu->dm[rra]);
	mcu->pc += 2;
	buf = (~rd & rr) | (rr & r) | (r & ~rd);

	UPDSR(mcu, SR_CARRY, (buf>>7)&1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r>>7)&1);
	UPDSR(mcu, SR_TCOF, (((rd & ~rr & ~r) | (~rd & rr & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_HCARRY, (buf>>3)&1);
}

static void exec_subi(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SUBI - Subtract Immediate */
	uint8_t rd, c, r;
	uint8_t rd_addr;
	int buf;

	rd_addr = (uint8_t)((inst & 0xF0) >> 4);
	c = (uint8_t)(((inst & 0xF00) >> 4) | (inst & 0xF));
	buf = rd = mcu->dm[rd_addr+16];
	buf -= c;
	r = mcu->dm[rd_addr+16] = (uint8_t) buf;
	mcu->pc += 2;
	buf = (~rd & c) | (c & r) | (r & ~rd);

	UPDSR(mcu, SR_CARRY, (buf >> 7) & 1);
	UPDSR(mcu, SR_HCARRY, (buf >> 3) & 1);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, (((rd & ~c & ~r) | (~rd & c & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_sbc(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SBC – Subtract with Carry */
	uint8_t rda, rra, rd, rr, r;
	int buf;

	rda = (uint8_t)((inst>>4)&0x1F);
	rra = (uint8_t)(((inst>>5)&0x10)|(inst&0xF));
	rd = mcu->dm[rda];
	rr = mcu->dm[rra];
	r = (uint8_t)(DM(rda)-DM(rra)-SR(mcu, SR_CARRY));
	mcu->dm[rda] = r;
	mcu->pc += 2;
	buf = (~rd & rr) | (rr & r) | (r & ~rd);

	UPDSR(mcu, SR_CARRY, (buf>>7)&1);
	if (r) {
		UPDSR(mcu, SR_ZERO, 0);
	}
	UPDSR(mcu, SR_NEG, (r>>7)&1);
	UPDSR(mcu, SR_TCOF, (((rd & ~rr & ~r) | (~rd & rr & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_HCARRY, (buf>>3)&1);
}

static void exec_adiw(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ADIW – Add Immediate to Word */
	const uint8_t regs[] = { 24, 26, 28, 30 };
	uint8_t rdh_addr, rdl_addr;
	uint32_t c, r, rd;

	SKIP_CYCLES(mcu, 1, 1);
	rdl_addr = regs[(inst >> 4) & 3];
	rdh_addr = (uint8_t)(rdl_addr + 1);
	c = ((inst >> 2) & 0x30) | (inst & 0x0F);
	rd = (uint32_t)((mcu->dm[rdh_addr] << 8) | (mcu->dm[rdl_addr]));
	r = rd + c;

	UPDSR(mcu, SR_CARRY, ((~r & rd) >> 15) & 1);
	UPDSR(mcu, SR_NEG, (uint8_t)((r>>15)&1));
	UPDSR(mcu, SR_TCOF, ((r & ~rd) >> 15) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);

	mcu->dm[rdh_addr] = (r >> 8) & 0xFF;
	mcu->dm[rdl_addr] = r & 0xFF;
	mcu->pc += 2;
}

static void exec_adc_rol(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ADC - Add with Carry */
	uint8_t rd_addr, rr_addr;
	uint8_t rd, rr, r;
	int buf;

	rd_addr = (uint8_t)((inst & 0x1F0) >> 4);
	rr_addr = (uint8_t)(((inst & 0x200) >> 5) | (inst & 0x0F));
	rd = mcu->dm[rd_addr];
	rr = mcu->dm[rr_addr];
	r = (uint8_t)(rd + rr + SR(mcu, SR_CARRY));
	mcu->dm[rd_addr] = r;
	buf = (rd & rr) | (rr & ~r) | (~r & rd);

	UPDSR(mcu, SR_CARRY, (buf >> 7) & 1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, (((rd & rr & ~r) | (~rd & ~rr & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_HCARRY, (buf >> 3) & 1);
	mcu->pc += 2;
}

static void exec_add_lsl(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ADD - Add without Carry */
	/* LSL - Logical Shift Left */
	uint8_t rd_addr, rr_addr;
	uint8_t rd, rr, r;
	int buf;

	rd_addr = (uint8_t)((inst & 0x1F0) >> 4);
	rr_addr = (uint8_t)(((inst & 0x200) >> 5) | (inst & 0x0F));
	rd = mcu->dm[rd_addr];
	rr = mcu->dm[rr_addr];
	mcu->dm[rd_addr] = r = (uint8_t)(rd + rr);
	buf = (rd & rr) | (rr & ~r) | (~r & rd);

	UPDSR(mcu, SR_CARRY, (buf >> 7) & 1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, (((rd & rr & ~r) | (~rd & ~rr & r)) >> 7) & 1);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_HCARRY, (buf >> 3) & 1);
	mcu->pc += 2;
}

static void exec_asr(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ASR – Arithmetic Shift Right */
	uint8_t rd_addr, rd, r;
	uint8_t msb_orig, lsb_orig;

	rd_addr = (uint8_t)((inst & 0x1F0) >> 4);
	rd = mcu->dm[rd_addr];
	msb_orig = (rd >> 7) & 1;
	lsb_orig = rd & 1;
	r = rd >> 1;
	if (msb_orig) {
		r |= 1<<7;
	} else {
		r &= (uint8_t)(~(1<<7));
	}
	mcu->dm[rd_addr] = r;

	UPDSR(mcu, SR_CARRY, lsb_orig);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, msb_orig);
	UPDSR(mcu, SR_TCOF, SR(mcu, SR_NEG) ^ SR(mcu, SR_CARRY));
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	mcu->pc += 2;
}

static void exec_bclr(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BCLR – Bit Clear in SREG */
	uint8_t bit;

	bit = (uint8_t)((inst & 0x70) >> 4);
	*mcu->sreg &= (uint8_t)(~((1<<bit)&0xFF));
	mcu->pc += 2;
}

static void exec_bld(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BLD - Bit Load from the T Flag in SREG to a Bit in Register */
	uint8_t bit, rd_addr, t;

	rd_addr = (uint8_t)((inst & 0x1F0) >> 4);
	bit = inst & 0x07;
	t = SR(mcu, SR_TBIT);
	if (t) {
		mcu->dm[rd_addr] |= (uint8_t)((1<<bit)&0xFF);
	} else {
		mcu->dm[rd_addr] &= (uint8_t)(~((1<<bit)&0xFF));
	}
	mcu->pc += 2;
}

static void exec_brbc(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRBC – Branch if Bit in SREG is Cleared */
	uint8_t cond = (*mcu->sreg >> (inst & 0x07))&1;
	int c = (inst>>3)&0x7F;

	if (c > 63) {
		c -= 128;
	}
	SKIP_CYCLES(mcu, !cond, 1);
	if (!cond) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brbs(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRBS – Branch if Bit in SREG is Set */
	uint8_t cond = (*mcu->sreg >> (inst & 0x07))&1;
	int c = (inst >> 3)&0x7F;

	if (c > 63) {
		c -= 128;
	}
	SKIP_CYCLES(mcu, cond, 1);
	if (cond) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_break(struct MSIM_AVR *mcu)
{
	/* BREAK – Break (the AVR CPU is set in the Stopped Mode). */
	mcu->state = AVR_STOPPED;
	mcu->read_from_mpm = 1;
}

static void exec_breq(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BREQ – Branch if Equal */
	uint8_t f;
	int c;

	f = SR(mcu, SR_ZERO);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, f, 1);
	if (f) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brhc(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRHC – Branch if Half Carry Flag is Cleared */
	uint8_t f;
	int c;

	f = SR(mcu, SR_HCARRY);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, !f, 1);
	if (!f) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brhs(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRHS – Branch if Half Carry Flag is Set */
	uint8_t f;
	int c;

	f = SR(mcu, SR_HCARRY);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, f, 1);
	if (f) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brid(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRID – Branch if Global Interrupt is Disabled */
	uint8_t f;
	int c;

	f = SR(mcu, SR_GLOBINT);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, !f, 1);
	if (!f) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brie(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRIE – Branch if Global Interrupt is Enabled */
	uint8_t f;
	int c;

	f = SR(mcu, SR_GLOBINT);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, f, 1);
	if (f) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brmi(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRMI – Branch if Minus */
	uint8_t f;
	int c;

	f = SR(mcu, SR_NEG);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, f, 1);
	if (f) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brpl(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRPL – Branch if Plus */
	uint8_t f;
	int c;

	f = SR(mcu, SR_NEG);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, !f, 1);
	if (!f) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brtc(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRTC – Branch if the T Flag is Cleared */
	uint8_t f;
	int c;

	f = SR(mcu, SR_TBIT);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, !f, 1);
	if (!f) {
		mcu->pc = (uint32_t)(((int32_t) mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brts(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRTS – Branch if the T Flag is Set */
	uint8_t f;
	int c;

	f = SR(mcu, SR_TBIT);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, f, 1);
	if (f) {
		mcu->pc = (uint32_t)(((int32_t)mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brvc(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRVC – Branch if Overflow Cleared */
	uint8_t f;
	int c;

	f = SR(mcu, SR_TCOF);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, !f, 1);
	if (!f) {
		mcu->pc = (uint32_t)(((int32_t)mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_brvs(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BRVS – Branch if Overflow Set */
	uint8_t f;
	int c;

	f = SR(mcu, SR_TCOF);
	c = (inst >> 3) & 0x7F;
	c = c > 63 ? c-128 : c;
	SKIP_CYCLES(mcu, f, 1);
	if (f) {
		mcu->pc = (uint32_t)(((int32_t)mcu->pc) + (c + 1) * 2);
	} else {
		mcu->pc += 2;
	}
}

static void exec_bset(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BSET – Bit Set in SREG */
	uint32_t bit;

	bit = (inst & 0x70) >> 4;
	*mcu->sreg |= (uint8_t)((1<<bit)&0xFF);
	mcu->pc += 2;
}

static void exec_bst(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* BST – Bit Store from Bit in Register to T Flag in SREG */
	uint8_t b, rd_addr;

	b = inst & 0x07;
	rd_addr = (inst >> 4) & 0x1F;
	UPDSR(mcu, SR_TBIT, (mcu->dm[rd_addr]>>b)&1);
	mcu->pc += 2;
}

static void exec_call(struct MSIM_AVR *mcu, uint32_t inst_msb)
{
	/*
	 * CALL – Long Call to a Subroutine
	 * NOTE: This is a multi-cycle instruction.
	 */
	uint8_t lsb, msb;
	uint32_t inst_lsb;
	uint64_t pc, c;

	if (!mcu->in_mcinst) {
		/* It is the first cycle of multi-cycle instruction */
		mcu->in_mcinst = 1;
		if (!mcu->xmega) {
			mcu->ic_left = mcu->pc_bits > 16 ? 4 : 3;
		} else {
			mcu->ic_left = mcu->pc_bits > 16 ? 3 : 2;
		}

		return;
	} else if (mcu->ic_left) {
		/* Skip intermediate cycles */
		if (--mcu->ic_left) {
			return;
		}
	}
	mcu->in_mcinst = 0;

	/* prepare the whole 32-bit instruction */
	lsb = mcu->pm[mcu->pc+2];
	msb = mcu->pm[mcu->pc+3];
	inst_lsb = (uint32_t) (lsb|(msb<<8));

	pc = mcu->pc+4;
	c = (uint64_t)((inst_lsb&0xFFFF) |
	                    ((((inst_msb>>3)&0x3E) | (inst_msb&1)) << 16));

	MSIM_AVR_StackPush(mcu, (uint8_t)(pc&0xFF));
	MSIM_AVR_StackPush(mcu, (uint8_t)((pc>>8)&0xFF));
	if (mcu->pc_bits > 16) {		/* for 22-bit PC or above */
		MSIM_AVR_StackPush(mcu, (uint8_t)((pc>>16)&0xFF));
	}
	mcu->pc = (uint32_t)(c << 1); // address is in words, not bytes
}

static void exec_clc(struct MSIM_AVR *mcu)
{
	/* CLC – Clear Carry Flag */
	UPDSR(mcu, SR_CARRY, 0);
	mcu->pc += 2;
}

static void exec_sec(struct MSIM_AVR *mcu)
{
	/* SEC – Set Carry Flag */
	UPDSR(mcu, SR_CARRY, 1);
	mcu->pc += 2;
}

static void exec_clh(struct MSIM_AVR *mcu)
{
	/* CLH – Clear Half Carry Flag */
	UPDSR(mcu, SR_HCARRY, 0);
	mcu->pc += 2;
}

static void exec_seh(struct MSIM_AVR *mcu)
{
	/* SEH – Set Half Carry Flag */
	UPDSR(mcu, SR_HCARRY, 1);
	mcu->pc += 2;
}

static void exec_cli(struct MSIM_AVR *mcu)
{
	/* CLI - Clear Global Interrupt Flag */
	UPDSR(mcu, SR_GLOBINT, 0);
	mcu->pc += 2;
}

static void exec_sei(struct MSIM_AVR *mcu)
{
	/* SEI – Set Global Interrupt Flag */
	UPDSR(mcu, SR_GLOBINT, 1);
	mcu->pc += 2;
}

static void exec_cln(struct MSIM_AVR *mcu)
{
	/* CLN – Clear Negative Flag */
	UPDSR(mcu, SR_NEG, 0);
	mcu->pc += 2;
}

static void exec_sen(struct MSIM_AVR *mcu)
{
	/* SEN – Set Negative Flag */
	UPDSR(mcu, SR_NEG, 1);
	mcu->pc += 2;
}

static void exec_cls(struct MSIM_AVR *mcu)
{
	/* CLS – Clear Signed Flag */
	UPDSR(mcu, SR_SIGN, 0);
	mcu->pc += 2;
}

static void exec_ses(struct MSIM_AVR *mcu)
{
	/* SES – Set Signed Flag */
	UPDSR(mcu, SR_SIGN, 1);
	mcu->pc += 2;
}

static void exec_clt(struct MSIM_AVR *mcu)
{
	/* CLT – Clear T Flag */
	UPDSR(mcu, SR_TBIT, 0);
	mcu->pc += 2;
}

static void exec_set(struct MSIM_AVR *mcu)
{
	/* SET – Set T Flag */
	UPDSR(mcu, SR_TBIT, 1);
	mcu->pc += 2;
}

static void exec_clv(struct MSIM_AVR *mcu)
{
	/* CLV – Clear Overflow Flag */
	UPDSR(mcu, SR_TCOF, 0);
	mcu->pc += 2;
}

static void exec_sev(struct MSIM_AVR *mcu)
{
	/* SEV – Set Overflow Flag */
	UPDSR(mcu, SR_TCOF, 1);
	mcu->pc += 2;
}

static void exec_clz(struct MSIM_AVR *mcu)
{
	/* CLZ – Clear Zero Flag */
	UPDSR(mcu, SR_ZERO, 0);
	mcu->pc += 2;
}

static void exec_sez(struct MSIM_AVR *mcu)
{
	/* SEZ – Set Zero Flag */
	UPDSR(mcu, SR_ZERO, 1);
	mcu->pc += 2;
}

static void exec_com(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* COM – One’s Complement */
	uint8_t rd_addr, r;

	rd_addr = (inst >> 4) & 0x1F;
	r = mcu->dm[rd_addr] = (uint8_t)(~mcu->dm[rd_addr]);
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, 1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, 0);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
}

static void exec_cpse(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* CPSE – Compare Skip if Equal */
	uint8_t rd_addr, rr_addr, f;
	int is32;

	rd_addr = (inst >> 4) & 0x1F;
	rr_addr = (uint8_t)(((inst >> 5) & 0x10) | (inst & 0x0F));
	is32 = MSIM_AVR_Is32(mcu->pm[mcu->pc+2]);
	f = (mcu->dm[rd_addr] == mcu->dm[rr_addr]);
	SKIP_CYCLES(mcu, f, (is32 ? 2 : 1));
	if (f) {
		mcu->pc += is32 ? 6 : 4;
	} else {
		mcu->pc += 2;
	}
}

static void exec_dec(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* DEC - Decrement */
	uint16_t rd_addr, r, rd;
	uint32_t val;

	rd_addr = (inst >> 4) & 0x1F;
	rd = mcu->dm[rd_addr];
	val = mcu->dm[rd_addr];
	val -= 1U;
	r = mcu->dm[rd_addr] = (uint8_t)val;
	mcu->pc += 2;

	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, rd == 0x80 ? 1 : 0);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
}

static void exec_fmul(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* FMUL – Fractional Multiply Unsigned */
	uint16_t rd_addr, rr_addr;
	uint16_t r;

	SKIP_CYCLES(mcu, 1, 1);
	rd_addr = (uint16_t)(0x10 + ((inst >> 4) & 7));
	rr_addr = (uint16_t)(0x10 + (inst & 7));
	r = (uint16_t)((uint8_t)(mcu->dm[rd_addr]) *
	                     (uint8_t)(mcu->dm[rr_addr]));
	mcu->dm[0] = (r << 1) & 0x0F;
	mcu->dm[1] = (r >> 7) & 0x0F;
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, (r >> 15) & 1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_fmuls(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* FMULS – Fractional Multiply Signed */
	uint16_t rd_addr, rr_addr;
	short r;

	SKIP_CYCLES(mcu, 1, 1);
	rd_addr = (uint16_t)(0x10 + ((inst >> 4) & 7));
	rr_addr = (uint16_t)(0x10 + (inst & 7));
	r = (short)((signed char)(mcu->dm[rd_addr]) *
	            (signed char)(mcu->dm[rr_addr]));
	mcu->dm[0] = (r << 1) & 0x0F;
	mcu->dm[1] = (r >> 7) & 0x0F;
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, (r >> 15) & 1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_fmulsu(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* FMULSU – Fractional Multiply Signed with Unsigned */
	uint16_t rd_addr, rr_addr;
	short r;

	SKIP_CYCLES(mcu, 1, 1);
	rd_addr = (uint16_t)(0x10 + ((inst >> 4) & 7));
	rr_addr = (uint16_t)(0x10 + (inst & 7));
	r = (short)((signed char)(mcu->dm[rd_addr]) *
	            (uint8_t)(mcu->dm[rr_addr]));
	mcu->dm[0] = (r << 1) & 0x0F;
	mcu->dm[1] = (r >> 7) & 0x0F;
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, (r >> 15) & 1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_icall(struct MSIM_AVR *mcu)
{
	/* ICALL – Indirect Call to Subroutine */
	uint64_t pc;
	uint8_t zh, zl;

	if (mcu->xmega) {
		SKIP_CYCLES(mcu, 1, mcu->pc_bits > 16 ? 2 : 1);
	} else {
		SKIP_CYCLES(mcu, 1, mcu->pc_bits > 16 ? 3 : 2);
	}

	pc = mcu->pc + 2;
	zh = mcu->dm[REG_ZH];
	zl = mcu->dm[REG_ZL];

	MSIM_AVR_StackPush(mcu, (uint8_t)(pc&0xFF));
	MSIM_AVR_StackPush(mcu, (uint8_t)((pc>>8) & 0xFF));
	if (mcu->pc_bits > 16) {	/* for 22-bit PC or above */
		MSIM_AVR_StackPush(mcu, (uint8_t)((pc>>16) & 0xFF));
	}
	mcu->pc = (uint32_t)(((zh<<8)&0xFF00) | (zl&0xFF));
}

static void exec_ijmp(struct MSIM_AVR *mcu)
{
	/* IJMP – Indirect Jump */
	uint8_t zh, zl;

	SKIP_CYCLES(mcu, 1, 1);

	zh = mcu->dm[REG_ZH];
	zl = mcu->dm[REG_ZL];
	mcu->pc = (uint32_t)(((zh<<8)&0xFF00) | (zl&0xFF));
}

static void exec_inc(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* INC - Increment */
	uint16_t rd_addr, r, rd;
	uint32_t val;

	rd_addr = (inst >> 4) & 0x1F;
	rd = mcu->dm[rd_addr];
	val = mcu->dm[rd_addr];
	val += 1U;
	r = (uint8_t)val;
	mcu->dm[rd_addr] = (uint8_t)r;
	mcu->pc += 2;

	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r >> 7) & 1);
	UPDSR(mcu, SR_TCOF, rd == 0x7F ? 1 : 0);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
}

static void exec_jmp(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* JMP - Jump */
	uint64_t c;
	uint32_t msb;

	SKIP_CYCLES(mcu, 1, 2);

	msb = (uint32_t)(((mcu->pm[mcu->pc+3] << 8) & 0xFF00) |
	                     (mcu->pm[mcu->pc+2] & 0xFF));
	c = msb | (((inst>>3)&0x3E) | (inst&0x01)) << 16;
	mcu->pc = (uint32_t)(c << 1); // address is in words, not bytes
}

static void exec_lac(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LAC - Load and Clear */
	uint16_t rd_addr, z;
	uint8_t zh, zl, rd;

	SKIP_CYCLES(mcu, 1, 1);

	zh = mcu->dm[REG_ZH];
	zl = mcu->dm[REG_ZL];
	z = (uint16_t)(((zh<<8)&0xFF00) | (zl&0xFF));
	rd_addr = (inst>>4)&0x1F;
	rd = mcu->dm[rd_addr];

	WRITE_DS(rd_addr, DM(z));
	WRITE_DS(z, DM(z) & (uint8_t)(~rd));
	mcu->pc += 2;
	mcu->read_io[0] = z;
}

static void exec_las(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LAS - Load and Set */
	uint16_t rd_addr, z;
	uint8_t zh, zl, rd;

	SKIP_CYCLES(mcu, 1, 1);

	zh = mcu->dm[REG_ZH];
	zl = mcu->dm[REG_ZL];
	z = (uint16_t)(((zh<<8)&0xFF00) | (zl&0xFF));
	rd_addr = (inst>>4)&0x1F;
	rd = mcu->dm[rd_addr];

	WRITE_DS(rd_addr, DM(z));
	WRITE_DS(z, DM(z) | (uint8_t)rd);
	mcu->pc += 2;
	mcu->read_io[0] = z;
}

static void exec_lat(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LAT - Load and Toggle */
	uint16_t rd_addr, z;
	uint8_t zh, zl, rd;

	SKIP_CYCLES(mcu, 1, 1);

	zh = mcu->dm[REG_ZH];
	zl = mcu->dm[REG_ZL];
	z = (uint16_t)(((zh<<8)&0xFF00) | (zl&0xFF));
	rd_addr = (inst>>4)&0x1F;
	rd = mcu->dm[rd_addr];

	WRITE_DS(rd_addr, DM(z));
	WRITE_DS(z, DM(z) ^ rd);
	mcu->pc += 2;
	mcu->read_io[0] = z;
}

static void exec_lds(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LDS - Load Direct from Data Space */
	uint16_t rd_addr, addr;
	uint8_t i2, i3;

	i2 = mcu->pm[mcu->pc+2];
	i3 = mcu->pm[mcu->pc+3];
	addr = (uint16_t)(((i3<<8)&0xFF00) | (i2&0xFF));

	if (!mcu->xmega) {
		SKIP_CYCLES(mcu, 1, 1);
	} else {
		SKIP_CYCLES(mcu, 1, ((addr <= mcu->ramend &&
		                      addr >= mcu->ramstart) ? 2 : 1));
	}

	rd_addr = (inst>>4)&0x1F;
	mcu->dm[rd_addr] = mcu->dm[addr];
	mcu->read_io[0] = addr;
	mcu->pc += 4;
}

static void exec_lds16(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LDS (16-bit) - Load Direct from Data Space */
	uint16_t rd_addr, addr;

	addr = (uint16_t)((((~inst)>>1)&0x80) | ((inst>>2)&0x40) |
	                        ((inst>>5)&0x30) | (inst&0x0F));
	rd_addr = (uint16_t)(((inst>>4)&0x0F) + 16);
	mcu->dm[rd_addr] = mcu->dm[addr];
	mcu->read_io[0] = addr;
	mcu->pc += 2;
}

static void exec_lpm(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LPM - Load Program Memory
	 * type I, R0 <- (Z)
	 * type II, Rd <- (Z)
	 * type III, Rd <- (Z), Z++ */
	uint32_t rd_addr, z;
	uint8_t zh, zl;

	SKIP_CYCLES(mcu, 1, 2);
	zh = mcu->dm[REG_ZH];
	zl = mcu->dm[REG_ZL];
	z = (uint16_t)(((zh<<8)&0xFF00) | (zl&0xFF));

	if (inst == 0x95C8) { /* type I */
		mcu->dm[0] = mcu->pm[z];
	} else if ((inst & 0xFE0F) == 0x9004) { /* type II */
		rd_addr = (inst>>4)&0x1F;
		mcu->dm[rd_addr] = mcu->pm[z];
	} else if ((inst & 0xFE0F) == 0x9005) { /* type III */
		rd_addr = (inst>>4)&0x1F;
		mcu->dm[rd_addr] = mcu->pm[z++];
		mcu->dm[REG_ZH] = (uint8_t)((z>>8)&0xFF);
		mcu->dm[REG_ZL] = (uint8_t)(z&0xFF);
	}
	mcu->pc += 2;
}

static void exec_lsr(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* LSR - Logical Shift Right */
	uint16_t rd_addr;
	uint8_t rd, r;

	rd_addr = (inst>>4)&0x1F;
	rd = mcu->dm[rd_addr];
	r = (rd>>1)&0xFF;
	mcu->dm[rd_addr] = r;
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, rd&1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, 0);
	UPDSR(mcu, SR_TCOF, SR(mcu, SR_NEG) ^ SR(mcu, SR_CARRY));
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
}

static void exec_sbrc(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SBRC – Skip if Bit in Register is Cleared */
	uint16_t rr_addr;
	uint8_t bit, r;

	rr_addr = (inst>>4)&0x1F;
	bit = inst&7;
	r = (mcu->dm[rr_addr]>>bit)&1;

	SKIP_CYCLES(mcu, !r, MSIM_AVR_Is32(mcu->pm[mcu->pc+2]) ? 2 : 1);
	if (!r) {
		mcu->pc += MSIM_AVR_Is32(mcu->pm[mcu->pc+2]) ? 6 : 4;
	} else {
		mcu->pc += 2;
	}
}

static void exec_sbrs(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SBRS – Skip if Bit in Register is Set */
	uint16_t rr_addr;
	uint8_t bit, r;

	rr_addr = (inst>>4)&0x1F;
	bit = inst&7;
	r = (mcu->dm[rr_addr]>>bit)&1;

	SKIP_CYCLES(mcu, r, MSIM_AVR_Is32(mcu->pm[mcu->pc+2]) ? 2 : 1);
	if (r) {
		mcu->pc += MSIM_AVR_Is32(mcu->pm[mcu->pc+2]) ? 6 : 4;
	} else {
		mcu->pc += 2;
	}
}

static void exec_eicall(struct MSIM_AVR *mcu)
{
	/* EICALL - Extended Indirect Call to Subroutine */
	uint8_t zh, zl, eind;
	uint64_t pc;
	uint8_t err;

	err = 0;
	if (!mcu->eind) {
		MSIM_LOG_FATAL("EICALL instruction is not supported on the "
		               "devices without EIND register");
		err = 1;
	}
	if (mcu->pc_bits < 22) {
		MSIM_LOG_FATAL("EICALL instruction is implemented in the "
		               "devices with 22-bit PC only");
		err = 1;
	}

	if (err == 0) {
		SKIP_CYCLES(mcu, 1, mcu->xmega ? 2 : 3);

		zh = mcu->dm[REG_ZH];
		zl = mcu->dm[REG_ZL];
		eind = *mcu->eind;
		pc = mcu->pc + 2;
		MSIM_AVR_StackPush(mcu, (uint8_t)(pc & 0xFF));
		MSIM_AVR_StackPush(mcu, (uint8_t)((pc >> 8) & 0xFF));
		MSIM_AVR_StackPush(mcu, (uint8_t)((pc >> 16) & 0xFF));

		pc = (uint64_t)(((eind<<16)&0xFF0000) |
		                     ((zh<<8)&0xFF00) | (zl&0xFF));
		mcu->pc = (uint32_t)pc;
	} else {
		/* There was an attempt to execute an illegal instruction.
		 * We'll have to terminate simulation with error code set. */
		mcu->state = AVR_MSIM_TESTFAIL;
	}
}

static void exec_eijmp(struct MSIM_AVR *mcu)
{
	/* EIJMP - Extended Indirect Jump */
	uint8_t zh, zl, eind;
	uint8_t err;

	err = 0;
	if (!mcu->eind) {
		MSIM_LOG_FATAL("EIJMP instruction is not supported on the "
		               "devices without EIND register");
		err = 1;
	}

	if (err == 0) {
		SKIP_CYCLES(mcu, 1, 1);
		zh = mcu->dm[REG_ZH];
		zl = mcu->dm[REG_ZL];
		eind = *mcu->eind;
		mcu->pc = (uint32_t)(((eind<<16)&0xFF0000) |
		                     ((zh<<8)&0xFF00) | (zl&0xFF));
	} else {
		/* There was an attempt to execute an illegal instruction.
		 * We'll have to terminate simulation with error code set. */
		mcu->state = AVR_MSIM_TESTFAIL;
	}
}

static void exec_xch(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* XCH - Exchange */
	uint16_t z, rd_addr;
	uint8_t v, zh, zl;

	SKIP_CYCLES(mcu, 1, 1);
	zh = mcu->dm[REG_ZH];
	zl = mcu->dm[REG_ZL];
	z = (uint16_t)(((zh<<8)&0xFF00) | (zl&0xFF));
	v = mcu->dm[z];
	rd_addr = (inst>>4)&0x1F;

	WRITE_DS(z, DM(rd_addr));
	WRITE_DS(rd_addr, v);
	mcu->pc += 2;
	mcu->read_io[0] = z;
}

static void exec_ror(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ROR – Rotate Right through Carry */
	uint16_t rd_addr;
	uint8_t c, rd, r;

	c = SR(mcu, SR_CARRY);
	rd_addr = (inst>>4)&0x1F;
	rd = mcu->dm[rd_addr];
	r = (uint8_t)(((rd>>1)&0x7F) | ((c<<7)&0x80));
	mcu->dm[rd_addr] = r;
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, rd&1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r>>7)&1);
	UPDSR(mcu, SR_TCOF, SR(mcu, SR_NEG) ^ SR(mcu, SR_CARRY));
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_HCARRY, (rd>>3)&1);
}

static void exec_reti(struct MSIM_AVR *mcu)
{
	/* RETI – Return from Interrupt */
	SKIP_CYCLES(mcu, 1, mcu->pc_bits > 16 ? 4 : 3);

	if (mcu->pc_bits > 16) {
		mcu->pc = (uint32_t)
		          (((MSIM_AVR_StackPop(mcu)<<16)&0xFF0000) |
		           ((MSIM_AVR_StackPop(mcu)<<8)&0xFF00) |
		           (MSIM_AVR_StackPop(mcu)&0xFF));
	} else {
		mcu->pc = (uint32_t)
		          (((MSIM_AVR_StackPop(mcu)<<8)&0xFF00) |
		           (MSIM_AVR_StackPop(mcu)&0xFF));
	}

	/* Enable interrupts globally (doesn't work for AVR XMEGA) */
	if (!mcu->xmega) {
		UPDSR(mcu, SR_GLOBINT, 1);
	}
	/* Execute one more instruction from the main program
	 * after an exit from interrupt service routine. */
	mcu->intr.exec_main = 1;
}

static void exec_swap(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SWAP – Swap Nibbles */
	uint16_t rd_addr;
	uint8_t rdh;

	rd_addr = (inst>>4)&0x1F;
	rdh = (mcu->dm[rd_addr]>>4)&0x0F;
	mcu->dm[rd_addr] = (uint8_t)
	                   (((mcu->dm[rd_addr]<<4)&0xF0) | rdh);
	mcu->pc += 2;
}

static void exec_or(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* OR – Logical OR */
	uint8_t rda, rra, rd, rr, r;

	rda = (inst>>4)&0x1F;
	rra = (uint8_t)(((inst>>5)&0x10) | (inst&0xF));
	rd = mcu->dm[rda];
	rr = mcu->dm[rra];
	r = rd | rr;
	mcu->dm[rda] = r;
	mcu->pc += 2;

	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r>>7)&1);
	UPDSR(mcu, SR_TCOF, 0);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
}

static void exec_neg(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* NEG – Two’s Complement */
	uint8_t rda, rd, r;

	rda = (inst>>4)&0x1F;
	rd = mcu->dm[rda];
	r = (uint8_t)(~rd + 1);
	mcu->dm[rda] = r;
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, r ? 1 : 0);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
	UPDSR(mcu, SR_NEG, (r>>7)&1);
	UPDSR(mcu, SR_TCOF, r == 0x80 ? 1 : 0);
	UPDSR(mcu, SR_SIGN, SR(mcu, SR_NEG) ^ SR(mcu, SR_TCOF));
	UPDSR(mcu, SR_HCARRY, ((r>>3)&1) | ((rd>>3)&1));
}

static void exec_ser(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SER – Set all Bits in Register */
	uint8_t rda;

	rda = (uint8_t)(((inst>>4)&0xF)+16);
	mcu->dm[rda] = 0xFF;
	mcu->pc += 2;
}

static void exec_mul(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* MUL – Multiply Unsigned */
	uint8_t rda, rra, rd, rr;
	uint32_t r;

	SKIP_CYCLES(mcu, 1, 1);

	rda = (inst>>4)&0x1F;
	rra = (uint8_t)(((inst>>5)&0x10) | (inst&0xF));
	rd = mcu->dm[rda];
	rr = mcu->dm[rra];
	r = (uint32_t)(rd*rr);
	mcu->dm[0] = r&0xFF;
	mcu->dm[1] = (r>>8)&0xFF;
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, (r>>15)&1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_muls(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* MULS – Multiply Signed */
	uint8_t rda, rra;
	signed char rd, rr;
	signed int r;

	SKIP_CYCLES(mcu, 1, 1);

	rda = (uint8_t)(((inst>>4)&0xF)+16);
	rra = (uint8_t)((inst&0xF)+16);
	rd = (signed char)mcu->dm[rda];
	rr = (signed char)mcu->dm[rra];
	r = rd*rr;
	mcu->dm[0] = (uint8_t)(r&0xFF);
	mcu->dm[1] = (uint8_t)((r>>8)&0xFF);
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, (r>>15)&1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_mulsu(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* MULSU – Multiply Signed with Unsigned */
	uint8_t rda, rra, rr;
	signed char rd;
	signed int r;

	SKIP_CYCLES(mcu, 1, 1);

	rda = (uint8_t)(((inst>>4)&0x7)+16);
	rra = (uint8_t)(((inst&0x7)+16));
	rd = (signed char)mcu->dm[rda];
	rr = mcu->dm[rra];
	r = rd*rr;
	mcu->dm[0] = (uint8_t)(r&0xFF);
	mcu->dm[1] = (uint8_t)((r>>8)&0xFF);
	mcu->pc += 2;

	UPDSR(mcu, SR_CARRY, (r>>15)&1);
	UPDSR(mcu, SR_ZERO, !r ? 1 : 0);
}

static void exec_elpm(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* ELPM - Extended Load Program Memory
	 * type I	R0 <- (RAMPZ:Z)
	 * type II	Rd <- (RAMPZ:Z)
	 * type III	Rd <- (RAMPZ:Z), (RAMPZ:Z)++
	 */
	uint8_t rda, zh, zl, ez;
	uint64_t z;
	uint8_t err;

	err = 0;
	if (mcu->rampz == NULL) {
		MSIM_LOG_FATAL("ELPM instruction is not supported on the "
		               "devices without RAMPZ register");
		err = 1;
	}

	if (err == 0) {
		SKIP_CYCLES(mcu, 1, 2); /* Skip 3 cycles anyway */
		ez = *mcu->rampz;
		zh = mcu->dm[REG_ZH];
		zl = mcu->dm[REG_ZL];
		z = (uint64_t)(((ez<<16)&0xFF0000) |
		                    ((zh<<8)&0xFF00) |
		                    (zl&0xFF));

		if (inst == 0x95D8) {				/* type I */
			mcu->dm[0] = mcu->pm[z];
		} else if ((inst & 0xFE0F) == 0x9006) {		/* type II */
			rda = (inst>>4)&0x1F;
			mcu->dm[rda] = mcu->pm[z];
		} else if ((inst & 0xFE0F) == 0x9007) {		/* type III */
			rda = (inst>>4)&0x1F;
			mcu->dm[rda] = mcu->pm[z++];
			*mcu->rampz = (uint8_t)((z>>16)&0xFF);
			mcu->dm[REG_ZH] = (uint8_t)((z>>8)&0xFF);
			mcu->dm[REG_ZL] = (uint8_t)(z&0xFF);
		}
		mcu->pc += 2;
	} else {
		/* There was an attempt to execute an illegal instruction.
		 * We'll have to terminate simulation with error code in this
		 * case. */
		mcu->state = AVR_MSIM_TESTFAIL;
	}
}

static void exec_spm(struct MSIM_AVR *mcu, uint32_t inst)
{
	/* SPM – Store Program Memory
	 * type I	(RAMPZ:Z) ← 0xFFFF, Erase program memory page
	 * type II	(RAMPZ:Z) ← R1:R0, Fill temporary buffer (word only!)
	 * type III	(RAMPZ:Z) ← BUF, Write buffer to PM
	 * type IV	(RAMPZ:Z) ← 0xFFFF, (Z) ← (Z + 2), *see above*
	 * type V	(RAMPZ:Z) ← R1:R0, (Z) ← (Z + 2), *see above*
	 * type VI	(RAMPZ:Z) ← BUF, (Z) ← (Z + 2), *see above*
	 */
	uint8_t zl, zh, ez, c;
	uint64_t z;
	uint8_t err = 0;

	if (mcu->spmcsr == NULL) {
		MSIM_LOG_FATAL("SPMCSR(SPMCR) register is not available "
		               "on this device");
		err = 1;
	}

	if (err == 0) {
		ez = (uint8_t)(mcu->rampz != NULL ? *mcu->rampz : 0);
		zh = mcu->dm[REG_ZH];
		zl = mcu->dm[REG_ZL];
		z = (uint64_t)(((ez<<16)&0xFF0000) | ((zh<<8)&0xFF00) |
		               (zl&0xFF));
		c = *mcu->spmcsr & 0x7;

		if (c == 0x3) {			/* erase PM page */
			memset(&mcu->pm[z], 0xFF, mcu->spm_pagesize);
		} else if (c == 0x1) {		/* fill the buffer */
			memcpy(&mcu->pmp[z], &mcu->dm[0], 2);
		} else if (c == 0x5) {		/* write a page */
			memcpy(&mcu->pm[z], &mcu->pmp[z], mcu->spm_pagesize);
		}
		mcu->pc += 2;

		/* Reset state of the SPM instruction */
		if (mcu->reset_spm != NULL) {
			mcu->reset_spm(mcu);
		}

		if (inst == 0x95F8) {
			z += 2;
			if (mcu->rampz != NULL) {
				*mcu->rampz = (uint8_t)((z>>16)&0xFF);
			}
			mcu->dm[REG_ZH] = (uint8_t)((z>>8)&0xFF);
			mcu->dm[REG_ZL] = (uint8_t)(z&0xFF);
		}
	} else {
		/* There was an attempt to execute an illegal instruction.
		 * We'll have to terminate simulation with error code in this
		 * case. */
		mcu->state = AVR_MSIM_TESTFAIL;
	}
}

static void exec_wdr(struct MSIM_AVR *mcu)
{
	/* WDR - Watchdog Timer Reset */
	mcu->wdt.sys_ticks = 0;
	mcu->wdt.ticks = 0;
	mcu->pc += 2;
}
