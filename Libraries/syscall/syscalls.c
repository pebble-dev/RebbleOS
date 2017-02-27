/****************************************************************************
 *  Copyright (c) 2009 by Michael Fischer. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the author nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 ****************************************************************************
 *  History:
 *
 *  28.03.09  mifi   First Version, based on the original syscall.c from
 *                   newlib version 1.17.0
 ****************************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "stdio.h"
#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"

#define DEBUG_USART3 USART3
//#define DEBUG_USART8 UART8

/***************************************************************************/

int _open(const char *name, int flags, int mode){
  return -1;
}

int _read(int file, char * ptr, int len) {
  ptr = ptr;
  len = len;
  errno = EINVAL;
  return -1;
}

/***************************************************************************/

int _lseek(int file, int ptr, int dir) {
  file = file;
  ptr = ptr;
  dir = dir;
  return 0;
}

/***************************************************************************/

int _write(int file, char * ptr, int len) {
  int index;
  if (!ptr) {
    return 0;
  }
  for (index = 0; index < len; index++) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    while (!(DEBUG_USART3->SR & 0x00000040));
    USART_SendData(DEBUG_USART3, ptr[index]);
    
#if defined(DEBUG_USART8)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
    while (!(DEBUG_USART8->SR & 0x00000040));
    USART_SendData(DEBUG_USART8, ptr[index]);
#endif
  }
  return len;
}

/***************************************************************************/

int _close(int file) {
  return 0;
}

/***************************************************************************/

caddr_t _sbrk(int incr) {
  extern char   _end; // Defined by the linker.
  extern char _ram_top;
  static void *heap_end;
  void *prev_heap_end;
  if (heap_end == NULL)
    heap_end = &_end;
  prev_heap_end = heap_end;
  if (heap_end + incr > &_ram_top) {
    // Some of the libstdc++-v3 tests rely upon detecting
    // out of memory errors, so do not abort here.
#if 0
    extern void abort (void);
    _write (1, "_sbrk: Heap and stack collision\n", 32);
    abort ();
#else
    errno = ENOMEM;
    return (caddr_t) -1;
#endif
  }
  
  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

/***************************************************************************/

int _fstat(int file, struct stat * st) {
  file = file;
  memset (st, 0, sizeof (* st));
  st->st_mode = S_IFCHR;
  return 0;
}

/***************************************************************************/

int _isatty(int fd) {
  fd = fd;
  return 1;
}

int _kill(int pid, int sig) {
  errno=EINVAL;
  return(-1);
}

int _getpid(void) {
  return 1;
}

/*** EOF ***/
