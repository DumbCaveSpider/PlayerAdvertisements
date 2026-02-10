#include <Geode/Geode.hpp>
#include "AdNode.hpp"
#include "AdManager.hpp"

using namespace geode::prelude;

AdNode* AdNode::create(const matjson::Value& adValue, AdManager* manager) {
    auto ret = new AdNode();
    if (ret->initWithValue(adValue, manager)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool AdNode::initWithValue(const matjson::Value& adValue, AdManager* manager) {
    this->setContentSize({ 200.f, 85.f });
    this->setAnchorPoint({ 0.5f, 0.5f });

    auto clipNode = CCClippingNode::create();
    clipNode->setContentSize({ 200.f, 85.f });
    clipNode->setAnchorPoint({ 0.0f, 0.0f });
    clipNode->setPosition({ 0.0f, 0.0f });

    auto stencil = CCScale9Sprite::create("square02_001.png");
    stencil->setContentSize({ 200.f, 85.f });
    stencil->setAnchorPoint({ 0.0f, 0.0f });
    stencil->setPosition({ 0.0f, 0.0f });
    clipNode->setStencil(stencil);
    clipNode->setAlphaThreshold(0.1f);

    // @geode-ignore(unknown-resource)
    auto bg = CCScale9Sprite::create("geode.loader/black-square.png");
    bg->setContentSize({ stencil->getContentSize() });
    bg->setAnchorPoint({ 0.0f, 0.0f });
    this->addChild(bg, 1);

    auto imageUrl = adValue["image_url"].asString();
    auto lazySprite = LazySprite::create({ 200.f, 85.f });
    if (imageUrl.isOk()) lazySprite->loadFromUrl(imageUrl.unwrap(), LazySprite::Format::kFmtWebp, true);

    lazySprite->setContentSize({ 200.f, 85.f });
    lazySprite->setScale(0.55f);
    lazySprite->setPosition({ this->getContentSize().width / 2, this->getContentSize().height / 2 });
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
    adLabel->setPosition({ this->getContentSize().width / 2, this->getContentSize().height - 10.f });
    adLabel->setAnchorPoint({ 0.5f, 0.5f });
    adLabel->setScale(0.4f);
    this->addChild(adLabel, 2);

    std::string levelIdStr = levelId.isOk() ? numToString(levelId.unwrap()) : "N/A";
    auto levelLabel = CCLabelBMFont::create(("Level ID: " + levelIdStr).c_str(), "goldFont.fnt");
    levelLabel->setPosition({ this->getContentSize().width / 2, this->getContentSize().height - 25.f });
    levelLabel->setAnchorPoint({ 0.5f, 0.5f });
    levelLabel->setScale(0.4f);
    this->addChild(levelLabel, 2);

    // views and clicks
    std::string viewsStr = viewCount.isOk() ? numToString(viewCount.unwrap()) : "0";
    auto viewsLabel = CCLabelBMFont::create(("Views: " + viewsStr).c_str(), "goldFont.fnt");
    viewsLabel->setPosition({ this->getContentSize().width / 4, this->getContentSize().height - 40.f });
    viewsLabel->setAnchorPoint({ 0.5f, 0.5f });
    viewsLabel->setColor({ 255, 125, 0 });
    viewsLabel->setScale(0.4f);
    this->addChild(viewsLabel, 2);

    std::string clicksStr = clickCount.isOk() ? numToString(clickCount.unwrap()) : "0";
    auto clicksLabel = CCLabelBMFont::create(("Clicks: " + clicksStr).c_str(), "goldFont.fnt");
    clicksLabel->setPosition({ this->getContentSize().width / 4 * 3, this->getContentSize().height - 40.f });
    clicksLabel->setAnchorPoint({ 0.5f, 0.5f });
    clicksLabel->setColor({ 0, 175, 255 });
    clicksLabel->setScale(0.4f);
    this->addChild(clicksLabel, 2);

    // pending label
    auto pendingLabel = CCLabelBMFont::create("Pending", "goldFont.fnt");
    pendingLabel->setPosition({ 5.f, 10.f });
    pendingLabel->setAnchorPoint({ 0.f, 0.5f });
    pendingLabel->setColor({ 255, 0, 0 });
    pendingLabel->setScale(0.3f);
    this->addChild(pendingLabel, 2);

    if (pending.isOk() && !pending.unwrap()) {
        pendingLabel->setString("Approved");
        pendingLabel->setColor({ 0, 255, 0 });
    }

    // created at
    auto createdAtLabel = CCLabelBMFont::create(("Created at: " + createdAt.unwrap()).c_str(), "chatFont.fnt");
    createdAtLabel->setPosition({ this->getContentSize().width / 2, 10.f });
    createdAtLabel->setAnchorPoint({ 0.5f, 0.5f });
    createdAtLabel->setScale(0.3f);
    this->addChild(createdAtLabel, 2);

    // play button at the bottom right
    auto playBtnSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    playBtnSprite->setScale(0.35f);
    auto playBtn = CCMenuItemSpriteExtra::create(
        playBtnSprite,
        manager,
        menu_selector(AdManager::onPlayButton));
    playBtn->setID("play-btn");
    playBtn->setTag(levelId.isOk() ? levelId.unwrap() : 0);

    auto playMenu = CCMenu::create();
    playMenu->setPosition({ this->getContentSize().width / 2, this->getContentSize().height / 2 - 5 });
    playMenu->addChild(playBtn);
    this->addChild(playMenu, 3);

    return true;
}
