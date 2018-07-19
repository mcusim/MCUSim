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
/*
 * Copyright (c) 2010 Ivan A. Sergeev
 *
 * ihex.c
 * Utility functions to create, read, write, and print Intel HEX8
 * binary records.
 *
 * Written by Vanya A. Sergeev <vsergeev@gmail.com>
 * Version 1.0.5 - February 2011
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "mcusim/hex/ihex.h"

/*
 * Initializes a new IHexRecord structure that the paramater
 * ihexRecord points to with the passed record type, 16-bit
 * integer address, 8-bit data array, and size of 8-bit data array.
 */
int New_IHexRecord(unsigned int type, unsigned int address,
                   const unsigned char *data, unsigned int dataLen,
                   IHexRecord *ihexRecord)
{
	/* Data length size check, assertion of ihexRecord pointer */
	if (dataLen > IHEX_MAX_DATA_LEN/2 || ihexRecord == NULL) {
		return IHEX_ERROR_INVALID_ARGUMENTS;
	}

	ihexRecord->type = type;
	ihexRecord->address = address;
	memcpy(ihexRecord->data, data, (size_t)dataLen);
	ihexRecord->dataLen = dataLen;
	ihexRecord->checksum = Checksum_IHexRecord(ihexRecord);

	return IHEX_OK;
}

/*
 * Utility function to read an Intel HEX8 record from a file.
 */
int Read_IHexRecord(IHexRecord *ihexRecord, FILE *in)
{
	char recordBuff[IHEX_RECORD_BUFF_SIZE];
	/*
	 * A temporary buffer to hold ASCII hex encoded data,
	 * set to the maximum length we would ever need.
	 */
	char hexBuff[IHEX_ADDRESS_LEN+1];
	unsigned int dataCount, i;
	long type, len;

	/* Check our record pointer and file pointer */
	if (ihexRecord == NULL || in == NULL) {
		return IHEX_ERROR_INVALID_ARGUMENTS;
	}

	if (fgets(recordBuff, IHEX_RECORD_BUFF_SIZE, in) == NULL) {
		/* In case we hit EOF, don't report a file error */
		if (feof(in) != 0) {
			return IHEX_ERROR_EOF;
		} else {
			return IHEX_ERROR_FILE;
		}
	}
	/* Null-terminate the string at the first sign of a \r or \n */
	for (i = 0; i < strlen(recordBuff); i++) {
		if (recordBuff[i] == '\r' || recordBuff[i] == '\n') {
			recordBuff[i] = 0;
			break;
		}
	}


	/* Check if we hit a newline */
	if (strlen(recordBuff) == 0) {
		return IHEX_ERROR_NEWLINE;
	}

	/* Size check for start code, count, addess, and type fields */
	if (strlen(recordBuff) < (unsigned int)
	                (1+IHEX_COUNT_LEN+IHEX_ADDRESS_LEN+IHEX_TYPE_LEN)) {
		return IHEX_ERROR_INVALID_RECORD;
	}

	/* Check the for colon start code */
	if (recordBuff[IHEX_START_CODE_OFFSET] != IHEX_START_CODE) {
		return IHEX_ERROR_INVALID_RECORD;
	}

	/* Copy the ASCII hex encoding of the count field into hexBuff,
	 * convert it to a usable integer.
	 */
	strncpy(hexBuff, recordBuff+IHEX_COUNT_OFFSET, IHEX_COUNT_LEN);
	hexBuff[IHEX_COUNT_LEN] = 0;
	/* dataCount = (int) strtol(hexBuff, (char **)NULL, 16); */
	len = strtol(hexBuff, (char **)NULL, 16);
	if (len < 0) {
		fprintf(stderr, "Byte count %ld shouldn't be negative\n", len);
	}
	dataCount = (unsigned int) len;

	/* Copy the ASCII hex encoding of the address field into hexBuff,
	 * convert it to a usable integer.
	 */
	strncpy(hexBuff, recordBuff+IHEX_ADDRESS_OFFSET, IHEX_ADDRESS_LEN);
	hexBuff[IHEX_ADDRESS_LEN] = 0;
	ihexRecord->address = (uint16_t)strtol(hexBuff, (char **)NULL, 16);

	/* Copy the ASCII hex encoding of the address field into hexBuff,
	 * convert it to a usable integer.
	 */
	strncpy(hexBuff, recordBuff+IHEX_TYPE_OFFSET, IHEX_TYPE_LEN);
	hexBuff[IHEX_TYPE_LEN] = 0;
	type = strtol(hexBuff, (char **)NULL, 16);
	if (type < 0) {
		fprintf(stderr, "Record type %ld shouldn't be negative!\n",
		        type);
	}
	ihexRecord->type = (unsigned int) type;

	/* Size check for start code, count, address, type, data and checksum fields */
	if (strlen(recordBuff) < (unsigned int)
	                (1+IHEX_COUNT_LEN+IHEX_ADDRESS_LEN+IHEX_TYPE_LEN+
	                 dataCount*2+IHEX_CHECKSUM_LEN)) {
		return IHEX_ERROR_INVALID_RECORD;
	}

	/* Loop through each ASCII hex byte of the data field, pull it out into hexBuff,
	 * convert it and store the result in the data buffer of the Intel HEX8 record */
	for (i = 0; i < dataCount; i++) {
		/* Times two i because every byte is represented by two ASCII hex characters */
		strncpy(hexBuff, recordBuff+IHEX_DATA_OFFSET+2*i, IHEX_ASCII_HEX_BYTE_LEN);
		hexBuff[IHEX_ASCII_HEX_BYTE_LEN] = 0;
		ihexRecord->data[i] = (uint8_t)strtol(hexBuff, (char **)NULL, 16);
	}
	ihexRecord->dataLen = dataCount;

	/* Copy the ASCII hex encoding of the checksum field into hexBuff, convert it to a usable integer */
	strncpy(hexBuff, recordBuff+IHEX_DATA_OFFSET+dataCount*2, IHEX_CHECKSUM_LEN);
	hexBuff[IHEX_CHECKSUM_LEN] = 0;
	ihexRecord->checksum = (uint8_t)strtol(hexBuff, (char **)NULL, 16);

	if (ihexRecord->checksum != Checksum_IHexRecord(ihexRecord)) {
		return IHEX_ERROR_INVALID_RECORD;
	}

	return IHEX_OK;
}

/* Utility function to write an Intel HEX8 record to a file */
int Write_IHexRecord(const IHexRecord *ihexRecord, FILE *out)
{
	unsigned int i;

	/* Check our record pointer and file pointer */
	if (ihexRecord == NULL || out == NULL) {
		return IHEX_ERROR_INVALID_ARGUMENTS;
	}

	/* Check that the data length is in range */
	if (ihexRecord->dataLen > IHEX_MAX_DATA_LEN/2) {
		return IHEX_ERROR_INVALID_RECORD;
	}

	/* Write the start code, data count, address, and type fields */
	if (fprintf(out, "%c%2.2X%2.4X%2.2X", IHEX_START_CODE,
	                ihexRecord->dataLen, ihexRecord->address,
	                ihexRecord->type) < 0) {
		return IHEX_ERROR_FILE;
	}

	/* Write the data bytes */
	for (i = 0; i < ihexRecord->dataLen; i++) {
		if (fprintf(out, "%2.2X", ihexRecord->data[i]) < 0) {
			return IHEX_ERROR_FILE;
		}
	}

	/* Calculate and write the checksum field */
	if (fprintf(out, "%2.2X\r\n", Checksum_IHexRecord(ihexRecord)) < 0) {
		return IHEX_ERROR_FILE;
	}

	return IHEX_OK;
}

/* Utility function to print the information stored in an Intel HEX8 record */
void Print_IHexRecord(const IHexRecord *ihexRecord)
{
	unsigned int i;
	printf("Intel HEX8 Record Type: \t%d\n", ihexRecord->type);
	printf("Intel HEX8 Record Address: \t0x%2.4X\n", ihexRecord->address);
	printf("Intel HEX8 Record Data: \t{");
	for (i = 0; i < ihexRecord->dataLen; i++) {
		if (i+1 < ihexRecord->dataLen) {
			printf("0x%02X, ", ihexRecord->data[i]);
		} else {
			printf("0x%02X", ihexRecord->data[i]);
		}
	}
	printf("}\n");
	printf("Intel HEX8 Record Checksum: \t0x%2.2X\n", ihexRecord->checksum);
}

/* Utility function to calculate the checksum of an Intel HEX8 record */
unsigned char Checksum_IHexRecord(const IHexRecord *ihexRecord)
{
	unsigned long checksum;
	unsigned int i;

	/* Calculate sum of the record fields */
	checksum = ihexRecord->dataLen;
	checksum += ihexRecord->type;
	checksum += ihexRecord->address;
	checksum += (ihexRecord->address >> 8) & 0xFF;
	for (i = 0; i < ihexRecord->dataLen; i++) {
		checksum += ihexRecord->data[i];
	}

	/* Two's complement on checksum */
	checksum = ~checksum + 1;

	return (unsigned char)(checksum & 0xFF);
}
