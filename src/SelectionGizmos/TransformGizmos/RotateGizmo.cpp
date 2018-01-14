#include "BangEditor/RotateGizmo.h"

#include "Bang/Mesh.h"
#include "Bang/Camera.h"
#include "Bang/Material.h"
#include "Bang/Transform.h"
#include "Bang/MeshFactory.h"
#include "Bang/MeshRenderer.h"
#include "Bang/LineRenderer.h"
#include "Bang/RendererFactory.h"

#include "BangEditor/RotateGizmoAxis.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

RotateGizmo::RotateGizmo()
{
    AddComponent<Transform>();

    p_sphereGo = GameObjectFactory::CreateGameObject();
    p_sphereGo->SetName("Sphere");

    p_sphereRenderer = p_sphereGo->AddComponent<MeshRenderer>();
    RendererFactory::ConvertToGizmoRenderer(p_sphereRenderer);
    p_sphereRenderer->SetMesh( MeshFactory::GetSphere().Get() );
    p_sphereRenderer->GetMaterial()->SetDiffuseColor( Color(1, 1, 1, 0.25f) );
    p_sphereGo->GetTransform()->SetLocalScale(0.97f);

    p_sphereBoundsGo = GameObjectFactory::CreateGameObject();
    p_sphereBoundsGo->SetName("SphereBounds");

    p_sphereBoundsRenderer = p_sphereBoundsGo->AddComponent<LineRenderer>();
    RendererFactory::ConvertToGizmoRenderer(p_sphereBoundsRenderer);
    p_sphereBoundsRenderer->GetMaterial()->SetDiffuseColor(Color::Black);
    CreateSphereBoundsPoints();

    p_sphereGo->SetParent(this);
    p_sphereBoundsGo->SetParent(this);

    p_axisX = GameObject::Create<RotateGizmoAxis>();
    p_axisY = GameObject::Create<RotateGizmoAxis>();
    p_axisZ = GameObject::Create<RotateGizmoAxis>();

    p_axisX->SetAxis(Axis3DExt::X);
    p_axisY->SetAxis(Axis3DExt::Y);
    p_axisZ->SetAxis(Axis3DExt::Z);

    p_axisX->SetParent(this);
    p_axisY->SetParent(this);
    p_axisZ->SetParent(this);
}

RotateGizmo::~RotateGizmo()
{
}

void RotateGizmo::Update()
{
    GameObject::Update();
}

void RotateGizmo::Render(RenderPass rp, bool renderChildren)
{
    // Update sphere so that it faces camera
    Camera *cam = Camera::GetActive();
    Transform *camT = cam->GetGameObject()->GetTransform();
    Vector3 camCenterDir = (GetTransform()->GetPosition() - camT->GetPosition());
    p_sphereBoundsGo->GetTransform()->SetRotation(
                                    Quaternion::LookDirection(camCenterDir) );

    GameObject::Render(rp, renderChildren); // Do Render
}

void RotateGizmo::CreateSphereBoundsPoints()
{
    constexpr int numSegments = 32;
    constexpr float angleStep = (Math::Pi * 2.0f) / numSegments;

    // Get circle points
    Array<Vector3> circlePoints;
    for (uint i = 0; i < numSegments + 1; ++i)
    {
        float angle = angleStep * i;
        Vector3 newPoint (Math::Sin(angle), Math::Cos(angle), 0.0f);
        circlePoints.PushBack(newPoint);
    }

    // Get renderer points
    Array<Vector3> rendererPoints;
    for (uint i = 0; i < circlePoints.Size() - 1; ++i)
    {
        rendererPoints.PushBack( circlePoints[i+0] );
        rendererPoints.PushBack( circlePoints[i+1] );
    }

    p_sphereBoundsRenderer->SetPoints(rendererPoints);
}


void RotateGizmo::SetReferencedGameObject(GameObject *referencedGameObject)
{
    SelectionGizmo::SetReferencedGameObject(referencedGameObject);

    p_axisX->SetReferencedGameObject( GetReferencedGameObject() );
    p_axisY->SetReferencedGameObject( GetReferencedGameObject() );
    p_axisZ->SetReferencedGameObject( GetReferencedGameObject() );
}