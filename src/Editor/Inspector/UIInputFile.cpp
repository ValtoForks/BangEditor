#include "BangEditor/UIInputFile.h"

#include "Bang/Assets.h"
#include "Bang/Color.h"
#include "Bang/GameObject.tcc"
#include "Bang/GameObjectFactory.h"
#include "Bang/IEventsDragDrop.h"
#include "Bang/IEventsValueChanged.h"
#include "Bang/Paths.h"
#include "Bang/Stretch.h"
#include "Bang/Texture2D.h"
#include "Bang/TextureFactory.h"
#include "Bang/UIButton.h"
#include "Bang/UICanvas.h"
#include "Bang/UIDragDroppable.h"
#include "Bang/UIFocusable.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UIInputText.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UITextRenderer.h"
#include "BangEditor/EditorDialog.h"
#include "BangEditor/EditorTextureFactory.h"
#include "BangEditor/Explorer.h"
#include "BangEditor/ExplorerItem.h"
#include "BangEditor/Inspector.h"

namespace Bang
{
class Texture2D;
}

using namespace Bang;
using namespace BangEditor;

UIInputFile::UIInputFile()
{
    SetName("UIInputFile");
    SetPath(Path::Empty());
}

UIInputFile::~UIInputFile()
{
}

bool UIInputFile::CanDoZoom() const
{
    return HasExistingPath();
}

bool UIInputFile::HasExistingPath() const
{
    return (GetPath().IsFile() || Assets::IsEmbeddedAsset(GetPath()));
}

AH<Texture2D> UIInputFile::GetPreviewTextureFromPath(const Path &path)
{
    return AH<Texture2D>(EditorTextureFactory::GetIconForPath(path));
}

bool UIInputFile::AcceptsDrag(EventEmitter<IEventsDragDrop> *dd_) const
{
    if (UIDragDroppable *dragDroppable = DCAST<UIDragDroppable *>(dd_))
    {
        if (ExplorerItem *expItem =
                DCAST<ExplorerItem *>(dragDroppable->GetGameObject()))
        {
            Path draggedPath = expItem->GetPath();
            return draggedPath.HasExtension(GetExtensions());
        }
    }
    return false;
}

void UIInputFile::OnDropped(EventEmitter<IEventsDragDrop> *dd_)
{
    if (UIDragDroppable *dragDroppable = DCAST<UIDragDroppable *>(dd_))
    {
        if (ExplorerItem *expItem =
                DCAST<ExplorerItem *>(dragDroppable->GetGameObject()))
        {
            Path draggedPath = expItem->GetPath();
            bool acceptedFileType = draggedPath.HasExtension(GetExtensions());
            if (acceptedFileType)
            {
                SetPath(draggedPath);
            }
        }
    }
}

void UIInputFile::OnSearchButtonClicked()
{
    Path openPath;
    bool accepted;
    EditorDialog::GetAsset(
        "Get Asset...", GetExtensions(), &openPath, &accepted);
    if (accepted)
    {
        SetPath(openPath);
    }
}

void UIInputFile::OnOpenButtonClicked()
{
    if (!Paths::IsEnginePath(GetPath()))
    {
        Explorer::GetInstance()->SelectPath(GetPath(), true);
    }
    else
    {
        // Dont select in explorer, but just show in inspector
        Inspector::GetActive()->ShowPath(GetPath());
    }
}

void UIInputFile::SetPath(const Path &path)
{
    if (path != GetPath())
    {
        m_path = path;

        bool pathIsGood = (GetPath().IsFile() || Assets::IsEmbeddedAsset(path));

        String textContent = "None";
        if (pathIsGood)
        {
            textContent = GetPath().GetNameExt();
            if (Assets::IsEmbeddedAsset(GetPath()))
            {
                textContent.Prepend(GetPath().GetDirectory().GetNameExt() +
                                    "/");
            }
        }
        GetInputText()->GetText()->SetContent(textContent);

        GetOpenButton()->SetBlocked(!pathIsGood);

        EventEmitter<IEventsValueChanged>::PropagateToListeners(
            &IEventsValueChanged::OnValueChanged, this);
    }

    AH<Texture2D> previewTex;
    if (HasExistingPath())
    {
        previewTex = GetPreviewTextureFromPath(path);
    }
    SetPreviewIcon(previewTex.Get());
}

void UIInputFile::SetExtensions(const Array<String> &extensions)
{
    m_extensions = extensions;
}

Path UIInputFile::GetPath() const
{
    return m_path;
}

const Array<String> &UIInputFile::GetExtensions() const
{
    return m_extensions;
}
