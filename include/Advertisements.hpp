#pragma once

#ifdef GEODE_IS_WINDOWS
#ifdef AWCW_ADS_API_EXPORTING
#define AWCW_ADS_API_DLL __declspec(dllexport)
#else
#define AWCW_ADS_API_DLL __declspec(dllimport)
#endif
#else
#ifdef AWCW_ADS_API_EXPORTING
#define AWCW_ADS_API_DLL __attribute__((visibility("default")))
#else
#define AWCW_ADS_API_DLL
#endif
#endif

#include <cocos2d.h>

#include <Geode/ui/LazySprite.hpp>

#include <Geode/utils/web.hpp>

namespace ads {
    enum class AdType : unsigned int {
        Banner = 1,
        Square = 2,
        Skyscraper = 3
    };

    struct Ad final {
        unsigned int id;
        std::string image;
        int level = 0;
        std::string user = "";
        AdType type;
        int viewCount = 0;
        int clickCount = 0;
        int glowLevel = 0;

        Ad() = default;

        Ad(
            unsigned int id,
            std::string image,
            int level,
            AdType type,
            std::string user,
            int viewCount = 0,
            int clickCount = 0,
            int glowLevel = 0
        ) : id(id),
            image(std::move(image)),
            level(level),
            type(type),
            user(std::move(user)),
            viewCount(viewCount),
            clickCount(clickCount),
            glowLevel(glowLevel) {
        };
    };

    /**
     * Get the size of an ad based on its type
     * @param type The type of ad
     */
    cocos2d::CCSize const getAdSize(AdType type);

    /**
     * Get the particle string based on an ad type
     * @param type The type of ad
     */
    constexpr const char* getParticlesForAdType(AdType type);

    class AWCW_ADS_API_DLL Advertisement final : public cocos2d::CCMenu {
    private:
        class Impl;
        std::unique_ptr<Impl> m_impl;

    protected:
        Advertisement();
        ~Advertisement();

        // Send the user to the level
        void activate(cocos2d::CCObject*);

        // Reloads the type of advertisement
        void reloadType();
        // Reloads the advertisement button
        void reload();

        void onEnter() override;
        void onExit() override;

        bool init(AdType type);

    public:
        /**
         * Create a new advertisement
         * @param type The type of ad to create
         */
        static Advertisement* create(AdType type = AdType::Banner);

        /**
         * Set the expected type of advertisement
         * @param type The type of ad to set
         */
        void setType(AdType type);

        /**
         * Load a random advertisement
         */
        void loadRandom();

        /**
         * Load a specific advertisement by its ID
         * @param id The ID of the ad to load
         * @warning This will override the current set type of ad
         */
        void load(int id);

        /**
         * Get the LazySprite associated with the advertisement
         */
        geode::LazySprite* getAdSprite() const;

        /**
         * Handle the response from the advertisement fetch request
         * @param res The web response containing the advertisement data
         */
        void handleAdResponse(geode::utils::web::WebResponse const& res);
    };
};