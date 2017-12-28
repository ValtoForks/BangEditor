#include "BangEditor/CIWMeshRenderer.h"

#include "Bang/Mesh.h"
#include "Bang/UILabel.h"
#include "Bang/Resources.h"
#include "Bang/Extensions.h"
#include "Bang/UIInputText.h"
#include "Bang/MeshRenderer.h"
#include "Bang/UITextRenderer.h"

#include "BangEditor/UIInputFile.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

CIWMeshRenderer::CIWMeshRenderer()
{
    SetName("CWMeshRenderer");
    SetTitle("Mesh Renderer");

    p_meshInputFile = GameObject::Create<UIInputFile>();
    p_meshInputFile->SetExtensions( Extensions::GetModelExtensions().To<Array>() );
    p_meshInputFile->EventEmitter<IValueChangedListener>::RegisterListener(this);

    AddWidget("Mesh", p_meshInputFile);
}

CIWMeshRenderer::~CIWMeshRenderer()
{
}

void CIWMeshRenderer::Update()
{
    CIWRenderer::Update();

    Mesh *mesh = GetMeshRenderer()->GetSharedMesh();
    Path meshPath = mesh ? mesh->GetResourceFilepath() : Path::Empty;
    p_meshInputFile->SetPath(meshPath);
}

void CIWMeshRenderer::OnValueChanged(Object *object)
{
    CIWRenderer::OnValueChanged(object);

    if (object == p_meshInputFile)
    {
        RH<Mesh> mesh = Resources::Load<Mesh>(p_meshInputFile->GetPath());
        GetMeshRenderer()->SetMesh(mesh.Get());
    }
}

MeshRenderer *CIWMeshRenderer::GetMeshRenderer() const
{
    return p_relatedMeshRenderer;
}

void CIWMeshRenderer::SetComponent(Component *comp)
{
    CIWRenderer::SetComponent(comp);

    MeshRenderer *meshRenderer = DCAST<MeshRenderer*>(comp);
    ASSERT(meshRenderer);

    p_relatedMeshRenderer = meshRenderer;
}
