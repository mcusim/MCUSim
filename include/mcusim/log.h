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
 * MCUSim logging functions.
 */
#ifndef MSIM_LOG_H_
#define MSIM_LOG_H_ 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Logging macros. This is a preferred way to log anything. */
#define MSIM_LOG_FATAL(msg) MSIM_LOG_Log(MSIM_LOG_LVLFATAL,		\
                "fatal", __FILENAME__, __LINE__, msg);

#define MSIM_LOG_ERROR(msg) MSIM_LOG_Log(MSIM_LOG_LVLERROR,		\
                "error", __FILENAME__, __LINE__, msg);

#define MSIM_LOG_WARN(msg) MSIM_LOG_Log(MSIM_LOG_LVLWARNING,		\
                "warning", __FILENAME__, __LINE__, msg);

#define MSIM_LOG_INFO(msg) MSIM_LOG_Log(MSIM_LOG_LVLINFO,		\
                "info", __FILENAME__, __LINE__, msg);

#ifdef DEBUG
#define MSIM_LOG_DEBUG(msg) MSIM_LOG_Log(MSIM_LOG_LVLDEBUG,		\
                "debug", __FILENAME__, __LINE__, msg);
#else /* DEBUG is not defined */
#define MSIM_LOG_DEBUG(msg)
#endif /* DEBUG */

/* Test logging level macros. This is a preferred way to check a current
 * logging level. */
#define MSIM_LOG_ISFATAL ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLFATAL))
#define MSIM_LOG_ISERROR ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLERROR))
#define MSIM_LOG_ISWARN ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLWARNING))
#define MSIM_LOG_ISINFO ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLINFO))
#define MSIM_LOG_ISDEBUG ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLDEBUG))

enum MSIM_LOG_Level {
	MSIM_LOG_LVLFATAL = 75,		/* Less informative logging level */
	MSIM_LOG_LVLERROR = 76,
	MSIM_LOG_LVLWARNING = 77,
	MSIM_LOG_LVLINFO = 78,
	MSIM_LOG_LVLDEBUG = 79		/* Most informative logging level */
};

/* MCUSim logging function. */
void MSIM_LOG_Log(enum MSIM_LOG_Level lvl, const char *lvlmsg,
                  const char *file, uint32_t line, const char *msg);

/* Access logging level functions. */
void MSIM_LOG_SetLevel(enum MSIM_LOG_Level level);
enum MSIM_LOG_Level MSIM_LOG_GetLevel(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_LOG_H_ */
