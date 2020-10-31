#include "FbxBoneParser.h"
#include "FbxAnimationTime.h"

FbxBoneParser::FbxBoneParser() :
    mAnimationTime(std::make_unique<FbxAnimationTime>()) {
}

FbxBoneParser::~FbxBoneParser() = default;

void FbxBoneParser::parse(
    std::vector<MeshVertices>& meshesVertices,
    std::vector<Bone>& bones,
    FbxScene* fbxScene
) {
    //アニメーション時間を取得する
    mAnimationTime->parse(fbxScene);

    //FbxMeshの数を取得
    auto numMeshes = fbxScene->GetSrcObjectCount<FbxMesh>();

    //シーンからメッシュを取得する
    FbxMesh* fbxMesh = fbxScene->GetSrcObject<FbxMesh>();

    //スキン情報の有無
    int skinCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
    if (skinCount <= 0) {
        return;
    }

    for (int i = 0; i < skinCount; ++i) {
        //i番目のスキンを取得
        FbxDeformer* fbxDeformer = fbxMesh->GetDeformer(i);
        //FbxDeformer* fbxDeformer = fbxMesh->GetDeformer(i, FbxDeformer::eSkin);
        FbxSkin* fbxSkin = static_cast<FbxSkin*>(fbxDeformer);

        //ボーンを読み込んでいく
        loadBone(bones, fbxSkin);

        //親子付け
        setParentChildren(bones, fbxSkin);



        for (int j = 0; j < numMeshes; ++j) {
            auto mesh = fbxScene->GetSrcObject<FbxMesh>(j);
            for (int k = 0; k < mBoneMap.size(); ++k) {
                FbxCluster* bone = fbxSkin->GetCluster(k);
                //ウェイト読み込み
                loadWeight(meshesVertices, mesh, bone, k);
            }
        }
    }

    //ウェイト正規化
    normalizeWeight(meshesVertices);
}

void FbxBoneParser::loadBone(
    std::vector<Bone>& bones,
    FbxSkin* fbxSkin
) {
    //ボーンの数を取得
    int boneCount = fbxSkin->GetClusterCount();
    if (boneCount <= 0) {
        return;
    }

    //ボーンの数に合わせて拡張
    bones.resize(boneCount);

    //ボーンの数だけ読み込んでいく
    for (int i = 0; i < boneCount; ++i) {
        //i番目のボーンを取得
        FbxCluster* bone = fbxSkin->GetCluster(i);

        //キーフレーム読み込み
        loadKeyFrames(bones[i], bone);

        //セットに登録
        mBoneMap.emplace(bones[i].name, &bones[i]);
    }
}

void FbxBoneParser::loadKeyFrames(
    Bone& bone,
    FbxCluster* fbxCluster
) {
    //ノードを取得
    FbxNode* fbxNode = fbxCluster->GetLink();

    //ボーン名取得
    bone.name = fbxNode->GetName();

    //フレーム数を取得
    bone.numFrame = mAnimationTime->getStopFrame();

    //モデルの初期姿勢を取得する
    FbxAMatrix linkMatrix;
    fbxCluster->GetTransformLinkMatrix(linkMatrix);
    bone.initMat = substitutionMatrix(linkMatrix);

    //初期姿勢からオフセット行列を計算する
    bone.offsetMat = Matrix4::inverse(bone.initMat);

    //フレーム数分拡張しとく
    bone.frameMat.resize(bone.numFrame);

    //フレーム数分フレーム時姿勢を取得する
    for (int j = 0; j < bone.numFrame; ++j) {
        //指定フレームでの時間を取得する
        auto time = mAnimationTime->getTime(j);
        //指定フレームでの姿勢
        bone.frameMat[j] = substitutionMatrix(fbxNode->EvaluateGlobalTransform(time));
    }
}

Matrix4 FbxBoneParser::substitutionMatrix(
    const FbxMatrix& src
) const {
    Matrix4 dst;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            dst.m[i][j] = static_cast<float>(src.mData[i][j]);
        }
    }

    return dst;
}

void FbxBoneParser::setParentChildren(
    std::vector<Bone>& bones,
    FbxSkin* fbxSkin
) {
    //親子付け
    for (size_t i = 0; i < mBoneMap.size(); i++) {
        //i番目のボーンを取得
        FbxNode* node = fbxSkin->GetCluster(i)->GetLink();

        //スケルトンを取得
        //FbxSkeleton* skeleton = node->GetSkeleton();
        ////スケルトンタイプを取得
        //FbxSkeleton::EType type = skeleton->GetSkeletonType();

        ////ノードじゃなければ終了
        //if (type != FbxSkeleton::eLimbNode) {
        //    return;
        //}

        //親ノードを取得
        FbxNode* parentNode = node->GetParent();
        //親の名前を取得
        std::string parentName = parentNode->GetName();

        //親の名前が登録されてなかったらルート
        auto itr = mBoneMap.find(parentName);
        //親がいるなら登録
        if (itr != mBoneMap.end()) {
            bones[i].parent = itr->second;
            itr->second->children.emplace_back(&bones[i]);
        }
    }

    Bone* root = nullptr;
    for (size_t i = 0; i < bones.size(); i++) {
        if (!bones[i].parent) {
            root = &bones[i];
            break;
        }
    }
    calcRelativeMatrix(*root, nullptr);
}

void FbxBoneParser::calcRelativeMatrix(
    Bone& me,
    const Matrix4* parentOffset
) {
    for (size_t i = 0; i < me.children.size(); i++) {
        calcRelativeMatrix(*me.children[i], &me.offsetMat);
    }
    if (parentOffset) {
        me.initMat *= *parentOffset;
    }
}

void FbxBoneParser::loadWeight(
    std::vector<MeshVertices>& meshesVertices,
    const FbxMesh* fbxMesh,
    const FbxCluster* bone,
    unsigned boneIndex
) {
    //影響する頂点の数
    int weightCount = bone->GetControlPointIndicesCount();
    //このボーンによって移動する頂点のインデックスの配列
    int* weightIndices = bone->GetControlPointIndices();
    //重み
    double* weights = bone->GetControlPointWeights();
    //頂点のインデックス
    int* polygonVertices = fbxMesh->GetPolygonVertices();

    for (int i = 0; i < weightCount; ++i) {
        int index = weightIndices[i];
        for (int j = 0; j < meshesVertices.size(); j++) {
            auto& meshVertices = meshesVertices[j];
            for (int k = 0; k < meshVertices.size(); ++k) {
                //頂点番号と一致するのを探す
                if (polygonVertices[k] != index) {
                    continue;
                }

                //頂点にウェイト保存
                int weightCount;
                for (weightCount = 0; weightCount < 4; ++weightCount) {
                    //ウェイトが0なら格納できる
                    if (Math::nearZero(meshVertices[k].weight[weightCount])) {
                        break;
                    }
                }

                //格納できる数を超えていたら
                if (weightCount >= 4) {
                    continue;
                }

                //頂点情報にウェイトを追加
                meshVertices[k].index[weightCount] = boneIndex;
                meshVertices[k].weight[weightCount] = static_cast<float>(weights[i]);
            }
        }
    }
}

void FbxBoneParser::normalizeWeight(
    std::vector<MeshVertices>& meshesVertice
) {
    //5本以上にまたっがてる場合のため
    for (size_t i = 0; i < meshesVertice.size(); i++) {
        auto& meshVertices = meshesVertice[i];
        for (size_t j = 0; j < meshVertices.size(); ++j) {
            auto& meshVertex = meshVertices[j];
            float sumWeight = 0.f;
            //頂点のウェイトの合計を求める
            for (int j = 0; j < 4; ++j) {
                if (meshVertex.weight[j] <= 0.f) {
                    break;
                }
                sumWeight += meshVertex.weight[j];
            }
            //正規化
            for (int j = 0; j < 4; ++j) {
                meshVertex.weight[j] /= sumWeight;
            }
        }
    }
}
