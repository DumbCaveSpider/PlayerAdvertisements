#include "AdPreview.hpp"
#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace geode::utils;
using namespace ads;

AdPreview *AdPreview::create(int adId, int levelId, int userId, AdType type)
{
    auto ret = new AdPreview();
    ret->m_adId = adId;
    ret->m_levelId = levelId;
    ret->m_userId = userId;
    ret->m_type = type;

    if (ret && ret->initAnchored(300.f, 200.f))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
};

bool AdPreview::setup()
{
    setTitle("ID: " + numToString(m_adId));
    auto levelIdLabel = CCLabelBMFont::create(numToString(m_levelId).c_str(), "bigFont.fnt");
    levelIdLabel->setID("level-id-label");
    levelIdLabel->setPosition({m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height - 40});
    levelIdLabel->setScale(0.5f);
    m_mainLayer->addChild(levelIdLabel);

    auto menu = CCMenu::create();
    menu->setPosition({0.f, 0.f});

    auto playAdLevelSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    auto playAdLevelButton = CCMenuItemSpriteExtra::create(playAdLevelSprite, this, menu_selector(AdPreview::onPlayButton));

    playAdLevelButton->setPosition({m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2});
    m_mainLayer->addChild(menu);
    menu->addChild(playAdLevelButton);

    return true;
};

void AdPreview::onPlayButton(CCObject* sender)
{
    log::info("opening level id {} from ad id {}", m_levelId, m_adId);
    
    auto searchStr = std::to_string(m_levelId);
    auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
};