#include "BangEditor/ResourceInspectorWidgetFactory.h"

#include "Bang/Path.h"
#include "Bang/String.h"
#include "Bang/Extensions.h"

#include "BangEditor/RIWModel.h"
#include "BangEditor/RIWTexture.h"
#include "BangEditor/RIWMaterial.h"
#include "BangEditor/RIWBehaviour.h"
#include "BangEditor/RIWTextureCubeMap.h"
#include "BangEditor/RIWPhysicsMaterial.h"
#include "BangEditor/ResourceInspectorWidget.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

InspectorWidget *ResourceInspectorWidgetFactory::Create(const Path &path)
{
    ResourceInspectorWidget *fiw = nullptr;
    if (path.HasExtension(Extensions::GetMaterialExtension()))
    {
        fiw = GameObject::Create<RIWMaterial>();
    }
    if (path.HasExtension(Extensions::GetPhysicsMaterialExtension()))
    {
        fiw = GameObject::Create<RIWPhysicsMaterial>();
    }
    else if (path.HasExtension(Extensions::GetImageExtensions()))
    {
        fiw = GameObject::Create<RIWTexture>();
    }
    else if (path.HasExtension(Extensions::GetModelExtensions()))
    {
        fiw = GameObject::Create<RIWModel>();
    }
    else if (path.HasExtension(Extensions::GetTextureCubeMapExtension()))
    {
        fiw = GameObject::Create<RIWTextureCubeMap>();
    }
    else if (path.HasExtension(Extensions::GetBehaviourExtensions()))
    {
        fiw = GameObject::Create<RIWBehaviour>();
    }

    if (fiw)
    {
        fiw->Init();
        fiw->SetPath(path);
    }

    return fiw;
}
