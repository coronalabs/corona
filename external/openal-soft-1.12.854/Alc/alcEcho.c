/**
 * OpenAL cross platform audio library
 * Copyright (C) 2009 by Chris Robinson.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "config.h"

#include <math.h>
#include <stdlib.h>

#include "alMain.h"
#include "alFilter.h"
#include "alAuxEffectSlot.h"
#include "alError.h"
#include "alu.h"


typedef struct ALechoState {
    // Must be first in all effects!
    ALeffectState state;

    ALfloat *SampleBuffer;
    ALuint BufferLength;

    // The echo is two tap. The delay is the number of samples from before the
    // current offset
    struct {
        ALuint delay;
    } Tap[2];
    ALuint Offset;
    // The LR gains for the first tap. The second tap uses the reverse
    ALfloat GainL;
    ALfloat GainR;

    ALfloat FeedGain;

    FILTER iirFilter;
    ALfloat history[2];
} ALechoState;

ALvoid EchoDestroy(ALeffectState *effect)
{
    ALechoState *state = (ALechoState*)effect;
    if(state)
    {
        free(state->SampleBuffer);
        state->SampleBuffer = NULL;
        free(state);
    }
}

ALboolean EchoDeviceUpdate(ALeffectState *effect, ALCdevice *Device)
{
    ALechoState *state = (ALechoState*)effect;
    ALuint maxlen, i;

    // Use the next power of 2 for the buffer length, so the tap offsets can be
    // wrapped using a mask instead of a modulo
    maxlen  = (ALuint)(AL_ECHO_MAX_DELAY * Device->Frequency) + 1;
    maxlen += (ALuint)(AL_ECHO_MAX_LRDELAY * Device->Frequency) + 1;
    maxlen  = NextPowerOf2(maxlen);

    if(maxlen != state->BufferLength)
    {
        void *temp;

        temp = realloc(state->SampleBuffer, maxlen * sizeof(ALfloat));
        if(!temp)
            return AL_FALSE;
        state->SampleBuffer = temp;
        state->BufferLength = maxlen;
    }
    for(i = 0;i < state->BufferLength;i++)
        state->SampleBuffer[i] = 0.0f;

    return AL_TRUE;
}

ALvoid EchoUpdate(ALeffectState *effect, ALCcontext *Context, const ALeffect *Effect)
{
    ALechoState *state = (ALechoState*)effect;
    ALuint frequency = Context->Device->Frequency;
    ALfloat lrpan, cw, a, g;

    state->Tap[0].delay = (ALuint)(Effect->Echo.Delay * frequency) + 1;
    state->Tap[1].delay = (ALuint)(Effect->Echo.LRDelay * frequency);
    state->Tap[1].delay += state->Tap[0].delay;

    lrpan = Effect->Echo.Spread*0.5f + 0.5f;
    state->GainL = aluSqrt(     lrpan);
    state->GainR = aluSqrt(1.0f-lrpan);

    state->FeedGain = Effect->Echo.Feedback;

    cw = cos(2.0*M_PI * LOWPASSFREQCUTOFF / frequency);
    g = 1.0f - Effect->Echo.Damping;
    a = 0.0f;
    if(g < 0.9999f) // 1-epsilon
        a = (1 - g*cw - aluSqrt(2*g*(1-cw) - g*g*(1 - cw*cw))) / (1 - g);
    state->iirFilter.coeff = a;
}

ALvoid EchoProcess(ALeffectState *effect, const ALeffectslot *Slot, ALuint SamplesToDo, const ALfloat *SamplesIn, ALfloat (*SamplesOut)[OUTPUTCHANNELS])
{
    ALechoState *state = (ALechoState*)effect;
    const ALuint mask = state->BufferLength-1;
    const ALuint tap1 = state->Tap[0].delay;
    const ALuint tap2 = state->Tap[1].delay;
    ALuint offset = state->Offset;
    const ALfloat gain = Slot->Gain;
    ALfloat samp[2], smp;
    ALuint i;

    for(i = 0;i < SamplesToDo;i++,offset++)
    {
        // Sample first tap
        smp = state->SampleBuffer[(offset-tap1) & mask];
        samp[0] = smp * state->GainL;
        samp[1] = smp * state->GainR;
        // Sample second tap. Reverse LR panning
        smp = state->SampleBuffer[(offset-tap2) & mask];
        samp[0] += smp * state->GainR;
        samp[1] += smp * state->GainL;

        // Apply damping and feedback gain to the second tap, and mix in the
        // new sample
        smp = lpFilter2P(&state->iirFilter, 0, smp+SamplesIn[i]);
        state->SampleBuffer[offset&mask] = smp * state->FeedGain;

        // Apply slot gain
        samp[0] *= gain;
        samp[1] *= gain;

        SamplesOut[i][FRONT_LEFT]  += samp[0];
        SamplesOut[i][FRONT_RIGHT] += samp[1];
        SamplesOut[i][SIDE_LEFT]   += samp[0];
        SamplesOut[i][SIDE_RIGHT]  += samp[1];
        SamplesOut[i][BACK_LEFT]   += samp[0];
        SamplesOut[i][BACK_RIGHT]  += samp[1];
    }
    state->Offset = offset;
}

ALeffectState *EchoCreate(void)
{
    ALechoState *state;

    state = malloc(sizeof(*state));
    if(!state)
        return NULL;

    state->state.Destroy = EchoDestroy;
    state->state.DeviceUpdate = EchoDeviceUpdate;
    state->state.Update = EchoUpdate;
    state->state.Process = EchoProcess;

    state->BufferLength = 0;
    state->SampleBuffer = NULL;

    state->Tap[0].delay = 0;
    state->Tap[1].delay = 0;
    state->Offset = 0;
    state->GainL = 0.0f;
    state->GainR = 0.0f;

    state->iirFilter.coeff = 0.0f;
    state->iirFilter.history[0] = 0.0f;
    state->iirFilter.history[1] = 0.0f;

    return &state->state;
}
