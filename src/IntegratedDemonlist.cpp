#include "IntegratedDemonlist.hpp"

using namespace geode::prelude;

#define AREDL_URL "https://api.aredl.net/v2/api/aredl/levels"
#define AREDL_PACKS_URL "https://api.aredl.net/v2/api/aredl/pack-tiers"
#define PEMONLIST_URL "https://pemonlist.com/api/list?limit=150&version=2"

void IntegratedDemonlist::loadAREDL(EventListener<web::WebTask>& listener, std::function<void()> success, std::function<void(int)> failure) {
    listener.bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            aredlLoaded = true;
            aredl.clear();

            auto json = res->json().andThen([](matjson::Value&& v) {
                return std::move(v).asArray();
            });
            if (!json.isOk()) return success();

            for (auto& level : json.unwrap()) {
                auto legacy = level.get("legacy").andThen([](const matjson::Value& v) {
                    return v.asBool();
                });
                if (legacy.isOk() && legacy.unwrap()) continue;

                auto id = level.get("level_id").andThen([](const matjson::Value& v) {
                    return v.as<int>();
                });
                if (!id.isOk()) continue;

                auto position = level.get("position").andThen([](const matjson::Value& v) {
                    return v.as<int>();
                });
                if (!position.isOk()) continue;

                auto name = level.get("name").andThen([](const matjson::Value& v) {
                    return v.asString();
                });
                if (!name.isOk()) continue;

                IDListDemon demon(id.unwrap(), position.unwrap(), std::move(name).unwrap());

                aredl.insert(std::ranges::upper_bound(aredl, demon, [](const IDListDemon& a, const IDListDemon& b) {
                    return a.position < b.position;
                }), std::move(demon));
            }

            success();
        }
    });

    listener.setFilter(web::WebRequest().get(AREDL_URL));
}

void IntegratedDemonlist::loadAREDLPacks(EventListener<web::WebTask>& listener, std::function<void()> success, std::function<void(int)> failure) {
    listener.bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            aredlPacks.clear();

            auto json = res->json().andThen([](matjson::Value&& v) {
                return std::move(v).asArray();
            });
            if (!json.isOk()) return success();

            for (auto& tier : json.unwrap()) {
                auto placement = tier.get("placement").andThen([](const matjson::Value& v) {
                    return v.as<int>();
                });
                if (!placement.isOk()) continue;

                auto tierName = tier.get("name").andThen([](const matjson::Value& v) {
                    return v.asString();
                });
                if (!tierName.isOk()) continue;

                auto packs = tier.get("packs").andThen([](const matjson::Value& v) {
                    return v.asArray();
                });
                if (!packs.isOk()) continue;

                for (auto& pack : packs.unwrap()) {
                    auto levelsRes = pack.get("levels").andThen([](const matjson::Value& v) {
                        return v.asArray();
                    });
                    if (!levelsRes.isOk()) continue;

                    auto name = pack.get("name").andThen([](const matjson::Value& v) {
                        return v.asString();
                    });
                    if (!name.isOk()) continue;

                    auto points = pack.get("points").andThen([](const matjson::Value& v) {
                        return v.asDouble();
                    });
                    if (!points.isOk()) continue;

                    std::vector<int> levels;
                    auto packValid = true;
                    for (auto& level : levelsRes.unwrap()) {
                        auto id = level.get("level_id").andThen([](const matjson::Value& v) {
                            return v.as<int>();
                        });
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

    listener.setFilter(web::WebRequest().get(AREDL_PACKS_URL));
}

void IntegratedDemonlist::loadPemonlist(EventListener<web::WebTask>& listener, std::function<void()> success, std::function<void(int)> failure) {
    listener.bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            pemonlistLoaded = true;
            pemonlist.clear();

            auto json = res->json().andThen([](matjson::Value&& v) {
                return v.get("data").andThen([](matjson::Value& v) {
                    return std::move(v).asArray();
                });
            });
            if (!json.isOk()) return success();

            for (auto& level : json.unwrap()) {
                auto id = level.get("level_id").andThen([](const matjson::Value& v) {
                    return v.as<int>();
                });
                if (!id.isOk()) continue;

                auto position = level.get("placement").andThen([](const matjson::Value& v) {
                    return v.as<int>();
                });
                if (!position.isOk()) continue;

                auto name = level.get("name").andThen([](const matjson::Value& v) {
                    return v.asString();
                });
                if (!name.isOk()) continue;

                IDListDemon demon(id.unwrap(), position.unwrap(), std::move(name).unwrap());

                pemonlist.insert(std::ranges::upper_bound(pemonlist, demon, [](const IDListDemon& a, const IDListDemon& b) {
                    return a.position < b.position;
                }), std::move(demon));
            }

            success();
        }
    });

    listener.setFilter(web::WebRequest().get(PEMONLIST_URL));
}
