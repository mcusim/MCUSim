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

/* MCUSim logging functions. */
#ifndef MSIM_LOG_H_
#define MSIM_LOG_H_ 1

#include <stdint.h>
#include <string.h>

/* Macro to denote a filename separator. */
#define __MSIM_FS_SEP__ '/'

/* Standard separator for Windows operating system. */
#if defined(_WIN64) || defined(_WIN32) || defined(__WIN32__)
	#undef __MSIM_FS_SEP__
	#define __MSIM_FS_SEP__ '\\'
#endif

/* Windows can be with the different POSIX flavors also. */
#if defined(__CYGWIN__) || defined(__MINGW32__)
	#undef __MSIM_FS_SEP__
	#define __MSIM_FS_SEP__ '/'
#endif

/* Let filename to be defined if it doesn't exist yet. */
#ifndef __MSIM_FILENAME__
#define __MSIM_FILENAME__ ((strrchr(__FILE__,__MSIM_FS_SEP__)==NULL)	\
                      ?(__FILE__):(strrchr(__FILE__,__MSIM_FS_SEP__)+1))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Logging macros. This is a preferred way to log anything. */
#define MSIM_LOG_FATAL(msg) MSIM_LOG_Log(MSIM_LOG_LVLFATAL,		\
                "(!!)", __MSIM_FILENAME__, __LINE__, msg);

#define MSIM_LOG_ERROR(msg) MSIM_LOG_Log(MSIM_LOG_LVLERROR,		\
                "(EE)", __MSIM_FILENAME__, __LINE__, msg);

#define MSIM_LOG_WARN(msg) MSIM_LOG_Log(MSIM_LOG_LVLWARNING,		\
                "(WW)", __MSIM_FILENAME__, __LINE__, msg);

#define MSIM_LOG_INFO(msg) MSIM_LOG_Log(MSIM_LOG_LVLINFO,		\
                "(--)", __MSIM_FILENAME__, __LINE__, msg);

#if defined(DEBUG)
/* Print debug messages if DEBUG is defined only. */
#define MSIM_LOG_DEBUG(msg) MSIM_LOG_Log(MSIM_LOG_LVLDEBUG,		\
                "(DD)", __MSIM_FILENAME__, __LINE__, msg);
#else
/* Print nothing otherwise. */
#define MSIM_LOG_DEBUG(msg)
#endif

/* Test logging level macros. This is a preferred way to check a current
 * logging level. */
#define MSIM_LOG_ISFATAL ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLFATAL))
#define MSIM_LOG_ISERROR ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLERROR))
#define MSIM_LOG_ISWARN ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLWARNING))
#define MSIM_LOG_ISINFO ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLINFO))
#define MSIM_LOG_ISDEBUG ((uint8_t)(MSIM_LOG_GetLevel()>=MSIM_LOG_LVLDEBUG))

enum MSIM_LOG_Level {
	MSIM_LOG_LVLNONE = 74,		/* No logging */
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

void MSIM_LOG_PrintMarkers(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_LOG_H_ */
