#pragma once

#include "../Component.h"
#include <memory>

class Camera;

//カメラの動きを扱うクラス
class CameraMove : public Component {
public:
    CameraMove(GameObject& gameObject);
    ~CameraMove();
    virtual void start() override;
    virtual void update() override;
    virtual void loadProperties(const rapidjson::Value& inObj) override;

private:
    CameraMove(const CameraMove&) = delete;
    CameraMove& operator=(const CameraMove&) = delete;

private:
    std::shared_ptr<Camera> mCamera;
    float mCameraSpeed;
    float mRotateSpeed;
};
