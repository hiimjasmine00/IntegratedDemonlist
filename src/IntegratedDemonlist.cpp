#include "IntegratedDemonlist.hpp"
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

#define AREDL_URL "https://api.aredl.net/api/aredl/levels"
#define AREDL_PACKS_URL "https://api.aredl.net/api/aredl/packs"
#define PEMONLIST_UPTIME_URL "https://pemonlist.com/api/uptime?version=2"
#define PEMONLIST_URL "https://pemonlist.com/api/list?limit=150&version=2"

void isOk(const std::string& url, EventListener<web::WebTask>* listener, bool head, std::function<void(bool, int)> callback) {
    listener->bind([callback = std::move(callback)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) callback(res->ok(), res->code());
    });
    listener->setFilter(head ? web::WebRequest().send("HEAD", url) : web::WebRequest().downloadRange({ 0, 0 }).get(url));
}

using ListDemons = std::vector<IDListDemon>;
using DemonPacks = std::vector<IDDemonPack>;

void IntegratedDemonlist::loadAREDL(
    EventListener<web::WebTask>* listener, EventListener<web::WebTask>* okListener,
    std::function<void()> success, std::function<void(int)> failure
) {
    listener->bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            aredlLoaded = true;
            aredl.clear();
            auto json = res->json().unwrapOr(std::vector<matjson::Value>());
            if (!json.isArray()) return success();

            aredl = ranges::reduce<ListDemons>(json.asArray().unwrap(), [](ListDemons& acc, const matjson::Value& level) {
                if ((level.contains("legacy") && level["legacy"].isBool() && level["legacy"].asBool().unwrap()) ||
                    !level.contains("level_id") || !level["level_id"].isNumber() &&
                    !level.contains("name") || !level["name"].isString() &&
                    !level.contains("position") || !level["position"].isNumber()) return;

                acc.push_back({
                    (int)level["level_id"].asInt().unwrap(),
                    level["name"].asString().unwrap(),
                    (int)level["position"].asInt().unwrap()
                });
            });

            success();
        }
    });

    isOk(AREDL_URL, okListener, true, [listener, failure = std::move(failure)](bool ok, int code) {
        if (ok) listener->setFilter(web::WebRequest().get(AREDL_URL));
        else failure(code);
    });
}

void IntegratedDemonlist::loadAREDLPacks(
    EventListener<web::WebTask>* listener, EventListener<web::WebTask>* okListener,
    std::function<void()> success, std::function<void(int)> failure
) {
    listener->bind([failure = std::move(failure), success = std::move(success)](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            aredlPacks.clear();
            auto json = res->json().unwrapOr(matjson::Value());
            if (!json.isArray()) return success();

            aredlPacks = ranges::reduce<DemonPacks>(json.asArray().unwrap(), [](DemonPacks& acc, const matjson::Value& pack) {
                if (!pack.contains("name") || !pack["name"].isString() ||
                    !pack.contains("points") || !pack["points"].isNumber() ||
                    !pack.contains("levels") || !pack["levels"].isArray()) return;

                acc.push_back({
                    pack["name"].asString().unwrap(),
                    pack["points"].asDouble().unwrap(),
                    ranges::reduce<std::vector<int>>(pack["levels"].asArray().unwrap(), [](std::vector<int>& acc, const matjson::Value& level) {
                        if (level.contains("level_id") && level["level_id"].isNumber()) acc.push_back(level["level_id"].asInt().unwrap());
                    })
                });
            });
            std::ranges::sort(aredlPacks, [](const IDDemonPack& a, const IDDemonPack& b) { return a.points < b.points; });

            success();
        }
    });

    isOk(AREDL_PACKS_URL, okListener, true, [listener, failure = std::move(failure)](bool ok, int code) {
        if (ok) listener->setFilter(web::WebRequest().get(AREDL_PACKS_URL));
        else failure(code);
    });
}

void IntegratedDemonlist::loadPemonlist(
    EventListener<web::WebTask>* listener, EventListener<web::WebTask>* okListener,
    std::function<void()> success, std::function<void(int)> failure
) {
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
                    (int)level["level_id"].asInt().unwrap(),
                    level["name"].asString().unwrap(),
                    (int)level["placement"].asInt().unwrap()
                });
            });

            success();
        }
    });

    isOk(PEMONLIST_UPTIME_URL, okListener, false, [listener, failure = std::move(failure)](bool ok, int code) {
        if (ok) listener->setFilter(web::WebRequest().get(PEMONLIST_URL));
        else failure(code);
    });
}
