#pragma once

#include "AdManager.hpp"  // forward my child. or adult

#include <Geode/Geode.hpp>

#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class AdNode final : public CCNode {
protected:
    void onPlayButton(CCObject* sender);
    void tryOpenOrFetchLevel(CCMenuItemSpriteExtra* menuItem, int levelId);
    void update(float dt) override;

    bool init(const matjson::Value& adValue, float width);

private:
    CCMenuItemSpriteExtra* m_playBtn = nullptr;
    CCMenu* m_playMenu = nullptr;
    std::string m_pendingKey;
    int m_pendingLevelId = -1;
    float m_pendingTimeout = 0.0f;
    LoadingSpinner* m_pendingSpinner = nullptr;

public:
    static AdNode* create(const matjson::Value& adValue, float width);
};
