#pragma once

#include "AdManager.hpp"  // forward my child. or adult

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class AdNode final : public CCNode {
protected:
    void onPlayButton(CCObject* sender);
    void tryOpenOrFetchLevel(CCMenuItemSpriteExtra* menuItem, int levelId);

    bool init(const matjson::Value& adValue);

public:
    static AdNode* create(const matjson::Value& adValue);
};
