#include "../IntegratedDemonlist.hpp"
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

#define AREDL_LEVEL_URL "https://api.aredl.net/api/aredl/levels/{}"
#define AREDL_LEVEL_2P_URL "https://api.aredl.net/api/aredl/levels/{}?two_player=true"
#define PEMONLIST_LEVEL_URL "https://pemonlist.com/api/level/{}?version=2"

class $modify(IDLevelCell, LevelCell) {
    struct Fields {
        EventListener<web::WebTask> m_soloListener;
        EventListener<web::WebTask> m_dualListener;
    };

    inline static std::set<int> loadedDemons;

    static void onModify(ModifyBase<ModifyDerive<IDLevelCell, LevelCell>>& self) {
        (void)self.setHookPriorityAfterPost("LevelCell::loadFromLevel", "hiimjustin000.level_size");

        (void)self.getHook("LevelCell::loadFromLevel").map([](Hook* hook) {
            auto mod = Mod::get();
            hook->setAutoEnable(mod->getSettingValue<bool>("enable-rank"));

            listenForSettingChanges<bool>("enable-rank", [hook](bool value) {
                (void)(value ? hook->enable().mapErr([](const std::string& err) {
                    return log::error("Failed to enable LevelCell::loadFromLevel hook: {}", err), err;
                }) : hook->disable().mapErr([](const std::string& err) {
                    return log::error("Failed to disable LevelCell::loadFromLevel hook: {}", err), err;
                }));
            }, mod);

            return hook;
        }).mapErr([](const std::string& err) {
            return log::error("Failed to get LevelCell::loadFromLevel hook: {}", err), err;
        });
    }

    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        if (level->m_levelType == GJLevelType::Editor || level->m_demon.value() <= 0 || (level->m_levelLength != 5 && level->m_demonDifficulty < 6) ||
            (level->m_levelLength == 5 && level->m_demonDifficulty != 0 && level->m_demonDifficulty < 5)) return;

        auto platformer = level->m_levelLength == 5;
        auto levelID = level->m_levelID.value();
        auto positions = ranges::reduce<std::vector<std::string>>(platformer ? IntegratedDemonlist::pemonlist : IntegratedDemonlist::aredl,
            [levelID](std::vector<std::string>& acc, const IDListDemon& demon) {
                if (demon.id == levelID) acc.push_back(std::to_string(demon.position));
            });
        if (!positions.empty()) return addRank(positions, platformer);

        if (loadedDemons.contains(levelID)) return;

        auto f = m_fields.self();
        f->m_soloListener.bind([this, level, levelID, platformer](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                loadedDemons.insert(levelID);
                if (!res->ok()) return;

                auto json = res->json().unwrapOr(matjson::Value());
                auto key = platformer ? "placement" : "position";
                if (!json.isObject() || !json.contains(key) || !json[key].isNumber()) return;

                int position1 = json[key].asInt().unwrap();
                if (platformer && position1 > 150) return;

                auto& list = platformer ? IntegratedDemonlist::pemonlist : IntegratedDemonlist::aredl;
                std::string levelName = level->m_levelName;
                list.push_back({ levelID, levelName, position1 });
                if (platformer) return addRank({ std::to_string(position1) }, platformer);

                auto f = m_fields.self();
                f->m_dualListener.bind([this, levelID, levelName, position1](web::WebTask::Event* e) {
                    if (auto res = e->getValue()) {
                        if (!res->ok()) return addRank({ std::to_string(position1) }, false);

                        auto json = res->json().unwrapOr(matjson::Value());
                        if (!json.isObject() || !json.contains("position") || !json["position"].isNumber()) return;

                        int position2 = json["position"].asInt().unwrap();
                        IntegratedDemonlist::aredl.push_back({ levelID, levelName, position2 });
                        addRank({ std::to_string(position1), std::to_string(position2) }, false);
                    }
                });

                f->m_dualListener.setFilter(web::WebRequest().get(fmt::format(AREDL_LEVEL_2P_URL, levelID)));
            }
        });

        f->m_soloListener.setFilter(
            web::WebRequest().get(platformer ? fmt::format(PEMONLIST_LEVEL_URL, levelID) : fmt::format(AREDL_LEVEL_URL, levelID)));
    }

    void addRank(const std::vector<std::string>& positions, bool platformer) {
        auto rankTextNode = CCLabelBMFont::create(
            fmt::format("#{} {}", string::join(positions, "/#"), platformer ? "Pemonlist" : "AREDL").c_str(), "chatFont.fnt");
        auto dailyLevel = m_level->m_dailyID.value() > 0;
        rankTextNode->setPosition({ 346.0f, dailyLevel ? 6.0f : 1.0f });
        rankTextNode->setAnchorPoint({ 1.0f, 0.0f });
        rankTextNode->setScale(m_compactView ? 0.45f : 0.6f);
        auto isWhite = Mod::get()->getSettingValue<bool>("white-rank");
        rankTextNode->setColor(dailyLevel || isWhite ? ccColor3B { 255, 255, 255 } : ccColor3B { 51, 51, 51 });
        rankTextNode->setOpacity(dailyLevel || isWhite ? 200 : 152);
        rankTextNode->setID("level-rank-label"_spr);
        m_mainLayer->addChild(rankTextNode);

        if (auto levelSizeLabel = m_mainLayer->getChildByID("hiimjustin000.level_size/size-label")) levelSizeLabel->setPosition({
            346.0f - (m_compactView ? rankTextNode->getScaledContentWidth() + 3.0f : 0.0f),
            !m_compactView ? 12.0f : 1.0f
        });
    }
};
