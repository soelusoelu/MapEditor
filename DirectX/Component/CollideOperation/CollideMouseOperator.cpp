#include "CollideMouseOperator.h"
#include "../Camera/Camera.h"
#include "../Collider/AABBCollider.h"
#include "../Mesh/MeshComponent.h"
#include "../../DebugLayer/Debug.h"
#include "../../GameObject/GameObject.h"
#include "../../GameObject/GameObjectManager.h"
#include "../../Input/Input.h"
#include "../../Transform/Transform3D.h"
#include "../../Utility/LevelLoader.h"

CollideMouseOperator::CollideMouseOperator(GameObject& gameObject) :
    Component(gameObject),
    mCamera(nullptr),
    mSelectMesh(nullptr),
    mSelectAABB(nullptr),
    mRayLenght(0.f),
    mPointRadius(0.f),
    mSelectSurfaceIndex(0),
    mIsSelectedPoint(false)
{
}

CollideMouseOperator::~CollideMouseOperator() = default;

void CollideMouseOperator::start() {
    const auto& gameObjectManager = gameObject().getGameObjectManager();
    mCamera = gameObjectManager.find("Camera")->componentManager().getComponent<Camera>();

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
    //マウスインターフェイスを取得
    const auto& mouse = Input::mouse();

    //カメラからマウスの位置へ向かうレイを取得
    auto rayCameraToMousePos = getRayCamraToMousePos(mouse.getMousePosition());

    //マウスの左ボタンを押した瞬間だったら
    if (mouse.getMouseButtonDown(MouseCode::LeftButton)) {
        selectMesh(rayCameraToMousePos);
        selectBoxPoint(rayCameraToMousePos);
    }
    //マウスの左ボタンを押し続けていたら
    if (mouse.getMouseButton(MouseCode::LeftButton)) {
        updateBox(rayCameraToMousePos);
    }
    //マウスの左ボタンを離した瞬間だったら
    if (mouse.getMouseButtonUp(MouseCode::LeftButton)) {
        mIsSelectedPoint = false;
    }
}

void CollideMouseOperator::loadProperties(const rapidjson::Value& inObj) {
    JsonHelper::getFloat(inObj, "rayLenght", &mRayLenght);
    JsonHelper::getFloat(inObj, "pointRadius", &mPointRadius);
}

Ray CollideMouseOperator::getRayCamraToMousePos(const Vector2& mousePos) const {
    //マウス座標をゲームウィンドウ範囲でクランプする
    //auto mp = mousePos.clamp(mousePos, Vector2::zero, Vector2(Window::width(), Window::height()));
    //ワールド座標におけるマウスの位置を取得
    auto worldPos = mCamera->screenToWorldPoint(mousePos, 1.f);

    //レイ作成
    const auto& cameraPos = mCamera->getPosition();
    Vector3 dir = Vector3::normalize(worldPos - cameraPos);
    Ray ray(cameraPos, dir, mRayLenght);

    return ray;
}

void CollideMouseOperator::selectMesh(const Ray& ray) {
    //地形とレイが衝突していなかったら終了
    if (!intersectRayGroundMeshes(ray)) {
        return;
    }

    //メッシュに付随するAABBを取得
    mSelectAABB = mSelectMesh->getComponent<AABBCollider>();
}

bool CollideMouseOperator::intersectRayGroundMeshes(const Ray& ray) {
    //すべての地形メッシュとレイの衝突判定
    Vector3 intersectPoint;
    for (const auto& gm : mGroundMeshes) {
        if (Intersect::intersectRayMesh(ray, gm->getMesh(), gm->transform(), intersectPoint)) {
            mSelectMesh = gm;
            return true;
        }
    }

    //どれとも衝突しなかった
    return false;
}

void CollideMouseOperator::selectBoxPoint(const Ray& ray) {
    //メッシュにAABBコンポーネントがついてなければ終了
    if (!mSelectAABB) {
        return;
    }

    //AABBを構成するボックスの点を取得する
    const auto& points = mSelectAABB->getBoxPoints();

    const auto& surfaces = mSelectAABB->getBoxSurfacesCenterAndNormal();

    //すべての面の中心位置を球に見立ててレイと球の当たり判定を行う
    Vector3 intersectPoint;
    for (int i = 0; i < surfaces.size(); ++i) {
        if (Intersect::intersectRaySphere(ray, { surfaces[i].first, mPointRadius }, intersectPoint)) {
            mSelectSurfaceIndex = i;
            mIsSelectedPoint = true;
            return;
        }
    }
}

void CollideMouseOperator::updateBox(const Ray& ray) {
    if (!mIsSelectedPoint) {
        return;
    }

    AABB temp(mSelectAABB->getAABB());
    const auto& surface = mSelectAABB->getBoxSurfacesCenterAndNormal()[mSelectSurfaceIndex];

    //面の移動量を決める
    auto amount = Vector3::dot(Vector3::normalize(ray.end - ray.start), surface.second);

    temp.updateMinMax(surface.first + surface.second * amount * 2.f);
    mSelectAABB->set(temp.min, temp.max);
}
