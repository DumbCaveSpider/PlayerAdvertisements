#include <Geode/Geode.hpp>
#include <Geode/modify/ShareCommentLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsShareCommentLayer, ShareCommentLayer) {
    bool init(gd::string title, int charLimit, CommentType type, int ID, gd::string desc) {
        if (!ShareCommentLayer::init(title, charLimit, type, ID, desc)) return false;

        if (Mod::get()->getSettingValue<bool>("ShareCommentLayer")) {
            auto const winSize = CCDirector::sharedDirector()->getWinSize();

            // square ad left
            if (auto adSquareLeft = Advertisement::create(AdType::Square)) {
                adSquareLeft->setID("square-left"_spr);
                adSquareLeft->setPosition({ winSize.width / 2.f - 140.f, winSize.height / 2.f - 70.f });

                m_mainLayer->addChild(adSquareLeft);
                this->positionForCommentType(adSquareLeft, type);

                adSquareLeft->loadRandom();
            };

            // square ad center
            if (auto adSquareCenter = Advertisement::create(AdType::Square)) {
                adSquareCenter->setID("square-center"_spr);
                adSquareCenter->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 70.f });

                m_mainLayer->addChild(adSquareCenter);
                this->positionForCommentType(adSquareCenter, type);

                adSquareCenter->loadRandom();
            };

            // square ad right
            if (auto adSquareRight = Advertisement::create(AdType::Square)) {
                adSquareRight->setID("advertisement-menu"_spr);
                adSquareRight->setPosition({ winSize.width / 2.f + 140.f, winSize.height / 2.f - 70.f });

                m_mainLayer->addChild(adSquareRight);

                this->positionForCommentType(adSquareRight, type);

                adSquareRight->loadRandom();
            };

            // skyscraper ad on the right side
            if (auto adSkyscraperRight = Advertisement::create(AdType::Skyscraper)) {
                adSkyscraperRight->setID("skyscraper-right"_spr);
                adSkyscraperRight->setPosition({ winSize.width - 30.f, winSize.height / 2.f });

                m_mainLayer->addChild(adSkyscraperRight);

                adSkyscraperRight->loadRandom();
            };

            // skyscraper ad on the left side
            if (auto adSkyscraperLeft = Advertisement::create(AdType::Skyscraper)) {
                adSkyscraperLeft->setID("skyscraper-left"_spr);
                adSkyscraperLeft->setPosition({ 30.f, winSize.height / 2.f });

                m_mainLayer->addChild(adSkyscraperLeft);

                adSkyscraperLeft->loadRandom();
            };
        };

        return true;
    };

    // for da squarez
    void positionForCommentType(Advertisement * ad, CommentType type) {
        if (type == CommentType::FriendRequest) {
            log::debug("comment type is friend request");
            if (ad) ad->setPositionY(ad->getPositionY() - 25.f);
        };
    };
};