#include "CollideMouseOperator.h"
#include "../Camera/Camera.h"
#include "../Collider/AABBCollider.h"
#include "../Mesh/MeshComponent.h"
#include "../../DebugLayer/Debug.h"
#include "../../Device/Time.h"
#include "../../GameObject/GameObject.h"
#include "../../GameObject/GameObjectManager.h"
#include "../../Input/Input.h"
#include "../../Transform/Transform3D.h"
#include "../../Utility/LevelLoader.h"

CollideMouseOperator::CollideMouseOperator(GameObject& gameObject) :
    Component(gameObject),
    mCamera(nullptr),
    mSelectAABB(nullptr),
    mRayLenght(0.f),
    mPointRadius(0.f),
    mCollisionExpantionAmount(0.f),
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
        if (!selectBoxPoint(rayCameraToMousePos)) {
            //当たり判定が選択されなかったら
            selectMesh(rayCameraToMousePos);
        }
    }
    //マウスの左ボタンを押し続けていたら
    if (mouse.getMouseButton(MouseCode::LeftButton)) {
        calculateNewBoxPoint(mouse.getMouseMoveAmount());
    }
    //マウスの左ボタンを離した瞬間だったら
    if (mouse.getMouseButtonUp(MouseCode::LeftButton)) {
        mIsSelectedPoint = false;
    }

    if (mSelectAABB) {
        const auto& surfaces = mSelectAABB->getBoxSurfacesCenterAndNormal();
        for (const auto& surface : surfaces) {
            Debug::renderPoint(surface.first);
        }
    }
}

void CollideMouseOperator::loadProperties(const rapidjson::Value& inObj) {
    JsonHelper::getFloat(inObj, "rayLenght", &mRayLenght);
    JsonHelper::getFloat(inObj, "pointRadius", &mPointRadius);
    JsonHelper::getFloat(inObj, "collisionExpantionAmount", &mCollisionExpantionAmount);
}

Ray CollideMouseOperator::getRayCamraToMousePos(const Vector2& mousePos) const {
    //ワールド座標におけるマウスの位置を取得
    auto worldPos = mCamera->screenToWorldPoint(mousePos, 1.f);

    //レイ作成
    const auto& cameraPos = mCamera->getPosition();
    Vector3 dir = Vector3::normalize(worldPos - cameraPos);
    Ray ray(cameraPos, dir, mRayLenght);

    return ray;
}

void CollideMouseOperator::selectMesh(const Ray& ray) {
    std::shared_ptr<MeshComponent> hit = nullptr;

    //地形とレイが衝突していなかったら終了
    if (!intersectRayGroundMeshes(hit, ray)) {
        return;
    }

    //メッシュに付随するAABBを取得
    mSelectAABB = hit->getComponent<AABBCollider>();
}

bool CollideMouseOperator::intersectRayGroundMeshes(std::shared_ptr<MeshComponent>& hit, const Ray& ray) {
    //すべての地形メッシュとレイの衝突判定
    Vector3 intersectPoint;
    for (const auto& gm : mGroundMeshes) {
        if (Intersect::intersectRayMesh(ray, gm->getMesh(), gm->transform(), intersectPoint)) {
            hit = gm;
            return true;
        }
    }

    //どれとも衝突しなかった
    return false;
}

bool CollideMouseOperator::selectBoxPoint(const Ray& ray) {
    //メッシュにAABBコンポーネントがついてなければ終了
    if (!mSelectAABB) {
        return false;
    }

    //AABBすべての面と法線を取得する
    const auto& surfaces = mSelectAABB->getBoxSurfacesCenterAndNormal();

    //すべての面の中心位置を球に見立ててレイと球の当たり判定を行う
    Vector3 intersectPoint;
    for (int i = 0; i < surfaces.size(); ++i) {
        if (Intersect::intersectRaySphere(ray, { surfaces[i].first, mPointRadius }, intersectPoint)) {
            mSelectSurfaceIndex = i;
            mIsSelectedPoint = true;
            return true;
        }
    }

    return false;
}

void CollideMouseOperator::calculateNewBoxPoint(const Vector2& mouseMoveAmount) {
    if (!mIsSelectedPoint) {
        return;
    }

    //マウスでクリックした面を取得する
    const auto& surface = mSelectAABB->getBoxSurfacesCenterAndNormal()[mSelectSurfaceIndex];
    //取得した面から法線を取得
    const auto& surfaceNormal = surface.second;

    //マウスの移動量を3次元に換算する
    auto dir = Vector3(
        mouseMoveAmount.x,
        -mouseMoveAmount.y,
        (mouseMoveAmount.x + -mouseMoveAmount.y) / 2.f
    );

    //カメラの向きを考慮する
    dir = Vector3::transform(dir, mCamera->transform().getRotation());

    //面の方向と移動量を決める
    float amount = Vector3::dot(dir, surfaceNormal);

    //初期位置 + マウスの移動量で更新する
    auto updatePoint = surface.first + surfaceNormal * amount * Time::deltaTime * mCollisionExpantionAmount;

    //当たり判定更新
    updateBox(updatePoint, surfaceNormal);
}

void CollideMouseOperator::updateBox(const Vector3& calcPoint, const Vector3& surfaceNormal) {
    //計算用の一時的AABBを作成する
    AABB temp(mSelectAABB->getAABB());

    const auto& aabb = mSelectAABB->getAABB();
    if (Vector3::equal(surfaceNormal, Vector3::right)) {
        if (calcPoint.x > aabb.max.x) {
            temp.updateMinMax(calcPoint);
        } else {
            temp.max.x = calcPoint.x;
        }
    } else if (Vector3::equal(surfaceNormal, Vector3::left)) {
        if (calcPoint.x < aabb.min.x) {
            temp.updateMinMax(calcPoint);
        } else {
            temp.min.x = calcPoint.x;
        }
    } else if (Vector3::equal(surfaceNormal, Vector3::up)) {
        if (calcPoint.y > aabb.max.y) {
            temp.updateMinMax(calcPoint);
        } else {
            temp.max.y = calcPoint.y;
        }
    } else if (Vector3::equal(surfaceNormal, Vector3::down)) {
        if (calcPoint.y < aabb.min.y) {
            temp.updateMinMax(calcPoint);
        } else {
            temp.min.y = calcPoint.y;
        }
    } else if (Vector3::equal(surfaceNormal, Vector3::forward)) {
        if (calcPoint.z > aabb.max.z) {
            temp.updateMinMax(calcPoint);
        } else {
            temp.max.z = calcPoint.z;
        }
    } else if (Vector3::equal(surfaceNormal, Vector3::back)) {
        if (calcPoint.z < aabb.min.z) {
            temp.updateMinMax(calcPoint);
        } else {
            temp.min.z = calcPoint.z;
        }
    }

    //AABB更新
    mSelectAABB->set(temp.min, temp.max);
}
