#include "BangEditor/EditorIconManager.h"

#include "Bang/Texture2D.h"
#include "Bang/Resources.h"
#include "Bang/Extensions.h"

#include "BangEditor/EditorPaths.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

RH<Texture2D> EditorIconManager::GetIcon(const Path &path)
{
    if (!path.IsDir())
    {
        if ( Extensions::Has(path, Extensions::GetImageExtensions()) )
        {
            return Resources::Load<Texture2D>(path);
        }
        else { return GetIcon(path.GetExtension()); }
    }
    return GetIconTexture("Folder");
}

RH<Texture2D> EditorIconManager::GetIcon(const String &ext)
{
    if ( Extensions::Equals(ext, Extensions::GetTTFExtensions()) )
    {
        return GetIconTexture("Letter");
    }
    else if ( Extensions::Equals(ext, Extensions::GetAudioClipExtensions()) )
    {
        return GetIconTexture("Audio");
    }
    else if ( Extensions::Equals(ext, Extensions::GetSceneExtension()) )
    {
        return GetIconTexture("Scene");
    }
    else if ( Extensions::Equals(ext, Extensions::GetPrefabExtension()) )
    {
        return GetIconTexture("Pill");
    }
    else if ( Extensions::Equals(ext, Extensions::GetModelExtensions()) )
    {
        return GetIconTexture("Cube");
    }
    else if ( Extensions::Equals(ext, Extensions::GetBehaviourExtensions()) )
    {
        return GetIconTexture("Brackets");
    }
    else if ( Extensions::Equals(ext, Extensions::GetMaterialExtension()) )
    {
        return GetWhiteSphereIcon();
    }
    else
    {
        return GetIconTexture("File");
    }
}

RH<Texture2D> EditorIconManager::GetRightArrowAndBarIcon()
{ return GetIconTexture("RightArrowAndBar"); }
RH<Texture2D> EditorIconManager::GetDoubleBarIcon()
{ return GetIconTexture("DoubleBar"); }
RH<Texture2D> EditorIconManager::GetBackArrowIcon()
{ return GetIconTexture("BackArrow"); }
RH<Texture2D> EditorIconManager::GetLensIcon()
{ return GetIconTexture("Lens"); }
RH<Texture2D> EditorIconManager::GetLensLittleIcon()
{ return GetIconTexture("LensLittle"); }
RH<Texture2D> EditorIconManager::GetSquareIcon()
{ return GetIconTexture("Square"); }
RH<Texture2D> EditorIconManager::GetWhiteSphereIcon()
{ return GetIconTexture("WhiteSphere"); }

RH<Texture2D> EditorIconManager::GetIconTexture(const String &filename)
{
    RH<Texture2D> iconTex =
        Resources::Load<Texture2D>(EditorPaths::GetEditorResourcesDir().Append("Icons").
                                   Append(filename).AppendExtension("png"));
    iconTex.Get()->SetFilterMode(GL::FilterMode::Bilinear);
    iconTex.Get()->SetWrapMode(GL::WrapMode::ClampToEdge);
    return iconTex;
}

