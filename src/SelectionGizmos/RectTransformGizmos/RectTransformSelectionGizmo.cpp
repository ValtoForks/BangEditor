#include "BangEditor/RectTransformSelectionGizmo.h"

#include "Bang/Rect.h"
#include "Bang/Gizmos.h"
#include "Bang/GEngine.h"
#include "Bang/Material.h"
#include "Bang/RectTransform.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/MaterialFactory.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/SelectionFramebuffer.h"

#include "BangEditor/RectTransformCornerSelectionGizmo.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

RectTransformSelectionGizmo::RectTransformSelectionGizmo()
{
    SetName("RectTransformSelectionGizmo");

    AddComponent<Transform>();

    p_botLeftCorner  = GameObject::Create<RectTransformCornerSelectionGizmo>();
    p_topLeftCorner  = GameObject::Create<RectTransformCornerSelectionGizmo>();
    p_topRightCorner = GameObject::Create<RectTransformCornerSelectionGizmo>();
    p_botRightCorner = GameObject::Create<RectTransformCornerSelectionGizmo>();

    p_selectionGo = GameObjectFactory::CreateUIGameObject(true);
    p_selectionGo->SetName("RectSelection");

    p_selectionGo->SetParent(this);
    p_botLeftCorner->SetParent(this);
    p_topLeftCorner->SetParent(this);
    p_topRightCorner->SetParent(this);
    p_botRightCorner->SetParent(this);

}

RectTransformSelectionGizmo::~RectTransformSelectionGizmo()
{
}

void RectTransformSelectionGizmo::Update()
{
    GameObject::Update();

    GameObject *refGo = GetReferencedGameObject();
    if (!refGo || !refGo->GetTransform()) { return; }
}

void RectTransformSelectionGizmo::Render(RenderPass renderPass, bool renderChildren)
{
    bool selection = GL::IsBound( GEngine::GetActiveSelectionFramebuffer() );
    p_selectionGo->SetEnabled(selection);

    GameObject::Render(renderPass, renderChildren);

    if (renderPass != RenderPass::Overlay) { return; }

    GameObject *refGo = GetReferencedGameObject();
    if (!refGo) { return; }

    // Gizmos rendering!
    Gizmos::Reset();

    // Rect
    Gizmos::SetSelectable(nullptr);
    Gizmos::SetThickness(1.0f);
    Gizmos::SetColor( Color::White );
    Gizmos::SetRenderPass(renderPass);
    Gizmos::RenderRect( refGo->GetRectTransform()->GetViewportRectNDC() );

    // Grabs

}

void RectTransformSelectionGizmo::SetReferencedGameObject(
                                    GameObject *referencedGameObject)
{
    SelectionGizmo::SetReferencedGameObject(referencedGameObject);

    p_botLeftCorner->SetReferencedGameObject (referencedGameObject);
    p_topLeftCorner->SetReferencedGameObject (referencedGameObject);
    p_topRightCorner->SetReferencedGameObject(referencedGameObject);
    p_botRightCorner->SetReferencedGameObject(referencedGameObject);
}