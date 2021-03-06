#include "BangEditor/EditorBehaviourManager.h"

#include <ostream>
#include <unordered_map>

#include "Bang/BangPreprocessor.h"
#include "Bang/CodePreprocessor.h"
#include "Bang/Debug.h"
#include "Bang/EventListener.tcc"
#include "Bang/Extensions.h"
#include "Bang/File.h"
#include "Bang/IEventsFileTracker.h"
#include "Bang/Library.h"
#include "Bang/List.h"
#include "Bang/List.tcc"
#include "Bang/MetaFilesManager.h"
#include "Bang/MutexLocker.h"
#include "Bang/Paths.h"
#include "Bang/StreamOperators.h"
#include "Bang/Time.h"
#include "Bang/USet.tcc"
#include "BangEditor/EditorFileTracker.h"
#include "BangEditor/EditorPaths.h"
#include "BangEditor/PlayState.h"
#include "BangEditor/ScenePlayer.h"

using namespace Bang;
using namespace BangEditor;

EditorBehaviourManager::EditorBehaviourManager()
{
    m_compileThreadPool.SetMaxThreadCount(2);
    m_compileThreadPool.SetName("BehaviourCompileThread");
}

EditorBehaviourManager::~EditorBehaviourManager()
{
}

void EditorBehaviourManager::Update()
{
    BehaviourManager::Update();

    if (ScenePlayer::GetPlayState() == PlayState::EDITING)
    {
        if (!AreAllBehavioursCompiledSuccessfully() && GetBehavioursLibrary())
        {
            SetBehavioursLibrary(nullptr);  // Invalidate current library
        }

        Array<Path> sourcesJustStartedToCompile;
        for (const Path &modifiedPath : m_modifiedBehaviourPaths)
        {
            GetMutex()->UnLock();

            if (!IsBeingCompiled(modifiedPath))
            {
                CompileBehaviourObjectAsync(modifiedPath);
            }

            if (IsBeingCompiled(modifiedPath))
            {
                sourcesJustStartedToCompile.PushBack(modifiedPath);
            }

            GetMutex()->Lock();
        }
        GetMutex()->UnLock();

        for (const Path &sourceJustStartedToCompile :
             sourcesJustStartedToCompile)
        {
            GetMutex()->Lock();
            m_modifiedBehaviourPaths.Remove(sourceJustStartedToCompile);
            GetMutex()->UnLock();
        }
    }
}

void EditorBehaviourManager::WaitForAsyncCompileJobs()
{
    while (IsSomeBehaviourBeingCompiled())
    {
        Thread::SleepCurrentThread(0.2f);
    }
}

bool EditorBehaviourManager::PrepareBehavioursLibrary()
{
    if (GetBehaviourSourcesPaths().Size() >= 1)
    {
        WaitForAsyncCompileJobs();

        // Update for changes
        if (EditorFileTracker *edf = EditorFileTracker::GetInstance())
        {
            edf->CheckFiles();
        }
        RemoveOrphanBehaviourLibrariesAndObjects();
        WaitForAsyncCompileJobs();

        // Compile
        if (!AreAllBehavioursCompiledSuccessfully())
        {
            CompileAllProjectBehaviours();
            if (!AreAllBehavioursCompiledSuccessfully())
            {
                Debug_Error("Can not prepare behaviours library. "
                            "Could not compile all behaviours.");
                return false;
            }
        }

        // Merge
        if (!IsBehavioursLibraryReady())
        {
            MergeIntoBehavioursLibrary();
            if (!IsBehavioursLibraryReady())
            {
                Debug_Error("Can not prepare behaviours library. "
                            "Could not link all behaviours into "
                            "behaviours library.");
                return false;
            }
        }
    }
    return true;
}

bool EditorBehaviourManager::AreAllBehavioursCompiled() const
{
    Array<Path> behaviourPaths = GetBehaviourSourcesPaths();
    for (const Path &behaviourPath : behaviourPaths)
    {
        if (IsBeingCompiled(behaviourPath) || !IsCompiled(behaviourPath))
        {
            return false;
        }
    }
    return true;
}

bool EditorBehaviourManager::IsSomeBehaviourBeingCompiled() const
{
    MutexLocker ml(GetMutex());
    BANG_UNUSED(ml);
    return !m_behavioursBeingCompiled.IsEmpty();
}

bool EditorBehaviourManager::AreAllBehavioursCompiledSuccessfully() const
{
    Array<Path> behaviourPaths = GetBehaviourSourcesPaths();
    for (const Path &behaviourPath : behaviourPaths)
    {
        if (IsBeingCompiled(behaviourPath) ||
            !IsCompiledSuccessfully(behaviourPath))
        {
            return false;
        }
    }
    return true;
}

bool EditorBehaviourManager::IsBehavioursLibraryReady() const
{
    return (GetBehavioursLibrary() != nullptr);
}

bool EditorBehaviourManager::IsCompiled(const Path &behaviourFilepath) const
{
    MutexLocker ml(GetMutex());
    BANG_UNUSED(ml);
    return m_compiledBehaviours.Contains(behaviourFilepath);
}

bool EditorBehaviourManager::IsCompiledWithError(
    const Path &behaviourFilepath) const
{
    const bool isCompiled = IsCompiled(behaviourFilepath);

    MutexLocker ml(GetMutex());
    BANG_UNUSED(ml);
    return isCompiled &&
           !m_successfullyCompiledBehaviours.Contains(behaviourFilepath);
}

bool EditorBehaviourManager::IsCompiledSuccessfully(
    const Path &behaviourFilepath) const
{
    const bool isCompiled = IsCompiled(behaviourFilepath);

    MutexLocker ml(GetMutex());
    BANG_UNUSED(ml);
    return isCompiled &&
           m_successfullyCompiledBehaviours.Contains(behaviourFilepath);
}

bool EditorBehaviourManager::IsBeingCompiled(
    const Path &behaviourFilepath) const
{
    MutexLocker ml(GetMutex());
    BANG_UNUSED(ml);
    return m_behavioursBeingCompiled.Contains(behaviourFilepath);
}

Behaviour *EditorBehaviourManager::CreateBehaviourInstance(
    const String &behaviourName)
{
    Library *behavioursLib =
        EditorBehaviourManager::GetActive()->GetBehavioursLibrary();
    return BehaviourManager::CreateBehaviourInstance(behaviourName,
                                                     behavioursLib);
}

bool EditorBehaviourManager::DeleteBehaviourInstance(
    const String &behaviourName,
    Behaviour *behaviour)
{
    Library *behavioursLib =
        EditorBehaviourManager::GetActive()->GetBehavioursLibrary();
    return BehaviourManager::DeleteBehaviourInstance(
        behaviourName, behaviour, behavioursLib);
}

bool EditorBehaviourManager::IsInstanceCreationAllowed() const
{
    return ScenePlayer::GetPlayState() != PlayState::EDITING;
}

EditorBehaviourManager *EditorBehaviourManager::GetActive()
{
    return DCAST<EditorBehaviourManager *>(BehaviourManager::GetActive());
}

Compiler::Result EditorBehaviourManager::CompileBehaviourObject(
    const Path &behaviourPath)
{
    Path outputObjPath = GetObjectOutputPath(behaviourPath);
    return EditorBehaviourManager::CompileBehaviourObject(
        behaviourPath, outputObjPath, BinType::BIN_DEBUG);
}
Compiler::Result EditorBehaviourManager::CompileBehaviourObject(
    const Path &behaviourFilepath,
    const Path &outputObjectFilepath,
    BinType binaryType)
{
    EditorBehaviourManager::RemoveBehaviourLibrariesOf(
        outputObjectFilepath.GetName());
    File::CreateDir(outputObjectFilepath.GetDirectory());

    Compiler::Job job = EditorBehaviourManager::CreateCompileBehaviourJob(
        behaviourFilepath, outputObjectFilepath, binaryType);
    job.libraries.Clear();

    Compiler::Result compileResult = Compiler::Compile(job);

    GetMutex()->Lock();
    m_compiledBehaviours.Add(behaviourFilepath);
    if (outputObjectFilepath.IsFile())
    {
        m_successfullyCompiledBehaviours.Add(behaviourFilepath);
    }
    else
    {
        m_successfullyCompiledBehaviours.Remove(behaviourFilepath);
    }
    m_behavioursBeingCompiled.Remove(behaviourFilepath);
    GetMutex()->UnLock();

    return compileResult;
}

void EditorBehaviourManager::CompileBehaviourObjectAsync(
    const Path &behaviourPath)
{
    BehaviourCompileRunnable *compileRunnable = new BehaviourCompileRunnable();
    compileRunnable->m_behaviourManager = this;
    compileRunnable->m_behaviourPath = behaviourPath;

    bool compilingThreadStarted = m_compileThreadPool.TryStart(compileRunnable);
    if (compilingThreadStarted)
    {
        MutexLocker ml(GetMutex());
        BANG_UNUSED(ml);
        Debug_DLog("Starting to compile " << behaviourPath);
        m_behavioursBeingCompiled.Add(behaviourPath);
    }
}

void EditorBehaviourManager::RemoveBehaviourLibrariesOf(
    const String &behaviourName)
{
    if (!behaviourName.IsEmpty())
    {
        Array<Path> libFilepaths = GetCompiledObjectsPaths();
        for (const Path &libFilepath : libFilepaths)
        {
            if (libFilepath.GetName() == behaviourName)
            {
                File::Remove(libFilepath);
            }
        }
    }
}

void EditorBehaviourManager::RemoveOrphanBehaviourLibrariesAndObjects()
{
    // Clean up unused behaviours shared libs
    if (EditorFileTracker *edf = EditorFileTracker::GetInstance())
    {
        Array<Path> currentSharedLibPaths = edf->GetTrackedPathsWithExtensions(
            {Extensions::GetDynamicLibExtension()});
        Library *behavioursLib =
            EditorBehaviourManager::GetActive()->GetBehavioursLibrary();
        {
            for (Path &libPath : currentSharedLibPaths)
            {
                if (!behavioursLib ||
                    (libPath != behavioursLib->GetLibraryPath()))
                {
                    File::Remove(libPath);
                }
            }
        }

        // Gather source names
        Array<String> behaviourSourcesNames;
        Array<Path> behaviourSourcePaths = GetBehaviourSourcesPaths();
        for (const Path &behaviourSourcePath : behaviourSourcePaths)
        {
            behaviourSourcesNames.PushBack(behaviourSourcePath.GetName());
        }

        // Clean up unused obj's
        Array<Path> currentObjPaths =
            edf->GetTrackedPathsWithExtensions({Extensions::GetObjExtension()});
        for (const Path &objPath : currentObjPaths)
        {
            if (objPath.GetDirectory().GetName() == "Libraries")
            {
                if (!behaviourSourcesNames.Contains(objPath.GetName()))
                {
                    File::Remove(objPath);
                }
            }
        }

        // Clean up unused metas
        MetaFilesManager::CreateMissingMetaFiles(
            EditorPaths::GetProjectLibrariesDir());
    }
}

void EditorBehaviourManager::CompileAllProjectBehaviours()
{
    Array<Path> behaviourSources =
        EditorBehaviourManager::GetBehaviourSourcesPaths();
    for (const Path &behaviourSourcePath : behaviourSources)
    {
        if (!IsCompiled(behaviourSourcePath) &&
            !IsBeingCompiled(behaviourSourcePath))
        {
            EditorBehaviourManager::CompileBehaviourObject(behaviourSourcePath);
        }
    }
}

void EditorBehaviourManager::MergeIntoBehavioursLibrary()
{
    Path outputLibPath =
        Paths::GetProjectLibrariesDir()
            .Append("Behaviours")
            .AppendExtension(Extensions::GetDynamicLibExtension())
            .AppendExtension(String(Time::GetNow().GetNanos()));

    if (EditorFileTracker *edf = EditorFileTracker::GetInstance())
    {
        edf->CheckFiles();
    }

    Array<Path> behaviourObjs = GetCompiledObjectsPaths();
    Compiler::Result mergeResult =
        MergeBehaviourObjects(behaviourObjs, outputLibPath, BinType::BIN_DEBUG);
    if (outputLibPath.IsFile())
    {
        Library *behavioursLibrary = new Library(outputLibPath);
        SetBehavioursLibrary(behavioursLibrary);
    }
    else
    {
        Debug_Error("Merge Behaviours library error: " << mergeResult.output);
    }
}

Compiler::Result EditorBehaviourManager::MergeBehaviourObjects(
    const Array<Path> &behaviourObjectFilepaths,
    const Path &outputLibFilepath,
    BinType binaryType)
{
    EditorBehaviourManager::RemoveOrphanBehaviourLibrariesAndObjects();
    File::CreateDir(outputLibFilepath.GetDirectory());

    Compiler::Job job =
        EditorBehaviourManager::CreateBaseCompileJob(binaryType, true);
    job.outputMode = Compiler::OutputType::SHARED_LIB;
    // job.libDirs.PushBack( Paths::GetEngineLibrariesDir( BinType::BIN_DEBUG )
    // );
    job.AddInputFiles(behaviourObjectFilepaths.To<Array>());
    job.outputFile = outputLibFilepath;

    return Compiler::Compile(job);
}

Array<Path> EditorBehaviourManager::GetCompiledObjectsPaths()
{
    Array<Path> objPaths;
    if (EditorFileTracker *edf = EditorFileTracker::GetInstance())
    {
        objPaths =
            edf->GetTrackedPathsWithExtensions({Extensions::GetObjExtension()});
        for (auto it = objPaths.Begin(); it != objPaths.End();)
        {
            bool removed = false;

            const Path &objPath = *it;
            if (objPath.IsHiddenFile() ||
                (objPath.GetDirectory().GetName() != "Libraries"))
            {
                it = objPaths.Remove(it);
                removed = true;
            }

            if (!removed)
            {
                ++it;
            }
        }
    }
    return objPaths;
}

Array<Path> EditorBehaviourManager::GetBehaviourSourcesPaths()
{
    if (EditorFileTracker *edf = EditorFileTracker::GetInstance())
    {
        return edf->GetTrackedPathsWithLastExtension(
            Extensions::GetSourceFileExtensions());
    }
    return Array<Path>();
}

Compiler::Job EditorBehaviourManager::CreateBaseCompileJob(BinType binaryType,
                                                           bool addLibs)
{
    const Path bangLibPath = EditorPaths::GetBangDynamicLibPath();
    if (!bangLibPath.Exists())
    {
        Debug_Error(bangLibPath << " not found! Needed to build behaviours..."
                                << "Will not compile them!");
        return Compiler::Job();
    }

    Compiler::Job job;

    if (addLibs)
    {
        bool isBangLibStatic = (bangLibPath.GetExtension() == "a");
        if (isBangLibStatic)
        {
#ifdef __linux__
            const String Quote = "\"";
            job.AddInputFile("   -Wl,--whole-archive " + Quote +
                             bangLibPath.GetAbsolute() + Quote +
                             " -Wl,--no-whole-archive");
#elif _WIN32
            job.AddInputFile(bangLibPath.GetAbsolute());
#endif
        }
        else
        {
            job.libDirs.PushBack(EditorPaths::GetEditorLibrariesDir());
#ifdef __linux__
            job.libraries.PushBack("Bang");
#elif _WIN32
            job.libraries.PushBack("libBang.lib");
#endif
        }
    }

#ifdef _WIN32
    // System
    job.libraries.PushBack("kernel32.lib");
    job.libraries.PushBack("user32.lib");
    job.libraries.PushBack("gdi32.lib");
    job.libraries.PushBack("winspool.lib");
    job.libraries.PushBack("comdlg32.lib");
    job.libraries.PushBack("advapi32.lib");
    job.libraries.PushBack("shell32.lib");
    job.libraries.PushBack("ole32.lib");
    job.libraries.PushBack("oleaut32.lib");
    job.libraries.PushBack("uuid.lib");
    job.libraries.PushBack("odbc32.lib");
    job.libraries.PushBack("opengl32.lib");
    job.libraries.PushBack("glu32.lib");
    job.libraries.PushBack("odbccp32.lib");
    job.libraries.PushBack("shlwapi.lib");

    // Bang deps
    job.libraries.PushBack("glew32.lib");
    job.libraries.PushBack("jpeg.lib");
    job.libraries.PushBack("libpng16d.lib");
    job.libraries.PushBack("libsndfile-1.lib");
    job.libraries.PushBack("SDL2d.lib");
    job.libraries.PushBack("OpenAL32.lib");
    job.libraries.PushBack("assimp-vc140-mt.lib");
    job.libraries.PushBack("libsdl2_ttf.lib");
    job.libraries.PushBack("libfreetype2.lib");
    job.libraries.PushBack("IrrXML.lib");
    job.libraries.PushBack("zlibstaticd.lib");

    Array<Path> dependenciesLibPaths = EditorPaths::GetEditorLibrariesDir()
                                           .Append(Paths::GetBuildType())
                                           .GetSubPaths(Bang::FindFlag::SIMPLE);
    for (const Path &depLib : dependenciesLibPaths)
    {
        if (depLib.GetExtension() == Extensions::GetStaticLibExtension())
        {
            job.libraries.PushBack(depLib.GetAbsolute());
        }
    }
#endif

#ifdef __linux__
    job.flags = {"-fPIC", "--std=c++11", "-Wl,-O0,--export-dynamic"};
    if (binaryType == BinType::BIN_DEBUG)
    {
        job.flags.PushBack(Array<String>({"-O0", "-g", "-Wl,-O0"}));
        job.flags.PushBack("-D_DEBUG");
        job.flags.PushBack("-DDEBUG");
    }
    else
    {
        job.flags.PushBack(Array<String>({"-O3", "-Wl,-O3"}));
        job.flags.PushBack("-DNDEBUG");
    }

#elif _WIN32

    job.flags = {""};
    if (binaryType == BinType::BIN_DEBUG)
    {
        job.flags.PushBack(Array<String>({""}));
        job.flags.PushBack("/D_DEBUG");
        job.flags.PushBack("/DDEBUG");
    }
    else
    {
        job.flags.PushBack(Array<String>({""}));
        job.flags.PushBack("/DNDEBUG");
    }

#endif

    return job;
}

Path EditorBehaviourManager::GetObjectOutputPath(const Path &inputBehaviourPath)
{
    return Paths::GetProjectLibrariesDir()
        .Append(inputBehaviourPath.GetName())
        .AppendExtension(Extensions::GetObjExtension())
        .AppendExtension(String(Time::GetNow().GetNanos()));
}

Compiler::Job EditorBehaviourManager::CreateCompileBehaviourJob(
    const Path &behaviourFilepath,
    const Path &outputObjectFilepath,
    BinType binaryType)
{
    Compiler::Job job =
        EditorBehaviourManager::CreateBaseCompileJob(binaryType, false);
    job.outputMode = Compiler::OutputType::OBJECT;
    job.includePaths.PushBack(Paths::GetEngineIncludeDirs());
    job.includePaths.PushBack(Paths::GetProjectIncludeDirs());
    job.AddInputFile(behaviourFilepath);
    job.outputFile = outputObjectFilepath;

    return job;
}

Mutex *EditorBehaviourManager::GetMutex() const
{
    return &m_mutex;
}

void GetAffectedBehaviourSourcesWhenModifying(
    const Path &modifiedPath,
    const Array<Path> &allBehaviourSources,
    USet<Path> *alreadySeenPaths,
    Array<Path> *affectedBehaviourSources)
{
    if (!alreadySeenPaths->Contains(modifiedPath))
    {
        alreadySeenPaths->Add(modifiedPath);

        for (const Path &behaviourSourcePath : allBehaviourSources)
        {
            Array<Path> behaviourSourceIncludes =
                CodePreprocessor::GetSourceIncludePaths(
                    behaviourSourcePath, Paths::GetProjectIncludeDirs(), true);
            if (behaviourSourceIncludes.Contains(modifiedPath))
            {
                affectedBehaviourSources->PushBack(behaviourSourcePath);
                GetAffectedBehaviourSourcesWhenModifying(
                    behaviourSourcePath,
                    allBehaviourSources,
                    alreadySeenPaths,
                    affectedBehaviourSources);
            }
        }
    }
}

Array<Path> GetAffectedBehaviourSourcesWhenModifying(
    const Path &modifiedPath,
    const Array<Path> &allBehaviourSources)
{
    USet<Path> alreadySeenPaths;
    Array<Path> affectedBehaviourSources;
    affectedBehaviourSources.PushBack(modifiedPath);
    GetAffectedBehaviourSourcesWhenModifying(modifiedPath,
                                             allBehaviourSources,
                                             &alreadySeenPaths,
                                             &affectedBehaviourSources);
    return affectedBehaviourSources;
}

void EditorBehaviourManager::OnPathAdded(const Path &path)
{
    if (Extensions::Equals(path.GetLastExtension(),
                           Extensions::GetBehaviourExtensions()))
    {
        OnPathModified(path);
    }
}

void EditorBehaviourManager::OnPathModified(const Path &path)
{
    if (!path.IsHiddenFile() &&
        Extensions::Equals(path.GetLastExtension(),
                           Extensions::GetBehaviourExtensions()))
    {
        Array<Path> allBehaviourSources = GetBehaviourSourcesPaths();
        Array<Path> affectedBehaviourSources =
            GetAffectedBehaviourSourcesWhenModifying(path, allBehaviourSources);

        MutexLocker ml(GetMutex());
        BANG_UNUSED(ml);
        for (const Path &affectedBehaviourSource : affectedBehaviourSources)
        {
            if (Extensions::Equals(affectedBehaviourSource.GetLastExtension(),
                                   Extensions::GetSourceFileExtensions()))
            {
                m_modifiedBehaviourPaths.Add(affectedBehaviourSource);
                m_compiledBehaviours.Remove(affectedBehaviourSource);
                RemoveBehaviourLibrariesOf(affectedBehaviourSource.GetName());
                m_successfullyCompiledBehaviours.Remove(
                    affectedBehaviourSource);
            }
        }
    }
}

void EditorBehaviourManager::OnPathRemoved(const Path &path)
{
    if (Extensions::Equals(path.GetLastExtension(),
                           Extensions::GetBehaviourExtensions()))
    {
        MutexLocker ml(GetMutex());
        BANG_UNUSED(ml);
        m_successfullyCompiledBehaviours.Remove(path);
        m_modifiedBehaviourPaths.Remove(path);
        m_compiledBehaviours.Remove(path);
    }
}

void EditorBehaviourManager::BehaviourCompileRunnable::Run()
{
    BangPreprocessor::Preprocess(m_behaviourPath.WithExtension("h"));

    Compiler::Result compileResult =
        m_behaviourManager->CompileBehaviourObject(m_behaviourPath);

    const Path &outputPath = compileResult.compileJob.outputFile;
    if (!outputPath.IsFile())
    {
        Debug_Error("Behaviour '" << outputPath.GetName()
                                  << "' did not compile correctly: "
                                  << compileResult.output);
    }
    else
    {
        Debug_DLog("Behaviour '" << outputPath.GetName()
                                 << "' correctly compiled!");
    }
}
