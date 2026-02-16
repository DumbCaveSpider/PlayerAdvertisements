#pragma once

#include "AdManager.hpp"  // forward my child. or adult

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class AdNode : public CCNode {
protected:
    bool init(const matjson::Value& adValue, AdManager* manager);

public:
    static AdNode* create(const matjson::Value& adValue, AdManager* manager);
};
