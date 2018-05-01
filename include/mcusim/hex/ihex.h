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
 * ihex.h
 * Low-level utility functions to create, read, write, and print Intel HEX8
 * binary records.
 *
 * Written by Vanya A. Sergeev <vsergeev@gmail.com>
 * Version 1.0.5 - February 2011
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef MSIM_TOOLS_INTEL_HEX_H_
#define MSIM_TOOLS_INTEL_HEX_H_ 1

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * General definition of the Intel HEX8 specification.
 */
enum _IHexDefinitions {
	/*
	 * 768 should be plenty of space to read in a Intel HEX8 record.
	 */
	IHEX_RECORD_BUFF_SIZE = 768,
	/*
	 * Offsets and lengths of various fields in an Intel HEX8 record.
	 */
	IHEX_COUNT_OFFSET = 1,
	IHEX_COUNT_LEN = 2,
	IHEX_ADDRESS_OFFSET = 3,
	IHEX_ADDRESS_LEN = 4,
	IHEX_TYPE_OFFSET = 7,
	IHEX_TYPE_LEN = 2,
	IHEX_DATA_OFFSET = 9,
	IHEX_CHECKSUM_LEN = 2,
	IHEX_MAX_DATA_LEN = 512,
	/*
	 * Ascii hex encoded length of a single byte.
	 */
	IHEX_ASCII_HEX_BYTE_LEN = 2,
	/*
	 * Start code offset and value
	 */
	IHEX_START_CODE_OFFSET = 0,
	IHEX_START_CODE = ':'
};

/*
 * All possible error codes the Intel HEX8 record utility functions may return.
 */
enum IHexErrors {
	/*
	 * Error code for success or no error.
	 */
	IHEX_OK = 0,
	/*
	 * Error code for error while reading from or writing to a file.
	 * You may check errno for the exact error if this error code is
	 * encountered.
	 */
	IHEX_ERROR_FILE = -1,
	/*
	 * Error code for encountering end-of-file when reading from a file.
	 */
	IHEX_ERROR_EOF = -2,
	/*
	 * Error code for error if an invalid record was read.
	 */
	IHEX_ERROR_INVALID_RECORD = -3,
	/*
	 * Error code for error from invalid arguments passed to function.
	 */
	IHEX_ERROR_INVALID_ARGUMENTS = -4,
	/*
	 * Error code for encountering a newline with no record when
	 * reading from a file.
	 */
	IHEX_ERROR_NEWLINE = -5
};

/*
 * Intel HEX8 Record Types 00-05
 */
enum IHexRecordTypes {
	IHEX_TYPE_00 = 0, /**< Data Record */
	IHEX_TYPE_01, /**< End of File Record */
	IHEX_TYPE_02, /**< Extended Segment Address Record */
	IHEX_TYPE_03, /**< Start Segment Address Record */
	IHEX_TYPE_04, /**< Extended Linear Address Record */
	IHEX_TYPE_05  /**< Start Linear Address Record */
};

/*
 * Structure to hold the fields of an Intel HEX8 record.
 */
typedef struct {
	/* The 16-bit address field. */
	unsigned int address;
	/* The 8-bit array data field, which has a maximum size of 256 bytes. */
	unsigned char data[IHEX_MAX_DATA_LEN/2];
	/* The number of bytes of data stored in this record. */
	unsigned int dataLen;
	/* The Intel HEX8 record type of this record. */
	unsigned int type;
	/* The checksum of this record. */
	unsigned char checksum;
} IHexRecord;

/**
 * Sets all of the record fields of an Intel HEX8 record structure.
 *
 * \param type The Intel HEX8 record type (integer value of 0 through 5).
 * \param address The 16-bit address of the data.
 * \param data A point to the 8-bit array of data.
 * \param dataLen The size of the 8-bit data array.
 * \param ihexRecord A pointer to the target Intel HEX8 record structure
 *	  where these fields will be set.
 * \return IHEX_OK on success, otherwise one of the IHEX_ERROR_ error codes.
 * \retval IHEX_OK on success.
 * \retval IHEX_ERROR_INVALID_ARGUMENTS if the record pointer is NULL, or
 *	   if the length of the 8-bit data array is out of range
 *	   (less than zero or greater than the maximum data length allowed
 *	   by record specifications, see IHexRecord.data).
*/
int New_IHexRecord(unsigned int type, unsigned int address,
                   const unsigned char *data, unsigned int dataLen,
                   IHexRecord *ihexRecord);

/**
 * Reads an Intel HEX8 record from an opened file.
 *
 * \param ihexRecord A pointer to the Intel HEX8 record structure that
 *	  will store the read record.
 * \param in A file pointer to an opened file that can be read.
 * \return IHEX_OK on success, otherwise one of the IHEX_ERROR_ error codes.
 * \retval IHEX_OK on success.
 * \retval IHEX_ERROR_INVALID_ARGUMENTS if the record pointer or file
 *	   pointer is NULL.
 * \retval IHEX_ERROR_EOF if end-of-file has been reached.
 * \retval IHEX_ERROR_FILE if a file reading error has occured.
 * \retval IHEX_INVALID_RECORD if the record read is invalid
 *	   (record did not match specifications or record checksum
 *	   was invalid).
*/
int Read_IHexRecord(IHexRecord *ihexRecord, FILE *in);

/**
 * Writes an Intel HEX8 record to an opened file.
 *
 * \param ihexRecord A pointer to the Intel HEX8 record structure.
 * \param out A file pointer to an opened file that can be written to.
 * \return IHEX_OK on success, otherwise one of the IHEX_ERROR_ error codes.
 * \retval IHEX_OK on success.
 * \retval IHEX_ERROR_INVALID_ARGUMENTS if the record pointer or file
 *	   pointer is NULL.
 * \retval IHEX_ERROR_INVALID_RECORD if the record's data length is
 *	   out of range (greater than the maximum data length allowed by
 *	   record specifications, see IHexRecord.data).
 * \retval IHEX_ERROR_FILE if a file writing error has occured.
*/
int Write_IHexRecord(const IHexRecord *ihexRecord, FILE *out);

/**
 * Prints the contents of an Intel HEX8 record structure to stdout.
 * The record dump consists of the type, address, entire data array,
 * and checksum fields of the record.
 *
 * \param ihexRecord A pointer to the Intel HEX8 record structure.
 * \return Always returns IHEX_OK (success).
 * \retval IHEX_OK on success.
*/
void Print_IHexRecord(const IHexRecord *ihexRecord);

/**
 * Calculates the checksum of an Intel HEX8 IHexRecord structure.
 * See the Intel HEX8 specifications for more details on the checksum
 * calculation.
 *
 * \param ihexRecord A pointer to the Intel HEX8 record structure.
 * \return The 8-bit checksum.
*/
unsigned char Checksum_IHexRecord(const IHexRecord *ihexRecord);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_TOOLS_INTEL_HEX_H_ */
