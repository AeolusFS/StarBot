#include "CCBot.h"
#include "Util.h"

CCBot::CCBot()
    : m_map(*this)
    , m_bases(*this)
    , m_unitInfo(*this)
    , m_workers(*this)
    , m_gameCommander(*this)
    , m_strategy(*this)
    , m_techTree(*this)
{
    
}

void CCBot::OnGameStart() 
{
    m_config.readConfigFile();

    // add all the possible start locations on the map
#ifdef SC2API
    for (auto & loc : Observation()->GetGameInfo().enemy_start_locations)
    {
        m_baseLocations.push_back(loc);
    }
    m_baseLocations.push_back(Observation()->GetStartLocation());
#else
    for (auto & loc : BWAPI::Broodwar->getStartLocations())
    {
        m_baseLocations.push_back(BWAPI::Position(loc));
    }

    // set the BWAPI game flags
    BWAPI::Broodwar->setLocalSpeed(m_config.SetLocalSpeed);
    BWAPI::Broodwar->setFrameSkip(m_config.SetFrameSkip);
    
    if (m_config.CompleteMapInformation)
    {
        BWAPI::Broodwar->enableFlag(BWAPI::Flag::CompleteMapInformation);
    }

    if (m_config.UserInput)
    {
        BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);
    }
#endif
    
    setUnits();
    m_techTree.onStart();
    m_strategy.onStart();
    m_map.onStart();
    m_unitInfo.onStart();
    m_bases.onStart();
    m_workers.onStart();

    m_gameCommander.onStart();
}

void CCBot::OnStep()
{
    setUnits();
	std::vector<Unit> gateways;
	std::vector<Unit> nexuses;
	std::vector<Unit> twilightCouncils;
	std::vector<Unit> cyberneticsCores;
	for (auto & unit : m_allUnits) {
		if (unit.getPlayer() == Players::Self) {
			if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_NEXUS) {
				//std::cout << "found nexus" << std::endl;
				nexuses.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_GATEWAY) {
				std::cout << "found gateway" << std::endl;
				gateways.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) {
				std::cout << "found cybernetics core" << std::endl;
				cyberneticsCores.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) {
				std::cout << "found twilight council" << std::endl;
				twilightCouncils.push_back(unit);
			}
		}
	}
	for (auto & nexus : nexuses) {
		for (auto & unit : twilightCouncils) {
			nexus.chronoBoost(unit);
		}
		for (auto & unit : cyberneticsCores) {
			nexus.chronoBoost(unit);
		}
		for (auto & unit : gateways) {
			nexus.chronoBoost(unit);
		}
		for (auto & unit : nexuses) {
			nexus.chronoBoost(unit);
		}
	}

    m_map.onFrame();
    m_unitInfo.onFrame();
    m_bases.onFrame();
    m_workers.onFrame();
    m_strategy.onFrame();
	
    m_gameCommander.onFrame();

#ifdef SC2API
    Debug()->SendDebug();
#endif
}

void CCBot::setUnits()
{
    m_allUnits.clear();
#ifdef SC2API
    Control()->GetObservation();
    for (auto & unit : Observation()->GetUnits())
    {
        m_allUnits.push_back(Unit(unit, *this));    
    }
#else
    for (auto & unit : BWAPI::Broodwar->getAllUnits())
    {
        m_allUnits.push_back(Unit(unit, *this));
    }
#endif
}

CCRace CCBot::GetPlayerRace(int player) const
{
#ifdef SC2API
    auto playerID = Observation()->GetPlayerID();
    for (auto & playerInfo : Observation()->GetGameInfo().player_info)
    {
        if (playerInfo.player_id == playerID)
        {
            return playerInfo.race_actual;
        }
    }

    BOT_ASSERT(false, "Didn't find player to get their race");
    return sc2::Race::Random;
#else
    if (player == Players::Self)
    {
        return BWAPI::Broodwar->self()->getRace();
    }
    else
    {
        return BWAPI::Broodwar->enemy()->getRace();
    }
#endif
}

BotConfig & CCBot::Config()
{
     return m_config;
}

const MapTools & CCBot::Map() const
{
    return m_map;
}

const StrategyManager & CCBot::Strategy() const
{
    return m_strategy;
}

const BaseLocationManager & CCBot::Bases() const
{
    return m_bases;
}

const UnitInfoManager & CCBot::UnitInfo() const
{
    return m_unitInfo;
}

const TypeData & CCBot::Data(const UnitType & type) const
{
    return m_techTree.getData(type);
}

const TypeData & CCBot::Data(const Unit & unit) const
{
    return m_techTree.getData(unit.getType());
}

const TypeData & CCBot::Data(const CCUpgrade & type) const
{
    return m_techTree.getData(type);
}

const TypeData & CCBot::Data(const MetaType & type) const
{
    return m_techTree.getData(type);
}

WorkerManager & CCBot::Workers()
{
    return m_workers;
}

int CCBot::GetMinerals() const
{
#ifdef SC2API
    return Observation()->GetMinerals();
#else
    return BWAPI::Broodwar->self()->minerals();
#endif
}

int CCBot::GetGas() const
{
#ifdef SC2API
    return Observation()->GetVespene();
#else
    return BWAPI::Broodwar->self()->gas();
#endif
}

Unit CCBot::GetUnit(const CCUnitID & tag) const
{
#ifdef SC2API
    return Unit(Observation()->GetUnit(tag), *(CCBot *)this);
#else
    return Unit(BWAPI::Broodwar->getUnit(tag), *(CCBot *)this);
#endif
}

const std::vector<Unit> & CCBot::GetUnits() const
{
    return m_allUnits;
}

CCPosition CCBot::GetStartLocation() const
{
#ifdef SC2API
    return Observation()->GetStartLocation();
#else
    return BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
#endif
}

const std::vector<CCPosition> & CCBot::GetStartLocations() const
{
    return m_baseLocations;
}

#ifdef SC2API
void CCBot::OnError(const std::vector<sc2::ClientError> & client_errors, const std::vector<std::string> & protocol_errors)
{
    
}
#endif