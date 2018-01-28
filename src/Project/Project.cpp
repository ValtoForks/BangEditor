#include "BangEditor/Project.h"

#include <string>

#include "Bang/Time.h"
#include "Bang/Paths.h"
#include "Bang/XMLNode.h"
#include "Bang/Extensions.h"
#include "Bang/SceneManager.h"

#include "BangEditor/EditorSceneManager.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

Project::Project()
{
    m_id = GUID::GetRandomGUID();
}

Project::~Project()
{
}

const Path&  Project::GetProjectDirPath() const
{
    return m_projectRootFilepath;
}

Path Project::GetProjectAssetsRootFilepath() const
{
    return Path(m_projectRootFilepath + "/Assets");
}

Path Project::GetProjectFileFilepath() const
{
    return Path(m_projectRootFilepath + "/" +
                GetProjectName() + "." + Extensions::GetProjectExtension());
}

String Project::GetProjectName() const
{
    return m_projectRootFilepath.GetName();
}

const GUID& Project::GetProjectRandomId() const
{
    return m_id;
}

void Project::SetProjectRootFilepath(const Path &projectDir)
{
    m_projectRootFilepath = projectDir;
}

bool Project::OpenFirstFoundScene() const
{
    Path firstFoundScenePath = GetFirstFoundScenePath();
    if (firstFoundScenePath.IsFile())
    {
        EditorSceneManager::LoadScene(firstFoundScenePath);
    }

    return (firstFoundScenePath.IsFile());
}

Path Project::GetFirstFoundScenePath() const
{
    List<Path> sceneFilepaths = GetProjectAssetsRootFilepath()
                                .GetFiles(Path::FindFlag::Recursive,
                                           {Extensions::GetSceneExtension()});
    Paths::SortPathsByName(&sceneFilepaths);
    return !sceneFilepaths.IsEmpty() ? sceneFilepaths.Back() : Path::Empty;
}


void Project::ImportXML(const XMLNode &xmlInfo)
{
    if (xmlInfo.Contains("RandomID"))
    { m_id = xmlInfo.Get<GUID>("RandomID"); }
}

void Project::ExportXML(XMLNode *xmlInfo) const
{
    xmlInfo->SetTagName("Project");
    xmlInfo->Set("RandomID", m_id);
}
