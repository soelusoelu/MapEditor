#pragma once

#include "../Component.h"
#include "../../Collision/Collision.h"
#include "../../Math/Math.h"
#include <array>
#include <memory>
#include <vector>
#include <utility>

class Camera;
class AABBCollider;
class MeshComponent;

class CollideMouseOperator : public Component {
public:
    CollideMouseOperator(GameObject& gameObject);
    ~CollideMouseOperator();
    virtual void start() override;
    virtual void update() override;
    virtual void loadProperties(const rapidjson::Value& inObj) override;

private:
    CollideMouseOperator(const CollideMouseOperator&) = delete;
    CollideMouseOperator& operator=(const CollideMouseOperator&) = delete;

    //カメラからマウスでクリックした地点へのレイを取得する
    Ray getRayCamraToMousePos(const Vector2& mousePos) const;
    //マウスでメッシュを選択する
    void selectMesh(const Ray& ray);
    //すべての地形メッシュとレイの衝突判定を行う
    bool intersectRayGroundMeshes(std::shared_ptr<MeshComponent>& hit, const Ray& ray);
    //AABBのボックスの点を選択する
    bool selectBoxPoint(const Ray& ray);
    //マウスの移動量から当たり判定を拡縮する
    void calculateNewBoxPoint(const Vector2& mouseMoveAmount);
    //法線からボックスを更新する
    void updateBox(const Vector3& calcPoint, const Vector3& surfaceNormal);

private:
    std::shared_ptr<Camera> mCamera;
    //全地形メッシュ配列
    std::vector<std::shared_ptr<MeshComponent>> mGroundMeshes;
    //現在選択中のAABB
    std::shared_ptr<AABBCollider> mSelectAABB;

    //カメラから伸びるレイの長さ
    float mRayLenght;
    //ボックス一個一個から球を作る際の半径
    float mPointRadius;
    //当たり判定の拡張具合
    float mCollisionExpantionAmount;

    //面の中心位置と法線
    int mSelectSurfaceIndex;
    //ボックスの点を選択中か
    bool mIsSelectedPoint;
};
