#include "IntegratedDemonlist.hpp"
#include <jasmine/web.hpp>

using namespace geode::prelude;

std::vector<IDListDemon> IntegratedDemonlist::aredl;
std::vector<IDDemonPack> IntegratedDemonlist::aredlPacks;
std::vector<IDListDemon> IntegratedDemonlist::pemonlist;
bool IntegratedDemonlist::aredlLoaded = false;
bool IntegratedDemonlist::pemonlistLoaded = false;

constexpr const char* aredlUrl = "https://api.aredl.net/v2/api/aredl/levels";
constexpr const char* aredlPacksUrl = "https://api.aredl.net/v2/api/aredl/pack-tiers";
constexpr const char* pemonlistUrl = "https://pemonlist.com/api/list?limit=150&version=2";

void IntegratedDemonlist::loadAREDL(EventListener<web::WebTask>& listener, std::function<void()> success, std::function<void(int)> failure) {
    listener.bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            aredlLoaded = true;
            aredl.clear();

            for (auto& level : jasmine::web::getArray(res)) {
                auto legacy = level.get<bool>("legacy");
                if (legacy.isOk() && legacy.unwrap()) continue;

                auto id = level.get<int>("level_id");
                if (!id.isOk()) continue;

                auto position = level.get<int>("position");
                if (!position.isOk()) continue;

                auto name = level.get<std::string>("name");
                if (!name.isOk()) continue;

                IDListDemon demon(id.unwrap(), position.unwrap(), std::move(name).unwrap());

                aredl.insert(std::ranges::upper_bound(aredl, demon, [](const IDListDemon& a, const IDListDemon& b) {
                    return a.position < b.position;
                }), std::move(demon));
            }

            success();
        }
    });

    listener.setFilter(web::WebRequest().get(aredlUrl));
}

void IntegratedDemonlist::loadAREDLPacks(EventListener<web::WebTask>& listener, std::function<void()> success, std::function<void(int)> failure) {
    listener.bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            aredlPacks.clear();

            for (auto& tier : jasmine::web::getArray(res)) {
                auto placement = tier.get<int>("placement");
                if (!placement.isOk()) continue;

                auto tierName = tier.get<std::string>("name");
                if (!tierName.isOk()) continue;

                auto packs = tier.get<std::vector<matjson::Value>>("packs");
                if (!packs.isOk()) continue;

                for (auto& pack : packs.unwrap()) {
                    auto levelsRes = pack.get<std::vector<matjson::Value>>("levels");
                    if (!levelsRes.isOk()) continue;

                    auto name = pack.get<std::string>("name");
                    if (!name.isOk()) continue;

                    auto points = pack.get<double>("points");
                    if (!points.isOk()) continue;

                    std::vector<int> levels;
                    auto packValid = true;
                    for (auto& level : levelsRes.unwrap()) {
                        auto id = level.get<int>("level_id");
                        if (id.isOk()) levels.push_back(id.unwrap());
                        else {
                            packValid = false;
                            break;
                        }
                    }
                    if (!packValid) continue;

                    IDDemonPack demonPack(
                        std::move(name).unwrap(), tierName.unwrap(),
                        std::move(levels), points.unwrap(), placement.unwrap()
                    );

                    aredlPacks.insert(std::ranges::upper_bound(aredlPacks, demonPack, [](const IDDemonPack& a, const IDDemonPack& b) {
                        return a.tier == b.tier ? a.points == b.points ? a.name < b.name : a.points < b.points : a.tier < b.tier;
                    }), std::move(demonPack));
                }
            }

            success();
        }
    });

    listener.setFilter(web::WebRequest().get(aredlPacksUrl));
}

void IntegratedDemonlist::loadPemonlist(EventListener<web::WebTask>& listener, std::function<void()> success, std::function<void(int)> failure) {
    listener.bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            pemonlistLoaded = true;
            pemonlist.clear();

            for (auto& level : jasmine::web::getArray(res, "data")) {
                auto id = level.get<int>("level_id");
                if (!id.isOk()) continue;

                auto position = level.get<int>("placement");
                if (!position.isOk()) continue;

                auto name = level.get<std::string>("name");
                if (!name.isOk()) continue;

                IDListDemon demon(id.unwrap(), position.unwrap(), std::move(name).unwrap());

                pemonlist.insert(std::ranges::upper_bound(pemonlist, demon, [](const IDListDemon& a, const IDListDemon& b) {
                    return a.position < b.position;
                }), std::move(demon));
            }

            success();
        }
    });

    listener.setFilter(web::WebRequest().get(pemonlistUrl));
}
