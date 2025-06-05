#include "IntegratedDemonlist.hpp"
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

#define AREDL_URL "https://api.aredl.net/v2/api/aredl/levels"
#define AREDL_PACKS_URL "https://api.aredl.net/v2/api/aredl/pack-tiers"
#define PEMONLIST_URL "https://pemonlist.com/api/list?limit=150&version=2"

using ListDemons = std::vector<IDListDemon>;
using DemonPacks = std::vector<IDDemonPack>;

void IntegratedDemonlist::loadAREDL(EventListener<web::WebTask>* listener, std::function<void()> success, std::function<void(int)> failure) {
    listener->bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            aredlLoaded = true;
            aredl.clear();
            auto json = res->json().unwrapOr(matjson::Value::array());
            if (!json.isArray()) return success();

            aredl = ranges::reduce<ListDemons>(json.asArray().unwrap(), [](ListDemons& acc, const matjson::Value& level) {
                if ((level.contains("legacy") && level["legacy"].isBool() && level["legacy"].asBool().unwrap()) ||
                    !level.contains("level_id") || !level["level_id"].isNumber() ||
                    !level.contains("position") || !level["position"].isNumber() ||
                    !level.contains("name") || !level["name"].isString()) return;

                acc.push_back({
                    level["level_id"].as<int>().unwrap(),
                    level["position"].as<int>().unwrap(),
                    level["name"].asString().unwrap()
                });
            });
            std::ranges::sort(aredl, [](const IDListDemon& a, const IDListDemon& b) { return a.position < b.position; });

            success();
        }
    });

    listener->setFilter(web::WebRequest().get(AREDL_URL));
}

void IntegratedDemonlist::loadAREDLPacks(EventListener<web::WebTask>* listener, std::function<void()> success, std::function<void(int)> failure) {
    listener->bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            aredlPacks.clear();
            auto json = res->json().unwrapOr(matjson::Value::array());
            if (!json.isArray()) return success();

            aredlPacks = ranges::reduce<DemonPacks>(json.asArray().unwrap(), [](DemonPacks& acc, const matjson::Value& tier) {
                if (!tier.contains("packs") || !tier["packs"].isArray()) return;

                ranges::push(acc, ranges::reduce<DemonPacks>(tier["packs"].asArray().unwrap(), [](DemonPacks& acc, const matjson::Value& pack) {
                    if (!pack.contains("name") || !pack["name"].isString() ||
                        !pack.contains("levels") || !pack["levels"].isArray() ||
                        !pack.contains("points") || !pack["points"].isNumber()) return;

                    acc.push_back({
                        pack["name"].asString().unwrap(),
                        ranges::reduce<std::vector<int>>(pack["levels"].asArray().unwrap(), [](std::vector<int>& acc, const matjson::Value& level) {
                            if (level.contains("level_id") && level["level_id"].isNumber())
                                acc.push_back(level["level_id"].as<int>().unwrap());
                        }),
                        pack["points"].asDouble().unwrap()
                    });
                }));
            });
            std::ranges::sort(aredlPacks, [](const IDDemonPack& a, const IDDemonPack& b) { return a.points < b.points; });

            success();
        }
    });

    listener->setFilter(web::WebRequest().get(AREDL_PACKS_URL));
}

void IntegratedDemonlist::loadPemonlist(EventListener<web::WebTask>* listener, std::function<void()> success, std::function<void(int)> failure) {
    listener->bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            pemonlistLoaded = true;
            pemonlist.clear();
            auto json = res->json().unwrapOr(matjson::Value::object());
            if (!json.isObject() || !json.contains("data") || !json["data"].isArray()) return success();

            pemonlist = ranges::reduce<ListDemons>(json["data"].asArray().unwrap(), [](ListDemons& acc, const matjson::Value& level) {
                if (!level.contains("level_id") || !level["level_id"].isNumber() ||
                    !level.contains("placement") || !level["placement"].isNumber() ||
                    !level.contains("name") || !level["name"].isString()) return;

                acc.push_back({
                    level["level_id"].as<int>().unwrap(),
                    level["placement"].as<int>().unwrap(),
                    level["name"].asString().unwrap()
                });
            });
            std::ranges::sort(pemonlist, [](const IDListDemon& a, const IDListDemon& b) { return a.position < b.position; });

            success();
        }
    });

    listener->setFilter(web::WebRequest().get(PEMONLIST_URL));
}
