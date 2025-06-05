#include "../IntegratedDemonlist.hpp"
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

#define AREDL_LEVEL_URL "https://api.aredl.net/v2/api/aredl/levels/{}"
#define AREDL_LEVEL_2P_URL "https://api.aredl.net/v2/api/aredl/levels/{}_2p"
#define PEMONLIST_LEVEL_URL "https://pemonlist.com/api/level/{}?version=2"

class $modify(IDLevelCell, LevelCell) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
    };

    inline static std::set<int> loadedDemons;

    static void onModify(ModifyBase<ModifyDerive<IDLevelCell, LevelCell>>& self) {
        (void)self.setHookPriorityAfterPost("LevelCell::loadFromLevel", "hiimjustin000.level_size");

        (void)self.getHook("LevelCell::loadFromLevel").inspect([](Hook* hook) {
            auto mod = Mod::get();
            hook->setAutoEnable(mod->getSettingValue<bool>("enable-rank"));

            listenForSettingChangesV3<bool>("enable-rank", [hook](bool value) {
                (void)(value ? hook->enable().inspectErr([](const std::string& err) {
                    log::error("Failed to enable LevelCell::loadFromLevel hook: {}", err);
                }) : hook->disable().inspectErr([](const std::string& err) {
                    log::error("Failed to disable LevelCell::loadFromLevel hook: {}", err);
                }));
            }, mod);
        }).inspectErr([](const std::string& err) { log::error("Failed to get LevelCell::loadFromLevel hook: {}", err); });
    }

    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        auto platformer = level->m_levelLength == 5;
        auto difficulty = level->m_demonDifficulty;
        if (level->m_levelType == GJLevelType::Editor || level->m_demon.value() <= 0 ||
            (!platformer && difficulty < 6) || (platformer && difficulty != 0 && difficulty < 5)) return;

        auto levelID = level->m_levelID.value();
        auto positions = ranges::reduce<std::vector<int>>(platformer ? IntegratedDemonlist::pemonlist : IntegratedDemonlist::aredl,
            [levelID](std::vector<int>& acc, const IDListDemon& demon) { if (demon.id == levelID) acc.push_back(demon.position); });
        if (!positions.empty()) return addRank(positions);

        if (loadedDemons.contains(levelID)) return;
        loadedDemons.insert(levelID);

        std::string levelName = level->m_levelName;
        auto twoPlayer = level->m_twoPlayerMode;
        auto f = m_fields.self();
        f->m_listener.bind([this, levelID, levelName, platformer, twoPlayer](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (!res->ok()) return;

                auto json = res->json().unwrapOr(matjson::Value::object());
                auto key = platformer ? "placement" : "position";
                if (!json.isObject() || !json.contains(key) || !json[key].isNumber()) return;

                auto position1 = json[key].as<int>().unwrap();
                if (platformer && position1 > 150) return;

                auto& list = platformer ? IntegratedDemonlist::pemonlist : IntegratedDemonlist::aredl;
                list.push_back({ levelID, position1, levelName });
                if (platformer || !twoPlayer) return addRank({ position1 });

                auto url = fmt::format(AREDL_LEVEL_2P_URL, levelID);

                auto f = m_fields.self();
                f->m_listener.bind([this, levelID, levelName, position1](web::WebTask::Event* e) {
                    if (auto res = e->getValue()) {
                        if (!res->ok()) return addRank({ position1 });

                        auto json = res->json().unwrapOr(matjson::Value::object());
                        if (!json.isObject() || !json.contains("position") || !json["position"].isNumber()) return addRank({ position1 });

                        auto position2 = json["position"].as<int>().unwrap();
                        IntegratedDemonlist::aredl.push_back({ levelID, position2, levelName });
                        addRank({ position1, position2 });
                    }
                });

                f->m_listener.setFilter(web::WebRequest().get(url));
            }
        });

        f->m_listener.setFilter(
            web::WebRequest().get(platformer ? fmt::format(PEMONLIST_LEVEL_URL, levelID) : fmt::format(AREDL_LEVEL_URL, levelID)));
    }

    void addRank(const std::vector<int>& positions) {
        auto dailyLevel = m_level->m_dailyID.value() > 0;
        auto isWhite = dailyLevel || Mod::get()->getSettingValue<bool>("white-rank");

        auto rankTextNode = CCLabelBMFont::create(fmt::format("#{} {}",
            ranges::reduce<std::string>(positions, [](std::string& acc, int pos) { acc += (acc.empty() ? "" : "/#") + fmt::to_string(pos); }),
            m_level->m_levelLength == 5 ? "Pemonlist" : "AREDL"
        ).c_str(), "chatFont.fnt");
        rankTextNode->setPosition({ 346.0f, 1.0f + dailyLevel * 5.0f });
        rankTextNode->setAnchorPoint({ 1.0f, 0.0f });
        rankTextNode->setScale(0.6f - m_compactView * 0.15f);
        rankTextNode->setColor({
            (uint8_t)(51 * (isWhite * 4 + 1)),
            (uint8_t)(51 * (isWhite * 4 + 1)),
            (uint8_t)(51 * (isWhite * 4 + 1))
        });
        rankTextNode->setOpacity(200 - isWhite * 48);
        rankTextNode->setID("level-rank-label"_spr);
        m_mainLayer->addChild(rankTextNode);

        if (auto levelSizeLabel = m_mainLayer->getChildByID("hiimjustin000.level_size/size-label")) levelSizeLabel->setPosition({
            346.0f - m_compactView * (rankTextNode->getScaledContentWidth() + 3.0f),
            12.0f - m_compactView * 11.0f
        });
    }
};
