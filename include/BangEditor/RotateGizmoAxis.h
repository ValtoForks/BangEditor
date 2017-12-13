#ifndef ROTATEGIZMOAXIS_H
#define ROTATEGIZMOAXIS_H

#include "BangEditor/TransformGizmoAxis.h"

NAMESPACE_BANG_BEGIN
NAMESPACE_BANG_END

USING_NAMESPACE_BANG
NAMESPACE_BANG_EDITOR_BEGIN

class RotateGizmoAxis : public TransformGizmoAxis
{
    GAMEOBJECT_EDITOR(RotateGizmoAxis);

public:
    // GameObject
    void Update() override;

    void SetAxis(Axis3D axis) override;

private:
    using SelectionState = SelectionGizmo::SelectionState;

    RotateGizmoAxis();
    virtual ~RotateGizmoAxis();

    void SetColor(const Color &color) override;

};

NAMESPACE_BANG_EDITOR_END

#endif // ROTATEGIZMOAXIS_H
