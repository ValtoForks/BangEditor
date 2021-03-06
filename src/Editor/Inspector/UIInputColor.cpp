#include "BangEditor/UIInputColor.h"

#include "Bang/Cursor.h"
#include "Bang/EventListener.tcc"
#include "Bang/GameObject.tcc"
#include "Bang/GameObjectFactory.h"
#include "Bang/IEventsFocus.h"
#include "Bang/IEventsValueChanged.h"
#include "Bang/Material.h"
#include "Bang/Object.h"
#include "Bang/Stretch.h"
#include "Bang/TextureFactory.h"
#include "Bang/UIButton.h"
#include "Bang/UIFocusable.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILayoutElement.h"
#include "BangEditor/EditorDialog.h"
#include "BangEditor/EditorTextureFactory.h"

namespace Bang
{
class Texture2D;
}

using namespace Bang;
using namespace BangEditor;

UIInputColor::UIInputColor()
{
    SetName("UIInputColor");
    GameObjectFactory::CreateUIGameObjectInto(this);

    UIHorizontalLayout *hl = AddComponent<UIHorizontalLayout>();
    hl->SetChildrenVerticalStretch(Stretch::FULL);
    hl->SetSpacing(5);

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetFlexibleWidth(1.0f);

    GameObject *colorImgCont = GameObjectFactory::CreateUIGameObject();
    UIFocusable *colorImgFocusable = colorImgCont->AddComponent<UIFocusable>();
    colorImgFocusable->SetCursorType(Cursor::Type::HAND);
    UILayoutElement *colorImgLE = colorImgCont->AddComponent<UILayoutElement>();
    colorImgLE->SetFlexibleWidth(1.0f);

    GameObjectFactory::AddOuterBorder(colorImgCont);

    p_bgCheckerboardImage = colorImgCont->AddComponent<UIImageRenderer>();
    p_bgCheckerboardImage->SetImageTexture(TextureFactory::GetCheckerboard());
    p_bgCheckerboardImage->GetMaterial()->SetAlbedoUvMultiply(Vector2(1, 1));

    p_colorImage = GameObjectFactory::CreateUIImage();

    m_colorPickerReporter = new ColorPickerReporter();
    m_colorPickerReporter->EventEmitter<IEventsValueChanged>::RegisterListener(
        this);

    Texture2D *lensIcon = EditorTextureFactory::GetLensLittleIcon();
    p_searchColorButton = GameObjectFactory::CreateUIButton("", lensIcon);
    p_searchColorButton->SetIcon(lensIcon, Vector2i(16));
    p_searchColorButton->AddClickedCallback([this]() {
        EditorDialog::GetColor(
            "Pick Color...", GetColor(), m_colorPickerReporter);
    });

    colorImgFocusable->AddEventCallback(
        [this](UIFocusable *, const UIEvent &event) {
            if (event.type == UIEvent::Type::MOUSE_CLICK_DOWN)
            {
                p_searchColorButton->Click();
                return UIEventResult::INTERCEPT;
            }
            return UIEventResult::IGNORE;
        });

    colorImgCont->SetParent(this);
    p_searchColorButton->GetGameObject()->SetParent(this);
    p_colorImage->GetGameObject()->SetParent(colorImgCont);
}

UIInputColor::~UIInputColor()
{
    GameObject::Destroy(m_colorPickerReporter);
}

void UIInputColor::Update()
{
    GameObject::Update();
    if (m_colorPickerReporter->IsPicking())
    {
        SetColor(m_colorPickerReporter->GetPickedColor());
    }
}

void UIInputColor::OnValueChanged(EventEmitter<IEventsValueChanged> *)
{
    // Dont do anything here, since this is being called from the
    // color picker window loop, and nasty things can happen
}

void UIInputColor::SetColor(const Color &color)
{
    if (color != GetColor())
    {
        m_color = color;
        p_colorImage->SetTint(GetColor());
        m_colorPickerReporter->SetPickedColor(GetColor());

        EventEmitter<IEventsValueChanged>::PropagateToListeners(
            &IEventsValueChanged::OnValueChanged, this);
    }
}

const Color &UIInputColor::GetColor() const
{
    return m_color;
}

bool UIInputColor::HasFocus() const
{
    return !m_colorPickerReporter->HasFinished();
}
