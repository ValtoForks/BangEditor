#ifndef EDITOR_H
#define EDITOR_H

#include <vector>

#include "Bang/Array.tcc"
#include "Bang/Bang.h"
#include "Bang/BangDefines.h"
#include "Bang/EventEmitter.h"
#include "Bang/EventEmitter.tcc"
#include "Bang/EventListener.h"
#include "Bang/EventListener.tcc"
#include "Bang/GameObject.h"
#include "Bang/IEvents.h"
#include "Bang/IEventsDestroy.h"
#include "Bang/Object.h"
#include "Bang/SceneManager.h"
#include "BangEditor/BangEditor.h"
#include "BangEditor/IEventsEditor.h"

namespace BangEditor {
class EditorSettings;
class IEventsEditor;
}  // namespace BangEditor
namespace Bang {
class GameObject;
class IEventsDestroy;
class IEventsSceneManager;
class Path;
}  // namespace Bang

NAMESPACE_BANG_BEGIN
FORWARD class Scene;

NAMESPACE_BANG_END

USING_NAMESPACE_BANG
NAMESPACE_BANG_EDITOR_BEGIN

class Editor : public EventEmitter<IEventsEditor>,
               public EventListener<IEventsSceneManager>,
               public EventListener<IEventsDestroy>
{
public:
    static GameObject *GetSelectedGameObject();
    static void SelectGameObject(GameObject *selectedGameObject,
                                 bool registerUndo = true);

    static bool IsEditingScene();

    static Editor* GetInstance();

private:
    GameObject *p_selectedGameObject = nullptr;
    EditorSettings *m_editorSettings = nullptr;

    Editor();
    virtual ~Editor();

    void Init();

    static void OnPathSelected(const Path &path);

    EditorSettings* GetEditorSettings() const;

    void SelectGameObject_(GameObject *selectedGameObject, bool registerUndo);

    // IEventsSceneManager
    void OnSceneLoaded(Scene *scene, const Path &sceneFilepath) override;

    // IEventsDestroy
    void OnDestroyed(EventEmitter<IEventsDestroy> *object) override;

    friend class Explorer;
    friend class EditorSettings;
    friend class EditorApplication;
    friend class UndoRedoGameObjectSelection;
};

NAMESPACE_BANG_EDITOR_END

#endif // EDITOR_H

