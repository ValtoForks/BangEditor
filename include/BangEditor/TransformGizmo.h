#ifndef TRANSFORMGIZMO_H
#define TRANSFORMGIZMO_H

#include "Bang/GameObject.h"

#include "BangEditor/SelectionGizmo.h"

USING_NAMESPACE_BANG
NAMESPACE_BANG_EDITOR_BEGIN

FORWARD class TranslateGizmo;
FORWARD class RotateGizmo;
FORWARD class ScaleGizmo;

class TransformGizmo : public GameObject,
                       public SelectionGizmo
{
    GAMEOBJECT_EDITOR(TransformGizmo);

public:
    // GameObject
    void Update() override;

    // SelectionGizmo
    void SetReferencedGameObject(GameObject *referencedGameObject) override;

private:
    enum class TransformMode { Translate, Rotate, Scale };
    TransformMode m_transformMode = TransformMode::Translate;

    TranslateGizmo *p_translateGizmo = nullptr;
    RotateGizmo    *p_rotateGizmo    = nullptr;
    ScaleGizmo     *p_scaleGizmo     = nullptr;

	TransformGizmo();
    virtual ~TransformGizmo();
};

NAMESPACE_BANG_EDITOR_END

#endif // TRANSFORMGIZMO_H

