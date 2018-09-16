#include "BangEditor/Inspector.h"

#include "Bang/GL.h"
#include "Bang/Input.h"
#include "Bang/Scene.h"
#include "Bang/UILabel.h"
#include "Bang/Material.h"
#include "Bang/MetaNode.h"
#include "Bang/Resources.h"
#include "Bang/GLUniforms.h"
#include "Bang/UIFocusable.h"
#include "Bang/SceneManager.h"
#include "Bang/UIScrollArea.h"
#include "Bang/RectTransform.h"
#include "Bang/UIScrollPanel.h"
#include "Bang/UITextRenderer.h"
#include "Bang/IEventsDragDrop.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UIVerticalLayout.h"
#include "Bang/UIRendererCacher.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/UIContentSizeFitter.h"

#include "BangEditor/MenuBar.h"
#include "BangEditor/MenuItem.h"
#include "BangEditor/Selection.h"
#include "BangEditor/EditorScene.h"
#include "BangEditor/UIContextMenu.h"
#include "BangEditor/EditorClipboard.h"
#include "BangEditor/InspectorWidget.h"
#include "BangEditor/UndoRedoManager.h"
#include "BangEditor/EditorFileTracker.h"
#include "BangEditor/EditorSceneManager.h"
#include "BangEditor/ComponentInspectorWidget.h"
#include "BangEditor/GameObjectInspectorWidget.h"
#include "BangEditor/UndoRedoSerializableChange.h"
#include "BangEditor/ResourceInspectorWidgetFactory.h"
#include "BangEditor/ComponentInspectorWidgetFactory.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

Inspector::Inspector()
{
    SetName("Inspector");

    UILayoutElement *le = AddComponent<UILayoutElement>();
    le->SetMinWidth(200);
    le->SetPreferredWidth(400);
    le->SetFlexibleSize( Vector2::One );

    GameObjectFactory::CreateUIGameObjectInto(this);
    UIRendererCacher *rendCacher = GameObjectFactory::CreateUIRendererCacherInto(this);
    GameObject *rendererCacherContainer = rendCacher->GetContainer();
    rendCacher->GetContainer()->GetRectTransform()->SetMargins(0, 10, 0, 5);

    GameObject *mainVLGo = rendererCacherContainer;
    UIVerticalLayout *mainVL = mainVLGo->AddComponent<UIVerticalLayout>();
    BANG_UNUSED(mainVL);
    UILayoutElement *mainVLLE = mainVLGo->AddComponent<UILayoutElement>();
    mainVLLE->SetFlexibleSize(Vector2::One);

    UIScrollPanel *scrollPanel = GameObjectFactory::CreateUIScrollPanel();
    scrollPanel->GetScrollArea()->GetBackground()->SetVisible(false);
    scrollPanel->GetScrollArea()->GetBackground()->SetTint(
                                            Color::White.WithValue(0.7f));

    UILayoutElement *scrollLE = scrollPanel->GetGameObject()->
                                AddComponent<UILayoutElement>();
    scrollLE->SetFlexibleSize( Vector2::One );

    // GameObject *topSpacer = GameObjectFactory::CreateUISpacer(LayoutSizeType::Min,
    //                                                           Vector2i(0, 30));

    GameObject *widgetsVLGo = GameObjectFactory::CreateUIGameObjectNamed("MainVL");
    widgetsVLGo->GetRectTransform()->SetPivotPosition( Vector2(-1, 1) );
    widgetsVLGo->SetParent(rendererCacherContainer);

    UILabel *titleGo = GameObjectFactory::CreateUILabel();
    GameObject *goNameLabelGo = titleGo->GetGameObject();
    UILayoutElement *goNameLE = goNameLabelGo->GetComponent<UILayoutElement>();
    goNameLE->SetFlexibleHeight(0.0f);
    p_titleText = titleGo->GetText();
    p_titleText->SetTextSize(18);
    p_titleText->SetHorizontalAlign(HorizontalAlignment::LEFT);

    UIVerticalLayout *widgetsVL = widgetsVLGo->AddComponent<UIVerticalLayout>();
    widgetsVL->SetSpacing(20);
    widgetsVL->SetPaddingTop(10);
    widgetsVL->SetPaddingLeft(10);
    widgetsVL->SetPaddingRight(10);

    UIContentSizeFitter *vlCSF = widgetsVLGo->AddComponent<UIContentSizeFitter>();
    vlCSF->SetVerticalSizeType(LayoutSizeType::PREFERRED);

    p_mainVL = widgetsVL;
    p_scrollPanel = scrollPanel;

    GetScrollPanel()->SetHorizontalScrollEnabled(false);
    GetScrollPanel()->SetVerticalScrollBarSide(HorizontalSide::RIGHT);
    GetScrollPanel()->GetScrollArea()->SetContainedGameObject(
                                                GetMainVL()->GetGameObject() );
    GetScrollPanel()->SetVerticalShowScrollMode(ShowScrollMode::WHEN_NEEDED);

    titleGo->GetGameObject()->SetParent(mainVLGo);
    GameObjectFactory::CreateUIVSpacer(LayoutSizeType::MIN, 5)->
                       SetParent(mainVLGo);
    scrollPanel->GetGameObject()->SetParent(mainVLGo);

    p_blockLayer = GameObjectFactory::CreateUIImage(Color::Red.WithAlpha(0.3f));
    p_blockLayer->GetGameObject()->AddComponent<UIFocusable>();
    p_blockLayer->GetGameObject()->SetParent(GetScrollPanel()->GetGameObject());
    SetCurrentWidgetBlocked(false);

    UIFocusable *focusable = AddComponent<UIFocusable>();
    p_contextMenu = AddComponent<UIContextMenu>();
    p_contextMenu->SetCreateContextMenuCallback([this](MenuItem *menuRootItem)
    {
        OnCreateContextMenu(menuRootItem);
    });
    p_contextMenu->SetFocusable(focusable);

    // Add a bit of margin below...
    GameObjectFactory::CreateUIVSpacer(LayoutSizeType::MIN, 150)->
                        SetParent(GetWidgetsContainer());

    EditorFileTracker::GetInstance()->
                      EventEmitter<IEventsFileTracker>::RegisterListener(this);
    Editor::GetInstance()->EventEmitter<IEventsEditor>::RegisterListener(this);
    SceneManager::GetActive()->EventEmitter<IEventsSceneManager>::
                               RegisterListener(this);
}

Inspector::~Inspector()
{

}

void Inspector::Update()
{
    GameObject::Update();

    if (p_currentGameObject)
    {
        p_titleText->SetContent( p_currentGameObject->GetName() );
    }
}

void Inspector::ShowSerializable(Serializable *serializable)
{
    if (serializable)
    {
        if (Resource *res = DCAST<Resource*>(serializable))
        {
            Path resPath = res->GetResourceFilepath();
            if (resPath.IsFile())
            {
                ShowPath(resPath);
            }
        }
        else if (GameObject *gameObject = DCAST<GameObject*>(serializable))
        {
            ShowGameObject(gameObject);
        }
        else if (Component *comp = DCAST<Component*>(serializable))
        {
            ShowGameObject(comp->GetGameObject());
        }
    }
    else
    {
        Clear();
    }
}

void Inspector::ShowPath(const Path &path)
{
    if (m_currentOpenPath != path)
    {
        if (!path.IsDir())
        {
            InspectorWidget *fiw = ResourceInspectorWidgetFactory::Create(path);
            if (fiw || path.IsFile())
            {
                Clear();
            }

            if (fiw)
            {
                p_titleText->SetContent(path.GetNameExt());
                m_currentOpenPath = path;
                AddWidget(fiw);

                bool isEnginePath = Paths::IsEnginePath(m_currentOpenPath);
                SetCurrentWidgetBlocked(isEnginePath);
            }
        }
    }
}

void Inspector::ShowGameObject(GameObject *go)
{
    Clear();
    if(!go || go->IsWaitingToBeDestroyed())
    {
        return;
    }

    p_currentGameObject = go;
    GetCurrentGameObject()->EventEmitter<IEventsComponent>::RegisterListener(this);

    p_titleText->SetContent(go->GetName());
    GetCurrentGameObject()->EventEmitter<IEventsDestroy>::RegisterListener(this);

    GameObjectInspectorWidget *giw = GameObject::Create<GameObjectInspectorWidget>();
    giw->Init();
    giw->SetGameObject(go);
    AddWidget(giw, 0);

    int i = 0;
    for (Component *comp : go->GetComponents())
    {
        OnComponentAdded(comp, i);
        ++i;
    }
}

void Inspector::ShowInspectorWidget(InspectorWidget *inspectorWidget)
{
    Clear();
    p_titleText->SetContent(inspectorWidget->GetTitle());
    AddWidget(inspectorWidget);
}

const Path &Inspector::GetCurrentPath() const
{
    return m_currentOpenPath;
}

GameObject *Inspector::GetCurrentGameObject() const
{
    return p_currentGameObject;
}

void Inspector::OnSceneLoaded(Scene *scene, const Path &)
{
    BANG_UNUSED(scene);
    if (p_currentGameObject)
    {
        Clear();
    }
}

void Inspector::OnDestroyed(EventEmitter<IEventsDestroy> *ee)
{
    ASSERT(ee == p_currentGameObject);
    Clear();
}

void Inspector::OnPathRemoved(const Path &removedPath)
{
    if (GetCurrentPath() == removedPath)
    {
        Clear();
    }
}

void Inspector::OnCreateContextMenu(MenuItem *menuRootItem)
{
    menuRootItem->SetFontSize(12);

    if (GameObject *currentGameObject = GetCurrentGameObject())
    {
        {
            MetaNode undoMetaBefore = currentGameObject->GetMeta();

            MenuItem *addComp = menuRootItem->AddItem("Add Component");
            MenuBar::CreateComponentsMenuInto(addComp);
            addComp->SetSelectedCallback([](MenuItem*){});

            MetaNode currentMeta = currentGameObject->GetMeta();
            UndoRedoManager::PushAction(
                    new UndoRedoSerializableChange(currentGameObject,
                                                   undoMetaBefore, currentMeta) );
        }

        // menuRootItem->AddSeparator();

        MenuItem *paste = menuRootItem->AddItem("Paste");
        paste->SetSelectedCallback([this, currentGameObject](MenuItem*)
        {
            MetaNode undoMetaBefore = currentGameObject->GetMeta();

            Component *copiedComp = EditorClipboard::GetCopiedComponent();
            Component *newComponent = copiedComp->Clone();
            GetCurrentGameObject()->AddComponent(newComponent);

            MetaNode currentMeta = currentGameObject->GetMeta();
            UndoRedoManager::PushAction(
                new UndoRedoSerializableChange(currentGameObject,
                                               undoMetaBefore, currentMeta) );
        });
        paste->SetOverAndActionEnabled( (EditorClipboard::HasCopiedComponent()) );
    }
}

void Inspector::OnExplorerPathSelected(const Path &path)
{
    ShowPath(path);
}

void Inspector::OnGameObjectSelected(GameObject *selectedGameObject)
{
    ShowGameObject(selectedGameObject);
}

void Inspector::OnComponentAdded(Component *addedComponent, int index)
{
    if (ComponentInspectorWidget *compWidget =
            ComponentInspectorWidgetFactory::Create(addedComponent))
    {
        m_objToWidget.Add(addedComponent, compWidget);
        AddWidget(compWidget, index+1);
    }
}

void Inspector::OnComponentRemoved(Component *removedComponent,
                                   GameObject *previousGameObject)
{
    BANG_UNUSED(previousGameObject);

    if (m_objToWidget.ContainsKey(removedComponent))
    {
        RemoveWidget(m_objToWidget.Get(removedComponent));
        m_objToWidget.Remove(removedComponent);
    }
}

Inspector* Inspector::GetActive()
{
    EditorSceneManager *esm = EditorSceneManager::GetActive();
    EditorScene *es = esm ? esm->GetEditorScene() : nullptr;
    return es ? es->GetInspector() : nullptr;
}

GameObject *Inspector::GetWidgetsContainer() const
{
    return GetScrollPanel()->GetScrollArea()->GetContainedGameObject();
}

UIVerticalLayout *Inspector::GetMainVL() const
{
    return p_mainVL;
}

UIScrollPanel* Inspector::GetScrollPanel() const
{
    return p_scrollPanel;
}

void Inspector::AddWidget(InspectorWidget *widget, int _index)
{
    int index = _index >= 0 ? _index : int(m_widgets.Size());

    m_widgets.Insert(widget, index);
    Color bgColor = Color::LightGray.WithValue(0.9f);
    widget->SetBackgroundColor(bgColor);
    widget->SetParent( GetWidgetsContainer(), index );
}

void Inspector::RemoveWidget(InspectorWidget *widget)
{
    m_widgets.Remove(widget);
    GameObject::Destroy(widget); // To allow move up/down to work e.g.
}

void Inspector::RemoveWidget(int index)
{
    auto it = m_widgets.Begin();
    std::advance(it, index);
    RemoveWidget( *it );
}

void Inspector::SetCurrentWidgetBlocked(bool blocked)
{
    p_blockLayer->GetGameObject()->SetEnabled( blocked );

    Array<Object*> childrenAndChildrenComps;

    Array<GameObject*> children = GetWidgetsContainer()->GetChildrenRecursively();
    for (GameObject *child : children)
    {
        childrenAndChildrenComps.PushBack(child);
        childrenAndChildrenComps.PushBack(child->GetComponents());
    }

    for (Object *obj : childrenAndChildrenComps)
    {
        if (EventListener<IEventsFocus> *focusListener =
                DCAST<EventListener<IEventsFocus>*>(obj))
        {
            focusListener->SetReceiveEvents(!blocked);
        }

        if (EventListener<IEventsDragDrop> *ddListener =
                DCAST<EventListener<IEventsDragDrop>*>(obj))
        {
            ddListener->SetReceiveEvents(!blocked);
        }
    }

}

void Inspector::Clear()
{
    p_titleText->SetContent("");
    GetScrollPanel()->SetScrolling(Vector2i::Zero);

    while (!m_widgets.IsEmpty())
    {
        InspectorWidget *widget = m_widgets.Front();
        RemoveWidget(widget);
    }

    if (GetCurrentGameObject())
    {
        GetCurrentGameObject()->EventEmitter<IEventsDestroy>::UnRegisterListener(this);
        GetCurrentGameObject()->EventEmitter<IEventsComponent>::UnRegisterListener(this);
        p_currentGameObject = nullptr;
    }
    m_currentOpenPath = Path::Empty;
    SetCurrentWidgetBlocked(false);
}
