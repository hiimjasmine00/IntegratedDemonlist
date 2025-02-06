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
        inline static std::set<int> LOADED_DEMONS;
        EventListener<web::WebTask> m_soloListener;
        EventListener<web::WebTask> m_dualListener;
    };

    static void onModify(ModifyBase<ModifyDerive<IDLevelCell, LevelCell>>& self) {
        auto hookRes = self.getHook("LevelCell::loadCustomLevelCell");
        if (hookRes.isErr()) return log::error("Failed to get LevelCell::loadCustomLevelCell hook: {}", hookRes.unwrapErr());

        auto hook = hookRes.unwrap();
        hook->setAutoEnable(Mod::get()->getSettingValue<bool>("enable-rank"));

        listenForSettingChanges<bool>("enable-rank", [hook](bool value) {
            auto changeRes = value ? hook->enable() : hook->disable();
            if (changeRes.isErr()) log::error("Failed to {} LevelCell::loadCustomLevelCell hook: {}", value ? "enable" : "disable", changeRes.unwrapErr());
        });
    }

    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();

        if (m_level->m_demon.value() <= 0 || (m_level->m_levelLength != 5 && m_level->m_demonDifficulty < 6) ||
            (m_level->m_levelLength == 5 && m_level->m_demonDifficulty != 0 && m_level->m_demonDifficulty < 5)) return;

        auto platformer = m_level->m_levelLength == 5;
        auto levelID = m_level->m_levelID.value();
        auto positions = ranges::map<std::vector<std::string>>(
            ranges::filter(platformer ? IntegratedDemonlist::PEMONLIST : IntegratedDemonlist::AREDL, [levelID](const IDListDemon& demon) {
                return demon.id == levelID;
            }), [levelID](const IDListDemon& demon) { return std::to_string(demon.position); });
        if (!positions.empty()) return addRank(positions, platformer);

        if (Fields::LOADED_DEMONS.contains(levelID)) return;

        auto f = m_fields.self();
        f->m_soloListener.bind([this, f, levelID, platformer](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                Fields::LOADED_DEMONS.insert(levelID);
                if (!res->ok()) return;

                auto json = res->json().unwrapOr(matjson::Value());
                auto key = platformer ? "placement" : "position";
                if (!json.isObject() || !json.contains(key) || !json[key].isNumber()) return;

                int position1 = json[key].asInt().unwrap();
                if (platformer && position1 > 150) return;

                auto& list = platformer ? IntegratedDemonlist::PEMONLIST : IntegratedDemonlist::AREDL;
                std::string levelName = m_level->m_levelName;
                list.push_back({ levelID, levelName, position1 });
                if (platformer) return addRank({ std::to_string(position1) }, platformer);

                f->m_dualListener.bind([this, levelID, levelName, position1](web::WebTask::Event* e) {
                    if (auto res = e->getValue()) {
                        if (!res->ok()) return addRank({ std::to_string(position1) }, false);

                        auto json = res->json().unwrapOr(matjson::Value());
                        if (!json.isObject() || !json.contains("position") || !json["position"].isNumber()) return;

                        int position2 = json["position"].asInt().unwrap();
                        IntegratedDemonlist::AREDL.push_back({ levelID, levelName, position2 });
                        addRank({ std::to_string(position1), std::to_string(position2) }, false);
                    }
                });

                f->m_dualListener.setFilter(web::WebRequest().get(fmt::format(AREDL_LEVEL_2P_URL, levelID)));
            }
        });

        f->m_soloListener.setFilter(web::WebRequest().get(platformer ? fmt::format(PEMONLIST_LEVEL_URL, levelID) : fmt::format(AREDL_LEVEL_URL, levelID)));
    }

    void addRank(const std::vector<std::string>& positions, bool platformer) {
        auto rankTextNode = CCLabelBMFont::create(fmt::format("#{} {}", string::join(positions, "/#"), platformer ? "Pemonlist" : "AREDL").c_str(), "chatFont.fnt");
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
