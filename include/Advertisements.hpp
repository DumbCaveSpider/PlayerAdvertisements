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

enum AdType {
    None = 0,
    Banner = 1,
    Square = 2,
    Skyscraper = 3
};

struct Ad {
    unsigned int id;
    std::string_view image;
    int level = 0;
    AdType type;

    Ad() = default;

    Ad(unsigned int id, std::string_view image, int level, AdType type)
        : id(id), image(image), level(level), type(type) {}
};

class Advertisements {
public:
    /**
     * Get the size of an ad based on its type
     * @param type The type of ad
     * @returns The size of the ad
     */
    static CCSize getAdSize(AdType type);

    /**
     * Get a random ad of the given type
     * @param type The type of ad
     * @param callBack The callback that fires with a constructed Ad struct parameter once the ad loads
     */
    static void getRandomAd(AdType type, std::function<void(Ad)> callBack);
    /**
     * Get an ad by its ID
     * @param id The ID of the ad
     * @param callBack The callback that fires with a constructed Ad struct parameter once the ad loads
     */
    static void getAdByID(int id, std::function<void(Ad)> callBack);

    /**
     * Load an ad's image into a LazySprite
     * @param ad The ad to load the image for
     * @returns The LazySprite containing the ad image
     */
    static LazySprite* loadAdImage(Ad);
};