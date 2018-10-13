#ifndef RIWAUDIOCLIP_H
#define RIWAUDIOCLIP_H

#include "Bang/Bang.h"
#include "Bang/AudioClip.h"

#include "BangEditor/BangEditor.h"
#include "BangEditor/RIWResource.h"

FORWARD NAMESPACE_BANG_BEGIN
FORWARD class UIButton;
FORWARD class UIInputText;
FORWARD class ALAudioSource;
FORWARD class UIInputNumber;
FORWARD NAMESPACE_BANG_END

USING_NAMESPACE_BANG
NAMESPACE_BANG_EDITOR_BEGIN

class RIWAudioClip : public RIWResource<AudioClip>
{
    GAMEOBJECT_EDITOR(RIWAudioClip);

public:
    // InspectorWidget
    void Init() override;

private:
    UIButton *p_playStopButton = nullptr;
    ALAudioSource *p_alAudioSourceBeingPlayed = nullptr;

    UIInputText *p_duration = nullptr;
    UIInputText *p_frequency = nullptr;
    UIInputNumber *p_bitDepth = nullptr;
    UIInputNumber *p_bufferSize = nullptr;
    UIInputNumber *p_numChannels = nullptr;

	RIWAudioClip();
	virtual ~RIWAudioClip();

    void Play();
    void Stop();
    AudioClip* GetAudioClip() const;

    // RIWResource
    void UpdateInputsFromResource() override;
    Texture2D *GetIconTexture() const override;
    Color GetIconTint() const override;

    // RIWResource
    void OnValueChangedRIWResource(
                        EventEmitter<IEventsValueChanged> *object) override;

    // IEventsDestroy
    void OnDestroyed(EventEmitter<IEventsDestroy> *object) override;
};

NAMESPACE_BANG_EDITOR_END

#endif // RIWAUDIOCLIP_H

