﻿#pragma once

#include "../Component.h"
#include "../../Math/Math.h"
#include <memory>
#include <utility>

class Camera;
class AABBCollider;

//AABBをマウスで拡縮する
class AABBMouseScaler : public Component {
public:
    AABBMouseScaler(GameObject& gameObject);
    ~AABBMouseScaler();
    virtual void start() override;
    virtual void update() override;
    virtual void loadProperties(const rapidjson::Value& inObj) override;

    //AABBをセットする
    void setAABB(const std::shared_ptr<AABBCollider>& aabb);
    //AABBのボックスの点を選択する
    bool selectBoxPoint();

private:
    AABBMouseScaler(const AABBMouseScaler&) = delete;
    AABBMouseScaler& operator=(const AABBMouseScaler&) = delete;

    //マウスの移動量から当たり判定を拡縮する
    void calculateNewBoxPoint();
    //法線からボックスを更新する
    void updateBox(const Vector3& calcPoint, const Vector3& surfaceNormal);

private:
    std::shared_ptr<Camera> mCamera;
    std::shared_ptr<AABBCollider> mCollider;

    //ボックス一個一個から球を作る際の半径
    float mEditPointRadius;
    //当たり判定の拡張具合
    float mCollisionExpantionAmount;

    //面の中心位置と法線
    std::pair<Vector3, Vector3> mSelectSurface;
    //ボックスの編集点を選択中か
    bool mSelectedEditPoint;
};
