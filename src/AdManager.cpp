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
        } else {
            log::error("Request failed or response is invalid");
            Notification::create("Failed to fetch advertisement data!", NotificationIcon::Error)->show();
        }
    });
    m_listener.setFilter(req.get(url));

    // add a background on the left side
    auto bg1 = CCScale9Sprite::create("square02_001.png");
    bg1->setPosition({winSize.width / 5, winSize.height / 2 - 30.f});
    bg1->setContentSize({200.f, 200.f});
    bg1->setOpacity(50);
    m_mainLayer->addChild(bg1);

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

    // button to the website at the bottom center of the main layer popup
    auto webButton = ButtonSprite::create("Upload Ads", 0, false, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.8f);
    auto webButtonMenu = CCMenuItemSpriteExtra::create(webButton, this, menu_selector(AdManager::onWebButton));

    auto menu = CCMenu::create();
    menu->setPosition({0.f, 0.f});
    menu->setContentSize(m_mainLayer->getContentSize());
    menu->addChild(webButtonMenu);
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
            }
        }
        else
        {
            log::error("Request failed with status code: {}", res->code());
        }
    }
}
