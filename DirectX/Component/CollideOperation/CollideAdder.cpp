#include "CollideAdder.h"
#include "../ComponentManager.h"
#include "../Collider/AABBCollider.h"
#include "../../GameObject/GameObject.h"
#include "../../Input/Input.h"

CollideAdder::CollideAdder(GameObject& gameObject) :
    Component(gameObject) {
}

CollideAdder::~CollideAdder() = default;

void CollideAdder::update() {
    if (Input::keyboard().getKeyDown(KeyCode::J)) {
        addComponent<AABBCollider>("AABBCollider");
    }
}
