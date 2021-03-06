#include "BangEditor/ExplorerItemFactory.h"

#include "Bang/Array.tcc"
#include "Bang/AssetHandle.h"
#include "Bang/Assets.h"
#include "Bang/Assets.tcc"
#include "Bang/Extensions.h"
#include "Bang/GameObject.h"
#include "Bang/GameObject.tcc"
#include "Bang/Model.h"
#include "Bang/Path.h"
#include "Bang/Paths.h"
#include "BangEditor/ExplorerItem.h"
#include "BangEditor/ModelExplorerItem.h"
#include "BangEditor/PrefabExplorerItem.h"

namespace Bang
{
class String;
}

using namespace Bang;
using namespace BangEditor;

ExplorerItem *ExplorerItemFactory::CreateExplorerItem(const Path &path)
{
    ExplorerItem *explorerItem = nullptr;
    if (path.HasExtension(Extensions::GetModelExtensions()))
    {
        explorerItem = new ModelExplorerItem();
    }
    else if (path.HasExtension(Extensions::GetPrefabExtension()))
    {
        explorerItem = new PrefabExplorerItem();
    }
    else
    {
        explorerItem = new ExplorerItem();
    }

    explorerItem->SetPath(path);

    return explorerItem;
}

Array<ExplorerItem *> ExplorerItemFactory::CreateAndGetSubPathsExplorerItems(
    const Path &path,
    bool addBackItem,
    bool recursive)
{
    Array<ExplorerItem *> expItems;

    if (addBackItem)
    {
        Path prevDirPath = path.GetDirectory();
        ExplorerItem *prevDirExpItem =
            ExplorerItemFactory::CreateExplorerItem(prevDirPath);
        prevDirExpItem->SetPathString("..");
        expItems.PushBack(prevDirExpItem);
    }

    if (path.IsDir())
    {
        Array<Path> subPaths = path.GetSubPaths(FindFlag::SIMPLE);
        Paths::SortPathsByExtension(&subPaths);
        Paths::SortPathsByName(&subPaths);
        for (const Path &subPath : subPaths)
        {
            ExplorerItem *childExpItem =
                ExplorerItemFactory::CreateExplorerItem(subPath);
            expItems.PushBack(childExpItem);

            if (recursive)
            {
                expItems.PushBack(CreateAndGetSubPathsExplorerItems(
                    subPath, false, recursive));
            }
        }
    }
    else if (path.IsFile())
    {
        if (path.HasExtension(Extensions::GetModelExtensions()))
        {
            AH<Model> model = Assets::Load<Model>(path);

            for (const String &meshName : model.Get()->GetMeshesNames())
            {
                Path meshPath = path.Append(meshName);
                ExplorerItem *meshExplorerItem =
                    ExplorerItemFactory::CreateExplorerItem(meshPath);
                expItems.PushBack(meshExplorerItem);
            }

            for (const String &materialName : model.Get()->GetMaterialsNames())
            {
                Path materialPath = path.Append(materialName);
                ExplorerItem *materialExplorerItem =
                    ExplorerItemFactory::CreateExplorerItem(materialPath);
                expItems.PushBack(materialExplorerItem);
            }

            for (const String &animationName :
                 model.Get()->GetAnimationsNames())
            {
                Path animationPath = path.Append(animationName);
                ExplorerItem *animationExplorerItem =
                    ExplorerItemFactory::CreateExplorerItem(animationPath);
                expItems.PushBack(animationExplorerItem);
            }
        }
    }

    return expItems;
}

bool ExplorerItemFactory::CanHaveSubpaths(const Path &path)
{
    return path.IsDir() || path.HasExtension(Extensions::GetModelExtensions());
}

ExplorerItemFactory::ExplorerItemFactory()
{
}

ExplorerItemFactory::~ExplorerItemFactory()
{
}
