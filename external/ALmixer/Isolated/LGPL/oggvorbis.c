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
 * Ogg Vorbis decoder for SDL_sound.
 *
 * This driver handles .OGG audio files, and depends on libvorbisfile to
 *  do the actual decoding work. libvorbisfile is part of libvorbis, which
 *  is part of the Ogg Vorbis project.
 *
 *   Ogg Vorbis: http://www.xiph.org/ogg/vorbis/
 *   vorbisfile documentation: http://www.xiph.org/ogg/vorbis/doc/vorbisfile/
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon. (icculus@icculus.org)
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef SOUND_SUPPORTS_OGG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "SoundDecoder.h"

#include "SoundDecoder_Internal.h"
#include "SDL_endian_minimal.h"
#include "ALmixer_RWops.h"

//#include <vorbis/codec.h>
#ifdef _WIN32
#include <vorbis/vorbisfile.h>
#else
#include <ivorbisfile.h>
#endif //_WIN32

#define ERR_IO_ERROR "I/O error"


static int OGG_init(void);
static void OGG_quit(void);
static int OGG_open(Sound_Sample *sample, const char *ext);
static void OGG_close(Sound_Sample *sample);
static uint32_t OGG_read(Sound_Sample *sample);
static int OGG_rewind(Sound_Sample *sample);
static int OGG_seek(Sound_Sample *sample, uint32_t ms);

static const char *extensions_ogg[] = { "OGG", NULL };
const Sound_DecoderFunctions __Sound_DecoderFunctions_OGG =
{
    {
        extensions_ogg,
        "Ogg Vorbis audio through VorbisFile",
        "Ryan C. Gordon <icculus@icculus.org>",
        "http://www.icculus.org/SDL_sound/"
    },

    OGG_init,       /*   init() method */
    OGG_quit,       /*   quit() method */
    OGG_open,       /*   open() method */
    OGG_close,      /*  close() method */
    OGG_read,       /*   read() method */
    OGG_rewind,     /* rewind() method */
    OGG_seek        /*   seek() method */
};


static int OGG_init(void)
{
    return(1);  /* always succeeds. */
} /* OGG_init */


static void OGG_quit(void)
{
    /* it's a no-op. */
} /* OGG_quit */



    /*
     * These are callbacks from vorbisfile that let them read data from
     *  a RWops...
     */

static size_t RWops_ogg_read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return((size_t) ALmixer_RWread((ALmixer_RWops *) datasource, ptr, size, nmemb));
} /* RWops_ogg_read */

static int RWops_ogg_seek(void *datasource, ogg_int64_t offset, int whence)
{
    return(ALmixer_RWseek((ALmixer_RWops *) datasource, offset, whence));
} /* RWops_ogg_seek */

static int RWops_ogg_close(void *datasource)
{
    /* do nothing; SDL_sound will delete the RWops at a higher level. */
    return(0);  /* this is success in fclose(), so I guess that's okay. */
} /* RWops_ogg_close */

static long RWops_ogg_tell(void *datasource)
{
    return((long) ALmixer_RWtell((ALmixer_RWops *) datasource));
} /* RWops_ogg_tell */

static const ov_callbacks RWops_ogg_callbacks =
{
    RWops_ogg_read,
    RWops_ogg_seek,
    RWops_ogg_close,
    RWops_ogg_tell
};


    /* Return a human readable version of an VorbisFile error code... */
#if (defined DEBUG_CHATTER)
static const char *ogg_error(int errnum)
{
    switch(errnum)
    {
        case OV_EREAD:
            return("i/o error");
        case OV_ENOTVORBIS:
            return("not a vorbis file");
        case OV_EVERSION:
            return("Vorbis version mismatch");
        case OV_EBADHEADER:
            return("invalid Vorbis bitstream header");
        case OV_EFAULT:
            return("internal logic fault in Vorbis library");
    } /* switch */

    return("unknown error");
} /* ogg_error */
#endif

static __inline__ void output_ogg_comments(OggVorbis_File *vf)
{
#if (defined DEBUG_CHATTER)
    int i;
    vorbis_comment *vc = ov_comment(vf, -1);

    if (vc == NULL)
        return;

    SNDDBG(("OGG: vendor == [%s].\n", vc->vendor));
    for (i = 0; i < vc->comments; i++)
    {
        SNDDBG(("OGG: user comment [%s].\n", vc->user_comments[i]));
    } /* for */
#endif
} /* output_ogg_comments */


static int OGG_open(Sound_Sample *sample, const char *ext)
{
    int rc;
    double total_time;
    OggVorbis_File *vf;
    vorbis_info *info;
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;

    vf = (OggVorbis_File *) malloc(sizeof (OggVorbis_File));
    BAIL_IF_MACRO(vf == NULL, ERR_OUT_OF_MEMORY, 0);

    rc = ov_open_callbacks(internal->rw, vf, NULL, 0, RWops_ogg_callbacks);
    if (rc != 0)
    {
#if (defined DEBUG_CHATTER)
        SNDDBG(("OGG: can't grok data. reason: [%s].\n", ogg_error(rc)));
#endif
        free(vf);
        BAIL_MACRO("OGG: Not valid Ogg Vorbis data.", 0);
    } /* if */

    info = ov_info(vf, -1);
    if (info == NULL)
    {
        ov_clear(vf);
        free(vf);
        BAIL_MACRO("OGG: failed to retrieve bitstream info", 0);
    } /* if */

    SNDDBG(("OGG: Accepting data stream.\n"));

    output_ogg_comments(vf);
    SNDDBG(("OGG: bitstream version == (%d).\n", info->version));
    SNDDBG(("OGG: bitstream channels == (%d).\n", info->channels));
    SNDDBG(("OGG: bitstream sampling rate == (%ld).\n", info->rate));
    SNDDBG(("OGG: seekable == {%s}.\n", ov_seekable(vf) ? "TRUE" : "FALSE"));
    SNDDBG(("OGG: number of logical bitstreams == (%ld).\n", ov_streams(vf)));
    SNDDBG(("OGG: serial number == (%ld).\n", ov_serialnumber(vf, -1)));
    SNDDBG(("OGG: total seconds of sample == (%f).\n", ov_time_total(vf, -1)));

    internal->decoder_private = vf;
    sample->flags = SOUND_SAMPLEFLAG_CANSEEK;
    sample->actual.rate = (uint32_t) info->rate;
    sample->actual.channels = (uint8_t) info->channels;
    total_time = ov_time_total(vf, -1);
    if (OV_EINVAL == total_time)
      internal->total_time = -1;
    else
      internal->total_time = (int32_t)(total_time * 1000.0 + 0.5);

    /*
     * Since we might have more than one logical bitstream in the OGG file,
     *  and these bitstreams may be in different formats, we might be
     *  converting two or three times: once in vorbisfile, once again in
     *  SDL_sound, and perhaps a third time to get it to the sound device's
     *  format. That's wickedly inefficient.
     *
     * To combat this a little, if the user specified a desired format, we
     *  claim that to be the "actual" format of the collection of logical
     *  bitstreams. This means that VorbisFile will do a conversion as
     *  necessary, and SDL_sound will not. If the user didn't specify a
     *  desired format, then we pretend the "actual" format is something that
     *  OGG files are apparently commonly encoded in.
     */
    sample->actual.format = (sample->desired.format == 0) ?
                             AUDIO_S16LSB : sample->desired.format;
    return(1);
} /* OGG_open */


static void OGG_close(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    OggVorbis_File *vf = (OggVorbis_File *) internal->decoder_private;
    ov_clear(vf);
    free(vf);
} /* OGG_close */

/* Note: According to the Vorbis documentation:
 * "ov_read() will  decode at most one vorbis packet per invocation,
 * so the value returned will generally be less than length."
 * Due to this, for buffer sizes like 16384, SDL_Sound was always getting
 * an underfilled buffer and always setting the EAGAIN flag.
 * Since the SDL_Sound API implies that the entire buffer
 * should be filled unless EOF, additional code has been added
 * to this function to call ov_read() until the buffer is filled.
 * However, there may still be some corner cases where the buffer
 * cannot be entirely filled. So be aware.
 */
static uint32_t OGG_read(Sound_Sample *sample)
{
    int rc;
    int bitstream;
	Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    OggVorbis_File *vf = (OggVorbis_File *) internal->decoder_private;

    rc = ov_read(vf, internal->buffer, internal->buffer_size,
            ((sample->actual.format & 0x1000) ? 1 : 0), /* bigendian? */
            ((sample->actual.format & 0xFF) / 8), /* bytes per sample point */
            ((sample->actual.format & 0x8000) ? 1 : 0), /* signed data? */
            &bitstream);

        /* Make sure the read went smoothly... */
    if (rc == 0)
        sample->flags |= SOUND_SAMPLEFLAG_EOF;

    else if (rc < 0)
        sample->flags |= SOUND_SAMPLEFLAG_ERROR;

    /* If the buffer isn't filled, keep trying to fill it
     * until no more data can be grabbed */
    else if ((uint32_t) rc < internal->buffer_size)
	{
        /* Creating a pointer to the buffer that denotes where to start
         * writing new data. */
        char* buffer_start_point = NULL;
        int total_bytes_read = rc;
        int bytes_remaining = internal->buffer_size - rc;

        /* Keep grabbing data until something prevents
         * us from getting more. (Could be EOF,
         * packets are too large to fit in remaining
         * space, or an error.)
         */
        while( (rc > 0) && (bytes_remaining > 0) )
        {
        /* Set buffer pointer to end of last write */
        /* All the messiness is to get rid of the warning for
         * dereferencing a void*
         */
            buffer_start_point = &(((char*)internal->buffer)[total_bytes_read]);
            rc = ov_read(vf, buffer_start_point, bytes_remaining,
                ((sample->actual.format & 0x1000) ? 1 : 0), /* bigendian? */
                ((sample->actual.format & 0xFF) / 8), /* bytes per sample point */
                ((sample->actual.format & 0x8000) ? 1 : 0), /* signed data? */
                &bitstream);
            /* Make sure rc > 0 because we don't accidently want
             * to change the counters if there was an error
             */
            if(rc > 0)
            {
                total_bytes_read += rc;
                bytes_remaining = bytes_remaining - rc;
            }
	    }
        /* I think the minimum read size is 2, though I'm
         * not sure about this. (I've hit cases where I
         * couldn't read less than 4.) What I don't want to do is
         * accidently claim we hit EOF when the reason rc == 0
         * is because the requested amount of data was smaller
         * than the minimum packet size.
         * For now, I will be conservative
         * and not set the EOF flag, and let the next call to
         * this function figure it out.
         * I think the ERROR flag is safe to set because
         * it looks like OGG simply returns 0 if the
         * read size is too small.
         * And in most cases for sensible buffer sizes,
         * this fix will fill the buffer,
         * so we can set the EAGAIN flag without worrying
         * that it will always be set.
         */
        if(rc < 0)
	    {
            sample->flags |= SOUND_SAMPLEFLAG_ERROR;
        }
        else if(rc == 0)
        {
            /* Do nothing for now until there is a better solution */
            /* sample->flags |= SOUND_SAMPLEFLAG_EOF; */
        }

        /* Test for a buffer underrun. It should occur less frequently
         * now, but it still may happen and not necessarily mean
         * anything useful. */
        if ((uint32_t) total_bytes_read < internal->buffer_size)
        {
            sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;
        }
        /* change rc to the total bytes read so function
         * can return the correct value.
         */
        rc = total_bytes_read;
    }

    return((uint32_t) rc);
} /* OGG_read */


static int OGG_rewind(Sound_Sample *sample)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    OggVorbis_File *vf = (OggVorbis_File *) internal->decoder_private;

    BAIL_IF_MACRO(ov_raw_seek(vf, 0) < 0, ERR_IO_ERROR, 0);
    return(1);
} /* OGG_rewind */


static int OGG_seek(Sound_Sample *sample, uint32_t ms)
{
    Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
    OggVorbis_File *vf = (OggVorbis_File *) internal->decoder_private;
    double timepos = (((double) ms) / 1000.0);
    BAIL_IF_MACRO(ov_time_seek(vf, timepos) < 0, ERR_IO_ERROR, 0);
    return(1);
} /* OGG_seek */

#endif /* SOUND_SUPPORTS_OGG */


/* end of ogg.c ... */

