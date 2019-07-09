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

/* Copyright (c) 2010, Joerg Wunsch
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

/* $Id$ */

/* avr/cpufunc.h - Special CPU functions */

#ifndef _AVR_CPUFUNC_H_
#define _AVR_CPUFUNC_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

/** \file */
/** \defgroup avr_cpufunc <avr/cpufunc.h>: Special AVR CPU functions
    \code #include <avr/cpufunc.h> \endcode

    This header file contains macros that access special functions of
    the AVR CPU which do not fit into any of the other header files.

*/

#if defined(__DOXYGEN__)
/**
   \ingroup avr_cpufunc
   \def _NOP

   Execute a <i>no operation</i> (NOP) CPU instruction.  This
   should not be used to implement delays, better use the functions
   from <util/delay_basic.h> or <util/delay.h> for this.  For
   debugging purposes, a NOP can be useful to have an instruction that
   is guaranteed to be not optimized away by the compiler, so it can
   always become a breakpoint in the debugger.
*/
#define _NOP()
#else  /* real code */
#define _NOP() __asm__ __volatile__("nop")
#endif  /* __DOXYGEN__ */

#if defined(__DOXYGEN__)
/**
   \ingroup avr_cpufunc
   \def _MemoryBarrier

   Implement a read/write <i>memory barrier</i>.  A memory
   barrier instructs the compiler to not cache any memory data in
   registers beyond the barrier.  This can sometimes be more effective
   than blocking certain optimizations by declaring some object with a
   \c volatile qualifier.

   See \ref optim_code_reorder for things to be taken into account
   with respect to compiler optimizations.
*/
#define _MemoryBarrier()
#else  /* real code */
#define _MemoryBarrier() __asm__ __volatile__("":::"memory")
#endif  /* __DOXYGEN__ */

#ifdef __cplusplus
}
#endif

#endif /* _AVR_CPUFUNC_H_ */
