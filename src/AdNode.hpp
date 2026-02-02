#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class AdManager; // forward my child. or adult

class AdNode : public CCNode {
public:
    static AdNode* create(const matjson::Value& adValue, AdManager* manager);
    bool initWithValue(const matjson::Value& adValue, AdManager* manager);
};
