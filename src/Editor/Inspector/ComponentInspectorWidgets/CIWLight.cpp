#include "BangEditor/CIWLight.h"

#include <vector>

#include "Bang/Array.tcc"
#include "Bang/EventEmitter.h"
#include "Bang/EventEmitter.tcc"
#include "Bang/EventListener.tcc"
#include "Bang/GameObject.h"
#include "Bang/GameObject.tcc"
#include "Bang/GameObjectFactory.h"
#include "Bang/IEvents.h"
#include "Bang/Light.h"
#include "Bang/UIComboBox.h"
#include "Bang/UIInputNumber.h"
#include "Bang/UISlider.h"
#include "BangEditor/UIInputColor.h"

namespace Bang
{
class IEventsValueChanged;
}

using namespace Bang;
using namespace BangEditor;

void CIWLight::InitInnerWidgets()
{
    ComponentInspectorWidget::InitInnerWidgets();

    SetName("CIWLight");
    SetTitle("Light");

    p_intensityInput = GameObjectFactory::CreateUIInputNumber();
    p_intensityInput->EventEmitter<IEventsValueChanged>::RegisterListener(this);
    p_intensityInput->SetMinValue(0.0f);
    AddWidget("Intensity", p_intensityInput->GetGameObject());

    p_colorInput = GameObject::Create<UIInputColor>();
    p_colorInput->EventEmitter<IEventsValueChanged>::RegisterListener(this);
    AddWidget("Color", p_colorInput);

    p_shadowBiasInput = GameObjectFactory::CreateUISlider();
    p_shadowBiasInput->SetMinMaxValues(0.0f, 0.1f);
    p_shadowBiasInput->EventEmitter<IEventsValueChanged>::RegisterListener(
        this);
    AddWidget("Shadow bias", p_shadowBiasInput->GetGameObject());

    p_shadowTypeInput = GameObjectFactory::CreateUIComboBox();
    p_shadowTypeInput->AddItem("None", SCAST<int>(Light::ShadowType::NONE));
    p_shadowTypeInput->AddItem("Hard", SCAST<int>(Light::ShadowType::HARD));
    p_shadowTypeInput->AddItem("Soft", SCAST<int>(Light::ShadowType::SOFT));
    p_shadowTypeInput->EventEmitter<IEventsValueChanged>::RegisterListener(
        this);
    AddWidget("Shadow type", p_shadowTypeInput->GetGameObject());

    p_shadowMapSizeInput = GameObjectFactory::CreateUISlider();
    p_shadowMapSizeInput->SetMinMaxValues(1.0f, 4096);
    p_shadowMapSizeInput->GetInputNumber()->SetDecimalPlaces(0);
    p_shadowMapSizeInput->EventEmitter<IEventsValueChanged>::RegisterListener(
        this);
    AddWidget("Shadow map size", p_shadowMapSizeInput->GetGameObject());

    SetLabelsWidth(100);
}

void CIWLight::UpdateFromReference()
{
    ComponentInspectorWidget::UpdateFromReference();

    if (!p_intensityInput->HasFocus())
    {
        p_intensityInput->SetValue(GetLight()->GetIntensity());
    }

    if (!p_colorInput->HasFocus())
    {
        p_colorInput->SetColor(GetLight()->GetColor());
    }

    if (!p_shadowBiasInput->HasFocus())
    {
        p_shadowBiasInput->SetValue(GetLight()->GetShadowBias());
    }

    if (!p_shadowTypeInput->HasFocus())
    {
        p_shadowTypeInput->SetSelectionByValue(
            SCAST<int>(GetLight()->GetShadowType()));
    }

    if (!p_shadowMapSizeInput->HasFocus())
    {
        p_shadowMapSizeInput->SetValue(GetLight()->GetShadowMapSize().x);
    }
}

Light *CIWLight::GetLight() const
{
    return SCAST<Light *>(GetComponent());
}

void CIWLight::OnValueChangedCIW(EventEmitter<IEventsValueChanged> *object)
{
    ComponentInspectorWidget::OnValueChangedCIW(object);

    GetLight()->SetIntensity(p_intensityInput->GetValue());
    GetLight()->SetColor(p_colorInput->GetColor());
    GetLight()->SetShadowBias(p_shadowBiasInput->GetValue());
    GetLight()->SetShadowType(
        SCAST<Light::ShadowType>(p_shadowTypeInput->GetSelectedValue()));
    GetLight()->SetShadowMapSize(Vector2i(p_shadowMapSizeInput->GetValue()));
}
