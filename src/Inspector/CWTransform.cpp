#include "BangEditor/CWTransform.h"

#include "Bang/UILabel.h"
#include "Bang/Transform.h"
#include "Bang/GameObject.h"
#include "Bang/UIInputNumber.h"
#include "Bang/UITextRenderer.h"
#include "Bang/UIImageRenderer.h"
#include "Bang/UILayoutElement.h"
#include "Bang/UIVerticalLayout.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/UIHorizontalLayout.h"
#include "Bang/UIContentSizeFitter.h"

#include "BangEditor/UIInputVector.h"

USING_NAMESPACE_BANG
USING_NAMESPACE_BANG_EDITOR

CWTransform::CWTransform(Transform *transform)
{
    SetName("CWTransform");
    SetTitle("Transform");

    GameObject *vlGo = GameObjectFactory::CreateUIGameObjectNamed("VLGo");
    UIVerticalLayout *vl = vlGo->AddComponent<UIVerticalLayout>();
    vl->SetSpacing(2);

    p_posIV   = GameObject::Create<UIInputVector>("Position", 3);
    p_rotIV   = GameObject::Create<UIInputVector>("Rotation", 3);
    p_scaleIV = GameObject::Create<UIInputVector>("Scale   ", 3);

    p_posIV->Set(transform->GetLocalPosition());
    p_rotIV->Set(transform->GetLocalRotation().GetEulerAngles());
    p_scaleIV->Set(transform->GetLocalScale());

    p_posIV->EventEmitter<IValueChangedListener>::RegisterListener(this);
    p_rotIV->EventEmitter<IValueChangedListener>::RegisterListener(this);
    p_scaleIV->EventEmitter<IValueChangedListener>::RegisterListener(this);

    GetContainer()->SetAsChild(vlGo);
    vlGo->SetAsChild(p_posIV);
    vlGo->SetAsChild(p_rotIV);
    vlGo->SetAsChild(p_scaleIV);

    p_relatedTransform = transform;
}

CWTransform::~CWTransform()
{
}

void CWTransform::OnValueChanged(Object *object)
{
    p_relatedTransform->SetLocalPosition(p_posIV->GetVector3());
    p_relatedTransform->SetLocalEuler(p_rotIV->GetVector3());
    p_relatedTransform->SetLocalScale(p_scaleIV->GetVector3());

}
