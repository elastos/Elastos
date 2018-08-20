/*
 * Copyright (c) 2017-2018 iwhisper.io
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

#ifndef __SPOPEN_H__
#define __SPOPEN_H__

#include <stdio.h>

#include <crystal_config.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _subprocess;
typedef struct _subprocess *subprocess_t;

//CRYSTAL_API
subprocess_t spopen(const char *command, const char *mode);

//CRYSTAL_API
int spclose(subprocess_t subprocess);

//CRYSTAL_API
FILE *spstdin(subprocess_t subprocess);

//CRYSTAL_API
FILE *spstdout(subprocess_t subprocess);

//CRYSTAL_API
FILE *spstderr(subprocess_t subprocess);

//CRYSTAL_API
int spid(subprocess_t subprocess);

//CRYSTAL_API
int spkill(subprocess_t subprocess);

#ifdef __cplusplus
}
#endif

#endif /* __SPOPEN_H__ */
