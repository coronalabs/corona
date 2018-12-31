/*
 * SDL_sound -- An abstract sound format decoding API.
 * Copyright (C) 2001  Ryan C. Gordon.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 Attention: This is a stripped down file of SDL_endian for our purposes.
 This code is licensed under the LGPL.
 This means we must not compile this code into anything that we are not willing to
 publicly release source code.
 You should compile this into a separate dynamic library that is isolated from proprietary code.
 */

/*
 * WAV decoder for SDL_sound.
 *
 * This driver handles Microsoft .WAVs, in as many of the thousands of
 *  variations as we can.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon. (icculus@icculus.org)
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef SOUND_SUPPORTS_WAV

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
#include "SDL_sound.h"

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"
*/

#include "SoundDecoder.h"

#include "SoundDecoder_Internal.h"
#include "SDL_endian_minimal.h"
#include "ALmixer_RWops.h"

#define ERR_IO_ERROR "I/O error"
#define assert(x)

extern uint32_t __Sound_convertMsToBytePos(SoundDecoder_AudioInfo*, uint32_t);

static int WAV_init(void);
static void WAV_quit(void);
static int WAV_open(Sound_Sample *sample, const char *ext);
static void WAV_close(Sound_Sample *sample);
static uint32_t WAV_read(Sound_Sample *sample);
static int WAV_rewind(Sound_Sample *sample);
static int WAV_seek(Sound_Sample *sample, uint32_t ms);

static const char *extensions_wav[] = { "WAV", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_WAV =
{
    {
        extensions_wav,
        "Microsoft WAVE audio format",
        "Ryan C. Gordon <icculus@icculus.org>",
        "http://www.icculus.org/SDL_sound/"
    },

    WAV_init,       /*   init() method */
    WAV_quit,       /*   quit() method */
    WAV_open,       /*   open() method */
    WAV_close,      /*  close() method */
    WAV_read,       /*   read() method */
    WAV_rewind,     /* rewind() method */
    WAV_seek        /*   seek() method */
};


/* Better than SDL_ReadLE16, since you can detect i/o errors... */
static __inline__ int read_le16(ALmixer_RWops *rw, uint16_t *ui16)
{
    int rc = ALmixer_RWread(rw, ui16, sizeof (uint16_t), 1);
    BAIL_IF_MACRO(rc != 1, ERR_IO_ERROR, 0);
    *ui16 = SDL_SwapLE16(*ui16);
    return(1);
} /* read_le16 */


/* Better than SDL_ReadLE32, since you can detect i/o errors... */
static __inline__ int read_le32(ALmixer_RWops *rw, uint32_t *ui32)
{
    int rc = ALmixer_RWread(rw, ui32, sizeof (uint32_t), 1);
    BAIL_IF_MACRO(rc != 1, ERR_IO_ERROR, 0);
    *ui32 = SDL_SwapLE32(*ui32);
    return(1);
} /* read_le32 */


/* This is just cleaner on the caller's end... */
static __inline__ int read_uint8_t(ALmixer_RWops *rw, uint8_t *ui8)
{
    int rc = ALmixer_RWread(rw, ui8, sizeof (uint8_t), 1);
    BAIL_IF_MACRO(rc != 1, ERR_IO_ERROR, 0);
    return(1);
} /* read_uint8_t */


static __inline__ uint16_t SDL_ReadLE16( ALmixer_RWops *rw )
{
	uint16_t result = 0;

	int rc = read_le16( rw, &result );

	return result;
}
static __inline__ uint32_t SDL_ReadLE32( ALmixer_RWops *rw )
{
	uint32_t result = 0;

	int rc = read_le32( rw, &result );

	return result;
}

    /* Chunk management code... */

#define riffID 0x46464952  /* "RIFF", in ascii. */
#define waveID 0x45564157  /* "WAVE", in ascii. */
#define factID 0x74636166  /* "fact", in ascii. */


/*****************************************************************************
 * The FORMAT chunk...                                                       *
 *****************************************************************************/

#define fmtID  0x20746D66  /* "fmt ", in ascii. */

#define FMT_NORMAL 0x0001    /* Uncompressed waveform data.     */
#define FMT_ADPCM  0x0002    /* ADPCM compressed waveform data. */

typedef struct
{
    int16_t iCoef1;
    int16_t iCoef2;
} ADPCMCOEFSET;

typedef struct
{
    uint8_t bPredictor;
    uint16_t iDelta;
    int16_t iSamp1;
    int16_t iSamp2;
} ADPCMBLOCKHEADER;

typedef struct S_WAV_FMT_T
{
    uint32_t chunkID;
    int32_t chunkSize;
    int16_t wFormatTag;
    uint16_t wChannels;
    uint32_t dwSamplesPerSec;
    uint32_t dwAvgBytesPerSec;
    uint16_t wBlockAlign;
    uint16_t wBitsPerSample;

    uint32_t next_chunk_offset;

    uint32_t sample_frame_size;
    uint32_t data_starting_offset;
    uint32_t total_bytes;

    void (*free)(struct S_WAV_FMT_T *fmt);
    uint32_t (*read_sample)(Sound_Sample *sample);
    int (*rewind_sample)(Sound_Sample *sample);
    int (*seek_sample)(Sound_Sample *sample, uint32_t ms);

    union
    {
        struct
        {
            uint16_t cbSize;
            uint16_t wSamplesPerBlock;
            uint16_t wNumCoef;
            ADPCMCOEFSET *aCoef;
            ADPCMBLOCKHEADER *blockheaders;
            uint32_t samples_left_in_block;
            int nibble_state;
            int8_t nibble;
        } adpcm;

        /* put other format-specific data here... */
    } fmt;
} fmt_t;


/*
 * Read in a fmt_t from disk. This makes this process safe regardless of
 *  the processor's byte order or how the fmt_t structure is packed.
 * Note that the union "fmt" is not read in here; that is handled as
 *  needed in the read_fmt_* functions.
 */
static int read_fmt_chunk(ALmixer_RWops *rw, fmt_t *fmt)
{
    /* skip reading the chunk ID, since it was already read at this point... */
    fmt->chunkID = fmtID;

    BAIL_IF_MACRO(!read_le32(rw, (uint32_t*)&fmt->chunkSize), NULL, 0);
    BAIL_IF_MACRO(fmt->chunkSize < 16, "WAV: Invalid chunk size", 0);
    fmt->next_chunk_offset = ALmixer_RWtell(rw) + fmt->chunkSize;

    BAIL_IF_MACRO(!read_le16(rw, (uint16_t*)&fmt->wFormatTag), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->wChannels), NULL, 0);
    BAIL_IF_MACRO(!read_le32(rw, &fmt->dwSamplesPerSec), NULL, 0);
    BAIL_IF_MACRO(!read_le32(rw, &fmt->dwAvgBytesPerSec), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->wBlockAlign), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->wBitsPerSample), NULL, 0);

    return(1);
} /* read_fmt_chunk */



/*****************************************************************************
 * The DATA chunk...                                                         *
 *****************************************************************************/

#define dataID 0x61746164  /* "data", in ascii. */

typedef struct
{
    uint32_t chunkID;

	/* Johnson Lin wanted to clean up compiler warnings on Windows/CodeBlocks. 
	 * This was originally a signed int32_t. The code usage and intent seems to imply that it should be a uint32_t.
	 */
    /* int32_t chunkSize; */
    uint32_t chunkSize;
    /* Then, (chunkSize) bytes of waveform data... */
} data_t;


/*
 * Read in a data_t from disk. This makes this process safe regardless of
 *  the processor's byte order or how the fmt_t structure is packed.
 */
static int read_data_chunk(ALmixer_RWops *rw, data_t *data)
{
    /* skip reading the chunk ID, since it was already read at this point... */
    data->chunkID = dataID;
    BAIL_IF_MACRO(!read_le32(rw, &data->chunkSize), NULL, 0);
    return(1);
} /* read_data_chunk */




/*****************************************************************************
 * this is what we store in our internal->decoder_private field...           *
 *****************************************************************************/

typedef struct
{
    fmt_t *fmt;
    int32_t bytesLeft;
} wav_t;




/*****************************************************************************
 * Normal, uncompressed waveform handler...                                  *
 *****************************************************************************/

/*
 * Sound_Decode() lands here for uncompressed WAVs...
 */
static uint32_t read_sample_fmt_normal(Sound_Sample *sample)
{
    uint32_t retval;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    uint32_t max = (internal->buffer_size < (uint32_t) w->bytesLeft) ?
                    internal->buffer_size : (uint32_t) w->bytesLeft;

    assert(max > 0);

        /*
         * We don't actually do any decoding, so we read the wav data
         *  directly into the internal buffer...
         */
    retval = ALmixer_RWread(internal->rw, internal->buffer, 1, max);

    w->bytesLeft -= retval;

        /* Make sure the read went smoothly... */
    if ((retval == 0) || (w->bytesLeft == 0))
        sample->flags |= SOUND_SAMPLEFLAG_EOF;

    else if (retval == -1)
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;

        /* (next call this EAGAIN may turn into an EOF or error.) */
    else if (retval < internal->buffer_size)
        sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;

    return(retval);
} /* read_sample_fmt_normal */


static int seek_sample_fmt_normal(Sound_Sample *sample, uint32_t ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    int offset = __Sound_convertMsToBytePos(&sample->actual, ms);
    int pos = (int) (fmt->data_starting_offset + offset);
    int rc = ALmixer_RWseek(internal->rw, pos, SEEK_SET);
    BAIL_IF_MACRO(rc != pos, ERR_IO_ERROR, 0);
    w->bytesLeft = fmt->total_bytes - offset;
    return(1);  /* success. */
} /* seek_sample_fmt_normal */


static int rewind_sample_fmt_normal(Sound_Sample *sample)
{
    /* no-op. */
    return(1);
} /* rewind_sample_fmt_normal */


static int read_fmt_normal(ALmixer_RWops *rw, fmt_t *fmt)
{
    /* (don't need to read more from the RWops...) */
    fmt->free = NULL;
    fmt->read_sample = read_sample_fmt_normal;
    fmt->rewind_sample = rewind_sample_fmt_normal;
    fmt->seek_sample = seek_sample_fmt_normal;
    return(1);
} /* read_fmt_normal */



/*****************************************************************************
 * ADPCM compression handler...                                              *
 *****************************************************************************/

#define FIXED_POINT_COEF_BASE      256
#define FIXED_POINT_ADAPTION_BASE  256
#define SMALLEST_ADPCM_DELTA       16


static __inline__ int read_adpcm_block_headers(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    ALmixer_RWops *rw = internal->rw;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    ADPCMBLOCKHEADER *headers = fmt->fmt.adpcm.blockheaders;
    int i;
    int max = fmt->wChannels;

    if (w->bytesLeft < fmt->wBlockAlign)
    {
        sample->flags |= SOUND_SAMPLEFLAG_EOF;
        return(0);
    } /* if */

    w->bytesLeft -= fmt->wBlockAlign;

    for (i = 0; i < max; i++)
        BAIL_IF_MACRO(!read_uint8_t(rw, &headers[i].bPredictor), NULL, 0);

    for (i = 0; i < max; i++)
        BAIL_IF_MACRO(!read_le16(rw, &headers[i].iDelta), NULL, 0);

    for (i = 0; i < max; i++)
        BAIL_IF_MACRO(!read_le16(rw, (uint16_t*)&headers[i].iSamp1), NULL, 0);

    for (i = 0; i < max; i++)
        BAIL_IF_MACRO(!read_le16(rw, (uint16_t*)&headers[i].iSamp2), NULL, 0);

    fmt->fmt.adpcm.samples_left_in_block = fmt->fmt.adpcm.wSamplesPerBlock;
    fmt->fmt.adpcm.nibble_state = 0;
    return(1);
} /* read_adpcm_block_headers */


static __inline__ void do_adpcm_nibble(uint8_t nib,
                                       ADPCMBLOCKHEADER *header,
                                       int32_t lPredSamp)
{
	static const int32_t max_audioval = ((1<<(16-1))-1);
	static const int32_t min_audioval = -(1<<(16-1));
	static const int32_t AdaptionTable[] =
    {
		230, 230, 230, 230, 307, 409, 512, 614,
		768, 614, 512, 409, 307, 230, 230, 230
	};

    int32_t lNewSamp;
    int32_t delta;

    if (nib & 0x08)
        lNewSamp = lPredSamp + (header->iDelta * (nib - 0x10));
	else
        lNewSamp = lPredSamp + (header->iDelta * nib);

        /* clamp value... */
    if (lNewSamp < min_audioval)
        lNewSamp = min_audioval;
    else if (lNewSamp > max_audioval)
        lNewSamp = max_audioval;

    delta = ((int32_t) header->iDelta * AdaptionTable[nib]) /
              FIXED_POINT_ADAPTION_BASE;

	if (delta < SMALLEST_ADPCM_DELTA)
	    delta = SMALLEST_ADPCM_DELTA;

    header->iDelta = delta;
	header->iSamp2 = header->iSamp1;
	header->iSamp1 = lNewSamp;
} /* do_adpcm_nibble */


static __inline__ int decode_adpcm_sample_frame(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    ADPCMBLOCKHEADER *headers = fmt->fmt.adpcm.blockheaders;
    ALmixer_RWops *rw = internal->rw;
    int i;
    int max = fmt->wChannels;
    uint8_t nib = fmt->fmt.adpcm.nibble;

    for (i = 0; i < max; i++)
    {
        int16_t iCoef1 = fmt->fmt.adpcm.aCoef[headers[i].bPredictor].iCoef1;
        int16_t iCoef2 = fmt->fmt.adpcm.aCoef[headers[i].bPredictor].iCoef2;
        int32_t lPredSamp = ((headers[i].iSamp1 * iCoef1) +
                            (headers[i].iSamp2 * iCoef2)) /
                             FIXED_POINT_COEF_BASE;

        if (fmt->fmt.adpcm.nibble_state == 0)
        {
            BAIL_IF_MACRO(!read_uint8_t(rw, &nib), NULL, 0);
            fmt->fmt.adpcm.nibble_state = 1;
            do_adpcm_nibble(nib >> 4, &headers[i], lPredSamp);
        } /* if */
        else
        {
            fmt->fmt.adpcm.nibble_state = 0;
            do_adpcm_nibble(nib & 0x0F, &headers[i], lPredSamp);
        } /* else */
    } /* for */

    fmt->fmt.adpcm.nibble = nib;
    return(1);
} /* decode_adpcm_sample_frame */


static __inline__ void put_adpcm_sample_frame1(void *_buf, fmt_t *fmt)
{
    uint16_t *buf = (uint16_t *) _buf;
    ADPCMBLOCKHEADER *headers = fmt->fmt.adpcm.blockheaders;
    int i;
    for (i = 0; i < fmt->wChannels; i++)
        *(buf++) = headers[i].iSamp1;
} /* put_adpcm_sample_frame1 */


static __inline__ void put_adpcm_sample_frame2(void *_buf, fmt_t *fmt)
{
    uint16_t *buf = (uint16_t *) _buf;
    ADPCMBLOCKHEADER *headers = fmt->fmt.adpcm.blockheaders;
    int i;
    for (i = 0; i < fmt->wChannels; i++)
        *(buf++) = headers[i].iSamp2;
} /* put_adpcm_sample_frame2 */


/*
 * Sound_Decode() lands here for ADPCM-encoded WAVs...
 */
static uint32_t read_sample_fmt_adpcm(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    uint32_t bw = 0;

    while (bw < internal->buffer_size)
    {
        /* write ongoing sample frame before reading more data... */
        switch (fmt->fmt.adpcm.samples_left_in_block)
        {
            case 0:  /* need to read a new block... */
                if (!read_adpcm_block_headers(sample))
                {
                    if ((sample->flags & SOUND_SAMPLEFLAG_EOF) == 0)
                        sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                    return(bw);
                } /* if */

                /* only write first sample frame for now. */
                put_adpcm_sample_frame2((uint8_t *) internal->buffer + bw, fmt);
                fmt->fmt.adpcm.samples_left_in_block--;
                bw += fmt->sample_frame_size;
                break;

            case 1:  /* output last sample frame of block... */
                put_adpcm_sample_frame1((uint8_t *) internal->buffer + bw, fmt);
                fmt->fmt.adpcm.samples_left_in_block--;
                bw += fmt->sample_frame_size;
                break;

            default: /* output latest sample frame and read a new one... */
                put_adpcm_sample_frame1((uint8_t *) internal->buffer + bw, fmt);
                fmt->fmt.adpcm.samples_left_in_block--;
                bw += fmt->sample_frame_size;

                if (!decode_adpcm_sample_frame(sample))
                {
                    sample->flags |= SOUND_SAMPLEFLAG_ERROR;
                    return(bw);
                } /* if */
        } /* switch */
    } /* while */

    return(bw);
} /* read_sample_fmt_adpcm */


/*
 * Sound_FreeSample() lands here for ADPCM-encoded WAVs...
 */
static void free_fmt_adpcm(fmt_t *fmt)
{
    if (fmt->fmt.adpcm.aCoef != NULL)
        free(fmt->fmt.adpcm.aCoef);

    if (fmt->fmt.adpcm.blockheaders != NULL)
        free(fmt->fmt.adpcm.blockheaders);
} /* free_fmt_adpcm */


static int rewind_sample_fmt_adpcm(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    w->fmt->fmt.adpcm.samples_left_in_block = 0;
    return(1);
} /* rewind_sample_fmt_adpcm */


static int seek_sample_fmt_adpcm(Sound_Sample *sample, uint32_t ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    uint32_t origsampsleft = fmt->fmt.adpcm.samples_left_in_block;
    int origpos = ALmixer_RWtell(internal->rw);
    int offset = __Sound_convertMsToBytePos(&sample->actual, ms);
    int bpb = (fmt->fmt.adpcm.wSamplesPerBlock * fmt->sample_frame_size);
    int skipsize = (offset / bpb) * fmt->wBlockAlign;
    int pos = skipsize + fmt->data_starting_offset;
    int rc = ALmixer_RWseek(internal->rw, pos, SEEK_SET);
    BAIL_IF_MACRO(rc != pos, ERR_IO_ERROR, 0);

    /* The offset we need is in this block, so we need to decode to there. */
    skipsize += (offset % bpb);
    rc = (offset % bpb);  /* bytes into this block we need to decode */
    if (!read_adpcm_block_headers(sample))
    {
        ALmixer_RWseek(internal->rw, origpos, SEEK_SET);  /* try to make sane. */
        return(0);
    } /* if */

    /* first sample frame of block is a freebie. :) */
    fmt->fmt.adpcm.samples_left_in_block--;
    rc -= fmt->sample_frame_size;
    while (rc > 0)
    {
        if (!decode_adpcm_sample_frame(sample))
        {
            ALmixer_RWseek(internal->rw, origpos, SEEK_SET);
            fmt->fmt.adpcm.samples_left_in_block = origsampsleft;
            return(0);
        } /* if */

        fmt->fmt.adpcm.samples_left_in_block--;
        rc -= fmt->sample_frame_size;
    } /* while */

    w->bytesLeft = fmt->total_bytes - skipsize;
    return(1);  /* success. */
} /* seek_sample_fmt_adpcm */


/*
 * Read in the adpcm-specific info from disk. This makes this process
 *  safe regardless of the processor's byte order or how the fmt_t
 *  structure is packed.
 */
static int read_fmt_adpcm(ALmixer_RWops *rw, fmt_t *fmt)
{
    size_t i;

    memset(&fmt->fmt.adpcm, '\0', sizeof (fmt->fmt.adpcm));
    fmt->free = free_fmt_adpcm;
    fmt->read_sample = read_sample_fmt_adpcm;
    fmt->rewind_sample = rewind_sample_fmt_adpcm;
    fmt->seek_sample = seek_sample_fmt_adpcm;

    BAIL_IF_MACRO(!read_le16(rw, &fmt->fmt.adpcm.cbSize), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->fmt.adpcm.wSamplesPerBlock), NULL, 0);
    BAIL_IF_MACRO(!read_le16(rw, &fmt->fmt.adpcm.wNumCoef), NULL, 0);

    /* fmt->free() is always called, so these malloc()s will be cleaned up. */

    i = sizeof (ADPCMCOEFSET) * fmt->fmt.adpcm.wNumCoef;
    fmt->fmt.adpcm.aCoef = (ADPCMCOEFSET *) malloc(i);
    BAIL_IF_MACRO(fmt->fmt.adpcm.aCoef == NULL, ERR_OUT_OF_MEMORY, 0);

    for (i = 0; i < fmt->fmt.adpcm.wNumCoef; i++)
    {
        BAIL_IF_MACRO(!read_le16(rw, (uint16_t*)&fmt->fmt.adpcm.aCoef[i].iCoef1), NULL, 0);
        BAIL_IF_MACRO(!read_le16(rw, (uint16_t*)&fmt->fmt.adpcm.aCoef[i].iCoef2), NULL, 0);
    } /* for */

    i = sizeof (ADPCMBLOCKHEADER) * fmt->wChannels;
    fmt->fmt.adpcm.blockheaders = (ADPCMBLOCKHEADER *) malloc(i);
    BAIL_IF_MACRO(fmt->fmt.adpcm.blockheaders == NULL, ERR_OUT_OF_MEMORY, 0);

    return(1);
} /* read_fmt_adpcm */



/*****************************************************************************
 * Everything else...                                                        *
 *****************************************************************************/

static int WAV_init(void)
{
    return(1);  /* always succeeds. */
} /* WAV_init */


static void WAV_quit(void)
{
    /* it's a no-op. */
} /* WAV_quit */


static int read_fmt(ALmixer_RWops *rw, fmt_t *fmt)
{
    /* if it's in this switch statement, we support the format. */
    switch (fmt->wFormatTag)
    {
        case FMT_NORMAL:
            SNDDBG(("WAV: Appears to be uncompressed audio.\n"));
            return(read_fmt_normal(rw, fmt));

        case FMT_ADPCM:
            SNDDBG(("WAV: Appears to be ADPCM compressed audio.\n"));
            return(read_fmt_adpcm(rw, fmt));

        /* add other types here. */

        default:
#ifdef ANDROID_NDK
            SNDDBG(("WAV: Format is unknown.\n"));
#else
            SNDDBG(("WAV: Format 0x%X is unknown.\n",
                    (unsigned int) fmt->wFormatTag));
#endif
            BAIL_MACRO("WAV: Unsupported format", 0);
    } /* switch */

    assert(0);  /* shouldn't hit this point. */
    return(0);
} /* read_fmt */


/*
 * Locate a specific chunk in the WAVE file by ID...
 */
static int find_chunk(ALmixer_RWops *rw, uint32_t id)
{
    uint32_t siz = 0;
    uint32_t _id = 0;
    uint32_t pos = ALmixer_RWtell(rw);

    while (1)
    {
        BAIL_IF_MACRO(!read_le32(rw, &_id), NULL, 0);
        if (_id == id)
            return(1);

            /* skip ahead and see what next chunk is... */
        BAIL_IF_MACRO(!read_le32(rw, &siz), NULL, 0);
        assert(siz >= 0);
        pos += (sizeof (uint32_t) * 2) + siz;
        if (siz > 0)
            BAIL_IF_MACRO(ALmixer_RWseek(rw, pos, SEEK_SET) != pos, NULL, 0);
    } /* while */

    return(0);  /* shouldn't hit this, but just in case... */
} /* find_chunk */


static int WAV_open_internal(Sound_Sample *sample, const char *ext, fmt_t *fmt)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    ALmixer_RWops *rw = internal->rw;
    data_t d;
    wav_t *w;

    BAIL_IF_MACRO(SDL_ReadLE32(rw) != riffID, "WAV: Not a RIFF file.", 0);
    SDL_ReadLE32(rw);  /* throw the length away; we get this info later. */
    BAIL_IF_MACRO(SDL_ReadLE32(rw) != waveID, "WAV: Not a WAVE file.", 0);
    BAIL_IF_MACRO(!find_chunk(rw, fmtID), "WAV: No format chunk.", 0);
    BAIL_IF_MACRO(!read_fmt_chunk(rw, fmt), "WAV: Can't read format chunk.", 0);

    sample->actual.channels = (uint8_t) fmt->wChannels;
    sample->actual.rate = fmt->dwSamplesPerSec;
    if ((fmt->wBitsPerSample == 4) /*|| (fmt->wBitsPerSample == 0) */ )
        sample->actual.format = AUDIO_S16SYS;
    else if (fmt->wBitsPerSample == 8)
        sample->actual.format = AUDIO_U8;
    else if (fmt->wBitsPerSample == 16)
        sample->actual.format = AUDIO_S16LSB;
    else
    {
#ifdef ANDROID_NDK
       SNDDBG(("WAV: unsupported sample size.\n"));
#else
        SNDDBG(("WAV: %d bits per sample!?\n", (int) fmt->wBitsPerSample));
#endif
        BAIL_MACRO("WAV: Unsupported sample size.", 0);
    } /* else */

    BAIL_IF_MACRO(!read_fmt(rw, fmt), NULL, 0);
    ALmixer_RWseek(rw, fmt->next_chunk_offset, SEEK_SET);
    BAIL_IF_MACRO(!find_chunk(rw, dataID), "WAV: No data chunk.", 0);
    BAIL_IF_MACRO(!read_data_chunk(rw, &d), "WAV: Can't read data chunk.", 0);

    w = (wav_t *) malloc(sizeof(wav_t));
    BAIL_IF_MACRO(w == NULL, ERR_OUT_OF_MEMORY, 0);
    w->fmt = fmt;
    fmt->total_bytes = w->bytesLeft = d.chunkSize;
    fmt->data_starting_offset = ALmixer_RWtell(rw);
    fmt->sample_frame_size = ( ((sample->actual.format & 0xFF) / 8) *
                               sample->actual.channels );
    internal->decoder_private = (void *) w;

    internal->total_time = (fmt->total_bytes / fmt->dwAvgBytesPerSec) * 1000;
    internal->total_time += (fmt->total_bytes % fmt->dwAvgBytesPerSec)
                              *  1000 / fmt->dwAvgBytesPerSec;

    sample->flags = SOUND_SAMPLEFLAG_NONE;
    if (fmt->seek_sample != NULL)
        sample->flags |= SOUND_SAMPLEFLAG_CANSEEK;

    SNDDBG(("WAV: Accepting data stream.\n"));
    return(1); /* we'll handle this data. */
} /* WAV_open_internal */


static int WAV_open(Sound_Sample *sample, const char *ext)
{
    int rc;

    fmt_t *fmt = (fmt_t *) malloc(sizeof (fmt_t));
    BAIL_IF_MACRO(fmt == NULL, ERR_OUT_OF_MEMORY, 0);
    memset(fmt, '\0', sizeof (fmt_t));

    rc = WAV_open_internal(sample, ext, fmt);
    if (!rc)
    {
        if (fmt->free != NULL)
            fmt->free(fmt);
        free(fmt);
    } /* if */

    return(rc);
} /* WAV_open */


static void WAV_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;

    if (w->fmt->free != NULL)
        w->fmt->free(w->fmt);

    free(w->fmt);
    free(w);
} /* WAV_close */


static uint32_t WAV_read(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    return(w->fmt->read_sample(sample));
} /* WAV_read */


static int WAV_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    fmt_t *fmt = w->fmt;
    int rc = ALmixer_RWseek(internal->rw, fmt->data_starting_offset, SEEK_SET);
    BAIL_IF_MACRO(rc != fmt->data_starting_offset, ERR_IO_ERROR, 0);
    w->bytesLeft = fmt->total_bytes;
    return(fmt->rewind_sample(sample));
} /* WAV_rewind */


static int WAV_seek(Sound_Sample *sample, uint32_t ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    wav_t *w = (wav_t *) internal->decoder_private;
    return(w->fmt->seek_sample(sample, ms));
} /* WAV_seek */

#endif /* SOUND_SUPPORTS_WAV */

/* end of wav.c ... */

