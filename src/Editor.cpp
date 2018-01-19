#include "BangEditor/Editor.h"

#include "Bang/AudioManager.h"

#include "BangEditor/EditorScene.h"
#include "BangEditor/EditorSettings.h"
#include "BangEditor/EditorSceneManager.h"
#include "BangEditor/NotSelectableInEditor.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

Editor::Editor()
{
}

Editor::~Editor()
{
    delete m_editorSettings;
}

void Editor::Init()
{
    m_editorSettings = new EditorSettings();
    GetEditorSettings()->Init();
}

GameObject *Editor::GetSelectedGameObject()
{
    Editor *ed = Editor::GetInstance();
    return ed ? ed->p_selectedGameObject : nullptr;
}

void Editor::SelectGameObject(GameObject *selectedGameObject)
{
    Editor *ed = Editor::GetInstance();
    if (ed) { ed->_SelectGameObject(selectedGameObject); }
}

void Editor::SetEditorPlayState(EditorPlayState playState)
{
    if (playState != Editor::GetEditorPlayState())
    {
        EditorPlayState previousPlayState = Editor::GetEditorPlayState();

        Editor *ed = Editor::GetInstance();
        ed->m_currentPlayState = playState;

        ed->EventEmitter<IEditorListener>::PropagateToListeners(
                    &IEditorListener::OnPlayStateChanged,
                    previousPlayState,
                    ed->m_currentPlayState);

        AudioManager::SetPlayOnStartBlocked( Editor::IsEditingScene() );
    }
}

EditorPlayState Editor::GetEditorPlayState()
{
    Editor *ed = Editor::GetInstance();
    return ed ? ed->m_currentPlayState : Undef<EditorPlayState>();
}

bool Editor::IsEditingScene()
{
    return Editor::GetEditorPlayState() == EditorPlayState::Editing;
}

void Editor::OnDestroyed(EventEmitter<IDestroyListener> *object)
{
    if (GetSelectedGameObject() == object)
    {
        _SelectGameObject(nullptr);
    }
}

void Editor::_SelectGameObject(GameObject *selectedGameObject)
{
    bool isSelectable = !selectedGameObject ||
         (!selectedGameObject->GetComponent<NotSelectableInEditor>() &&
          !selectedGameObject->GetComponentInParent<NotSelectableInEditor>());

    if (selectedGameObject != GetSelectedGameObject() && isSelectable)
    {
        p_selectedGameObject = selectedGameObject;
        if (GetSelectedGameObject())
        {
            GetSelectedGameObject()->
                    EventEmitter<IDestroyListener>::RegisterListener(this);
        }

        // Propagate event
        EventEmitter<IEditorListener>::
            PropagateToListeners(&IEditorListener::OnGameObjectSelected,
                                 selectedGameObject);
    }
}

void Editor::OnPathSelected(const Path &path)
{
    Editor *ed = Editor::GetInstance();
    ASSERT(ed);

    Editor::SelectGameObject(nullptr);

    ed->EventEmitter<IEditorListener>::PropagateToListeners(
                &IEditorListener::OnExplorerPathSelected, path);
}

EditorSettings *Editor::GetEditorSettings() const
{
    return m_editorSettings;
}

Editor *Editor::GetInstance()
{
    EditorScene *edScene = EditorSceneManager::GetEditorScene();
    return edScene ? edScene->GetEditor() : nullptr;
}

