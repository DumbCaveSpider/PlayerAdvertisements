#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AdManager : public Popup<>
{
protected:
    bool setup() override;
    
    public:
    static AdManager *create();
    void onWebButton(CCObject* sender);
};