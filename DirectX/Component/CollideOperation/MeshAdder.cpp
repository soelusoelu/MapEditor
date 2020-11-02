#include "MeshAdder.h"
#include "../Mesh/MeshComponent.h"
#include "../../DebugLayer/Debug.h"
#include "../../GameObject/GameObject.h"
#include "../../Utility/FileUtil.h"

MeshAdder::MeshAdder(GameObject& gameObject)
    : Component(gameObject) {
}

MeshAdder::~MeshAdder() = default;

std::shared_ptr<MeshComponent> MeshAdder::addMesh(GameObject& gameObject) const {
    std::string filePath, fileName;
    if (!FileUtil::openFileDialog(filePath, fileName)) {
        Debug::logError("Failed open dialog.");
        return nullptr;
    }

    //絶対パスからアセットディレクトリ部分を抜き出す
    auto assetsDir = FileUtil::getAssetsFromAbsolutePath(filePath);
    //抜き出したアセットディレクトリからファイル名を抜いたディレクトリパスを取得する
    auto directoryPath = FileUtil::getDirectryFromFilePath(assetsDir);
    //ゲームオブジェクトにメッシュをアタッチする
    auto newMesh = Component::addComponent<MeshComponent>(gameObject, "MeshComponent");
    //メッシュとシェーダーを生成する
    newMesh->createMesh(fileName, directoryPath);
    newMesh->setDefaultShader();

    return newMesh;
}
