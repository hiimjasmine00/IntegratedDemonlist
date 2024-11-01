#include "IntegratedDemonlist.hpp"

using namespace geode::prelude;

#define AREDL_URL "https://api.aredl.net/api/aredl/levels"
#define AREDL_PACKS_URL "https://api.aredl.net/api/aredl/packs"
#define PEMONLIST_UPTIME_URL "https://pemonlist.com/api/uptime?version=2"
#define PEMONLIST_URL "https://pemonlist.com/api/list?limit=500&version=2"

void IntegratedDemonlist::isOk(std::string const& url, EventListener<web::WebTask>&& listenerRef, bool head, std::function<void(bool, int)> const& callback) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) callback(res->ok(), res->code());
    });
    listenerRef.setFilter(head ? web::WebRequest().send("HEAD", url) : web::WebRequest().downloadRange({ 0, 0 }).get(url));
}

void IntegratedDemonlist::loadAREDL(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    LoadingCircle* circle, std::function<void()> const& callback
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback, circle](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) {
                queueInMainThread([circle, res] {
                    FLAlertLayer::create(fmt::format("Load Failed ({})", res->code()).c_str(), "Failed to load AREDL. Please try again later.", "OK")->show();
                    circle->setVisible(false);
                });
                return;
            }

            AREDL_LOADED = true;
            AREDL.clear();
            auto str = res->string().value();
            std::string error;
            auto json = matjson::parse(str, error).value_or(matjson::Array());
            if (!error.empty()) log::error("Failed to parse AREDL: {}", error);
            if (json.is_array()) for (auto const& level : json.as_array()) {
                if (level.contains("legacy") && level["legacy"].is_bool() && level["legacy"].as_bool()) continue;
                if (!level.contains("level_id") || !level["level_id"].is_number()) continue;
                if (!level.contains("name") || !level["name"].is_string()) continue;
                if (!level.contains("position") || !level["position"].is_number()) continue;

                AREDL.push_back({
                    level["level_id"].as_int(),
                    level["name"].as_string(),
                    level["position"].as_int()
                });
            }
            callback();
        }
    });

    isOk(AREDL_URL, std::move(okListener), true, [&listener, circle](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(AREDL_URL));
        else queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load AREDL. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });
}

void IntegratedDemonlist::loadAREDLPacks(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    LoadingCircle* circle, std::function<void()> const& callback
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback, circle](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) {
                queueInMainThread([circle, res] {
                    FLAlertLayer::create(fmt::format("Load Failed ({})", res->code()).c_str(), "Failed to load AREDL packs. Please try again later.", "OK")->show();
                    circle->setVisible(false);
                });
                return;
            }

            AREDL_PACKS.clear();
            auto str = res->string().value();
            std::string error;
            auto json = matjson::parse(str, error).value_or(matjson::Array());
            if (!error.empty()) log::error("Failed to parse AREDL packs: {}", error);
            if (json.is_array()) for (auto const& pack : json.as_array()) {
                std::vector<int> levels;
                for (auto const& level : pack["levels"].as_array()) levels.push_back(level["level_id"].as_int());
                AREDL_PACKS.push_back({
                    pack["name"].as_string(),
                    pack["points"].as_double(),
                    levels
                });
            }
            std::sort(AREDL_PACKS.begin(), AREDL_PACKS.end(), [](auto const& a, auto const& b) {
                return a.points < b.points;
            });
            callback();
        }
    });

    isOk(AREDL_PACKS_URL, std::move(okListener), true, [&listener, circle](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(AREDL_PACKS_URL));
        else queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load AREDL packs. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });
}

void IntegratedDemonlist::loadPemonlist(
    EventListener<web::WebTask>&& listenerRef, EventListener<web::WebTask>&& okListener,
    LoadingCircle* circle, std::function<void()> const& callback
) {
    auto&& listener = std::move(listenerRef);
    listener.bind([callback, circle](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) {
                queueInMainThread([circle, res] {
                    FLAlertLayer::create(fmt::format("Load Failed ({})", res->code()).c_str(), "Failed to load Pemonlist. Please try again later.", "OK")->show();
                    circle->setVisible(false);
                });
                return;
            }

            PEMONLIST_LOADED = true;
            PEMONLIST.clear();
            auto str = res->string().value();
            std::string error;
            auto json = matjson::parse(str, error).value_or(matjson::Object());
            if (!error.empty()) log::error("Failed to parse Pemonlist: {}", error);
            if (json.is_object() && json.contains("data") && json["data"].is_array()) for (auto const& level : json["data"].as_array()) {
                if (!level.contains("level_id") || !level["level_id"].is_number()) continue;
                if (!level.contains("name") || !level["name"].is_string()) continue;
                if (!level.contains("placement") || !level["placement"].is_number()) continue;

                PEMONLIST.push_back({
                    level["level_id"].as_int(),
                    level["name"].as_string(),
                    level["placement"].as_int()
                });
            }
            callback();
        }
    });

    isOk(PEMONLIST_UPTIME_URL, std::move(okListener), false, [&listener, circle](bool ok, int code) {
        if (ok) listener.setFilter(web::WebRequest().get(PEMONLIST_URL));
        else queueInMainThread([circle, code] {
            FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load Pemonlist. Please try again later.", "OK")->show();
            circle->setVisible(false);
        });
    });
}
