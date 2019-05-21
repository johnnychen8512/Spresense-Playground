/****************************************************************************
 * examples/voice_changer/resource.h
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __EXAMPLES_CHG_RESOURCE_H
#define __EXAMPLES_CHG_RESOURCE_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* These IDs are sharing with dsp */

#define DSP_MQID   10
#define DSP_MQID2  11

/* Determine which is RPC, they are magic numbers. */

#define DSP_RPC_MSG         30
#define DSP_RPC_MSG_ASYNC   31
#define DSP_RPC_RET         40
#define DSP_RPC_UNDEF       41
#define DSP_RPC_UNLOAD      90

/* DSP offload function hashes.
 * These hashes are generated by following commands (by hands!).
 *
 * echo <define name> | sha1sum - | cut -b -8
 *
 * e.g. $ echo DSP_FFT_SYNC | sha1sum - | cut -b -8
 */

#define DSP_CHG_REQUEST             0x70a53d38
#define DSP_CHANGER_INIT            0x30f58304
#define DSP_CHANGER_INPUT           0x66fb6c98
#define DSP_FFT_WINDOW              0xe9586243

#define FFT_WIN_RECTANGLE           0x00000000
#define FFT_WIN_HAMMING             0x00000001
#define FFT_WIN_HANNING             0x00000002

#define FFT_ATTR_NOTIFY             0x80000000

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fft_desc fft_desc_t;
struct _fft_desc
{
  fft_desc_t    *pNext;
  void          *pSrc;
  void          *pDst;
  uint32_t      fftLen;
  uint32_t      attribute;
};

typedef enum _fft_stream_type {
  TYPE_FLOAT,
  TYPE_SHORT,
} fft_stream_type_t;

#ifdef __cplusplus
}
#endif

#endif /* __EXAMPLES_FFT_RESOURCE_H */
