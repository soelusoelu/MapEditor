#include "AABBSelector.h"
#include "AABBMouseScaler.h"
#include "../Camera/Camera.h"
#include "../Collider/AABBCollider.h"
#include "../Mesh/MeshComponent.h"
#include "../../Collision/Collision.h"
#include "../../GameObject/GameObject.h"
#include "../../GameObject/GameObjectManager.h"
#include "../../Input/Input.h"

AABBSelector::AABBSelector(GameObject& gameObject)
    : Component(gameObject)
    , mAABBMouseScaler(nullptr)
    , mSelectAABBIndex(INVALID_INDEX)
{
}

AABBSelector::~AABBSelector() = default;

void AABBSelector::start() {
    const auto& gameObjectManager = gameObject().getGameObjectManager();
    mCamera = gameObjectManager.find("Camera")->componentManager().getComponent<Camera>();

    mAABBMouseScaler = getComponent<AABBMouseScaler>();
}

void AABBSelector::update() {
    //マウスの左ボタンが押した瞬間じゃなければ終了
    if (!Input::mouse().getMouseButtonDown(MouseCode::LeftButton)) {
        return;
    }

    //AABBの面の編集点を選択したか
    bool selectedPoint = false;

    //いずれかのAABBが選択されているなら編集可
    if (mSelectAABBIndex != INVALID_INDEX) {
        selectedPoint = mAABBMouseScaler->selectBoxPoint();
    }

    //編集点を選択していないならコライダーを選択する
    if (!selectedPoint) {
        selectAABB();
    }
}

void AABBSelector::setAABBsFromMesh(const MeshComponent& mesh) {
    mColliders = mesh.getComponents<AABBCollider>();
    //インデックスは無効という意味で-1
    mSelectAABBIndex = INVALID_INDEX;
}

void AABBSelector::selectAABB() {
    //カメラからマウスの位置へ向かうレイを取得
    const auto& rayCameraToMousePos = mCamera->screenToRay(Input::mouse().getMousePosition());

    //すべてのコライダーとレイの衝突判定
    for (size_t i = 0; i < mColliders.size(); ++i) {
        if (Intersect::intersectRayAABB(rayCameraToMousePos, mColliders[i]->getAABB())) {
            mAABBMouseScaler->setAABB(mColliders[i]);
            mSelectAABBIndex = i;
        }
    }
}
