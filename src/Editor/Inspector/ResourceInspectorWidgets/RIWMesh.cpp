#include "BangEditor/RIWMesh.h"

#include "Bang/UIInputText.h"
#include "Bang/RectTransform.h"
#include "Bang/TextureFactory.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/UIAspectRatioFitter.h"
#include "Bang/UIContentSizeFitter.h"

#include "BangEditor/PreviewViewer.h"
#include "BangEditor/MeshPreviewFactory.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

RIWMesh::RIWMesh()
{
}

RIWMesh::~RIWMesh()
{
}

void RIWMesh::Init()
{
    ResourceInspectorWidget::Init();

    SetName("RIWMesh");
    SetTitle("Mesh");

    GameObject *meshPreviewGo = GameObjectFactory::CreateUIGameObject();
    meshPreviewGo->GetRectTransform()->SetAnchors(Vector2::Zero);
    meshPreviewGo->GetRectTransform()->SetPivotPosition(Vector2::Zero);

    p_meshPreviewViewer = GameObject::Create<PreviewViewer>();

    p_numVertices = GameObjectFactory::CreateUIInputNumber();
    p_numVertices->SetDecimalPlaces(0);
    p_numVertices->GetInputText()->SetBlocked(true);

    p_numTriangles = GameObjectFactory::CreateUIInputNumber();
    p_numTriangles->SetDecimalPlaces(0);
    p_numTriangles->GetInputText()->SetBlocked(true);

    p_numBones = GameObjectFactory::CreateUIInputNumber();
    p_numBones->SetDecimalPlaces(0);
    p_numBones->GetInputText()->SetBlocked(true);

    AddWidget(p_meshPreviewViewer, 256);
    AddWidget( GameObjectFactory::CreateUIHSeparator(LayoutSizeType::PREFERRED, 5) );
    AddWidget("Num vertices",  p_numVertices->GetGameObject());
    AddWidget("Num triangles", p_numTriangles->GetGameObject());
    AddWidget("Num bones",     p_numBones->GetGameObject());

    SetLabelsWidth(130);
}

Mesh *RIWMesh::GetMesh() const
{
    return SCAST<Mesh*>(GetResource().Get());
}

void RIWMesh::OnResourceSet()
{
    RIWResource<Mesh>::OnResourceSet();
    p_numVertices->SetValue( GetMesh()->GetNumVertices() );
    p_numTriangles->SetValue( GetMesh()->GetNumTriangles() );
    p_numBones->SetValue( GetMesh()->GetBonesPool().Size() );
}

void RIWMesh::UpdateInputsFromResource()
{
    p_meshPreviewViewer->SetPreviewImageProvider([this](
                      const ResourcePreviewFactoryParameters &params)
    {
        return MeshPreviewFactory::GetPreviewTextureFor(GetMesh(), params);
    });
}

Texture2D* RIWMesh::GetIconTexture() const
{
    return MeshPreviewFactory::GetPreviewTextureFor( GetMesh() ).Get();
}

void RIWMesh::OnValueChangedRIWResource(EventEmitter<IEventsValueChanged>*)
{
    if (!GetMesh())
    {
        return;
    }
}

