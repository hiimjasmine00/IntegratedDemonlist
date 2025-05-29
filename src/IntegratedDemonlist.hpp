#include <Geode/utils/web.hpp>

struct IDListDemon {
    int id;
    int position;
    std::string name;
};

struct IDDemonPack {
    std::string name;
    std::vector<int> levels;
    double points;
};

class IntegratedDemonlist {
public:
    inline static std::vector<IDListDemon> aredl;
    inline static std::vector<IDDemonPack> aredlPacks;
    inline static std::vector<IDListDemon> pemonlist;
    inline static bool aredlLoaded = false;
    inline static bool pemonlistLoaded = false;

    static void loadAREDL(geode::EventListener<geode::utils::web::WebTask>*, std::function<void()>, std::function<void(int)>);
    static void loadAREDLPacks(geode::EventListener<geode::utils::web::WebTask>*, std::function<void()>, std::function<void(int)>);
    static void loadPemonlist(geode::EventListener<geode::utils::web::WebTask>*, std::function<void()>, std::function<void(int)>);
};
