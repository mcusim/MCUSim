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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* We would like to include headers specific to the ATMega8A microcontroller */
#define __AVR_ATmega8A__ 1
#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/hex/ihex.h"

static int is_ckopt_programmed(uint8_t ckopt_f);
static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t fuse_high,
			  uint8_t fuse_low);
static int set_bldr_size(struct MSIM_AVR *mcu, uint8_t fuse_high);
static int set_frequency(struct MSIM_AVR *mcu, uint8_t fuse_high,
			 uint8_t fuse_low);
static int set_reset_vector(struct MSIM_AVR *mcu, uint8_t fuse_high);
static int set_progmem(struct MSIM_AVR *mcu, uint8_t *mem, uint32_t size);
static int set_datamem(struct MSIM_AVR *mcu, uint8_t *mem, uint32_t size);

/*
 * Set ATmega8A lock bits to the default values
 * according to the datasheet. ATmega8A has 6 lock bits only.
 *
 * They are:
 * 5     4     3     2     1   0
 * BLB12 BLB11 BLB02 BLB01 LB2 LB1
 *
 * Default value means:
 *	- no memory lock features enabled;
 *	- no restrictions for SPM or Load Program Memory (LPM)
 *	  instruction accessing the Application section;
 *	- no restrictions for SPM or LPM accessing
 *	  the Boot Loader section.
 *
 * Set ATmega8A fuse bits to the default values. ATmega8A has
 * only two of them, high and low.
 *
 * The high fuse byte is:
 * 7        6     5     4     3      2       1       0
 * RSTDISBL WDTON SPIEN CKOPT EESAVE BOOTSZ1 BOOTSZ0 BOOTRST
 *
 * The low fuse byte is:
 * 7        6     5    4    3      2      1      0
 * BODLEVEL BODEN SUT1 SUT0 CKSEL3 CKSEL2 CKSEL1 CKSEL0
 *
 * Default value for high byte means:
 *	- boot sector in program memory is 1024 words,
 *	  from 0xC00-0xFFF,
 *	  application sector is 3072 words,
 *	  from 0x000-0xBFF;
 *	- ...
 *
 * Default value for low byte means:
 *	- ...
 */
int MSIM_M8AInit(struct MSIM_AVR *mcu,
		 uint8_t *pm, uint32_t pm_size,
		 uint8_t *dm, uint32_t dm_size)
{
	uint32_t i;

	if (!mcu) {
		fprintf(stderr, "MCU should not be NULL\n");
		return -1;
	}

	srand((unsigned int) time(NULL));
	mcu->id = (uint32_t) rand();
	strcpy(mcu->name, "atmega8a");
	mcu->signature[0] = SIGNATURE_0;
	mcu->signature[1] = SIGNATURE_1;
	mcu->signature[2] = SIGNATURE_2;

	/*
	 * Set values according to the header file included
	 * by avr/io.h.
	 */
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

	mcu->lockbits = 0x3F;

	mcu->sfr_off = __SFR_OFFSET;

	/* Invalidate I/O ports addresses before initialization */
	for (i = 0; i < sizeof(mcu->io_addr)/sizeof(mcu->io_addr[0]); i++)
		mcu->io_addr[i] = -1;
	mcu->io_addr[SREG_ADDRI] = 0x3F;
	mcu->io_addr[SPH_ADDRI] = 0x3E;
	mcu->io_addr[SPL_ADDRI] = 0x3D;
	mcu->io_addr[GICR_ADDRI] = 0x3B;
	mcu->io_addr[GIFR_ADDRI] = 0x3A;
	mcu->io_addr[TIMSK_ADDRI] = 0x39;
	mcu->io_addr[TIFR_ADDRI] = 0x38;
	mcu->io_addr[SPMCR_ADDRI] = 0x37;
	mcu->io_addr[TWCR_ADDRI] = 0x36;
	mcu->io_addr[MCUCR_ADDRI] = 0x35;
	mcu->io_addr[MCUCSR_ADDRI] = 0x34;
	mcu->io_addr[TCCR0_ADDRI] = 0x33;
	mcu->io_addr[TCNT0_ADDRI] = 0x32;
	mcu->io_addr[OSCCAL_ADDRI] = 0x31;
	mcu->io_addr[SFIOR_ADDRI] = 0x30;
	mcu->io_addr[TCCR1A_ADDRI] = 0x2F;
	mcu->io_addr[TCCR1B_ADDRI] = 0x2E;
	mcu->io_addr[TCNT1H_ADDRI] = 0x2D;
	mcu->io_addr[TCNT1L_ADDRI] = 0x2C;
	mcu->io_addr[OCR1AH_ADDRI] = 0x2B;
	mcu->io_addr[OCR1AL_ADDRI] = 0x2A;
	mcu->io_addr[OCR1BH_ADDRI] = 0x29;
	mcu->io_addr[OCR1BL_ADDRI] = 0x28;
	mcu->io_addr[ICR1H_ADDRI] = 0x27;
	mcu->io_addr[ICR1L_ADDRI] = 0x26;
	mcu->io_addr[TCCR2_ADDRI] = 0x25;
	mcu->io_addr[TCNT2_ADDRI] = 0x24;
	mcu->io_addr[OCR2_ADDRI] = 0x23;
	mcu->io_addr[ASSR_ADDRI] = 0x22;
	mcu->io_addr[WDTCR_ADDRI] = 0x21;
	mcu->io_addr[UBRRH_ADDRI] = 0x20;
	mcu->io_addr[UCSRC_ADDRI] = 0x20;
	mcu->io_addr[EEARH_ADDRI] = 0x1F;
	mcu->io_addr[EEARL_ADDRI] = 0x1E;
	mcu->io_addr[EEDR_ADDRI] = 0x1D;
	mcu->io_addr[EECR_ADDRI] = 0x1C;
	mcu->io_addr[PORTB_ADDRI] = 0x18;
	mcu->io_addr[DDRB_ADDRI] = 0x17;
	mcu->io_addr[PINB_ADDRI] = 0x16;
	mcu->io_addr[PORTC_ADDRI] = 0x15;
	mcu->io_addr[DDRC_ADDRI] = 0x14;
	mcu->io_addr[PINC_ADDRI] = 0x13;
	mcu->io_addr[PORTD_ADDRI] = 0x12;
	mcu->io_addr[DDRD_ADDRI] = 0x11;
	mcu->io_addr[PIND_ADDRI] = 0x10;
	mcu->io_addr[SPDR_ADDRI] = 0x0F;
	mcu->io_addr[SPSR_ADDRI] = 0x0E;
	mcu->io_addr[SPCR_ADDRI] = 0x0D;
	mcu->io_addr[UDR_ADDRI] = 0x0C;
	mcu->io_addr[UCSRA_ADDRI] = 0x0B;
	mcu->io_addr[UCSRB_ADDRI] = 0x0A;
	mcu->io_addr[UBRRL_ADDRI] = 0x09;
	mcu->io_addr[ACSR_ADDRI] = 0x08;
	mcu->io_addr[ADMUX_ADDRI] = 0x07;
	mcu->io_addr[ADCSRA_ADDRI] = 0x06;
	mcu->io_addr[ADCH_ADDRI] = 0x05;
	mcu->io_addr[ADCL_ADDRI] = 0x04;
	mcu->io_addr[TWDR_ADDRI] = 0x03;
	mcu->io_addr[TWAR_ADDRI] = 0x02;
	mcu->io_addr[TWSR_ADDRI] = 0x01;
	mcu->io_addr[TWBR_ADDRI] = 0x00;

	if (set_progmem(mcu, pm, pm_size))
		return -1;
	if (set_datamem(mcu, dm, dm_size))
		return -1;
	if (set_fuse_bytes(mcu, 0xD9, 0xE1)) {
		fprintf(stderr, "Fuse bytes cannot be set correctly\n");
		return -1;
	}
	return 0;
}

int MSIM_M8ALoadProgmem(struct MSIM_AVR *mcu, FILE *fp)
{
	IHexRecord rec, mem_rec;

	if (!fp) {
		fprintf(stderr, "Cannot read from the filestream");
		return -1;
	}

	/*
	 * Copy HEX data to program memory of the MCU.
	 */
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

	/*
	 * Verify checksum of the loaded data.
	 */
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

static int set_progmem(struct MSIM_AVR *mcu, uint8_t *mem, uint32_t size)
{
	uint32_t flash_size;

	/* Size in bytes */
	flash_size= (mcu->flashend - mcu->flashstart) + 1;
	if (size != flash_size) {
		fprintf(stderr, "Program memory is limited by %d KiB,"
				" %u.%03u KiB doesn't match\n",
				flash_size/1024, size/1024, size%1024);
		return -1;
	}

	mcu->prog_mem = mem;
	mcu->pm_size = size;
	return 0;
}

static int set_datamem(struct MSIM_AVR *mcu, uint8_t *mem, uint32_t size)
{
	uint32_t sfr;

	if ((mcu->ramsize + 96) != size) {
		fprintf(stderr, "Data memory is limited by %u.%03u KiB,"
				" %u.%03u KiB doesn't match\n",
				(mcu->ramsize + 96) / 1024,
				(mcu->ramsize + 96) % 1024,
				size / 1024,
				size % 1024);
		return -1;
	}

	sfr = mcu->sfr_off;
	mcu->data_mem = mem;
	mcu->dm_size = size;
	mcu->sreg = &mcu->data_mem[(uint32_t)mcu->io_addr[SREG_ADDRI] + sfr];

	mcu->data_mem[(uint32_t)mcu->io_addr[SREG_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[SPH_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[SPL_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[GICR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[GIFR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TIMSK_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TIFR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[SPMCR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TWCR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[MCUCR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[MCUCSR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TCCR0_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TCNT0_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[OSCCAL_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[SFIOR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TCCR1A_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TCCR1B_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TCNT1H_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TCNT1L_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[OCR1AH_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[OCR1AL_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[OCR1BH_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[OCR1BL_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[ICR1H_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[ICR1L_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TCCR2_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TCNT2_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[OCR2_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[ASSR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[WDTCR_ADDRI]+sfr] = 0x00;
	/*
	 * From datasheet:
	 *
	 * The UBRRH Register shares the same I/O location
	 * as the UCSRC Register. When doing a write access of this
	 * I/O location, the high bit of the value written, the
	 * USART Register Select (URSEL) bit, controls which one of the two
	 * registers that will be written.
	 *
	 * If URSEL is zero during a write operation, the UBRRH value
	 * will be updated. If URSEL is one, the UCSRC setting will be updated.
	 *
	 *	// Set UBRRH to 2
	 *	UBRRH = 0x02;
	 *
	 *	// Set the USBS and the UCSZ1 bit to one, and
	 *	// the remaining bits to zero.
	 *	UCSRC = (1<<URSEL) | (1<<USBS) | (1<<UCSZ1);
	 *
	 * Doing a read access to the UBRRH or the UCSRC Register is a
	 * more complex operation. The read access is controlled by a
	 * timed sequence. Reading the I/O location once returns the UBRRH
	 * Register contents. If the register location was read in previous
	 * system clock cycle, reading the register in the current clock
	 * cycle will return the UCSRC contents. Note that the timed
	 * sequence for reading the UCSRC is an atomic operation.
	 * Interrupts must therefore be controlled (e.g., by disabling
	 * interrupts globally) during the read operation.
	 *
	 *	unsigned char USART_ReadUCSRC(void)
	 *	{
	 *		unsigned char ucsrc;
	 *		// Read UCSRC
	 *		ucsrc = UBRRH;
	 *		ucsrc = UCSRC;
	 *		return ucsrc;
	 *	}
	 */
	mcu->data_mem[(uint32_t)mcu->io_addr[UBRRH_ADDRI]+sfr] = 0x00;
	/* mcu->data_mem[(uint32_t)mcu->io_addr[UCSRC_ADDRI]+sfr] = 0x82; */
	mcu->data_mem[(uint32_t)mcu->io_addr[EEARH_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[EEARL_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[EECR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[EECR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[PORTB_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[DDRB_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[PINB_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[PORTC_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[DDRC_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[PINC_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[PORTD_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[DDRD_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[PIND_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[SPDR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[SPDR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[SPSR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[SPCR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[UDR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[UCSRA_ADDRI]+sfr] = 0x20;
	mcu->data_mem[(uint32_t)mcu->io_addr[UCSRB_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[UBRRL_ADDRI]+sfr] = 0x00;
	/*
	 * ACSR:5(ACO) - The output of the Analog Comparator is synchronized
	 * and then directly connected to ACO, i.e. it is an analog output.
	 */
	mcu->data_mem[(uint32_t)mcu->io_addr[ACSR_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[ADMUX_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[ADCSRA_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[ADCH_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[ADCL_ADDRI]+sfr] = 0x00;
	mcu->data_mem[(uint32_t)mcu->io_addr[TWDR_ADDRI]+sfr] = 0x01;
	mcu->data_mem[(uint32_t)mcu->io_addr[TWAR_ADDRI]+sfr] = 0x02;
	mcu->data_mem[(uint32_t)mcu->io_addr[TWSR_ADDRI]+sfr] = 0x08;
	mcu->data_mem[(uint32_t)mcu->io_addr[TWBR_ADDRI]+sfr] = 0x00;

	return 0;
}

static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t high, uint8_t low)
{
	mcu->fuse[1] = high;
	mcu->fuse[0] = low;

	if (set_bldr_size(mcu, high)) {
		fprintf(stderr, "Cannot set size of bootloader!\n");
		return -1;
	}

	if (set_frequency(mcu, high, low)) {
		fprintf(stderr, "Cannoe set frequency configuration!\n");
		return -1;
	}

	if (set_reset_vector(mcu, high))
		return -1;

	return 0;
}

static int set_bldr_size(struct MSIM_AVR *mcu, uint8_t fuse_high)
{
	/*
	 * Check BOOTSZ1:0 flags and set bootloader
	 * parameters accordingly.
	 */
	switch ((fuse_high >> 1) & 0x03) {
	case 0x01:
		mcu->boot_loader->start = 0xE00;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 512;
		break;
	case 0x02:
		mcu->boot_loader->start = 0xF00;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 256;
		break;
	case 0x03:
		mcu->boot_loader->start = 0xF80;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 128;
		break;
	case 0x00:
	default:
		mcu->boot_loader->start = 0xC00;
		mcu->boot_loader->end = 0xFFF;
		mcu->boot_loader->size = 1024;
		break;
	}

	return 0;
}

static int set_frequency(struct MSIM_AVR *mcu, uint8_t fuse_high,
			 uint8_t fuse_low)
{
	uint8_t cksel_f, ckopt_f;

	/*
	 * Check CKOPT and CKSEL3:0 in order to understand where
	 * clock signal comes from and expected frequency.
	 *
	 * The default option for ATmega8A is 1MHz internal RC oscillator.
	 * CKOPT should always be unprogrammed (value is 1) when using
	 * internal oscillator.
	 */
	ckopt_f = (fuse_high >> 4) & 0x01;
	cksel_f = fuse_low & 0x0F;
	switch(cksel_f) {
	case 0x02:					/* Internal, 2 MHz */
		if (is_ckopt_programmed(ckopt_f))
			return -1;
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 2000;
		break;
	case 0x03:					/* Internal, 4 MHz */
		if (is_ckopt_programmed(ckopt_f))
			return -1;
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 4000;
		break;
	case 0x04:					/* Internal, 8 MHz */
		if (is_ckopt_programmed(ckopt_f))
			return -1;
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 8000;
		break;
	case 0x01:
	default:					/* Internal, 1 MHz */
		if (is_ckopt_programmed(ckopt_f))
			return -1;
		mcu->clk_source = AVR_INT_CLK;
		mcu->freq = 1000;
		break;
	case 0x00:
		/*
		 * External Clock
		 *
		 * It is not meant to be a crystal/ceramic resonator,
		 * crystal oscillator or RC oscillator, so we cannot
		 * expect any frequency.
		 */
		mcu->clk_source = AVR_EXT_CLK;
		mcu->freq = UINT32_MAX;
		break;
	}

	return 0;
}

static int is_ckopt_programmed(uint8_t ckopt_f)
{
	if (!ckopt_f) {
		fprintf(stderr, "CKOPT fuse bit should be unprogrammed "
				"(CKOPT == 1) using internal clock source\n");
		return -1;
	}
	return 0;
}

static int set_reset_vector(struct MSIM_AVR *mcu, uint8_t fuse_high)
{
	/*
	 * BOOTRST and IVSEL bit in GICR register define
	 * reset address and start address of the interrupt
	 * vectors table (IVT).
	 *
	 * BOOTRST IVSEL Reset Address       IVT
	 * 1       0     0x000               0x001
	 * 1       1     0x000               Boot Reset Address + 0x001
	 * 0       0     Boot Reset Address  0x001
	 * 0       1     Boot Reset Address  Boot Reset Address + 0x001
	 */
	switch (fuse_high & 0x01) {
	case 0x00:
		/*
		 * Boot reset address.
		 */
		mcu->reset_pc = mcu->boot_loader->start;
		break;
	case 0x01:
	default:
		/*
		 * Lowest address of the program memory.
		 */
		mcu->reset_pc = 0x000;
		break;
	}
	mcu->pc = mcu->reset_pc;
	mcu->sp_high = &mcu->data_mem[(uint32_t)mcu->io_addr[SPH_ADDRI] +
				      mcu->sfr_off];
	mcu->sp_low = &mcu->data_mem[(uint32_t)mcu->io_addr[SPL_ADDRI] +
				     mcu->sfr_off];

	return 0;
}
