#pragma once

#include "../Component.h"
#include "../../Collision/Collision.h"
#include "../../Math/Math.h"
#include <memory>
#include <vector>

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
    //カメラからマウスでクリックした地点へのレイを取得する
    Ray getRayCamraToMousePos(const Vector2& mousePos) const;
    //マウスでメッシュを選択する
    void selectMesh(const Ray& ray);
    //すべての地形メッシュとレイの衝突判定を行う
    bool intersectRayGroundMeshes(const Ray& ray);
    //AABBのボックスの点を選択する
    void selectBoxPoint(const Ray& ray);

private:
    std::shared_ptr<Camera> mCamera;
    //全地形メッシュ配列
    std::vector<std::shared_ptr<MeshComponent>> mGroundMeshes;
    //現在選択中のメッシュ
    std::shared_ptr<MeshComponent> mSelectMesh;
    //現在選択中のAABB
    std::shared_ptr<AABBCollider> mSelectAABB;

    //カメラから伸びるレイの長さ
    float mRayLenght;
    //ボックス一個一個から球を作る際の半径
    float mPointRadius;


    Vector3 mSelectPoint;
    bool mIsSelectedPoint;
};
