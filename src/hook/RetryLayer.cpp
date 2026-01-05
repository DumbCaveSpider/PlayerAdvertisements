#include <Geode/Geode.hpp>
#include <Geode/modify/RetryLevelLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsRetryLevelLayer, RetryLevelLayer) {
    void customSetup() override {
        log::debug("RetryLevelLayer customSetup - layer is being set up");
        RetryLevelLayer::customSetup();

        if (Mod::get()->getSettingValue<bool>("RetryLevelLayer")) {
            // remove all CCLabelBMFont and i swear all of the labels doesnt have a freaking tag
            // this is so stupid why would you not tag them
            // robtop i hate you
            if (this->m_mainLayer) {
                CCArray* children = this->m_mainLayer->getChildren();
                if (children) {
                    int count = static_cast<int>(children->count());
                    for (int i = count - 1; i >= 0; --i) {
                        CCObject* obj = children->objectAtIndex(static_cast<unsigned int>(i));
                        if (!obj) continue;

                        if (auto label = typeinfo_cast<CCLabelBMFont*>(obj)) this->m_mainLayer->removeChild(label, true);
                        // gonna remove that progress bar too
                        if (auto sprite = typeinfo_cast<CCSprite*>(obj)) this->m_mainLayer->removeChild(sprite, true);
                    };
                };

                // add funny banner
                auto const winSize = CCDirector::sharedDirector()->getWinSize();

                // banner ad at the top
                auto adBanner = Advertisement::create();
                if (adBanner) {
                    adBanner->setID("advertisement-menu");
                    adBanner->setType(AdType::Square);
                    adBanner->setPosition({ winSize.width / 2.f, winSize.height / 2.f });

                    m_mainLayer->addChild(adBanner);

                    adBanner->loadRandom();
                };
            };
        };
    };
};