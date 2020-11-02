#include "CollideMouseOperator.h"
#include "AABBMouseScaler.h"
#include "CollideAdder.h"
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
    mCollideAdder(nullptr),
    mSelecteMesh(nullptr) {
}

CollideMouseOperator::~CollideMouseOperator() = default;

void CollideMouseOperator::start() {
    const auto& gameObjectManager = gameObject().getGameObjectManager();
    mCamera = gameObjectManager.find("Camera")->componentManager().getComponent<Camera>();

    mAABBScaler = getComponent<AABBMouseScaler>();
    mCollideAdder = getComponent<CollideAdder>();

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
    if (Input::keyboard().getKeyDown(KeyCode::J)) {
        if (mSelecteMesh) {
            mCollideAdder->addAABBCollide(*mSelecteMesh);
        }
    }
}

void CollideMouseOperator::clickMouseLeftButton() {
    if (mSelecteMesh) {
        //メッシュが選択されているなら編集可
        mAABBScaler->selectBoxPoint();
    } else {
        //メッシュが選択されていないならメッシュを選択する
        selectMesh();
    }
}

void CollideMouseOperator::selectMesh() {
    //地形とレイが衝突していなかったら終了
    if (!intersectRayGroundMeshes(mSelecteMesh)) {
        return;
    }

    //メッシュに付随するAABBを送る
    mAABBScaler->setAABBFromMesh(*mSelecteMesh);
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
    out = nullptr;
    return false;
}
