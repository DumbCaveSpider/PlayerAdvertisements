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

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace ads {
    enum AdType {
        Banner = 1,
        Square = 2,
        Skyscraper = 3
    };

    struct Ad {
        int id;
        std::string image;
        int level = 0;
        std::string user = "";
        AdType type;
        int viewCount = 0;
        int clickCount = 0;

        Ad() = default;

        Ad(unsigned int id, std::string image, int level, AdType type, std::string user, int viewCount = 0, int clickCount = 0)
            : id(id), image(image), level(level), user(user), type(type), viewCount(viewCount), clickCount(clickCount) {}
    };

    /**
     * Get the size of an ad based on its type
     * @param type The type of ad
     * @returns The size of the ad
     */
    CCSize getAdSize(AdType type);

    class Advertisement : public CCMenu {
    protected:
        class Impl;
        std::unique_ptr<Impl> m_impl;

        Advertisement();
        virtual ~Advertisement();

        // Send the user to the level
        void activate(CCObject*);

        // Reloads the type of advertisement
        void reloadType();
        // Reloads the advertisement button
        void reload();

        bool init() override;
        void onEnter() override;
        void onExit() override;

    public:
        // Create a new advertisement
        static Advertisement* create();

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
        LazySprite* getAdSprite() const;
    };
};