#include "../IntegratedDemonlist.hpp"
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <jasmine/hook.hpp>
#include <jasmine/setting.hpp>

using namespace geode::prelude;

constexpr const char* aredlLevelUrl = "https://api.aredl.net/v2/api/aredl/levels/{}";
constexpr const char* aredlLevel2pUrl = "https://api.aredl.net/v2/api/aredl/levels/{}_2p";
constexpr const char* pemonlistLevelUrl = "https://pemonlist.com/api/level/{}?version=2";

class $modify(IDLevelCell, LevelCell) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
    };

    inline static std::set<int> loadedDemons;

    static void onModify(ModifyBase<ModifyDerive<IDLevelCell, LevelCell>>& self) {
        (void)self.setHookPriorityAfterPost("LevelCell::loadFromLevel", "hiimjustin000.level_size");
        jasmine::hook::modify(self.m_hooks, "LevelCell::loadFromLevel", "enable-rank");
    }

    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        auto platformer = level->isPlatformer();
        auto difficulty = level->m_demonDifficulty;
        if (level->m_levelType == GJLevelType::Editor || level->m_demon.value() <= 0 ||
            (!platformer && difficulty < 6) || (platformer && difficulty != 0 && difficulty < 5)) return;

        auto levelID = level->m_levelID.value();
        std::vector<int> positions;
        for (auto& demon : platformer ? IntegratedDemonlist::pemonlist : IntegratedDemonlist::aredl) {
            if (demon.id == levelID) positions.push_back(demon.position);
        }
        if (!positions.empty()) return addRank(positions);

        if (loadedDemons.contains(levelID)) return;
        loadedDemons.insert(levelID);

        auto f = m_fields.self();
        f->m_listener.bind([
            this,
            levelID,
            levelName = std::string(level->m_levelName),
            platformer,
            twoPlayer = level->m_twoPlayerMode
        ](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (!res->ok()) return;

                auto json = res->json();
                if (!json.isOk()) return;

                auto position = json.unwrap().get<int>(platformer ? "placement" : "position");
                if (!position.isOk()) return;

                auto position1 = position.unwrap();
                if (platformer && position1 > 150) return;

                IDListDemon demon(levelID, position1, levelName);
                auto& list = platformer ? IntegratedDemonlist::pemonlist : IntegratedDemonlist::aredl;
                if (!std::ranges::contains(list, demon)) {
                    list.push_back(std::move(demon));
                }

                std::vector<int> positions = { position1 };
                if (platformer || !twoPlayer) return addRank(positions);

                auto url = fmt::format(aredlLevel2pUrl, levelID);

                auto f = m_fields.self();
                f->m_listener.bind([this, levelID, levelName, positions](web::WebTask::Event* e) mutable {
                    if (auto res = e->getValue()) {
                        if (!res->ok()) return addRank(positions);

                        auto json = res->json();
                        if (!json.isOk()) return addRank(positions);

                        auto position = json.unwrap().get<int>("position");
                        if (!position.isOk()) return addRank(positions);

                        auto position2 = position.unwrap();
                        IDListDemon demon(levelID, position2, levelName);
                        if (!std::ranges::contains(IntegratedDemonlist::aredl, demon)) {
                            IntegratedDemonlist::aredl.push_back(std::move(demon));
                        }

                        positions.push_back(position2);
                        addRank(positions);
                    }
                });

                f->m_listener.setFilter(web::WebRequest().get(url));
            }
        });

        f->m_listener.setFilter(web::WebRequest().get(platformer ? fmt::format(pemonlistLevelUrl, levelID) : fmt::format(aredlLevelUrl, levelID)));
    }

    void addRank(const std::vector<int>& positions) {
        if (m_mainLayer->getChildByID("level-rank-label"_spr)) return;

        auto dailyLevel = m_level->m_dailyID.value() > 0;
        auto isWhite = dailyLevel || jasmine::setting::getValue<bool>("white-rank");

        fmt::memory_buffer positionsStr;
        for (auto it = positions.begin(); it != positions.end(); ++it) {
            if (it != positions.begin()) positionsStr.push_back('/');
            fmt::format_to(std::back_inserter(positionsStr), "#{}", *it);
        }
        if (m_level->isPlatformer()) fmt::format_to(std::back_inserter(positionsStr), " Pemonlist");
        else fmt::format_to(std::back_inserter(positionsStr), " AREDL");

        auto rankTextNode = CCLabelBMFont::create(fmt::to_string(positionsStr).c_str(), "chatFont.fnt");
        rankTextNode->setPosition({ 346.0f, dailyLevel ? 6.0f : 1.0f });
        rankTextNode->setAnchorPoint({ 1.0f, 0.0f });
        rankTextNode->setScale(m_compactView ? 0.45f : 0.6f);
        auto rlc = Loader::get()->getLoadedMod("raydeeux.revisedlevelcells");
        if (rlc && rlc->getSettingValue<bool>("enabled") && rlc->getSettingValue<bool>("blendingText")) {
            rankTextNode->setBlendFunc({ GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA });
        }
        else if (isWhite) {
            rankTextNode->setOpacity(152);
        }
        else {
            rankTextNode->setColor({ 51, 51, 51 });
            rankTextNode->setOpacity(200);
        }
        rankTextNode->setID("level-rank-label"_spr);
        m_mainLayer->addChild(rankTextNode);

        if (auto levelSizeLabel = m_mainLayer->getChildByID("hiimjustin000.level_size/size-label")) {
            levelSizeLabel->setPosition({
                m_compactView ? 343.0f - rankTextNode->getScaledContentWidth() : 346.0f,
                m_compactView ? 1.0f : 12.0f
            });
        }
    }
};
