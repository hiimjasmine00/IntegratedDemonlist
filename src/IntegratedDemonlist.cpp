#include "IntegratedDemonlist.hpp"
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

#define AREDL_UPTIME_URL "https://api.aredl.net/v2/api/health"
#define AREDL_URL "https://api.aredl.net/v2/api/aredl/levels"
#define AREDL_PACKS_URL "https://api.aredl.net/v2/api/aredl/pack-tiers"
#define PEMONLIST_UPTIME_URL "https://pemonlist.com/api/uptime?version=2"
#define PEMONLIST_URL "https://pemonlist.com/api/list?limit=150&version=2"

void isOk(std::string_view url, EventListener<web::WebTask>* listener, std::function<void(bool, int)> callback) {
    listener->bind([callback = std::move(callback)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) callback(res->ok(), res->code());
    });
    listener->setFilter(web::WebRequest().get(url));
}

using ListDemons = std::vector<IDListDemon>;
using DemonPacks = std::vector<IDDemonPack>;

void IntegratedDemonlist::loadAREDL(EventListener<web::WebTask>* listener, std::function<void()> success, std::function<void(int)> failure) {
    isOk(AREDL_UPTIME_URL, listener, [list = listener, failure = std::move(failure), success = std::move(success)](bool ok, int code) {
        if (!ok) return failure(code);

        auto listener = list;

        listener->bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (!res->ok()) return failure(res->code());

                aredlLoaded = true;
                aredl.clear();
                auto json = res->json().unwrapOr(std::vector<matjson::Value>());
                if (!json.isArray()) return success();

                aredl = ranges::reduce<ListDemons>(json.asArray().unwrap(), [](ListDemons& acc, const matjson::Value& level) {
                    if ((level.contains("legacy") && level["legacy"].isBool() && level["legacy"].asBool().unwrap()) ||
                        !level.contains("level_id") || !level["level_id"].isNumber() ||
                        !level.contains("name") || !level["name"].isString() ||
                        !level.contains("position") || !level["position"].isNumber()) return;

                    acc.push_back({
                        level["level_id"].as<int>().unwrap(),
                        level["position"].as<int>().unwrap(),
                        level["name"].asString().unwrap()
                    });
                });

                success();
            }
        });

        listener->setFilter(web::WebRequest().get(AREDL_URL));
    });
}

void IntegratedDemonlist::loadAREDLPacks(EventListener<web::WebTask>* listener, std::function<void()> success, std::function<void(int)> failure) {
    isOk(AREDL_UPTIME_URL, listener, [list = listener, failure = std::move(failure), success = std::move(success)](bool ok, int code) {
        if (!ok) return failure(code);

        auto listener = list;

        listener->bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (!res->ok()) return failure(res->code());

                aredlPacks.clear();
                auto json = res->json().unwrapOr(matjson::Value());
                if (!json.isArray()) return success();

                aredlPacks = ranges::reduce<DemonPacks>(json.asArray().unwrap(), [](DemonPacks& acc, const matjson::Value& tier) {
                    if (!tier.contains("packs") || !tier["packs"].isArray()) return;

                    auto packs = ranges::reduce<DemonPacks>(tier["packs"].asArray().unwrap(), [](DemonPacks& acc, const matjson::Value& pack) {
                        if (!pack.contains("name") || !pack["name"].isString() ||
                            !pack.contains("points") || !pack["points"].isNumber() ||
                            !pack.contains("levels") || !pack["levels"].isArray()) return;

                        acc.push_back({
                            pack["name"].asString().unwrap(),
                            ranges::reduce<std::vector<int>>(
                                pack["levels"].asArray().unwrap(),
                                [](std::vector<int>& acc, const matjson::Value& level) {
                                    if (level.contains("level_id") && level["level_id"].isNumber())
                                        acc.push_back(level["level_id"].as<int>().unwrap());
                                }
                            ),
                            pack["points"].asDouble().unwrap()
                        });
                    });

                    acc.insert(acc.end(), packs.begin(), packs.end());
                });
                std::ranges::sort(aredlPacks, [](const IDDemonPack& a, const IDDemonPack& b) { return a.points < b.points; });

                success();
            }
        });

        listener->setFilter(web::WebRequest().get(AREDL_PACKS_URL));
    });
}

void IntegratedDemonlist::loadPemonlist(EventListener<web::WebTask>* listener, std::function<void()> success, std::function<void(int)> failure) {
    isOk(PEMONLIST_UPTIME_URL, listener, [list = listener, failure = std::move(failure), success = std::move(success)](bool ok, int code) {
        if (!ok) return failure(code);

        auto listener = list;

        listener->bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (!res->ok()) return failure(res->code());

                pemonlistLoaded = true;
                pemonlist.clear();
                auto json = res->json().unwrapOr(matjson::Value());
                if (!json.isObject() || !json.contains("data") || !json["data"].isArray()) return success();

                pemonlist = ranges::reduce<ListDemons>(json["data"].asArray().unwrap(), [](ListDemons& acc, const matjson::Value& level) {
                    if (!level.contains("level_id") || !level["level_id"].isNumber() ||
                        !level.contains("name") || !level["name"].isString() ||
                        !level.contains("placement") || !level["placement"].isNumber()) return;

                    acc.push_back({
                        level["level_id"].as<int>().unwrap(),
                        level["placement"].as<int>().unwrap(),
                        level["name"].asString().unwrap()
                    });
                });

                success();
            }
        });

        listener->setFilter(web::WebRequest().get(PEMONLIST_URL));
    });
}
