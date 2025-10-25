#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include "AdManager.hpp"

using namespace geode::prelude;

bool AdManager::setup()
{
    setTitle("Advertisement Manager");
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // get the user id
    std::string userID = Mod::get()->getSettingValue<std::string>("user-id");
    if (userID == "")
    {
        this->onClose(nullptr);
        geode::createQuickPopup(
            "No UserID Set",
            "You have not set a User ID yet. <cy>Do you want to open the Mod Settings to set it now?</c>",
            "No", "Yes",
            [](auto, bool ok)
            {
                if (ok)
                {
                    openSettingsPopup(getMod());
                }
            });
        return false;
    }

    // fetch user ads and data
    std::string urlStr = "https://ads.arcticwoof.xyz/users/fetch?id=";
    urlStr += userID;
    auto url = urlStr;
    auto req = web::WebRequest();
    req.header("User-Agent", "PlayerAdvertisements/1.0");
    m_listener.bind([this](web::WebTask::Event *e)
                    {
        if (auto res = e->getValue()) {
            onFetchComplete(e);
        } else if (e->isCancelled()) {
            log::error("Request was cancelled");
            Notification::create("Advertisement data fetch was cancelled!", NotificationIcon::Warning)->show();
        } });
    m_listener.setFilter(req.get(url));

    // add a background on the left side
    auto bg1 = CCScale9Sprite::create("square02_001.png");
    bg1->setPosition({winSize.width / 5, winSize.height / 2 - 30.f});
    bg1->setContentSize({200.f, 200.f});
    bg1->setOpacity(50);
    m_mainLayer->addChild(bg1);

    // create scroll layer for ads
    m_adsScrollLayer = ScrollLayer::create({200.f, 200.f}, true, true);
    m_adsScrollLayer->setPosition({0, 0});
    bg1->addChild(m_adsScrollLayer);
    
    // add a background on the right side
    auto bg2 = CCScale9Sprite::create("square02_001.png");
    bg2->setPosition({winSize.width / 5 * 3 - 5.f, winSize.height / 2 - 30.f});
    bg2->setContentSize({200.f, 200.f});
    bg2->setOpacity(50);
    m_mainLayer->addChild(bg2);
    

    // title label at the top of each of the backgrounds
    auto titleLabel = CCLabelBMFont::create("Your Ads", "bigFont.fnt");
    titleLabel->setPosition({bg1->getContentSize().width / 2, bg1->getContentSize().height + 10.f});
    titleLabel->setScale(0.4f);
    bg1->addChild(titleLabel);

    auto titleLabel2 = CCLabelBMFont::create("Statistics", "bigFont.fnt");
    titleLabel2->setPosition({bg2->getContentSize().width / 2, bg2->getContentSize().height + 10.f});
    titleLabel2->setScale(0.4f);
    bg2->addChild(titleLabel2);

    // total views and clicks labels on the right background
    m_viewsLabel = CCLabelBMFont::create("Total Views: -", "bigFont.fnt");
    m_viewsLabel->setPosition({bg2->getContentSize().width / 2, bg2->getContentSize().height - 20.f});
    m_viewsLabel->setScale(0.5f);
    bg2->addChild(m_viewsLabel);

    m_clicksLabel = CCLabelBMFont::create("Total Clicks: -", "bigFont.fnt");
    m_clicksLabel->setPosition({bg2->getContentSize().width / 2, bg2->getContentSize().height - 50.f});
    m_clicksLabel->setScale(0.5f);
    bg2->addChild(m_clicksLabel);

    // button to the website at the bottom center of the main layer popup
    auto webButton = ButtonSprite::create("Upload Ads", 0, false, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.8f);
    auto webButtonMenu = CCMenuItemSpriteExtra::create(webButton, this, menu_selector(AdManager::onWebButton));

    // button to open mod settings
    auto modSettingsBtnSprite = CircleButtonSprite::createWithSpriteFrameName(
        // @geode-ignore(unknown-resource)
        "geode.loader/settings.png",
        1.f,
        CircleBaseColor::Green,
        CircleBaseSize::Medium);
    modSettingsBtnSprite->setScale(0.75f);

    auto modSettingsButton = CCMenuItemSpriteExtra::create(
        modSettingsBtnSprite,
        this,
        menu_selector(AdManager::onModSettingsButton));
    modSettingsButton->setPosition(m_mainLayer->getContentSize());

    auto menu = CCMenu::create();
    menu->setPosition({0.f, 0.f});
    menu->setContentSize(m_mainLayer->getContentSize());
    menu->addChild(webButtonMenu);
    menu->addChild(modSettingsButton);
    
    webButtonMenu->setPosition({menu->getContentSize().width / 2, 0.f});
    m_mainLayer->addChild(menu);

    return true;
}

AdManager *AdManager::create()
{
    auto ret = new AdManager();
    if (ret && ret->initAnchored(450.f, 280.f))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void AdManager::onWebButton(CCObject *sender)
{
    geode::utils::web::openLinkInBrowser("https://ads.arcticwoof.xyz/");
    Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
}

void AdManager::onModSettingsButton(CCObject *sender)
{
    openSettingsPopup(getMod());
}

void AdManager::populateAdsScrollLayer()
{
    if (!m_adsScrollLayer)
        return;

    auto adsArray = m_adsData.asArray();
    if (!adsArray.isOk())
        return;

    auto ads = adsArray.unwrap();
    
    auto layout = SimpleAxisLayout::create(Axis::Column);
    layout->setGap(5.f);
    layout->setMainAxisScaling(AxisScaling::Fit);
    m_adsScrollLayer->m_contentLayer->setLayout(layout);
    
    for (const auto& adValue : ads) {
        auto adContainer = CCNode::create();
        adContainer->setContentSize({200.f, 85.f});
        adContainer->setAnchorPoint({0.5f, 0.5f});
        
        auto clipNode = CCClippingNode::create();
        clipNode->setContentSize({200.f, 85.f});
        clipNode->setAnchorPoint({0.0f, 0.0f});
        clipNode->setPosition({0.0f, 0.0f});
        
        auto stencil = CCScale9Sprite::create("square02_001.png");
        stencil->setContentSize({200.f, 85.f});
        stencil->setAnchorPoint({0.0f, 0.0f});
        stencil->setPosition({0.0f, 0.0f});
        clipNode->setStencil(stencil);
        clipNode->setAlphaThreshold(0.0f);

        auto bg = CCScale9Sprite::create("square02_001.png");
        bg->setContentSize({stencil->getContentSize()});
        bg->setOpacity(100);
        bg->setAnchorPoint({0.0f, 0.0f});
        clipNode->addChild(bg, 1);

        auto imageUrl = adValue["image_url"].asString();
        auto lazySprite = LazySprite::create({200.f, 85.f});
        if (imageUrl.isOk()) {
            lazySprite->loadFromUrl(imageUrl.unwrap());
        }
        lazySprite->setContentSize({200.f, 85.f});
        lazySprite->setScale(0.6f);
        lazySprite->setPosition({adContainer->getContentSize().width / 2, adContainer->getContentSize().height / 2});
        clipNode->addChild(lazySprite);
        
        adContainer->addChild(clipNode);
        
        std::string adInfo;
        
        auto adId = adValue["ad_id"].asInt();
        if (adId.isOk())
            adInfo += fmt::format("ID: {}\n", adId.unwrap());
        
        auto levelId = adValue["level_id"].asInt();
        if (levelId.isOk())
            adInfo += fmt::format("Level: {}\n", levelId.unwrap());
        
        auto type = adValue["type"].asInt();
        if (type.isOk())
            adInfo += fmt::format("Type: {}\n", type.unwrap());
        
        auto viewCount = adValue["view_count"].asInt();
        if (viewCount.isOk())
            adInfo += fmt::format("Views: {}\n", viewCount.unwrap());
        
        auto clickCount = adValue["click_count"].asInt();
        if (clickCount.isOk())
            adInfo += fmt::format("Clicks: {}\n", clickCount.unwrap());
        
        auto createdAt = adValue["created_at"].asString();
        if (createdAt.isOk())
            adInfo += fmt::format("Created: {}\n", createdAt.unwrap());
        
        auto pending = adValue["pending"].asBool();
        if (pending.isOk())
            adInfo += fmt::format("Pending: {}", pending.unwrap() ? "Yes" : "No");
        
        auto label = CCLabelBMFont::create(adInfo.c_str(), "bigFont.fnt");
        label->setPosition({70.f, 42.5f});
        label->setScale(0.25f);
        label->setAnchorPoint({0.5f, 0.5f});
        adContainer->addChild(label);
        
        m_adsScrollLayer->m_contentLayer->addChild(adContainer);
    }
    
    m_adsScrollLayer->m_contentLayer->updateLayout();
}

void AdManager::onFetchComplete(web::WebTask::Event *event)
{
    if (auto res = event->getValue())
    {
        if (res->ok())
        {
            auto jsonStr = res->string().unwrapOr("");

            auto json = matjson::parse(jsonStr);
            if (!json.isOk())
            {
                log::error("Failed to parse JSON");
                return;
            }

            auto jsonValue = json.unwrap();

            if (jsonValue.contains("ads"))
            {
                m_adsData = jsonValue["ads"];
                auto adsArray = m_adsData.asArray();
                if (adsArray.isOk())
                {
                    log::info("Fetched {} ads", adsArray.unwrap().size());
                    populateAdsScrollLayer();
                }
                else
                {
                    log::info("Fetched 0 ads");
                }
            }

            if (jsonValue.contains("user"))
            {
                m_userData = jsonValue["user"];
                auto username = m_userData["username"].asString();
                if (username)
                {
                    log::info("Fetched user data for: {}", username.unwrap());
                }
                if (m_userData.contains("total_views"))
                {
                    auto totalViews = m_userData["total_views"].asInt();
                    if (totalViews)
                    {
                        m_totalViews = totalViews.unwrap();
                        log::info("Total Views: {}", m_totalViews);
                    }
                }
                if (m_userData.contains("total_clicks"))
                {
                    auto totalClicks = m_userData["total_clicks"].asInt();
                    if (totalClicks)
                    {
                        m_totalClicks = totalClicks.unwrap();
                        log::info("Total Clicks: {}", m_totalClicks);
                    }
                }
                // Update labels after fetching
                if (m_viewsLabel)
                    m_viewsLabel->setString(fmt::format("Total Views: {}", m_totalViews).c_str());
                if (m_clicksLabel)
                    m_clicksLabel->setString(fmt::format("Total Clicks: {}", m_totalClicks).c_str());
            }
        }
        else
        {
            log::error("Request failed with status code: {}", res->code());
        }
    }
}
