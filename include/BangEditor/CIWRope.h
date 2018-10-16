#ifndef CIWROPE_H
#define CIWROPE_H

#include "Bang/Bang.h"
#include "Bang/BangDefines.h"
#include "Bang/String.h"
#include "BangEditor/BangEditor.h"
#include "BangEditor/CIWRenderer.h"

namespace Bang {
class IEventsValueChanged;
class UICheckBox;
template <class > class EventEmitter;
}  // namespace Bang

FORWARD NAMESPACE_BANG_BEGIN
FORWARD class Rope;
FORWARD class UIInputNumber;
FORWARD class UISlider;

FORWARD NAMESPACE_BANG_END

USING_NAMESPACE_BANG
NAMESPACE_BANG_EDITOR_BEGIN

class CIWRope : public CIWRenderer
{
    GAMEOBJECT_EDITOR(CIWRope);

public:
    // InspectorWidget
    virtual void InitInnerWidgets() override;
    virtual void UpdateFromReference() override;

    // ComponentInspectorWidget
    virtual void OnValueChangedCIW(EventEmitter<IEventsValueChanged> *object) override;

private:
    UIInputNumber *p_numPointsInput      = nullptr;
    UISlider      *p_bouncinessInput     = nullptr;
    UISlider      *p_dampingInput        = nullptr;
    UIInputNumber *p_springsForceInput   = nullptr;
    UIInputNumber *p_ropeLengthInput     = nullptr;
    UICheckBox    *p_seeDebugPointsInput = nullptr;

    CIWRope();
    virtual ~CIWRope();

    Rope *GetRope() const;
};

NAMESPACE_BANG_EDITOR_END

#endif // CIWROPE_H

