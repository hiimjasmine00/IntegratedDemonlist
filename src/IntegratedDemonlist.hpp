#include <Geode/utils/web.hpp>

struct IDListDemon {
    int id;
    std::string name;
    int position;
};

struct IDDemonPack {
    std::string name;
    double points;
    std::vector<int> levels;
};

class IntegratedDemonlist {
public:
    inline static std::vector<IDListDemon> aredl = {};
    inline static std::vector<IDDemonPack> aredlPacks = {};
    inline static std::vector<IDListDemon> pemonlist = {};
    inline static bool aredlLoaded = false;
    inline static bool pemonlistLoaded = false;

    static void loadAREDL(
        geode::EventListener<geode::utils::web::WebTask>*,
        geode::EventListener<geode::utils::web::WebTask>*,
        std::function<void()>,
        std::function<void(int)>
    );
    static void loadAREDLPacks(
        geode::EventListener<geode::utils::web::WebTask>*,
        geode::EventListener<geode::utils::web::WebTask>*,
        std::function<void()>,
        std::function<void(int)>
    );
    static void loadPemonlist(
        geode::EventListener<geode::utils::web::WebTask>*,
        geode::EventListener<geode::utils::web::WebTask>*,
        std::function<void()>,
        std::function<void(int)>
    );
};
