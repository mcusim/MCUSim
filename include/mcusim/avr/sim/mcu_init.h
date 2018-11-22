/*
 * Copyright (c) 2017, 2018, The MCUSim Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
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
 *
 * This header contains a model-independent function to initialize AVR models.
 */
#ifndef MSIM_AVR_MCUINIT_H_
#define MSIM_AVR_MCUINIT_H_ 1

static inline int mcu_init(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
	uint32_t i;
	uint32_t pmsz, pm_size;
	uint32_t dmsz, dm_size;
#ifdef AVR_INIT_IOREGS
	struct MSIM_AVR_IOReg ioregs[] = AVR_INIT_IOREGS;
	uint32_t ioregs_num = sizeof ioregs/sizeof ioregs[0];
#endif

	if (mcu == NULL) {
		MSIM_LOG_FATAL("MCU instance should not be null");
		return 255;
	}
	pm_size = args->pmsz;
	dm_size = args->dmsz;
	strcpy(mcu->name, MCU_NAME);
	mcu->signature[0] = SIGNATURE_0;
	mcu->signature[1] = SIGNATURE_1;
	mcu->signature[2] = SIGNATURE_2;
#ifdef XMEGA
	mcu->xmega = 1;
#else
	mcu->xmega = 0;
#endif
#ifdef REDUCED_CORE
	mcu->reduced_core = 1;
#else
	mcu->reduced_core = 0;
#endif

	if (SPMCSR > 0) {
		mcu->spmcsr = &mcu->dm[SPMCSR];
	} else if (SPMCR > 0) {
		mcu->spmcsr = &mcu->dm[SPMCR];
	} else {
		mcu->spmcsr = NULL;
	}
	mcu->spm_pagesize = SPM_PAGESIZE;
	mcu->flashstart = FLASHSTART;
	mcu->flashend = FLASHEND;
	mcu->ramstart = RAMSTART;
	mcu->ramend = RAMEND;
	mcu->ramsize = RAMSIZE;
	mcu->e2start = E2START;
	mcu->e2end = E2END;
	mcu->e2size = E2SIZE;
	mcu->e2pagesize = E2PAGESIZE;
	mcu->lockbits = LBITS_DEFAULT;
	mcu->sfr_off = __SFR_OFFSET;
	mcu->regs_num = GP_REGS;
	mcu->ioregs_num = IO_REGS;

	/* Program memory */
	pmsz = mcu->flashend - mcu->flashstart + 1;
	if (pm_size < pmsz) {
		snprintf(mcu->log, sizeof mcu->log, "program memory is "
		         "limited by %" PRIu32 " bytes, %" PRIu32 " bytes is "
		         "not enough", pmsz, pm_size);
		MSIM_LOG_FATAL(mcu->log);
		return 255;
	}
	mcu->pm_size = pm_size;

	/* Data memory */
	dmsz = mcu->regs_num + mcu->ioregs_num + mcu->ramsize;
	if (dm_size < dmsz) {
		snprintf(mcu->log, sizeof mcu->log, "data memory is limited "
		         "by %" PRIu32 " bytes, %" PRIu32 " bytes is not "
		         "enough", dmsz, dm_size);
		MSIM_LOG_FATAL(mcu->log);
		return 255;
	}
	mcu->dm_size = dm_size;

	mcu->sreg = &mcu->dm[SREG];
	mcu->sph = &mcu->dm[SPH];
	mcu->spl = &mcu->dm[SPL];
	if (EIND > 0) {
		mcu->eind = &mcu->dm[EIND];
	} else {
		mcu->eind = NULL;
	}
	if (RAMPZ > 0) {
		mcu->rampz = &mcu->dm[RAMPZ];
	} else {
		mcu->rampz = NULL;
	}
	if (RAMPY > 0) {
		mcu->rampy = &mcu->dm[RAMPY];
	} else {
		mcu->rampy = NULL;
	}
	if (RAMPX > 0) {
		mcu->rampx = &mcu->dm[RAMPX];
	} else {
		mcu->rampx = NULL;
	}
	if (RAMPD > 0) {
		mcu->rampd = &mcu->dm[RAMPD];
	} else {
		mcu->rampd = NULL;
	}

#ifdef EFUSE_DEFAULT
	mcu->fuse[2] = EFUSE_DEFAULT;
#endif
#ifdef HFUSE_DEFAULT
	mcu->fuse[1] = HFUSE_DEFAULT;
#endif
	mcu->fuse[0] = LFUSE_DEFAULT;

	/* MCU-specific functions */
#ifdef SET_FUSE_F
	mcu->set_fusef = SET_FUSE_F;
#else
	mcu->set_fusef = NULL;
#endif
#ifdef SET_LOCK_F
	mcu->set_lockf = SET_LOCK_F;
#else
	mcu->set_lockf = NULL;
#endif
#ifdef TICK_PERF_F
	mcu->tick_perf = TICK_PERF_F;
#else
	mcu->tick_perf = NULL;
#endif
#ifdef PASS_IRQS_F
	mcu->pass_irqs = PASS_IRQS_F;
#else
	mcu->pass_irqs = NULL;
#endif
#ifdef RESET_SPM_F
	mcu->reset_spm = RESET_SPM_F;
#else
	mcu->reset_spm = NULL;
#endif

#ifdef BLS_START
	mcu->bls.start = BLS_START;
	mcu->bls.end = BLS_END;
	mcu->bls.size = BLS_SIZE;
#else
	mcu->bls.start = 0;
	mcu->bls.end = 0;
	mcu->bls.size = 0;
#endif

	/* Init descriptors of the I/O registers */
	for (i = 0; i < MSIM_AVR_DMSZ; i++) {
		mcu->ioregs[i].off = -1;
	}
	/* Init registers to be included into VCD dump */
	for (i = 0; i < MSIM_AVR_VCD_REGS; i++) {
		mcu->vcd[i].i = -1;
		mcu->vcd[i].reg_lowi = -1;
	}

#ifdef AVR_INIT_IOREGS
	/* Fill descriptors of the available I/O registers */
	for (i = 0; i < ioregs_num; i++) {
		if (ioregs[i].off > 0) {
			mcu->ioregs[ioregs[i].off] = ioregs[i];
			mcu->ioregs[ioregs[i].off].addr =
			        &mcu->dm[ioregs[i].off];
			/* Write reset/default value of the register */
			mcu->dm[ioregs[i].off] = ioregs[i].reset;
		}
	}
#endif

	mcu->clk_source = CLK_SOURCE;
	mcu->freq = CLK_FREQ;
	mcu->pc_bits = PC_BITS;
	mcu->pc = RESET_PC;

	/* Set up interrupts and IRQs */
	mcu->intr.reset_pc = RESET_PC;
	mcu->intr.ivt = IVT_ADDR;
	return 0;
}

#endif /* MSIM_AVR_MCUINIT_H_ */
