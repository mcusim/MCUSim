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

#ifndef MSIM_AVR_MCUINIT_H_
#define MSIM_AVR_MCUINIT_H_ 1

/* A model-independent function to initialize an AVR MCU */
static inline int
mcu_init(const MSIM_AVR *orig, MSIM_AVR *mcu, MSIM_InitArgs *args)
{
	uint32_t i, pmsz, dmsz;
	uint32_t pm_size = args->pmsz;
	uint32_t dm_size = args->dmsz;

	if (mcu == NULL) {
		MSIM_LOG_FATAL("MCU instance should not be null");
		return 255;
	}

	/* Copy MCU from the original one declared in a header file. */
	(*mcu) = (*orig);

	if (SPMCSR > 0) {
		mcu->spmcsr = &mcu->dm[SPMCSR];
	} else if (SPMCR > 0) {
		mcu->spmcsr = &mcu->dm[SPMCR];
	} else {
		mcu->spmcsr = NULL;
	}

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

	/* Init descriptors of the I/O registers */
	for (i = 0; i < MSIM_AVR_DMSZ; i++) {
		mcu->ioregs[i].off = -1;
	}
	/* Init registers to be included into VCD dump */
	for (i = 0; i < MSIM_AVR_VCD_REGS; i++) {
		mcu->vcd.regs[i].i = -1;
		mcu->vcd.regs[i].reg_lowi = -1;
	}

#ifdef AVR_INIT_IOREGS
	struct MSIM_AVR_IOReg ioregs[] = AVR_INIT_IOREGS;
	uint32_t ioregs_num = sizeof ioregs/sizeof ioregs[0];

	/* Fill descriptors of the available I/O registers */
	for (i = 0; i < ioregs_num; i++) {
		if ((ioregs[i].off > 0) && (ioregs[i].off < MSIM_AVR_DMSZ)) {
			mcu->ioregs[ioregs[i].off] = ioregs[i];
			mcu->ioregs[ioregs[i].off].addr =
			        &mcu->dm[ioregs[i].off];

			/* Write reset/default value of the register */
			mcu->dm[ioregs[i].off] = ioregs[i].reset;
		}
	}
#endif

	return 0;
}

#endif /* MSIM_AVR_MCUINIT_H_ */
