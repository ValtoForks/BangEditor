#ifndef HIDEINHIERARCHY_H
#define HIDEINHIERARCHY_H

#include "Bang/Component.h"

#include "BangEditor/BangEditor.h"

using namespace Bang;
namespace BangEditor
{
class HideInHierarchy : public Component
{
    COMPONENT_WITHOUT_CLASS_ID(HideInHierarchy)

public:
    HideInHierarchy() = default;
    virtual ~HideInHierarchy() override = default;
};
}

#endif  // HIDEINHIERARCHY_H
