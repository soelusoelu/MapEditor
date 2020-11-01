#pragma once

#include "../Component.h"

//コライダーを追加するクラス
class CollideAdder : public Component {
public:
    CollideAdder(GameObject& gameObject);
    ~CollideAdder();
    virtual void start() override;
    virtual void update() override;

private:
    CollideAdder(CollideAdder&) = delete;
    CollideAdder& operator=(const CollideAdder&) = delete;
};
