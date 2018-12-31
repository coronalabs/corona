/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
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

#include <stdlib.h>
#include <math.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "alMain.h"
#include "alEffect.h"
#include "alThunk.h"
#include "alError.h"


ALboolean DisabledEffects[MAX_EFFECTS];


static void InitEffectParams(ALeffect *effect, ALenum type);

DECL_VERIFIER(Effect, ALeffect, effect)

AL_API ALvoid AL_APIENTRY alGenEffects(ALsizei n, ALuint *effects)
{
    ALCcontext *Context;
    ALsizei i=0;

    Context = GetContextSuspended();
    if(!Context) return;

    if (n > 0)
    {
        ALCdevice *device = Context->Device;

        // Check that enough memory has been allocted in the 'effects' array for n Effects
        if (!IsBadWritePtr((void*)effects, n * sizeof(ALuint)))
        {
            ALeffect *end;
            ALeffect **list = &device->EffectList;
            while(*list)
                list = &(*list)->next;

            end = *list;
            while(i < n)
            {
                *list = calloc(1, sizeof(ALeffect));
                if(!(*list))
                {
                    while(end->next)
                    {
                        ALeffect *temp = end->next;
                        end->next = temp->next;

                        ALTHUNK_REMOVEENTRY(temp->effect);
                        device->EffectCount--;
                        free(temp);
                    }
                    alSetError(Context, AL_OUT_OF_MEMORY);
                    break;
                }

                effects[i] = (ALuint)ALTHUNK_ADDENTRY(*list);
                (*list)->effect = effects[i];

                InitEffectParams(*list, AL_EFFECT_NULL);
                device->EffectCount++;
                i++;

                list = &(*list)->next;
            }
        }
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alDeleteEffects(ALsizei n, ALuint *effects)
{
    ALCcontext *Context;
    ALeffect *ALEffect;
    ALsizei i;

    Context = GetContextSuspended();
    if(!Context) return;

    if (n >= 0)
    {
        ALCdevice *device = Context->Device;

        // Check that all effects are valid
        for (i = 0; i < n; i++)
        {
            if(!effects[i])
                continue;

            if(!VerifyEffect(device->EffectList, effects[i]))
            {
                alSetError(Context, AL_INVALID_NAME);
                break;
            }
        }

        if (i == n)
        {
            // All effects are valid
            for (i = 0; i < n; i++)
            {
                // Recheck that the effect is valid, because there could be duplicated names
                if((ALEffect=VerifyEffect(device->EffectList, effects[i])) != NULL)
                {
                    ALeffect **list;

                    // Remove Effect from list of effects
                    list = &device->EffectList;
                    while(*list && *list != ALEffect)
                         list = &(*list)->next;

                    if(*list)
                        *list = (*list)->next;
                    ALTHUNK_REMOVEENTRY(ALEffect->effect);

                    memset(ALEffect, 0, sizeof(ALeffect));
                    free(ALEffect);

                    device->EffectCount--;
                }
            }
        }
    }
    else
        alSetError(Context, AL_INVALID_VALUE);

    ProcessContext(Context);
}

AL_API ALboolean AL_APIENTRY alIsEffect(ALuint effect)
{
    ALCcontext *Context;
    ALboolean  result = AL_TRUE;

    Context = GetContextSuspended();
    if(!Context) return AL_FALSE;

    if(effect)
        result = (VerifyEffect(Context->Device->EffectList, effect) ?
                  AL_TRUE : AL_FALSE);

    ProcessContext(Context);

    return result;
}

AL_API ALvoid AL_APIENTRY alEffecti(ALuint effect, ALenum param, ALint iValue)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALEffect=VerifyEffect(Device->EffectList, effect)) != NULL)
    {
        if(param == AL_EFFECT_TYPE)
        {
            ALboolean isOk = (iValue == AL_EFFECT_NULL ||
                (iValue == AL_EFFECT_EAXREVERB && !DisabledEffects[EAXREVERB]) ||
                (iValue == AL_EFFECT_REVERB && !DisabledEffects[REVERB]) ||
                (iValue == AL_EFFECT_ECHO && !DisabledEffects[ECHO]));

            if(isOk)
                InitEffectParams(ALEffect, iValue);
            else
                alSetError(Context, AL_INVALID_VALUE);
        }
        else if(ALEffect->type == AL_EFFECT_EAXREVERB)
        {
            switch(param)
            {
            case AL_EAXREVERB_DECAY_HFLIMIT:
                if(iValue >= AL_EAXREVERB_MIN_DECAY_HFLIMIT &&
                   iValue <= AL_EAXREVERB_MAX_DECAY_HFLIMIT)
                    ALEffect->Reverb.DecayHFLimit = iValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_REVERB)
        {
            switch(param)
            {
            case AL_REVERB_DECAY_HFLIMIT:
                if(iValue >= AL_REVERB_MIN_DECAY_HFLIMIT &&
                   iValue <= AL_REVERB_MAX_DECAY_HFLIMIT)
                    ALEffect->Reverb.DecayHFLimit = iValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_ECHO)
        {
            switch(param)
            {
            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(Context, AL_INVALID_ENUM);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alEffectiv(ALuint effect, ALenum param, ALint *piValues)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALEffect=VerifyEffect(Device->EffectList, effect)) != NULL)
    {
        if(param == AL_EFFECT_TYPE)
        {
            alEffecti(effect, param, piValues[0]);
        }
        else if(ALEffect->type == AL_EFFECT_EAXREVERB)
        {
            switch(param)
            {
            case AL_EAXREVERB_DECAY_HFLIMIT:
                alEffecti(effect, param, piValues[0]);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_REVERB)
        {
            switch(param)
            {
            case AL_REVERB_DECAY_HFLIMIT:
                alEffecti(effect, param, piValues[0]);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_ECHO)
        {
            switch(param)
            {
            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(Context, AL_INVALID_ENUM);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alEffectf(ALuint effect, ALenum param, ALfloat flValue)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALEffect=VerifyEffect(Device->EffectList, effect)) != NULL)
    {
        if(ALEffect->type == AL_EFFECT_EAXREVERB)
        {
            switch(param)
            {
            case AL_EAXREVERB_DENSITY:
                if(flValue >= AL_EAXREVERB_MIN_DENSITY &&
                   flValue <= AL_EAXREVERB_MAX_DENSITY)
                    ALEffect->Reverb.Density = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_DIFFUSION:
                if(flValue >= AL_EAXREVERB_MIN_DIFFUSION &&
                   flValue <= AL_EAXREVERB_MAX_DIFFUSION)
                    ALEffect->Reverb.Diffusion = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_GAIN:
                if(flValue >= AL_EAXREVERB_MIN_GAIN &&
                   flValue <= AL_EAXREVERB_MAX_GAIN)
                    ALEffect->Reverb.Gain = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_GAINHF:
                if(flValue >= AL_EAXREVERB_MIN_GAINHF &&
                   flValue <= AL_EAXREVERB_MAX_GAIN)
                    ALEffect->Reverb.GainHF = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_GAINLF:
                if(flValue >= AL_EAXREVERB_MIN_GAINLF &&
                   flValue <= AL_EAXREVERB_MAX_GAINLF)
                    ALEffect->Reverb.GainLF = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_DECAY_TIME:
                if(flValue >= AL_EAXREVERB_MIN_DECAY_TIME &&
                   flValue <= AL_EAXREVERB_MAX_DECAY_TIME)
                    ALEffect->Reverb.DecayTime = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_DECAY_HFRATIO:
                if(flValue >= AL_EAXREVERB_MIN_DECAY_HFRATIO &&
                   flValue <= AL_EAXREVERB_MAX_DECAY_HFRATIO)
                    ALEffect->Reverb.DecayHFRatio = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_DECAY_LFRATIO:
                if(flValue >= AL_EAXREVERB_MIN_DECAY_LFRATIO &&
                   flValue <= AL_EAXREVERB_MAX_DECAY_LFRATIO)
                    ALEffect->Reverb.DecayLFRatio = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_REFLECTIONS_GAIN:
                if(flValue >= AL_EAXREVERB_MIN_REFLECTIONS_GAIN &&
                   flValue <= AL_EAXREVERB_MAX_REFLECTIONS_GAIN)
                    ALEffect->Reverb.ReflectionsGain = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_REFLECTIONS_DELAY:
                if(flValue >= AL_EAXREVERB_MIN_REFLECTIONS_DELAY &&
                   flValue <= AL_EAXREVERB_MAX_REFLECTIONS_DELAY)
                    ALEffect->Reverb.ReflectionsDelay = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_LATE_REVERB_GAIN:
                if(flValue >= AL_EAXREVERB_MIN_LATE_REVERB_GAIN &&
                   flValue <= AL_EAXREVERB_MAX_LATE_REVERB_GAIN)
                    ALEffect->Reverb.LateReverbGain = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_LATE_REVERB_DELAY:
                if(flValue >= AL_EAXREVERB_MIN_LATE_REVERB_DELAY &&
                   flValue <= AL_EAXREVERB_MAX_LATE_REVERB_DELAY)
                    ALEffect->Reverb.LateReverbDelay = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_AIR_ABSORPTION_GAINHF:
                if(flValue >= AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF &&
                   flValue <= AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF)
                    ALEffect->Reverb.AirAbsorptionGainHF = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_ECHO_TIME:
                if(flValue >= AL_EAXREVERB_MIN_ECHO_TIME &&
                   flValue <= AL_EAXREVERB_MAX_ECHO_TIME)
                    ALEffect->Reverb.EchoTime = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_ECHO_DEPTH:
                if(flValue >= AL_EAXREVERB_MIN_ECHO_DEPTH &&
                   flValue <= AL_EAXREVERB_MAX_ECHO_DEPTH)
                    ALEffect->Reverb.EchoDepth = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_MODULATION_TIME:
                if(flValue >= AL_EAXREVERB_MIN_MODULATION_TIME &&
                   flValue <= AL_EAXREVERB_MAX_MODULATION_TIME)
                    ALEffect->Reverb.ModulationTime = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_MODULATION_DEPTH:
                if(flValue >= AL_EAXREVERB_MIN_MODULATION_DEPTH &&
                   flValue <= AL_EAXREVERB_MAX_MODULATION_DEPTH)
                    ALEffect->Reverb.ModulationDepth = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_HFREFERENCE:
                if(flValue >= AL_EAXREVERB_MIN_HFREFERENCE &&
                   flValue <= AL_EAXREVERB_MAX_HFREFERENCE)
                    ALEffect->Reverb.HFReference = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_LFREFERENCE:
                if(flValue >= AL_EAXREVERB_MIN_LFREFERENCE &&
                   flValue <= AL_EAXREVERB_MAX_LFREFERENCE)
                    ALEffect->Reverb.LFReference = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_EAXREVERB_ROOM_ROLLOFF_FACTOR:
                if(flValue >= 0.0f && flValue <= 10.0f)
                    ALEffect->Reverb.RoomRolloffFactor = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_REVERB)
        {
            switch(param)
            {
            case AL_REVERB_DENSITY:
                if(flValue >= AL_REVERB_MIN_DENSITY &&
                   flValue <= AL_REVERB_MAX_DENSITY)
                    ALEffect->Reverb.Density = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_DIFFUSION:
                if(flValue >= AL_REVERB_MIN_DIFFUSION &&
                   flValue <= AL_REVERB_MAX_DIFFUSION)
                    ALEffect->Reverb.Diffusion = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_GAIN:
                if(flValue >= AL_REVERB_MIN_GAIN &&
                   flValue <= AL_REVERB_MAX_GAIN)
                    ALEffect->Reverb.Gain = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_GAINHF:
                if(flValue >= AL_REVERB_MIN_GAINHF &&
                   flValue <= AL_REVERB_MAX_GAINHF)
                    ALEffect->Reverb.GainHF = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_DECAY_TIME:
                if(flValue >= AL_REVERB_MIN_DECAY_TIME &&
                   flValue <= AL_REVERB_MAX_DECAY_TIME)
                    ALEffect->Reverb.DecayTime = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_DECAY_HFRATIO:
                if(flValue >= AL_REVERB_MIN_DECAY_HFRATIO &&
                   flValue <= AL_REVERB_MAX_DECAY_HFRATIO)
                    ALEffect->Reverb.DecayHFRatio = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_REFLECTIONS_GAIN:
                if(flValue >= AL_REVERB_MIN_REFLECTIONS_GAIN &&
                   flValue <= AL_REVERB_MAX_REFLECTIONS_GAIN)
                    ALEffect->Reverb.ReflectionsGain = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_REFLECTIONS_DELAY:
                if(flValue >= AL_REVERB_MIN_REFLECTIONS_DELAY &&
                   flValue <= AL_REVERB_MAX_REFLECTIONS_DELAY)
                    ALEffect->Reverb.ReflectionsDelay = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_LATE_REVERB_GAIN:
                if(flValue >= AL_REVERB_MIN_LATE_REVERB_GAIN &&
                   flValue <= AL_REVERB_MAX_LATE_REVERB_GAIN)
                    ALEffect->Reverb.LateReverbGain = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_LATE_REVERB_DELAY:
                if(flValue >= AL_REVERB_MIN_LATE_REVERB_DELAY &&
                   flValue <= AL_REVERB_MAX_LATE_REVERB_DELAY)
                    ALEffect->Reverb.LateReverbDelay = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_AIR_ABSORPTION_GAINHF:
                if(flValue >= AL_REVERB_MIN_AIR_ABSORPTION_GAINHF &&
                   flValue <= AL_REVERB_MAX_AIR_ABSORPTION_GAINHF)
                    ALEffect->Reverb.AirAbsorptionGainHF = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_REVERB_ROOM_ROLLOFF_FACTOR:
                if(flValue >= AL_REVERB_MIN_ROOM_ROLLOFF_FACTOR &&
                   flValue <= AL_REVERB_MAX_ROOM_ROLLOFF_FACTOR)
                    ALEffect->Reverb.RoomRolloffFactor = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_ECHO)
        {
            switch(param)
            {
            case AL_ECHO_DELAY:
                if(flValue >= AL_ECHO_MIN_DELAY && flValue <= AL_ECHO_MAX_DELAY)
                    ALEffect->Echo.Delay = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_ECHO_LRDELAY:
                if(flValue >= AL_ECHO_MIN_LRDELAY && flValue <= AL_ECHO_MAX_LRDELAY)
                    ALEffect->Echo.LRDelay = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_ECHO_DAMPING:
                if(flValue >= AL_ECHO_MIN_DAMPING && flValue <= AL_ECHO_MAX_DAMPING)
                    ALEffect->Echo.Damping = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_ECHO_FEEDBACK:
                if(flValue >= AL_ECHO_MIN_FEEDBACK && flValue <= AL_ECHO_MAX_FEEDBACK)
                    ALEffect->Echo.Feedback = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            case AL_ECHO_SPREAD:
                if(flValue >= AL_ECHO_MIN_SPREAD && flValue <= AL_ECHO_MAX_SPREAD)
                    ALEffect->Echo.Spread = flValue;
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(Context, AL_INVALID_ENUM);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alEffectfv(ALuint effect, ALenum param, ALfloat *pflValues)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALEffect=VerifyEffect(Device->EffectList, effect)) != NULL)
    {
        if(ALEffect->type == AL_EFFECT_EAXREVERB)
        {
            switch(param)
            {
            case AL_EAXREVERB_DENSITY:
            case AL_EAXREVERB_DIFFUSION:
            case AL_EAXREVERB_GAIN:
            case AL_EAXREVERB_GAINHF:
            case AL_EAXREVERB_GAINLF:
            case AL_EAXREVERB_DECAY_TIME:
            case AL_EAXREVERB_DECAY_HFRATIO:
            case AL_EAXREVERB_DECAY_LFRATIO:
            case AL_EAXREVERB_REFLECTIONS_GAIN:
            case AL_EAXREVERB_REFLECTIONS_DELAY:
            case AL_EAXREVERB_LATE_REVERB_GAIN:
            case AL_EAXREVERB_LATE_REVERB_DELAY:
            case AL_EAXREVERB_AIR_ABSORPTION_GAINHF:
            case AL_EAXREVERB_ECHO_TIME:
            case AL_EAXREVERB_ECHO_DEPTH:
            case AL_EAXREVERB_MODULATION_TIME:
            case AL_EAXREVERB_MODULATION_DEPTH:
            case AL_EAXREVERB_HFREFERENCE:
            case AL_EAXREVERB_LFREFERENCE:
            case AL_EAXREVERB_ROOM_ROLLOFF_FACTOR:
                alEffectf(effect, param, pflValues[0]);
                break;

            case AL_EAXREVERB_REFLECTIONS_PAN:
                if(!isnan(pflValues[0]) && !isnan(pflValues[1]) && !isnan(pflValues[2]))
                {
                    ALEffect->Reverb.ReflectionsPan[0] = pflValues[0];
                    ALEffect->Reverb.ReflectionsPan[1] = pflValues[1];
                    ALEffect->Reverb.ReflectionsPan[2] = pflValues[2];
                }
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;
            case AL_EAXREVERB_LATE_REVERB_PAN:
                if(!isnan(pflValues[0]) && !isnan(pflValues[1]) && !isnan(pflValues[2]))
                {
                    ALEffect->Reverb.LateReverbPan[0] = pflValues[0];
                    ALEffect->Reverb.LateReverbPan[1] = pflValues[1];
                    ALEffect->Reverb.LateReverbPan[2] = pflValues[2];
                }
                else
                    alSetError(Context, AL_INVALID_VALUE);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_REVERB)
        {
            switch(param)
            {
            case AL_REVERB_DENSITY:
            case AL_REVERB_DIFFUSION:
            case AL_REVERB_GAIN:
            case AL_REVERB_GAINHF:
            case AL_REVERB_DECAY_TIME:
            case AL_REVERB_DECAY_HFRATIO:
            case AL_REVERB_REFLECTIONS_GAIN:
            case AL_REVERB_REFLECTIONS_DELAY:
            case AL_REVERB_LATE_REVERB_GAIN:
            case AL_REVERB_LATE_REVERB_DELAY:
            case AL_REVERB_AIR_ABSORPTION_GAINHF:
            case AL_REVERB_ROOM_ROLLOFF_FACTOR:
                alEffectf(effect, param, pflValues[0]);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_ECHO)
        {
            switch(param)
            {
            case AL_ECHO_DELAY:
            case AL_ECHO_LRDELAY:
            case AL_ECHO_DAMPING:
            case AL_ECHO_FEEDBACK:
            case AL_ECHO_SPREAD:
                alEffectf(effect, param, pflValues[0]);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(Context, AL_INVALID_ENUM);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetEffecti(ALuint effect, ALenum param, ALint *piValue)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALEffect=VerifyEffect(Device->EffectList, effect)) != NULL)
    {
        if(param == AL_EFFECT_TYPE)
        {
            *piValue = ALEffect->type;
        }
        else if(ALEffect->type == AL_EFFECT_EAXREVERB)
        {
            switch(param)
            {
            case AL_EAXREVERB_DECAY_HFLIMIT:
                *piValue = ALEffect->Reverb.DecayHFLimit;
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_REVERB)
        {
            switch(param)
            {
            case AL_REVERB_DECAY_HFLIMIT:
                *piValue = ALEffect->Reverb.DecayHFLimit;
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_ECHO)
        {
            switch(param)
            {
            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(Context, AL_INVALID_ENUM);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetEffectiv(ALuint effect, ALenum param, ALint *piValues)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALEffect=VerifyEffect(Device->EffectList, effect)) != NULL)
    {
        if(param == AL_EFFECT_TYPE)
        {
            alGetEffecti(effect, param, piValues);
        }
        else if(ALEffect->type == AL_EFFECT_EAXREVERB)
        {
            switch(param)
            {
            case AL_EAXREVERB_DECAY_HFLIMIT:
                alGetEffecti(effect, param, piValues);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_REVERB)
        {
            switch(param)
            {
            case AL_REVERB_DECAY_HFLIMIT:
                alGetEffecti(effect, param, piValues);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_ECHO)
        {
            switch(param)
            {
            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(Context, AL_INVALID_ENUM);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetEffectf(ALuint effect, ALenum param, ALfloat *pflValue)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALEffect=VerifyEffect(Device->EffectList, effect)) != NULL)
    {
        if(ALEffect->type == AL_EFFECT_EAXREVERB)
        {
            switch(param)
            {
            case AL_EAXREVERB_DENSITY:
                *pflValue = ALEffect->Reverb.Density;
                break;

            case AL_EAXREVERB_DIFFUSION:
                *pflValue = ALEffect->Reverb.Diffusion;
                break;

            case AL_EAXREVERB_GAIN:
                *pflValue = ALEffect->Reverb.Gain;
                break;

            case AL_EAXREVERB_GAINHF:
                *pflValue = ALEffect->Reverb.GainHF;
                break;

            case AL_EAXREVERB_GAINLF:
                *pflValue = ALEffect->Reverb.GainLF;
                break;

            case AL_EAXREVERB_DECAY_TIME:
                *pflValue = ALEffect->Reverb.DecayTime;
                break;

            case AL_EAXREVERB_DECAY_HFRATIO:
                *pflValue = ALEffect->Reverb.DecayHFRatio;
                break;

            case AL_EAXREVERB_DECAY_LFRATIO:
                *pflValue = ALEffect->Reverb.DecayLFRatio;
                break;

            case AL_EAXREVERB_REFLECTIONS_GAIN:
                *pflValue = ALEffect->Reverb.ReflectionsGain;
                break;

            case AL_EAXREVERB_REFLECTIONS_DELAY:
                *pflValue = ALEffect->Reverb.ReflectionsDelay;
                break;

            case AL_EAXREVERB_LATE_REVERB_GAIN:
                *pflValue = ALEffect->Reverb.LateReverbGain;
                break;

            case AL_EAXREVERB_LATE_REVERB_DELAY:
                *pflValue = ALEffect->Reverb.LateReverbDelay;
                break;

            case AL_EAXREVERB_AIR_ABSORPTION_GAINHF:
                *pflValue = ALEffect->Reverb.AirAbsorptionGainHF;
                break;

            case AL_EAXREVERB_ECHO_TIME:
                *pflValue = ALEffect->Reverb.EchoTime;
                break;

            case AL_EAXREVERB_ECHO_DEPTH:
                *pflValue = ALEffect->Reverb.EchoDepth;
                break;

            case AL_EAXREVERB_MODULATION_TIME:
                *pflValue = ALEffect->Reverb.ModulationTime;
                break;

            case AL_EAXREVERB_MODULATION_DEPTH:
                *pflValue = ALEffect->Reverb.ModulationDepth;
                break;

            case AL_EAXREVERB_HFREFERENCE:
                *pflValue = ALEffect->Reverb.HFReference;
                break;

            case AL_EAXREVERB_LFREFERENCE:
                *pflValue = ALEffect->Reverb.LFReference;
                break;

            case AL_EAXREVERB_ROOM_ROLLOFF_FACTOR:
                *pflValue = ALEffect->Reverb.RoomRolloffFactor;
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_REVERB)
        {
            switch(param)
            {
            case AL_REVERB_DENSITY:
                *pflValue = ALEffect->Reverb.Density;
                break;

            case AL_REVERB_DIFFUSION:
                *pflValue = ALEffect->Reverb.Diffusion;
                break;

            case AL_REVERB_GAIN:
                *pflValue = ALEffect->Reverb.Gain;
                break;

            case AL_REVERB_GAINHF:
                *pflValue = ALEffect->Reverb.GainHF;
                break;

            case AL_REVERB_DECAY_TIME:
                *pflValue = ALEffect->Reverb.DecayTime;
                break;

            case AL_REVERB_DECAY_HFRATIO:
                *pflValue = ALEffect->Reverb.DecayHFRatio;
                break;

            case AL_REVERB_REFLECTIONS_GAIN:
                *pflValue = ALEffect->Reverb.ReflectionsGain;
                break;

            case AL_REVERB_REFLECTIONS_DELAY:
                *pflValue = ALEffect->Reverb.ReflectionsDelay;
                break;

            case AL_REVERB_LATE_REVERB_GAIN:
                *pflValue = ALEffect->Reverb.LateReverbGain;
                break;

            case AL_REVERB_LATE_REVERB_DELAY:
                *pflValue = ALEffect->Reverb.LateReverbDelay;
                break;

            case AL_REVERB_AIR_ABSORPTION_GAINHF:
                *pflValue = ALEffect->Reverb.AirAbsorptionGainHF;
                break;

            case AL_REVERB_ROOM_ROLLOFF_FACTOR:
                *pflValue = ALEffect->Reverb.RoomRolloffFactor;
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_ECHO)
        {
            switch(param)
            {
            case AL_ECHO_DELAY:
                *pflValue = ALEffect->Echo.Delay;
                break;

            case AL_ECHO_LRDELAY:
                *pflValue = ALEffect->Echo.LRDelay;
                break;

            case AL_ECHO_DAMPING:
                *pflValue = ALEffect->Echo.Damping;
                break;

            case AL_ECHO_FEEDBACK:
                *pflValue = ALEffect->Echo.Feedback;
                break;

            case AL_ECHO_SPREAD:
                *pflValue = ALEffect->Echo.Spread;
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(Context, AL_INVALID_ENUM);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetEffectfv(ALuint effect, ALenum param, ALfloat *pflValues)
{
    ALCcontext *Context;
    ALCdevice  *Device;
    ALeffect   *ALEffect;

    Context = GetContextSuspended();
    if(!Context) return;

    Device = Context->Device;
    if((ALEffect=VerifyEffect(Device->EffectList, effect)) != NULL)
    {
        if(ALEffect->type == AL_EFFECT_EAXREVERB)
        {
            switch(param)
            {
            case AL_EAXREVERB_DENSITY:
            case AL_EAXREVERB_DIFFUSION:
            case AL_EAXREVERB_GAIN:
            case AL_EAXREVERB_GAINHF:
            case AL_EAXREVERB_GAINLF:
            case AL_EAXREVERB_DECAY_TIME:
            case AL_EAXREVERB_DECAY_HFRATIO:
            case AL_EAXREVERB_DECAY_LFRATIO:
            case AL_EAXREVERB_REFLECTIONS_GAIN:
            case AL_EAXREVERB_REFLECTIONS_DELAY:
            case AL_EAXREVERB_LATE_REVERB_GAIN:
            case AL_EAXREVERB_LATE_REVERB_DELAY:
            case AL_EAXREVERB_AIR_ABSORPTION_GAINHF:
            case AL_EAXREVERB_ECHO_TIME:
            case AL_EAXREVERB_ECHO_DEPTH:
            case AL_EAXREVERB_MODULATION_TIME:
            case AL_EAXREVERB_MODULATION_DEPTH:
            case AL_EAXREVERB_HFREFERENCE:
            case AL_EAXREVERB_LFREFERENCE:
            case AL_EAXREVERB_ROOM_ROLLOFF_FACTOR:
                alGetEffectf(effect, param, pflValues);
                break;

            case AL_EAXREVERB_REFLECTIONS_PAN:
                pflValues[0] = ALEffect->Reverb.ReflectionsPan[0];
                pflValues[1] = ALEffect->Reverb.ReflectionsPan[1];
                pflValues[2] = ALEffect->Reverb.ReflectionsPan[2];
                break;
            case AL_EAXREVERB_LATE_REVERB_PAN:
                pflValues[0] = ALEffect->Reverb.LateReverbPan[0];
                pflValues[1] = ALEffect->Reverb.LateReverbPan[1];
                pflValues[2] = ALEffect->Reverb.LateReverbPan[2];
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_REVERB)
        {
            switch(param)
            {
            case AL_REVERB_DENSITY:
            case AL_REVERB_DIFFUSION:
            case AL_REVERB_GAIN:
            case AL_REVERB_GAINHF:
            case AL_REVERB_DECAY_TIME:
            case AL_REVERB_DECAY_HFRATIO:
            case AL_REVERB_REFLECTIONS_GAIN:
            case AL_REVERB_REFLECTIONS_DELAY:
            case AL_REVERB_LATE_REVERB_GAIN:
            case AL_REVERB_LATE_REVERB_DELAY:
            case AL_REVERB_AIR_ABSORPTION_GAINHF:
            case AL_REVERB_ROOM_ROLLOFF_FACTOR:
                alGetEffectf(effect, param, pflValues);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else if(ALEffect->type == AL_EFFECT_ECHO)
        {
            switch(param)
            {
            case AL_ECHO_DELAY:
            case AL_ECHO_LRDELAY:
            case AL_ECHO_DAMPING:
            case AL_ECHO_FEEDBACK:
            case AL_ECHO_SPREAD:
                alGetEffectf(effect, param, pflValues);
                break;

            default:
                alSetError(Context, AL_INVALID_ENUM);
                break;
            }
        }
        else
            alSetError(Context, AL_INVALID_ENUM);
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}


ALvoid ReleaseALEffects(ALCdevice *device)
{
    while(device->EffectList)
    {
        ALeffect *temp = device->EffectList;
        device->EffectList = temp->next;

        // Release effect structure
        ALTHUNK_REMOVEENTRY(temp->effect);
        memset(temp, 0, sizeof(ALeffect));
        free(temp);
    }
    device->EffectCount = 0;
}


static void InitEffectParams(ALeffect *effect, ALenum type)
{
    effect->type = type;
    switch(type)
    {
    /* NOTE: Standard reverb and EAX reverb use the same defaults for the
     *       shared parameters, and EAX's additional parameters default to
     *       values assumed by standard reverb.
     */
    case AL_EFFECT_EAXREVERB:
    case AL_EFFECT_REVERB:
        effect->Reverb.Density = AL_EAXREVERB_DEFAULT_DENSITY;
        effect->Reverb.Diffusion = AL_EAXREVERB_DEFAULT_DIFFUSION;
        effect->Reverb.Gain = AL_EAXREVERB_DEFAULT_GAIN;
        effect->Reverb.GainHF = AL_EAXREVERB_DEFAULT_GAINHF;
        effect->Reverb.GainLF = AL_EAXREVERB_DEFAULT_GAINLF;
        effect->Reverb.DecayTime = AL_EAXREVERB_DEFAULT_DECAY_TIME;
        effect->Reverb.DecayHFRatio = AL_EAXREVERB_DEFAULT_DECAY_HFRATIO;
        effect->Reverb.DecayLFRatio = AL_EAXREVERB_DEFAULT_DECAY_LFRATIO;
        effect->Reverb.ReflectionsGain = AL_EAXREVERB_DEFAULT_REFLECTIONS_GAIN;
        effect->Reverb.ReflectionsDelay = AL_EAXREVERB_DEFAULT_REFLECTIONS_DELAY;
        effect->Reverb.ReflectionsPan[0] = AL_EAXREVERB_DEFAULT_REFLECTIONS_PAN_XYZ;
        effect->Reverb.ReflectionsPan[1] = AL_EAXREVERB_DEFAULT_REFLECTIONS_PAN_XYZ;
        effect->Reverb.ReflectionsPan[2] = AL_EAXREVERB_DEFAULT_REFLECTIONS_PAN_XYZ;
        effect->Reverb.LateReverbGain = AL_EAXREVERB_DEFAULT_LATE_REVERB_GAIN;
        effect->Reverb.LateReverbDelay = AL_EAXREVERB_DEFAULT_LATE_REVERB_DELAY;
        effect->Reverb.LateReverbPan[0] = AL_EAXREVERB_DEFAULT_LATE_REVERB_PAN_XYZ;
        effect->Reverb.LateReverbPan[1] = AL_EAXREVERB_DEFAULT_LATE_REVERB_PAN_XYZ;
        effect->Reverb.LateReverbPan[2] = AL_EAXREVERB_DEFAULT_LATE_REVERB_PAN_XYZ;
        effect->Reverb.EchoTime = AL_EAXREVERB_DEFAULT_ECHO_TIME;
        effect->Reverb.EchoDepth = AL_EAXREVERB_DEFAULT_ECHO_DEPTH;
        effect->Reverb.ModulationTime = AL_EAXREVERB_DEFAULT_MODULATION_TIME;
        effect->Reverb.ModulationDepth = AL_EAXREVERB_DEFAULT_MODULATION_DEPTH;
        effect->Reverb.AirAbsorptionGainHF = AL_EAXREVERB_DEFAULT_AIR_ABSORPTION_GAINHF;
        effect->Reverb.HFReference = AL_EAXREVERB_DEFAULT_HFREFERENCE;
        effect->Reverb.LFReference = AL_EAXREVERB_DEFAULT_LFREFERENCE;
        effect->Reverb.RoomRolloffFactor = AL_EAXREVERB_DEFAULT_ROOM_ROLLOFF_FACTOR;
        effect->Reverb.DecayHFLimit = AL_EAXREVERB_DEFAULT_DECAY_HFLIMIT;
        break;
    case AL_EFFECT_ECHO:
        effect->Echo.Delay = AL_ECHO_DEFAULT_DELAY;
        effect->Echo.LRDelay = AL_ECHO_DEFAULT_LRDELAY;
        effect->Echo.Damping = AL_ECHO_DEFAULT_DAMPING;
        effect->Echo.Feedback = AL_ECHO_DEFAULT_FEEDBACK;
        effect->Echo.Spread = AL_ECHO_DEFAULT_SPREAD;
        break;
    }
}
