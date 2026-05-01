#include "../AdNode.hpp"

#include "../AdManager.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

bool AdNode::init(const matjson::Value& adValue, float width) {
    if (!CCNode::init()) return false;

    this->setContentSize({width, 85.f});
    this->setAnchorPoint({0.5f, 0.5f});

    auto clipNode = CCClippingNode::create();
    clipNode->setContentSize({width, 85.f});
    clipNode->setAnchorPoint({0.0f, 0.0f});
    clipNode->setPosition({0.0f, 0.0f});

    auto stencil = NineSlice::create("square02_001.png");
    stencil->setContentSize({width, 85.f});
    stencil->setAnchorPoint({0.0f, 0.0f});
    stencil->setPosition({0.0f, 0.0f});

    clipNode->setStencil(stencil);
    clipNode->setAlphaThreshold(0.1f);

    // @geode-ignore(unknown-resource)
    auto bg = NineSlice::create("geode.loader/black-square.png");
    bg->setContentSize({stencil->getContentSize()});
    bg->setAnchorPoint({0.0f, 0.0f});

    this->addChild(bg, 1);

    auto imageUrl = adValue["image_url"].asString();
    auto lazySprite = LazySprite::create({width, 85.f});
    if (imageUrl.isOk()) lazySprite->loadFromUrl(imageUrl.unwrap(), LazySprite::Format::kFmtWebp, true);

    lazySprite->setContentSize({width, 85.f});
    lazySprite->setScale(0.55f);
    lazySprite->setPosition({this->getScaledContentWidth() / 2, this->getScaledContentHeight() / 2});

    clipNode->addChild(lazySprite);

    this->addChild(clipNode);

    // ad data
    auto adId = adValue["ad_id"].asInt();
    auto levelId = adValue["level_id"].asInt();
    auto type = adValue["type"].asInt();
    auto viewCount = adValue["views"].asInt();
    auto clickCount = adValue["clicks"].asInt();
    auto createdAt = adValue["created_at"].asString();
    auto pending = adValue["pending"].asBool();

    // labels
    std::string adIdStr = adId.isOk() ? numToString(adId.unwrap()) : "N/A";

    auto adLabel = CCLabelBMFont::create(("Ad ID: " + adIdStr).c_str(), "goldFont.fnt");
    adLabel->setPosition({this->getScaledContentWidth() / 2, this->getScaledContentHeight() - 10.f});
    adLabel->setAnchorPoint({0.5f, 0.5f});
    adLabel->setScale(0.4f);

    this->addChild(adLabel, 2);

    std::string levelIdStr = levelId.isOk() ? numToString(levelId.unwrap()) : "N/A";

    auto levelLabel = CCLabelBMFont::create(("Level ID: " + levelIdStr).c_str(), "goldFont.fnt");
    levelLabel->setPosition({this->getScaledContentWidth() / 2, this->getScaledContentHeight() - 25.f});
    levelLabel->setAnchorPoint({0.5f, 0.5f});
    levelLabel->setScale(0.4f);

    this->addChild(levelLabel, 2);

    // views and clicks
    std::string viewsStr = viewCount.isOk() ? numToString(GameToolbox::pointsToString(viewCount.unwrap())) : "0";
    auto viewsLabel = CCLabelBMFont::create(("Views: " + viewsStr).c_str(), "goldFont.fnt");

    viewsLabel->setPosition({this->getScaledContentWidth() / 4, this->getScaledContentHeight() - 40.f});
    viewsLabel->setAnchorPoint({0.5f, 0.5f});
    viewsLabel->setColor({255, 125, 0});
    viewsLabel->setScale(0.4f);

    this->addChild(viewsLabel, 2);

    std::string clicksStr = clickCount.isOk() ? numToString(GameToolbox::pointsToString(clickCount.unwrap())) : "0";
    auto clicksLabel = CCLabelBMFont::create(("Clicks: " + clicksStr).c_str(), "goldFont.fnt");

    clicksLabel->setPosition({this->getScaledContentWidth() / 4 * 3, this->getScaledContentHeight() - 40.f});
    clicksLabel->setAnchorPoint({0.5f, 0.5f});
    clicksLabel->setColor({0, 175, 255});
    clicksLabel->setScale(0.4f);

    this->addChild(clicksLabel, 2);

    // pending label
    auto pendingLabel = CCLabelBMFont::create("Pending", "goldFont.fnt");
    pendingLabel->setPosition({5.f, 10.f});
    pendingLabel->setAnchorPoint({0.f, 0.5f});
    pendingLabel->setColor({255, 0, 0});
    pendingLabel->setScale(0.3f);

    this->addChild(pendingLabel, 2);

    if (pending.isOk() && !pending.unwrap()) {
        pendingLabel->setString("Approved");
        pendingLabel->setColor({0, 255, 0});
    };

    // created at
    auto createdAtLabel = CCLabelBMFont::create(("Created at: " + createdAt.unwrap()).c_str(), "chatFont.fnt");
    createdAtLabel->setPosition({this->getScaledContentWidth() / 2, 10.f});
    createdAtLabel->setAnchorPoint({0.5f, 0.5f});
    createdAtLabel->setScale(0.3f);

    this->addChild(createdAtLabel, 2);

    // play button at the bottom right
    auto playBtnSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    playBtnSprite->setScale(0.35f);

    auto playBtn = CCMenuItemSpriteExtra::create(
        playBtnSprite,
        this,
        menu_selector(AdNode::onPlayButton));
    playBtn->setID("play-btn");
    playBtn->setTag(levelId.isOk() ? levelId.unwrap() : 0);
    m_playBtn = playBtn;

    auto playMenu = CCMenu::create();
    playMenu->setPosition({this->getScaledContentWidth() / 2, this->getScaledContentHeight() / 2 - 5});
    playMenu->addChild(playBtn);
    m_playMenu = playMenu;

    this->addChild(playMenu, 3);
    this->scheduleUpdate();

    return true;
};

// open LevelInfo if stored otherwise prepare pending state and request
void AdNode::tryOpenOrFetchLevel(CCMenuItemSpriteExtra* playBtn, int levelId) {
    if (!playBtn) return;

    auto searchObj = GJSearchObject::create(SearchType::Search, numToString(levelId));
    auto key = std::string(searchObj->getKey());
    auto glm = GameLevelManager::sharedState();

    // check stored cache first
    auto stored = glm->getStoredOnlineLevels(key.c_str());

    if (stored && stored->count() > 0) {
        auto level = typeinfo_cast<GJGameLevel*>(stored->objectAtIndex(0));

        if (level && level->m_levelID == levelId) {
            auto scene = LevelInfoLayer::scene(level, false);
            auto transitionFade = CCTransitionFade::create(0.5f, scene);
            if (PlayLayer::get()) {
                CCDirector::sharedDirector()->replaceScene(transitionFade);
                FMODAudioEngine::sharedEngine()->resumeAllAudio();
            } else {
                CCDirector::sharedDirector()->pushScene(transitionFade);
            }
            return;
        };
    };

    // prepare pending state
    m_pendingKey = key;
    m_pendingLevelId = levelId;
    m_pendingTimeout = 10.0f;  // seconds

    if (m_pendingSpinner) {
        m_pendingSpinner->removeFromParent();
        m_pendingSpinner = nullptr;
        if (m_playBtn) {
            m_playBtn->setVisible(true);
        }
    };

    if (auto spinner = LoadingSpinner::create(25.f)) {
        spinner->setPosition(playBtn->getPosition());
        spinner->setVisible(true);
        if (m_playMenu) {
            m_playMenu->addChild(spinner);
        } else {
            this->addChild(spinner);
        }

        if (m_playBtn) {
            m_playBtn->setVisible(false);
        }

        m_pendingSpinner = spinner;
    };

    glm->getOnlineLevels(searchObj);
};

void AdNode::onPlayButton(CCObject* sender) {
    if (auto playBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
        if (PlayLayer::get()) {
            createQuickPopup(
                "Warning",
                "You are already inside of a level, attempt to play another level before closing the current level will <cr>crash your game</c>.\n<cy>Do you still want to proceed?</c>",
                "Cancel",
                "Proceed",
                [this, playBtn](auto, bool ok) {
                    if (ok) this->tryOpenOrFetchLevel(playBtn, playBtn->getTag());
                });
        } else {
            this->tryOpenOrFetchLevel(playBtn, playBtn->getTag());
        };
    };
};

void AdNode::update(float dt) {
    if (m_pendingKey.empty()) {
        return;
    }

    auto glm = GameLevelManager::sharedState();
    auto stored = glm->getStoredOnlineLevels(m_pendingKey.c_str());

    if (stored && stored->count() > 0) {
        auto level = typeinfo_cast<GJGameLevel*>(stored->objectAtIndex(0));

        if (level && level->m_levelID == m_pendingLevelId) {
            auto scene = LevelInfoLayer::scene(level, false);
            auto transitionFade = CCTransitionFade::create(0.5f, scene);
            if (PlayLayer::get()) {
                CCDirector::sharedDirector()->replaceScene(transitionFade);
                FMODAudioEngine::sharedEngine()->resumeAllAudio();
            } else {
                CCDirector::sharedDirector()->pushScene(transitionFade);
            }

            if (m_pendingSpinner) {
                m_pendingSpinner->removeFromParent();
                m_pendingSpinner = nullptr;
            }
            if (m_playBtn) {
                m_playBtn->setVisible(true);
            }

            m_pendingKey.clear();
            m_pendingLevelId = -1;
            m_pendingTimeout = 0.0f;

            glm->m_levelManagerDelegate = nullptr;
            return;
        };
    }

    m_pendingTimeout -= dt;
    if (m_pendingTimeout <= 0.0f) {
        if (m_pendingSpinner) {
            m_pendingSpinner->removeFromParent();
            m_pendingSpinner = nullptr;
        }
        if (m_playBtn) {
            m_playBtn->setVisible(true);
        }

        Notification::create("Level not found", NotificationIcon::Warning)->show();

        m_pendingKey.clear();
        m_pendingLevelId = -1;
        m_pendingTimeout = 0.0f;
    }
};

AdNode* AdNode::create(const matjson::Value& adValue, float width) {
    auto ret = new AdNode();
    if (ret->init(adValue, width)) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
};