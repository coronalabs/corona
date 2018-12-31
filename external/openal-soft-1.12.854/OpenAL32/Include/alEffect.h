// NOTE:  The effect structure is getting too large, it may be a good idea to
//        start using a union or another form of unified storage.
#ifndef _AL_EFFECT_H_
#define _AL_EFFECT_H_

#include "AL/al.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    EAXREVERB = 0,
    REVERB,
    ECHO,

    MAX_EFFECTS
};
extern ALboolean DisabledEffects[MAX_EFFECTS];

typedef struct ALeffect
{
    // Effect type (AL_EFFECT_NULL, ...)
    ALenum type;

    struct {
        // Shared Reverb Properties
        ALfloat Density;
        ALfloat Diffusion;
        ALfloat Gain;
        ALfloat GainHF;
        ALfloat DecayTime;
        ALfloat DecayHFRatio;
        ALfloat ReflectionsGain;
        ALfloat ReflectionsDelay;
        ALfloat LateReverbGain;
        ALfloat LateReverbDelay;
        ALfloat AirAbsorptionGainHF;
        ALfloat RoomRolloffFactor;
        ALboolean DecayHFLimit;

        // Additional EAX Reverb Properties
        ALfloat GainLF;
        ALfloat DecayLFRatio;
        ALfloat ReflectionsPan[3];
        ALfloat LateReverbPan[3];
        ALfloat EchoTime;
        ALfloat EchoDepth;
        ALfloat ModulationTime;
        ALfloat ModulationDepth;
        ALfloat HFReference;
        ALfloat LFReference;
    } Reverb;

    struct {
        ALfloat Delay;
        ALfloat LRDelay;

        ALfloat Damping;
        ALfloat Feedback;

        ALfloat Spread;
    } Echo;

    // Index to itself
    ALuint effect;

    struct ALeffect *next;
} ALeffect;


ALvoid ReleaseALEffects(ALCdevice *device);

#ifdef __cplusplus
}
#endif

#endif
