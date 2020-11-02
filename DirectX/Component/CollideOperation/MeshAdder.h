#pragma once

#include "../Component.h"
#include <memory>

class MeshComponent;

//メッシュを追加するクラス
class MeshAdder : public Component {
public:
    MeshAdder(GameObject& gameObject);
    ~MeshAdder();
    std::shared_ptr<MeshComponent> addMesh(GameObject& gameObject) const;

private:
    MeshAdder(const MeshAdder&) = delete;
    MeshAdder& operator=(const MeshAdder&) = delete;
};
