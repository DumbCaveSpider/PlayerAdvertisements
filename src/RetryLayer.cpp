#include <Geode/Geode.hpp>
#include <Geode/modify/RetryLevelLayer.hpp>

using namespace geode::prelude;

class $modify(MyRetryLayer, RetryLevelLayer) {
    void customSetup() override {
        log::debug("RetryLevelLayer customSetup - layer is being set up");
        RetryLevelLayer::customSetup();
        // remove all CCLabelBMFont and i swear all of the labels doesnt have a freaking tag
        // this is so stupid why would you not tag them
        // robtop i hate you
        if (this->m_mainLayer) {
            cocos2d::CCArray* children = this->m_mainLayer->getChildren();
            if (children) {
                int count = static_cast<int>(children->count());
                for (int i = count - 1; i >= 0; --i) {
                    cocos2d::CCObject* obj = children->objectAtIndex(static_cast<unsigned int>(i));
                    if (!obj)
                        continue;
                    if (auto label = typeinfo_cast<CCLabelBMFont*>(obj)) {
                        this->m_mainLayer->removeChild(label, true);
                    }
                    // gonna remove that progress bar too
                    if (auto sprite = typeinfo_cast<CCSprite*>(obj)) {
                        this->m_mainLayer->removeChild(sprite, true);
                    }
                }
            }

            // add funny banner
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto adBanner = LazySprite::create({ 1100.f, 685.f }, true);
            auto tempBanner = CCSprite::create("squareTemp.png"_spr);
            tempBanner->setVisible(false);
            auto adButton = CCMenuItemSpriteExtra::create(tempBanner, this, menu_selector(RetryLevelLayer::onReplay));
            adBanner->loadFromUrl("https://john.citrons.xyz/static/img/merrybot-2.png"); // insert like a api url that serves ads here

            // banner to the menu
            m_mainMenu->addChild(adButton);
            adButton->addChild(adBanner, -1);

            // center the adbutton
            adBanner->setPosition({ adButton->getContentSize().width / 2, adButton->getContentSize().height / 2 });
        }
    }
};