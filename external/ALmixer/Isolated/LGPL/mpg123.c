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
 * libmpg123 decoder for SDL_sound. This is a very lightweight MP3 decoder,
 *  which is included with the SDL_sound source, so that it doesn't rely on
 *  unnecessary external libraries.
 *
 * libmpg123 is part of mpg123, and can be found in its original
 *  form at: http://www.mpg123.org/
 *
 * Please see the file LICENSE.txt in the source's root directory. The included
 *  source code for libmpg123 falls under the LGPL, which is the same license
 *  as SDL_sound (so you can consider it a single work).
 *
 *  This file written by Ryan C. Gordon. (icculus@icculus.org)
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef SOUND_SUPPORTS_MPG123

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mpg123.h"

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
#include "SoundDecoder.h"
#include "SoundDecoder_Internal.h"
#include "SimpleMutex.h"
#include "ALmixer_RWops.h"
#include <stdint.h>
		typedef struct SimpleMutex SDL_mutex;
		typedef struct ALmixer_RWops SDL_RWops;
		#define SDL_CreateMutex SimpleMutex_CreateMutex
		#define SDL_DestroyMutex SimpleMutex_DestroyMutex
		#define SDL_LockMutex SimpleMutex_LockMutex
		#define SDL_UnlockMutex SimpleMutex_UnlockMutex
		typedef uint32_t Uint32;
		#define SDL_RWseek ALmixer_RWseek
		#define SDL_RWread ALmixer_RWread

#else
#include "SDL_sound.h"

#define __SDL_SOUND_INTERNAL__
#include "SDL_sound_internal.h"
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif


static int MPG123_init(void);
static void MPG123_quit(void);
static int MPG123_open(Sound_Sample *sample, const char *ext);
static void MPG123_close(Sound_Sample *sample);
static Uint32 MPG123_read(Sound_Sample *sample);
static int MPG123_rewind(Sound_Sample *sample);
static int MPG123_seek(Sound_Sample *sample, Uint32 ms);

/* !!! FIXME: MPEG and MPG extensions? */
static const char *extensions_mpg123[] = { "MP3", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_MPG123 =
{
    {
        extensions_mpg123,
        "MP3 decoding via internal libmpg123",
        "Ryan C. Gordon <icculus@icculus.org>",
        "http://www.icculus.org/SDL_sound/"
    },

    MPG123_init,       /*   init() method */
    MPG123_quit,       /*   quit() method */
    MPG123_open,       /*   open() method */
    MPG123_close,      /*  close() method */
    MPG123_read,       /*   read() method */
    MPG123_rewind,     /* rewind() method */
    MPG123_seek        /*   seek() method */
};


/* this is what we store in our internal->decoder_private field... */
typedef mpg123_handle mpg123_t;

static SDL_mutex *mpg123_mutex = NULL;
static int mpg123_rwops_count = 0;
static SDL_RWops **mpg123_rwops = NULL;

static void print_decoders(const char *kind, char **decoders)
{
    SNDDBG(("%s:", kind));
    if (*decoders == NULL)
	{
        SNDDBG((" [none]"));
	}
    else
    {
        do
        {
            SNDDBG((" %s", *decoders));
        } while (*(++decoders));
    } /* else */
    SNDDBG(("\n"));
} /* print_decoders */


static int MPG123_init(void)
{
    int retval = 0;
    assert(mpg123_mutex == NULL);
    if (mpg123_init() == MPG123_OK)
    {
        char **supported = mpg123_supported_decoders();
        print_decoders("ALL MPG123 DECODERS", mpg123_decoders());
        print_decoders("SUPPORTED MPG123 DECODERS", mpg123_supported_decoders());
        if ((supported != NULL) && (*supported != NULL))
        {
            mpg123_mutex = SDL_CreateMutex();
            if (mpg123_mutex != NULL)
                retval = 1;  /* at least one decoder available. */
        } /* if */
    } /* if */

    return retval;
} /* MPG123_init */


static void MPG123_quit(void)
{
    mpg123_exit();
    SDL_DestroyMutex(mpg123_mutex);
    mpg123_mutex = NULL;
    free(mpg123_rwops);
    mpg123_rwops = NULL;
    mpg123_rwops_count = 0;
} /* MPG123_quit */


/* bridge rwops reading to libmpg123 hooks. */
static ssize_t rwread(int fd, void *buf, size_t len)
{
    SDL_RWops *rw = NULL;
    SDL_LockMutex(mpg123_mutex);
    rw = mpg123_rwops[fd];
    SDL_UnlockMutex(mpg123_mutex);
    return (ssize_t) SDL_RWread(rw, buf, 1, len);
} /* rwread */


/* bridge rwops seeking to libmpg123 hooks. */
static off_t rwseek(int fd, off_t pos, int whence)
{
    SDL_RWops *rw = NULL;
    SDL_LockMutex(mpg123_mutex);
    rw = mpg123_rwops[fd];
    SDL_UnlockMutex(mpg123_mutex);
    return (off_t) SDL_RWseek(rw, pos, whence);
} /* rwseek */


static const char *set_error(mpg123_handle *mp, const int err)
{
    char buffer[128];
    const char *str = NULL;
    if ((err == MPG123_ERR) && (mp != NULL))
        str = mpg123_strerror(mp);
    else
        str = mpg123_plain_strerror(err);

    snprintf(buffer, sizeof (buffer), "MPG123: %s", str);
    __Sound_SetError(buffer);
    
    return(NULL);  /* this is for BAIL_MACRO to not try to reset the string. */
} /* set_error */


/* Make sure we are only given decoded data in a format we can handle. */
static int set_formats(mpg123_handle *mp)
{
    int rc = 0;
    const long *rates = NULL;
    size_t ratecount = 0;
    const int channels = MPG123_STEREO | MPG123_MONO;
    const int encodings = /* !!! FIXME: SDL 1.3 can do sint32 and float32.
                          MPG123_ENC_SIGNED_32 | MPG123_ENC_FLOAT_32 | */
                          MPG123_ENC_SIGNED_8 | MPG123_ENC_UNSIGNED_8 |
                          MPG123_ENC_SIGNED_16 | MPG123_ENC_UNSIGNED_16;

    mpg123_rates(&rates, &ratecount);

    rc = mpg123_format_none(mp);
    while ((ratecount--) && (rc == MPG123_OK))
        rc = mpg123_format(mp, *(rates++), channels, encodings);

    return(rc);
} /* set_formats */


static int MPG123_open(Sound_Sample *sample, const char *ext)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    mpg123_handle *mp = NULL;
    long rate = 0;
    int channels = 0;
    int fmt = 0;
    int rc = 0;
    off_t len = 0;
    int seekable = 0;
    void *ptr = NULL;
    int rwops = 0;

    /* !!! FIXME: so much tapdance because we need a pointer, not an int. */
    SDL_LockMutex(mpg123_mutex);
    for (rwops = 0; rwops < mpg123_rwops_count; rwops++)
    {
        if (mpg123_rwops[rwops] == NULL)
        break;
    } /* for */
    if (rwops < mpg123_rwops_count)
        ptr = mpg123_rwops;
    else
    {
        mpg123_rwops_count++;
        ptr = realloc(mpg123_rwops, sizeof (SDL_RWops *) * mpg123_rwops_count);
        if (ptr != NULL)
            mpg123_rwops = (SDL_RWops **) ptr;
    } /* else */
    if (ptr != NULL)
        mpg123_rwops[rwops] = internal->rw;
    SDL_UnlockMutex(mpg123_mutex);
    BAIL_IF_MACRO(ptr == NULL, ERR_OUT_OF_MEMORY, 0);

    if ((mp = mpg123_new(NULL, &rc)) == NULL)
        goto mpg123_open_failed;
    else if ((rc = set_formats(mp)) != MPG123_OK)
        goto mpg123_open_failed;
    else if ((rc = mpg123_replace_reader(mp, rwread, rwseek)) != MPG123_OK)
        goto mpg123_open_failed;
    else if ((rc = mpg123_open_fd(mp, rwops)) != MPG123_OK)
        goto mpg123_open_failed;
    else if ((rc = mpg123_scan(mp)) != MPG123_OK)
        goto mpg123_open_failed;  /* !!! FIXME: this may be wrong. */
    else if ((rc = mpg123_getformat(mp, &rate, &channels, &fmt)) != MPG123_OK)
        goto mpg123_open_failed;

    if (mpg123_seek(mp, 0, SEEK_END) >= 0)  /* can seek? */
    {
        len = mpg123_tell(mp);
        if ((rc = (int) mpg123_seek(mp, 0, SEEK_SET)) < 0)
            goto mpg123_open_failed;
        seekable = 1;
    } /* if */

    internal->decoder_private = mp;
    sample->actual.rate = rate;
    sample->actual.channels = channels;

    rc = MPG123_BAD_OUTFORMAT;  /* in case this fails... */
    if (fmt == MPG123_ENC_SIGNED_8)
        sample->actual.format = AUDIO_S8;
    else if (fmt == MPG123_ENC_UNSIGNED_8)
        sample->actual.format = AUDIO_U8;
    else if (fmt == MPG123_ENC_SIGNED_16)
        sample->actual.format = AUDIO_S16SYS;
    else if (fmt == MPG123_ENC_UNSIGNED_16)
         sample->actual.format = AUDIO_U16SYS;
    /* !!! FIXME: SDL 1.3 can do sint32 and float32 ...
    else if (fmt == MPG123_ENC_SIGNED_32)
        sample->actual.format = AUDIO_S32SYS;
    else if (fmt == MPG123_ENC_FLOAT_32)
        sample->actual.format = AUDIO_F32SYS;
    */
    else
        goto mpg123_open_failed;

    SNDDBG(("MPG123: Accepting data stream.\n"));

    sample->flags = SOUND_SAMPLEFLAG_NONE;
    internal->total_time = -1;
    if (seekable)
    {
        sample->flags |= SOUND_SAMPLEFLAG_CANSEEK;
        internal->total_time = ((len / rate) * 1000) +
                               (((len % rate) * 1000) / rate);
    } /* if */

    return(1); /* we'll handle this data. */

mpg123_open_failed:
    SDL_LockMutex(mpg123_mutex);
    mpg123_rwops[rwops] = NULL;
    if (rwops == mpg123_rwops_count)
        mpg123_rwops_count--;
    SDL_UnlockMutex(mpg123_mutex);
    set_error(mp, rc);
    mpg123_delete(mp);  /* NULL is safe. */
    return(0);
} /* MPG123_open */


static void MPG123_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    mpg123_t *mp = ((mpg123_t *) internal->decoder_private);
    int i;

    SDL_LockMutex(mpg123_mutex);
    for (i = 0; i < mpg123_rwops_count; i++)
    {
        if (mpg123_rwops[i] == internal->rw)
            mpg123_rwops[i] = NULL;
    } /* for */

    for (i = mpg123_rwops_count-1; i >= 0; i--)
    {
        if (mpg123_rwops[i] != NULL)
            break;
    } /* for */
    mpg123_rwops_count = i + 1;
    SDL_UnlockMutex(mpg123_mutex);

    mpg123_close(mp);  /* don't need this at the moment, but it's safe. */
    mpg123_delete(mp);
} /* MPG123_close */


static Uint32 MPG123_read(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    mpg123_t *mp = ((mpg123_t *) internal->decoder_private);
    size_t bw = 0;
    const int rc = mpg123_read(mp, (unsigned char *) internal->buffer,
                               internal->buffer_size, &bw);
    if (rc == MPG123_DONE)
        sample->flags |= SOUND_SAMPLEFLAG_EOF;
    else if (rc != MPG123_OK)
    {
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;
        set_error(mp, rc);
    } /* else if */

    return((Uint32) bw);
} /* MPG123_read */


static int MPG123_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    mpg123_t *mp = ((mpg123_t *) internal->decoder_private);
    const int rc = (int) mpg123_seek(mp, 0, SEEK_SET);
    BAIL_IF_MACRO(rc < 0, set_error(mp, rc), 0);
    return(1);
} /* MPG123_rewind */


static int MPG123_seek(Sound_Sample *sample, Uint32 ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    mpg123_t *mp = ((mpg123_t *) internal->decoder_private);
    const float frames_per_ms = ((float) sample->actual.rate) / 1000.0f;
    const off_t frame_offset = (off_t) (frames_per_ms * ((float) ms));
    const int rc = (int) mpg123_seek(mp, frame_offset , SEEK_SET);
    BAIL_IF_MACRO(rc < 0, set_error(mp, rc), 0);
    return(1);
} /* MPG123_seek */

#endif /* SOUND_SUPPORTS_MPG123 */

/* end of mpg123.c ... */

