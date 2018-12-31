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
 Attention: This was extracted from SDL_sound to go with the wav.c plugin (also from SDL_sound). 
 This code is licensed under the LGPL.
 This means we must not compile this code into anything that we are not willing to
 publicly release source code. 
 You should compile this into a separate dynamic library that is isolated from proprietary code.
 */
 
 
#include <stdint.h>
#include "SoundDecoder.h"

uint32_t __Sound_convertMsToBytePos(SoundDecoder_AudioInfo *info, uint32_t ms)
{
    /* "frames" == "sample frames" */
    float frames_per_ms = ((float) info->rate) / 1000.0f;
    uint32_t frame_offset = (uint32_t) (frames_per_ms * ((float) ms));
    uint32_t frame_size = (uint32_t) ((info->format & 0xFF) / 8) * info->channels;
    return(frame_offset * frame_size);
} /* __Sound_convertMsToBytePos */


