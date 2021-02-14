#include "common.h"
#include "oal_utils.h"

#ifdef AUDIO_OAL

/*
 * When linking to a static openal-soft library,
 * the extension function inside the openal library conflict with the variables here.
 * Therefore declare these re3 owned symbols in a private namespace.
 */

namespace re3_openal {

LPALGENEFFECTS alGenEffects;
LPALDELETEEFFECTS alDeleteEffects;
LPALISEFFECT alIsEffect;
LPALEFFECTI alEffecti;
LPALEFFECTIV alEffectiv;
LPALEFFECTF alEffectf;
LPALEFFECTFV alEffectfv;
LPALGETEFFECTI alGetEffecti;
LPALGETEFFECTIV alGetEffectiv;
LPALGETEFFECTF alGetEffectf;
LPALGETEFFECTFV alGetEffectfv;
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
LPALGENFILTERS alGenFilters;
LPALDELETEFILTERS alDeleteFilters;
LPALISFILTER alIsFilter;
LPALFILTERI alFilteri;
LPALFILTERIV alFilteriv;
LPALFILTERF alFilterf;
LPALFILTERFV alFilterfv;
LPALGETFILTERI alGetFilteri;
LPALGETFILTERIV alGetFilteriv;
LPALGETFILTERF alGetFilterf;
LPALGETFILTERFV alGetFilterfv;

}

using namespace re3_openal;

void EFXInit()
{
	/* Define a macro to help load the function pointers. */
#define LOAD_PROC(T, x)  ((x) = (T)alGetProcAddress(#x))
	LOAD_PROC(LPALGENEFFECTS, alGenEffects);
	LOAD_PROC(LPALDELETEEFFECTS, alDeleteEffects);
	LOAD_PROC(LPALISEFFECT, alIsEffect);
	LOAD_PROC(LPALEFFECTI, alEffecti);
	LOAD_PROC(LPALEFFECTIV, alEffectiv);
	LOAD_PROC(LPALEFFECTF, alEffectf);
	LOAD_PROC(LPALEFFECTFV, alEffectfv);
	LOAD_PROC(LPALGETEFFECTI, alGetEffecti);
	LOAD_PROC(LPALGETEFFECTIV, alGetEffectiv);
	LOAD_PROC(LPALGETEFFECTF, alGetEffectf);
	LOAD_PROC(LPALGETEFFECTFV, alGetEffectfv);
	
	LOAD_PROC(LPALGENFILTERS, alGenFilters);
	LOAD_PROC(LPALDELETEFILTERS, alDeleteFilters);
	LOAD_PROC(LPALISFILTER, alIsFilter);
	LOAD_PROC(LPALFILTERI, alFilteri);
	LOAD_PROC(LPALFILTERIV, alFilteriv);
	LOAD_PROC(LPALFILTERF, alFilterf);
	LOAD_PROC(LPALFILTERFV, alFilterfv);
	LOAD_PROC(LPALGETFILTERI, alGetFilteri);
	LOAD_PROC(LPALGETFILTERIV, alGetFilteriv);
	LOAD_PROC(LPALGETFILTERF, alGetFilterf);
	LOAD_PROC(LPALGETFILTERFV, alGetFilterfv);
	
	LOAD_PROC(LPALGENAUXILIARYEFFECTSLOTS, alGenAuxiliaryEffectSlots);
	LOAD_PROC(LPALDELETEAUXILIARYEFFECTSLOTS, alDeleteAuxiliaryEffectSlots);
	LOAD_PROC(LPALISAUXILIARYEFFECTSLOT, alIsAuxiliaryEffectSlot);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTI, alAuxiliaryEffectSloti);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTIV, alAuxiliaryEffectSlotiv);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTF, alAuxiliaryEffectSlotf);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTFV, alAuxiliaryEffectSlotfv);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTI, alGetAuxiliaryEffectSloti);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTIV, alGetAuxiliaryEffectSlotiv);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTF, alGetAuxiliaryEffectSlotf);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTFV, alGetAuxiliaryEffectSlotfv);
#undef LOAD_PROC
}

void SetEffectsLevel(ALuint uiFilter, float level)
{
	alFilteri(uiFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	alFilterf(uiFilter, AL_LOWPASS_GAIN, 1.0f);
	alFilterf(uiFilter, AL_LOWPASS_GAINHF, level);
}

static inline float gain_to_mB(float gain)
{
    return (gain > 1e-5f) ? (float)(log10f(gain) * 2000.0f) : -10000l;
}

static inline float mB_to_gain(float millibels)
{
    return (millibels > -10000.0f) ? powf(10.0f, millibels/2000.0f) : 0.0f;
}

static inline float clampF(float val, float minval, float maxval)
{
    if(val >= maxval) return maxval;
    if(val <= minval) return minval;
    return val;
}

void EAX3_Set(ALuint effect, const EAXLISTENERPROPERTIES *props)
{
	alEffecti (effect, AL_EFFECT_TYPE,                     AL_EFFECT_EAXREVERB);
	alEffectf (effect, AL_EAXREVERB_DENSITY,               clampF(powf(props->flEnvironmentSize, 3.0f) / 16.0f, 0.0f, 1.0f));
	alEffectf (effect, AL_EAXREVERB_DIFFUSION,             props->flEnvironmentDiffusion);
	alEffectf (effect, AL_EAXREVERB_GAIN,                  mB_to_gain((float)props->lRoom));
	alEffectf (effect, AL_EAXREVERB_GAINHF,                mB_to_gain((float)props->lRoomHF));
	alEffectf (effect, AL_EAXREVERB_GAINLF,                mB_to_gain((float)props->lRoomLF));
	alEffectf (effect, AL_EAXREVERB_DECAY_TIME,            props->flDecayTime);
	alEffectf (effect, AL_EAXREVERB_DECAY_HFRATIO,         props->flDecayHFRatio);
	alEffectf (effect, AL_EAXREVERB_DECAY_LFRATIO,         props->flDecayLFRatio);
	alEffectf (effect, AL_EAXREVERB_REFLECTIONS_GAIN,      clampF(mB_to_gain((float)props->lReflections), AL_EAXREVERB_MIN_REFLECTIONS_GAIN, AL_EAXREVERB_MAX_REFLECTIONS_GAIN));
	alEffectf (effect, AL_EAXREVERB_REFLECTIONS_DELAY,     props->flReflectionsDelay);
	alEffectfv(effect, AL_EAXREVERB_REFLECTIONS_PAN,       &props->vReflectionsPan.x);
	alEffectf (effect, AL_EAXREVERB_LATE_REVERB_GAIN,      clampF(mB_to_gain((float)props->lReverb), AL_EAXREVERB_MIN_LATE_REVERB_GAIN, AL_EAXREVERB_MAX_LATE_REVERB_GAIN));
	alEffectf (effect, AL_EAXREVERB_LATE_REVERB_DELAY,     props->flReverbDelay);
	alEffectfv(effect, AL_EAXREVERB_LATE_REVERB_PAN,       &props->vReverbPan.x);
	alEffectf (effect, AL_EAXREVERB_ECHO_TIME,             props->flEchoTime);
	alEffectf (effect, AL_EAXREVERB_ECHO_DEPTH,            props->flEchoDepth);
	alEffectf (effect, AL_EAXREVERB_MODULATION_TIME,       props->flModulationTime);
	alEffectf (effect, AL_EAXREVERB_MODULATION_DEPTH,      props->flModulationDepth);
	alEffectf (effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, clampF(mB_to_gain(props->flAirAbsorptionHF), AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF, AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF));
	alEffectf (effect, AL_EAXREVERB_HFREFERENCE,           props->flHFReference);
	alEffectf (effect, AL_EAXREVERB_LFREFERENCE,           props->flLFReference);
	alEffectf (effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR,   props->flRoomRolloffFactor);
	alEffecti (effect, AL_EAXREVERB_DECAY_HFLIMIT,         (props->ulFlags&EAXLISTENERFLAGS_DECAYHFLIMIT) ? AL_TRUE : AL_FALSE);
}

void EFX_Set(ALuint effect, const EAXLISTENERPROPERTIES *props)
{
	alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
	
	alEffectf(effect, AL_REVERB_DENSITY,               clampF(powf(props->flEnvironmentSize, 3.0f) / 16.0f, 0.0f, 1.0f));
	alEffectf(effect, AL_REVERB_DIFFUSION,             props->flEnvironmentDiffusion);
	alEffectf(effect, AL_REVERB_GAIN,                  mB_to_gain((float)props->lRoom));
	alEffectf(effect, AL_REVERB_GAINHF,                mB_to_gain((float)props->lRoomHF));
	alEffectf(effect, AL_REVERB_DECAY_TIME,            props->flDecayTime);
	alEffectf(effect, AL_REVERB_DECAY_HFRATIO,         props->flDecayHFRatio);
	alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN,      clampF(mB_to_gain((float)props->lReflections), AL_EAXREVERB_MIN_REFLECTIONS_GAIN, AL_EAXREVERB_MAX_REFLECTIONS_GAIN));
	alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY,     props->flReflectionsDelay);
	alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN,      clampF(mB_to_gain((float)props->lReverb), AL_EAXREVERB_MIN_LATE_REVERB_GAIN, AL_EAXREVERB_MAX_LATE_REVERB_GAIN));
	alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY,     props->flReverbDelay);
	alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, clampF(mB_to_gain(props->flAirAbsorptionHF), AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF, AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF));
	alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR,   props->flRoomRolloffFactor);
	alEffecti(effect, AL_REVERB_DECAY_HFLIMIT,         (props->ulFlags&EAXLISTENERFLAGS_DECAYHFLIMIT) ? AL_TRUE : AL_FALSE);
}

void EAX3_SetReverbMix(ALuint filter, float mix)
{
	//long vol=(long)linear_to_dB(mix);
	//DSPROPERTY_EAXBUFFER_ROOMHF,
	//DSPROPERTY_EAXBUFFER_ROOM,
	//DSPROPERTY_EAXBUFFER_REVERBMIX,
	
	long mbvol = gain_to_mB(mix);
	float mb   = mbvol;
	float mbhf = mbvol;
	
	alFilteri(filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	alFilterf(filter, AL_LOWPASS_GAIN,   mB_to_gain(Min(mb, 0.0f)));
	alFilterf(filter, AL_LOWPASS_GAINHF, mB_to_gain(mbhf));
}

#endif