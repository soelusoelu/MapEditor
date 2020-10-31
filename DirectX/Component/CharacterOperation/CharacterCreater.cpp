#include "CharacterCreater.h"
#include "../Mesh/MeshComponent.h"
#include "../Sprite/SpriteComponent.h"
#include "../../Collision/Collision.h"
#include "../../GameObject/GameObjectFactory.h"
#include "../../Input/Input.h"
#include "../../Transform/Transform2D.h"
#include "../../System/Window.h"
#include "../../Utility/LevelLoader.h"

CharacterCreater::CharacterCreater(GameObject& gameObject) :
    Component(gameObject),
    mClickedSprite(false),
    mClickedSpriteID(0),
    mSpriteStartPos(Vector2::zero),
    mSpriteScale(Vector2::zero),
    mSpriteSpace(0.f)
{
}

CharacterCreater::~CharacterCreater() = default;

void CharacterCreater::start() {
    mSprites = getComponents<SpriteComponent>();

    //スプライトの位置を調整する
    for (int i = 0; i < mSprites.size(); ++i) {
        auto& s = mSprites[i];
        auto& st = s->transform();
        auto texSize = s->getTextureSize() * mSpriteScale;
        st.setScale(mSpriteScale);

        //スプライトの位置を計算し配置していく
        st.setPosition(mSpriteStartPos + Vector2(texSize.x * i + mSpriteSpace * i, 0.f));
    }
}

void CharacterCreater::update() {
    //マウスインターフェイスを取得
    const auto& mouse = Input::mouse();

    //マウスの左ボタンを押した瞬間だったら
    if (mouse.getMouseButtonDown(MouseCode::LeftButton)) {
        mClickedSprite = selectSprite(mouse.getMousePosition());
    }
    //マウスの左ボタンを押し続けていたら
    if (mouse.getMouseButton(MouseCode::LeftButton)) {
        //スプライトをクリックしていないなら終了
        if (!mClickedSprite) {
            return;
        }

        //マウス位置がスプライトの外にあるなら対応するキャラクターを生成する
        if (!selectSprite(mouse.getMousePosition())) {
            //対応するIDのキャラクターを生成
            createCharacter(mClickedSpriteID);

            //多重生成を阻止するため
            mClickedSprite = false;
        }
    }
    //マウスの左ボタンを離した瞬間だったら
    if (mouse.getMouseButtonUp(MouseCode::LeftButton)) {
        mClickedSprite = false;
    }
}

void CharacterCreater::loadProperties(const rapidjson::Value& inObj) {
    JsonHelper::getStringArray(inObj, "characterNames", &mCharacterNames);
    JsonHelper::getVector2(inObj, "spriteStartPosition", &mSpriteStartPos);
    JsonHelper::getVector2(inObj, "spriteScale", &mSpriteScale);
    JsonHelper::getFloat(inObj, "spriteSpace", &mSpriteSpace);
}

bool CharacterCreater::selectSprite(const Vector2& mousePos) {
    //ウィンドウ補正値を取得する
    auto compen = Window::getWindowCompensate();
    //すべてのスプライトで検証する
    for (int i = 0; i < mSprites.size(); ++i) {
        //計算に必要な要素を取得する
        const auto& st = mSprites[i]->transform();
        auto sPos = st.getPosition() * compen;
        auto sScale = st.getScale() * compen;
        auto texSize = mSprites[i]->getTextureSize() * sScale;

        //スプライトをもとに矩形作成
        Square square(sPos, sPos + texSize);

        //矩形の中にマウスの座標が含まれているか
        if (square.contains(mousePos)) {
            mClickedSpriteID = i;
            return true;
        }
    }

    //すべてのスプライトの矩形範囲外
    return false;
}

void CharacterCreater::createCharacter(int id) {
    //IDに対応するメッシュを作成
    GameObjectCreater::create(mCharacterNames[id]);
}
