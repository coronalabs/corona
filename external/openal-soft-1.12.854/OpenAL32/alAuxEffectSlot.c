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
#include "alAuxEffectSlot.h"
#include "alThunk.h"
#include "alError.h"
#include "alSource.h"


static ALvoid InitializeEffect(ALCcontext *Context, ALeffectslot *EffectSlot, ALeffect *effect);

DECL_VERIFIER(EffectSlot, ALeffectslot, effectslot)
DECL_VERIFIER(Effect, ALeffect, effect)

AL_API ALvoid AL_APIENTRY alGenAuxiliaryEffectSlots(ALsizei n, ALuint *effectslots)
{
    ALCcontext *Context;
    ALsizei i=0, j;

    Context = GetContextSuspended();
    if(!Context) return;

    if (n > 0)
    {
        ALCdevice *Device = Context->Device;

        if(Context->EffectSlotCount+n <= Device->AuxiliaryEffectSlotMax)
        {
            // Check that enough memory has been allocted in the 'effectslots' array for n Effect Slots
            if (!IsBadWritePtr((void*)effectslots, n * sizeof(ALuint)))
            {
                ALeffectslot *end;
                ALeffectslot **list = &Context->EffectSlotList;
                while(*list)
                    list = &(*list)->next;

                end = *list;
                while(i < n)
                {
                    *list = calloc(1, sizeof(ALeffectslot));
                    if(!(*list) || !((*list)->EffectState=NoneCreate()))
                    {
                        // We must have run out or memory
                        free(*list); *list = NULL;
                        while(end->next)
                        {
                            ALeffectslot *temp = end->next;
                            end->next = temp->next;

                            ALEffect_Destroy(temp->EffectState);
                            ALTHUNK_REMOVEENTRY(temp->effectslot);
                            Context->EffectSlotCount--;
                            free(temp);
                        }
                        alSetError(Context, AL_OUT_OF_MEMORY);
                        break;
                    }

                    (*list)->Gain = 1.0;
                    (*list)->AuxSendAuto = AL_TRUE;
                    for(j = 0;j < BUFFERSIZE;j++)
                        (*list)->WetBuffer[j] = 0.0f;
                    (*list)->refcount = 0;

                    effectslots[i] = (ALuint)ALTHUNK_ADDENTRY(*list);
                    (*list)->effectslot = effectslots[i];

                    Context->EffectSlotCount++;
                    i++;

                    list = &(*list)->next;
                }
            }
        }
        else
            alSetError(Context, AL_INVALID_OPERATION);
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alDeleteAuxiliaryEffectSlots(ALsizei n, ALuint *effectslots)
{
    ALCcontext *Context;
    ALeffectslot *EffectSlot;
    ALsizei i;

    Context = GetContextSuspended();
    if(!Context) return;

    if (n >= 0)
    {
        // Check that all effectslots are valid
        for (i = 0; i < n; i++)
        {
            if((EffectSlot=VerifyEffectSlot(Context->EffectSlotList, effectslots[i])) == NULL)
            {
                alSetError(Context, AL_INVALID_NAME);
                break;
            }
            else
            {
                if(EffectSlot->refcount > 0)
                {
                    alSetError(Context, AL_INVALID_NAME);
                    break;
                }
            }
        }

        if (i == n)
        {
            // All effectslots are valid
            for (i = 0; i < n; i++)
            {
                // Recheck that the effectslot is valid, because there could be duplicated names
                if((EffectSlot=VerifyEffectSlot(Context->EffectSlotList, effectslots[i])) != NULL)
                {
                    ALeffectslot **list;

                    // Remove Source from list of Sources
                    list = &Context->EffectSlotList;
                    while(*list && *list != EffectSlot)
                         list = &(*list)->next;

                    if(*list)
                        *list = (*list)->next;
                    ALTHUNK_REMOVEENTRY(EffectSlot->effectslot);

                    ALEffect_Destroy(EffectSlot->EffectState);

                    memset(EffectSlot, 0, sizeof(ALeffectslot));
                    free(EffectSlot);

                    Context->EffectSlotCount--;
                }
            }
        }
    }
    else
        alSetError(Context, AL_INVALID_VALUE);

    ProcessContext(Context);
}

AL_API ALboolean AL_APIENTRY alIsAuxiliaryEffectSlot(ALuint effectslot)
{
    ALCcontext *Context;
    ALboolean  result;

    Context = GetContextSuspended();
    if(!Context) return AL_FALSE;

    result = (VerifyEffectSlot(Context->EffectSlotList, effectslot) ?
              AL_TRUE : AL_FALSE);

    ProcessContext(Context);

    return result;
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint iValue)
{
    ALCcontext *Context;
    ALboolean updateSources = AL_FALSE;
    ALeffectslot *EffectSlot;

    Context = GetContextSuspended();
    if(!Context) return;

    if((EffectSlot=VerifyEffectSlot(Context->EffectSlotList, effectslot)) != NULL)
    {
        switch(param)
        {
        case AL_EFFECTSLOT_EFFECT: {
            ALeffect *effect = NULL;

            if(iValue == 0 ||
               (effect=VerifyEffect(Context->Device->EffectList, iValue)) != NULL)
            {
                InitializeEffect(Context, EffectSlot, effect);
                updateSources = AL_TRUE;
            }
            else
                alSetError(Context, AL_INVALID_VALUE);
        }   break;

        case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
            if(iValue == AL_TRUE || iValue == AL_FALSE)
            {
                EffectSlot->AuxSendAuto = iValue;
                updateSources = AL_TRUE;
            }
            else
                alSetError(Context, AL_INVALID_VALUE);
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    // Force updating the sources that use this slot, since it affects the
    // sending parameters
    if(updateSources)
    {
        ALsource *source = Context->SourceList;
        while(source)
        {
            ALuint i;
            for(i = 0;i < MAX_SENDS;i++)
            {
                if(!source->Send[i].Slot ||
                   source->Send[i].Slot->effectslot != effectslot)
                    continue;
                source->NeedsUpdate = AL_TRUE;
                break;
            }
            source = source->next;
        }
    }

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, ALint *piValues)
{
    ALCcontext *Context;

    Context = GetContextSuspended();
    if(!Context) return;

    if(VerifyEffectSlot(Context->EffectSlotList, effectslot) != NULL)
    {
        switch(param)
        {
        case AL_EFFECTSLOT_EFFECT:
        case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
            alAuxiliaryEffectSloti(effectslot, param, piValues[0]);
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat flValue)
{
    ALCcontext *Context;
    ALeffectslot *EffectSlot;

    Context = GetContextSuspended();
    if(!Context) return;

    if((EffectSlot=VerifyEffectSlot(Context->EffectSlotList, effectslot)) != NULL)
    {
        switch(param)
        {
        case AL_EFFECTSLOT_GAIN:
            if(flValue >= 0.0f && flValue <= 1.0f)
                EffectSlot->Gain = flValue;
            else
                alSetError(Context, AL_INVALID_VALUE);
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, ALfloat *pflValues)
{
    ALCcontext *Context;

    Context = GetContextSuspended();
    if(!Context) return;

    if(VerifyEffectSlot(Context->EffectSlotList, effectslot) != NULL)
    {
        switch(param)
        {
        case AL_EFFECTSLOT_GAIN:
            alAuxiliaryEffectSlotf(effectslot, param, pflValues[0]);
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint *piValue)
{
    ALCcontext *Context;
    ALeffectslot *EffectSlot;

    Context = GetContextSuspended();
    if(!Context) return;

    if((EffectSlot=VerifyEffectSlot(Context->EffectSlotList, effectslot)) != NULL)
    {
        switch(param)
        {
        case AL_EFFECTSLOT_EFFECT:
            *piValue = EffectSlot->effect.effect;
            break;

        case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
            *piValue = EffectSlot->AuxSendAuto;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, ALint *piValues)
{
    ALCcontext *Context;

    Context = GetContextSuspended();
    if(!Context) return;

    if(VerifyEffectSlot(Context->EffectSlotList, effectslot) != NULL)
    {
        switch(param)
        {
        case AL_EFFECTSLOT_EFFECT:
        case AL_EFFECTSLOT_AUXILIARY_SEND_AUTO:
            alGetAuxiliaryEffectSloti(effectslot, param, piValues);
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat *pflValue)
{
    ALCcontext *Context;
    ALeffectslot *EffectSlot;

    Context = GetContextSuspended();
    if(!Context) return;

    if((EffectSlot=VerifyEffectSlot(Context->EffectSlotList, effectslot)) != NULL)
    {
        switch(param)
        {
        case AL_EFFECTSLOT_GAIN:
            *pflValue = EffectSlot->Gain;
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}

AL_API ALvoid AL_APIENTRY alGetAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, ALfloat *pflValues)
{
    ALCcontext *Context;

    Context = GetContextSuspended();
    if(!Context) return;

    if(VerifyEffectSlot(Context->EffectSlotList, effectslot) != NULL)
    {
        switch(param)
        {
        case AL_EFFECTSLOT_GAIN:
            alGetAuxiliaryEffectSlotf(effectslot, param, pflValues);
            break;

        default:
            alSetError(Context, AL_INVALID_ENUM);
            break;
        }
    }
    else
        alSetError(Context, AL_INVALID_NAME);

    ProcessContext(Context);
}


static ALvoid NoneDestroy(ALeffectState *State)
{ free(State); }
static ALboolean NoneDeviceUpdate(ALeffectState *State, ALCdevice *Device)
{
    return AL_TRUE;
    (void)State;
    (void)Device;
}
static ALvoid NoneUpdate(ALeffectState *State, ALCcontext *Context, const ALeffect *Effect)
{
    (void)State;
    (void)Context;
    (void)Effect;
}
static ALvoid NoneProcess(ALeffectState *State, const ALeffectslot *Slot, ALuint SamplesToDo, const ALfloat *SamplesIn, ALfloat (*SamplesOut)[OUTPUTCHANNELS])
{
    (void)State;
    (void)Slot;
    (void)SamplesToDo;
    (void)SamplesIn;
    (void)SamplesOut;
}
ALeffectState *NoneCreate(void)
{
    ALeffectState *state;

    state = calloc(1, sizeof(*state));
    if(!state)
        return NULL;

    state->Destroy = NoneDestroy;
    state->DeviceUpdate = NoneDeviceUpdate;
    state->Update = NoneUpdate;
    state->Process = NoneProcess;

    return state;
}

static ALvoid InitializeEffect(ALCcontext *Context, ALeffectslot *EffectSlot, ALeffect *effect)
{
    if(EffectSlot->effect.type != (effect?effect->type:AL_EFFECT_NULL))
    {
        ALeffectState *NewState = NULL;
        if(!effect || effect->type == AL_EFFECT_NULL)
            NewState = NoneCreate();
        else if(effect->type == AL_EFFECT_EAXREVERB)
            NewState = EAXVerbCreate();
        else if(effect->type == AL_EFFECT_REVERB)
            NewState = VerbCreate();
        else if(effect->type == AL_EFFECT_ECHO)
            NewState = EchoCreate();
        /* No new state? An error occured.. */
        if(NewState == NULL ||
           ALEffect_DeviceUpdate(NewState, Context->Device) == AL_FALSE)
        {
            if(NewState)
                ALEffect_Destroy(NewState);
            alSetError(Context, AL_OUT_OF_MEMORY);
            return;
        }
        if(EffectSlot->EffectState)
            ALEffect_Destroy(EffectSlot->EffectState);
        EffectSlot->EffectState = NewState;
    }
    if(!effect)
        memset(&EffectSlot->effect, 0, sizeof(EffectSlot->effect));
    else
        memcpy(&EffectSlot->effect, effect, sizeof(*effect));
    ALEffect_Update(EffectSlot->EffectState, Context, effect);
}


ALvoid ReleaseALAuxiliaryEffectSlots(ALCcontext *Context)
{
    while(Context->EffectSlotList)
    {
        ALeffectslot *temp = Context->EffectSlotList;
        Context->EffectSlotList = temp->next;

        // Release effectslot structure
        ALEffect_Destroy(temp->EffectState);

        ALTHUNK_REMOVEENTRY(temp->effectslot);
        memset(temp, 0, sizeof(ALeffectslot));
        free(temp);
    }
    Context->EffectSlotCount = 0;
}
