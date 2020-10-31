#include "CameraMove.h"
#include "Camera.h"
#include "../../Device/Time.h"
#include "../../Input/Input.h"
#include "../../Transform/Transform3D.h"
#include "../../Utility/LevelLoader.h"

CameraMove::CameraMove(GameObject& gameObject) :
    Component(gameObject),
    mCamera(nullptr),
    mCameraSpeed(0.f),
    mRotateSpeed(0.f) {
}

CameraMove::~CameraMove() = default;

void CameraMove::start() {
    mCamera = getComponent<Camera>();
}

void CameraMove::update() {
    if (Input::keyboard().getKey(KeyCode::W)) {
        transform().translate(transform().forward() * mCameraSpeed * Time::deltaTime);
    }
    if (Input::keyboard().getKey(KeyCode::S)) {
        transform().translate(-transform().forward() * mCameraSpeed * Time::deltaTime);
    }
    if (Input::keyboard().getKey(KeyCode::D)) {
        transform().translate(transform().right() * mCameraSpeed * Time::deltaTime);
    }
    if (Input::keyboard().getKey(KeyCode::A)) {
        transform().translate(-transform().right() * mCameraSpeed * Time::deltaTime);
    }
    if (Input::keyboard().getKey(KeyCode::Z)) {
        transform().translate(-transform().up() * mCameraSpeed * Time::deltaTime);
    }
    if (Input::keyboard().getKey(KeyCode::C)) {
        transform().translate(transform().up() * mCameraSpeed * Time::deltaTime);
    }
    if (Input::keyboard().getKey(KeyCode::E)) {
        transform().rotate(Vector3::up, mRotateSpeed * Time::deltaTime);
    }
    if (Input::keyboard().getKey(KeyCode::Q)) {
        transform().rotate(Vector3::up, -mRotateSpeed * Time::deltaTime);
    }

    mCamera->lookAt({ transform().getPosition() + transform().forward() * 10.f });
}

void CameraMove::loadProperties(const rapidjson::Value& inObj) {
    JsonHelper::getFloat(inObj, "cameraSpeed", &mCameraSpeed);
    JsonHelper::getFloat(inObj, "rotateSpeed", &mRotateSpeed);
}
