#include <Geode/utils/web.hpp>

struct IDListDemon {
    int id = 0;
    int position = 0;
    std::string name;

    bool operator==(const IDListDemon& other) const {
        return id == other.id && position == other.position;
    }
};

struct IDDemonPack {
    std::string name;
    std::string tierName;
    std::vector<int> levels;
    double points = 0.0;
    int tier = 0;
};

class IntegratedDemonlist {
public:
    static std::vector<IDListDemon> aredl;
    static std::vector<IDDemonPack> aredlPacks;
    static std::vector<IDListDemon> pemonlist;
    static bool aredlLoaded;
    static bool pemonlistLoaded;

    static void loadAREDL(geode::EventListener<geode::utils::web::WebTask>&, std::function<void()>, std::function<void(int)>);
    static void loadAREDLPacks(geode::EventListener<geode::utils::web::WebTask>&, std::function<void()>, std::function<void(int)>);
    static void loadPemonlist(geode::EventListener<geode::utils::web::WebTask>&, std::function<void()>, std::function<void(int)>);
};
