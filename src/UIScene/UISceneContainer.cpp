#include "BangEditor/UISceneContainer.h"

#include "Bang/Rect.h"
#include "Bang/Scene.h"
#include "Bang/Camera.h"
#include "Bang/GBuffer.h"
#include "Bang/UIButton.h"
#include "Bang/Texture2D.h"
#include "Bang/UICheckBox.h"
#include "Bang/IconManager.h"
#include "Bang/RectTransform.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIVerticalLayout.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/SelectionFramebuffer.h"

#include "BangEditor/ScenePlayer.h"
#include "BangEditor/UISceneImage.h"
#include "BangEditor/UISceneToolbar.h"
#include "BangEditor/EditorIconManager.h"
#include "BangEditor/EditorSceneManager.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

UISceneContainer::UISceneContainer()
{
    SetName("SceneTab");

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetFlexibleSize( Vector2(6.0f, 1.0f) );

    GameObjectFactory::CreateUIGameObjectInto(this);

    UIVerticalLayout *vl = AddComponent<UIVerticalLayout>();
    vl->SetChildrenHorizontalStretch(Stretch::Full);
    vl->SetPaddings(2);
    vl->SetSpacing(3);

    p_sceneToolbar = GameObject::Create<UISceneToolbar>();
    p_sceneImage = GameObject::Create<UISceneImage>();

    GetSceneToolbar()->SetParent(this);
    p_sceneImage->SetParent(this);

    GetSceneToolbar()->EventEmitter<IValueChangedListener>::RegisterListener(this);
}

UISceneContainer::~UISceneContainer()
{
}

void UISceneContainer::SetScene(Scene *scene)
{
    p_scene = scene;

    Texture2D *sceneTex = nullptr;
    if (GetScene())
    {
        Camera *sceneCam = GetSceneCamera(GetScene());
        if (sceneCam)
        {
            GBuffer *gbuffer =  sceneCam->GetGBuffer();
            sceneTex = gbuffer->GetAttachmentTexture(GBuffer::AttColor);
            if (Input::GetKey(Key::Z))
            {
                sceneTex = sceneCam->GetSelectionFramebuffer()->
                           GetAttachmentTexture(SelectionFramebuffer::AttColor);
            }
        }
    }
    p_sceneImage->SetSceneImageTexture(sceneTex);
}

Scene *UISceneContainer::GetScene() const
{
    return p_scene;
}

Rect UISceneContainer::GetSceneImageRectNDC() const
{
    return p_sceneImage->GetRectTransform()->GetViewportRectNDC();
}

UISceneToolbar *UISceneContainer::GetSceneToolbar() const
{
    return p_sceneToolbar;
}

void UISceneContainer::OnValueChanged(Object*)
{
    p_sceneImage->SetShowDebugStats( GetSceneToolbar()->IsShowDebugStatsChecked() );
}
