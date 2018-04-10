/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

unsigned int i;
unsigned char *pm;
unsigned char *dm;
unsigned long pmsz, pm_size;
unsigned long dmsz, dm_size;

#ifdef VCD_DUMP_REGS
static struct MSIM_VCDRegister known_regs[] = VCD_DUMP_REGS;
unsigned short known_regsn = sizeof known_regs/sizeof known_regs[0];
#endif

if (!mcu)
{
	fprintf(stderr, "MCU should not be NULL\n");
	return -1;
}

pm = args->pm;
dm = args->dm;
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
mcu->regs = GP_REGS;
mcu->io_regs = IO_REGS;

/* Program memory */
pmsz = (mcu->flashend - mcu->flashstart) + 1;
if (pm_size < pmsz)
{
	fprintf(stderr, "Program memory is limited by %lu bytes, %lu bytes "
	        "is not enough\n", pmsz, pm_size);
	return -1;
}
mcu->pm = pm;
mcu->pm_size = pm_size;
/* END Program memory */

/* Data memory */
dmsz = mcu->regs + mcu->io_regs + mcu->ramsize;
if (dm_size < dmsz)
{
	fprintf(stderr, "Data memory is limited by %lu bytes, %lu bytes "
	        "is not enough\n", dmsz, dm_size);
	return -1;
}
mcu->dm = dm;
mcu->dm_size = dm_size;
/* END Data memory */

mcu->sreg = &mcu->dm[SREG];
mcu->sph = &mcu->dm[SPH];
mcu->spl = &mcu->dm[SPL];

/* Extended registers */
#ifdef EIND
mcu->eind = &mcu->dm[_SFR_IO8(EIND)];
#else
mcu->eind = NULL;
#endif
#ifdef RAMPZ
mcu->rampz = &mcu->dm[_SFR_IO8(RAMPZ)];
#else
mcu->rampz = NULL;
#endif
#ifdef RAMPY
mcu->rampy = &mcu->dm[_SFR_IO8(RAMPY)];
#else
mcu->rampy = NULL;
#endif
#ifdef RAMPX
mcu->rampx = &mcu->dm[_SFR_IO8(RAMPX)];
#else
mcu->rampx = NULL;
#endif
#ifdef RAMPD
mcu->rampd = &mcu->dm[_SFR_IO8(RAMPD)];
#else
mcu->rampd = NULL;
#endif
/* END Extended registers */

/* Fuses */
#ifdef EFUSE_DEFAULT
mcu->fuse[2] = EFUSE_DEFAULT;
#endif
#ifdef HFUSE_DEFAULT
mcu->fuse[1] = HFUSE_DEFAULT;
#endif
mcu->fuse[0] = LFUSE_DEFAULT;

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
/* END Fuses */

/* Timers */
#ifdef TICK_TIMERS_F
mcu->tick_timers = TICK_TIMERS_F;
#else
mcu->tick_timers = NULL;
#endif
/* END Timers */

#ifdef BLS_START
mcu->bls->start = BLS_START;
mcu->bls->end = BLS_END;
mcu->bls->size = BLS_SIZE;
#else
mcu->bls->start = 0;
mcu->bls->end = 0;
mcu->bls->size = 0;
#endif

#if defined(SPMCSR)
mcu->spmcsr = &mcu->dm[_SFR_IO8(SPMCSR)];
#elif defined(SPMCR)
mcu->spmcsr = &mcu->dm[_SFR_IO8(SPMCR)];
#else
mcu->spmcsr = NULL;
#endif

/* Do not include any register into dump by default */
for (i = 0; i < sizeof mcu->vcdd->bit/sizeof mcu->vcdd->bit[0]; i++)
	mcu->vcdd->bit[i].regi = -1;
for (i = 0; i < sizeof mcu->vcdd->regs/sizeof mcu->vcdd->regs[0]; i++)
	mcu->vcdd->regs[i].off = -1;
#ifdef VCD_DUMP_REGS
for (i = 0; i < sizeof mcu->vcdd->regs/sizeof mcu->vcdd->regs[0]; i++)
{
	if (i < known_regsn) {
		known_regs[i].addr = &mcu->dm[known_regs[i].off];
		known_regs[i].oldv = *known_regs[i].addr;
		mcu->vcdd->regs[i] = known_regs[i];
	} else {
		break;
	}
}
#endif

mcu->clk_source = CLK_SOURCE;
mcu->freq = CLK_FREQ;
mcu->pc_bits = PC_BITS;
mcu->pc = RESET_PC;

/* Set up interrupts and IRQs */
mcu->intr->reset_pc = RESET_PC;
mcu->intr->ivt = IVT_ADDR;
#ifdef PROVIDE_IRQS_F
mcu->provide_irqs = PROVIDE_IRQS_F;
#else
mcu->provide_irqs = NULL;
#endif

/* Do not forget to add "return 0;" right after this init body in the
 * MCU-specific initialization functions!
 */
