#include "CollideMouseOperator.h"
#include "AABBMouseScaler.h"
#include "../Camera/Camera.h"
#include "../Mesh/MeshComponent.h"
#include "../../Collision/Collision.h"
#include "../../GameObject/GameObject.h"
#include "../../GameObject/GameObjectManager.h"
#include "../../Input/Input.h"

CollideMouseOperator::CollideMouseOperator(GameObject& gameObject) :
    Component(gameObject),
    mCamera(nullptr),
    mAABBScaler(nullptr),
    mSelectedMesh(false) {
}

CollideMouseOperator::~CollideMouseOperator() = default;

void CollideMouseOperator::start() {
    const auto& gameObjectManager = gameObject().getGameObjectManager();
    mCamera = gameObjectManager.find("Camera")->componentManager().getComponent<Camera>();

    mAABBScaler = getComponent<AABBMouseScaler>();

    //指定のタグを含んでいるオブジェクトをすべて取得する
    const auto& grounds = gameObjectManager.findGameObjects("Ground");
    //取得したオブジェクトからメッシュを取得する
    for (const auto& g : grounds) {
        auto mesh = g->componentManager().getComponent<MeshComponent>();
        if (mesh) {
            mGroundMeshes.emplace_back(mesh);
        }
    }
}

void CollideMouseOperator::update() {
    //マウスの左ボタンを押した瞬間だったら
    if (Input::mouse().getMouseButtonDown(MouseCode::LeftButton)) {
        clickMouseLeftButton();
    }
}

void CollideMouseOperator::clickMouseLeftButton() {
    if (mSelectedMesh) {
        //メッシュが選択されているなら編集可
        mAABBScaler->selectBoxPoint();
    } else {
        //メッシュが選択されていないならメッシュを選択する
        selectMesh();
    }
}

void CollideMouseOperator::selectMesh() {
    //地形とレイとの衝突判定
    std::shared_ptr<MeshComponent> hit;
    mSelectedMesh = intersectRayGroundMeshes(hit);

    //衝突していなかったら終了
    if (!mSelectedMesh) {
        return;
    }

    //メッシュに付随するAABBを送る
    mAABBScaler->setAABBFromMesh(*hit);
}

bool CollideMouseOperator::intersectRayGroundMeshes(std::shared_ptr<MeshComponent>& out) {
    //カメラからマウスの位置へ向かうレイを取得
    auto rayCameraToMousePos = mCamera->screenToRay(Input::mouse().getMousePosition());

    //すべての地形メッシュとレイの衝突判定
    Vector3 intersectPoint;
    for (const auto& gm : mGroundMeshes) {
        if (Intersect::intersectRayMesh(rayCameraToMousePos, gm->getMesh(), gm->transform(), intersectPoint)) {
            out = gm;
            return true;
        }
    }

    //どれとも衝突しなかった
    return false;
}
