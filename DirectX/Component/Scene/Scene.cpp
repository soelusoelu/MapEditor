﻿#include "Scene.h"

Scene::Scene(GameObject& gameObject) :
    Component(gameObject),
    mNext("") {
}

Scene::~Scene() = default;

void Scene::next(const std::string& next) {
    mNext = next;
}

const std::string& Scene::getNext() const {
    return mNext;
}

void Scene::addObjectToNext(const std::string& tag) {
    mTagsToNext.emplace(tag);
}

std::unordered_set<std::string> Scene::getObjectToNext() const {
    return mTagsToNext;
}
