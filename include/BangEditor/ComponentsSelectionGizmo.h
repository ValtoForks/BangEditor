#ifndef COMPONENTSSELECTIONGIZMO_H
#define COMPONENTSSELECTIONGIZMO_H

#include "Bang/GameObject.h"
#include "BangEditor/SelectionGizmo.h"

USING_NAMESPACE_BANG
NAMESPACE_BANG_EDITOR_BEGIN

class ComponentsSelectionGizmo : public GameObject,
                                 public SelectionGizmo
{
    GAMEOBJECT_EDITOR(ComponentsSelectionGizmo);

public:
    ComponentsSelectionGizmo();
    virtual ~ComponentsSelectionGizmo();

    // GameObject
    void Update() override;
    void RenderGizmos() override;

    // SelectionGizmo
    void SetReferencedGameObject(GameObject *referencedGameObject) override;

private:
    void RenderCameraGizmo(Camera *cam);
};

NAMESPACE_BANG_EDITOR_END

#endif // COMPONENTSSELECTIONGIZMO_H

