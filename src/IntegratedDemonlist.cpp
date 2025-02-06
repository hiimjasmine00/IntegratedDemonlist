#include "IntegratedDemonlist.hpp"
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

#define AREDL_URL "https://api.aredl.net/api/aredl/levels"
#define AREDL_PACKS_URL "https://api.aredl.net/api/aredl/packs"
#define PEMONLIST_UPTIME_URL "https://pemonlist.com/api/uptime?version=2"
#define PEMONLIST_URL "https://pemonlist.com/api/list?limit=150&version=2"

void isOk(const std::string& url, EventListener<web::WebTask>&& listenerRef, bool head, const std::function<void(bool, int)>& callback) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) callback(res->ok(), res->code());
    });
    listenerRef.setFilter(head ? web::WebRequest().send("HEAD", url) : web::WebRequest().downloadRange({ 0, 0 }).get(url));
}

void IntegratedDemonlist::loadAREDL(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    const std::function<void()>& success, const std::function<void(int)>& failure
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([failure, success](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            AREDL_LOADED = true;
            AREDL.clear();
            auto json = res->json().unwrapOr(matjson::Value());
            if (!json.isArray()) return success();

            AREDL = ranges::map<std::vector<IDListDemon>>(ranges::filter(json.asArray().unwrap(), [](const matjson::Value& level) {
                return (!level.contains("legacy") || !level["legacy"].isBool() || !level["legacy"].asBool().unwrap()) &&
                    level.contains("level_id") && level["level_id"].isNumber() &&
                    level.contains("name") && level["name"].isString() &&
                    level.contains("position") && level["position"].isNumber();
            }), [](const matjson::Value& level) {
                return IDListDemon {
                    (int)level["level_id"].asInt().unwrap(),
                    level["name"].asString().unwrap(),
                    (int)level["position"].asInt().unwrap()
                };
            });
            success();
        }
    });

    isOk(AREDL_URL, std::move(okListener), true, [&listener, failure](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(AREDL_URL));
        else failure(code);
    });
}

void IntegratedDemonlist::loadAREDLPacks(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    const std::function<void()>& success, const std::function<void(int)>& failure
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([failure, success](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            AREDL_PACKS.clear();
            auto json = res->json().unwrapOr(matjson::Value());
            if (!json.isArray()) return success();

            AREDL_PACKS = ranges::map<std::vector<IDDemonPack>>(ranges::filter(json.asArray().unwrap(), [](const matjson::Value& pack) {
                return pack.contains("name") && pack["name"].isString() &&
                    pack.contains("points") && pack["points"].isNumber() &&
                    pack.contains("levels") && pack["levels"].isArray();
            }), [](const matjson::Value& pack) {
                return IDDemonPack {
                    pack["name"].asString().unwrap(),
                    pack["points"].asDouble().unwrap(),
                    ranges::map<std::vector<int>>(ranges::filter(pack["levels"].asArray().unwrap(), [](const matjson::Value& level) {
                        return level.contains("level_id") && level["level_id"].isNumber();
                    }), [](const matjson::Value& level) { return level["level_id"].asInt().unwrap(); })
                };
            });
            std::sort(AREDL_PACKS.begin(), AREDL_PACKS.end(), [](const IDDemonPack& a, const IDDemonPack& b) { return a.points < b.points; });
            success();
        }
    });

    isOk(AREDL_PACKS_URL, std::move(okListener), true, [&listener, failure](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(AREDL_PACKS_URL));
        else failure(code);
    });
}

void IntegratedDemonlist::loadPemonlist(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    const std::function<void()>& success, const std::function<void(int)>& failure
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([failure, success](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return failure(res->code());

            PEMONLIST_LOADED = true;
            PEMONLIST.clear();
            auto json = res->json().unwrapOr(matjson::Value());
            if (!json.isObject() || !json.contains("data") || !json["data"].isArray()) return success();

            PEMONLIST = ranges::map<std::vector<IDListDemon>>(ranges::filter(json["data"].asArray().unwrap(), [](const matjson::Value& level) {
                return level.contains("level_id") && level["level_id"].isNumber() &&
                    level.contains("name") && level["name"].isString() &&
                    level.contains("placement") && level["placement"].isNumber();
            }), [](const matjson::Value& level) {
                return IDListDemon {
                    (int)level["level_id"].asInt().unwrap(),
                    level["name"].asString().unwrap(),
                    (int)level["placement"].asInt().unwrap()
                };
            });
            success();
        }
    });

    isOk(PEMONLIST_UPTIME_URL, std::move(okListener), false, [&listener, failure](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(PEMONLIST_URL));
        else failure(code);
    });
}
