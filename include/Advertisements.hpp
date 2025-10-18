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
    static CCSize getAdSize(AdType type);

    static void getRandomAd(AdType type, std::function<void(Ad)> onComplete);

    static Ad getAdByID(int id, std::function<void(Ad)> onComplete);
    static LazySprite* loadAdImage(Ad);
};