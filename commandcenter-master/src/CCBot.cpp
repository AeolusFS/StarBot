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
	, expanded(false)
	, prevTile(0,0)
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
	std::vector<Unit> forges;
	std::vector<Unit> probes;
	std::vector<Unit> stalkers;
	std::vector<Unit> pylons;
	std::vector<Unit> cores;
	std::set<const BaseLocation*> test;
	stalkers.clear();
	test.clear();
	probes.clear();
	//for chronoboosting units
	for (auto & unit : m_allUnits) {
		if (unit.getPlayer() == Players::Self) {
			if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_NEXUS) {
				nexuses.push_back(unit);
			}
			else if ((unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_GATEWAY) || (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_WARPGATE)) {
				gateways.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_FORGE) {
				forges.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) {
				twilightCouncils.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_PROBE) {
				probes.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_STALKER) {
				stalkers.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_PYLON) {
				//unit.getHitPoints()
				pylons.push_back(unit);
			}
			else if (unit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) {
				cores.push_back(unit);
			}
		}
	}
	for (auto & nexus : nexuses) {
		for (auto & unit : twilightCouncils) {
			nexus.chronoBoost(unit);
		}
		for (auto & unit : forges) {
			nexus.chronoBoost(unit);
		}
		for (auto & unit : gateways) {
			nexus.chronoBoost(unit);
		}
		for (auto & unit : nexuses) {
			nexus.chronoBoost(unit);
		}
	}

	int count = 0;
	for (auto unit : m_allUnits) {
		if (unit.getPlayer() == Players::Self && unit.getType().is(sc2::UNIT_TYPEID::PROTOSS_STALKER)) {
			count++;
		}
	}

	if (count > 11) {
		for (auto unit : m_allUnits) {
			if (unit.getPlayer() == Players::Self && unit.getType().is(sc2::UNIT_TYPEID::PROTOSS_GATEWAY)) {
				unit.morphWarpGate();
			}
		}
	}
	// Stuff for warpgate
	// Static so its only initalized once
	static bool isPylonBuilt = false;
	std::vector<const BaseLocation *> thebases;
	enemybase = m_bases.getPlayerStartingBaseLocation(Players::Enemy);
	static const BaseLocation *warpTobase;
	static CCPosition warpTo;
	if (enemybase != 0) {
		// If base is found it can be seen here
		const CCPosition enemyPos = enemybase->getPosition();
		thebases = m_bases.getBaseLocations();
		//static CCPosition warpTo;
		static float max = 100000;
		static size_t selected;
		for (size_t index = 0; index != thebases.size(); ++index) {
			if (Util::Dist(thebases[index]->getPosition(), enemyPos) > 36 && Util::Dist(thebases[index]->getPosition(), enemyPos) < 50 && thebases[index]->getPosition().x != enemyPos.x && thebases[index]->getPosition().y != enemyPos.y) {
				max = Util::Dist(thebases[index]->getPosition(), enemyPos);
				selected = index;
			}
		}
		warpTo = thebases[selected]->getPosition();
		warpTobase = thebases[selected];

	}

	// Get static unit so this only happens once
	static Unit aProbe = probes[0];

	if (isPylonBuilt) {
		if (aProbe.isAlive()) {
			aProbe.move(Util::GetTilePosition(m_bases.getPlayerStartingBaseLocation(Players::Self)->getPosition()));
		}
		else {
			aProbe = probes[0];
		}


		float aNum;
		bool isDead = true;
		for (auto & pylon : pylons) {
			aNum = Util::Dist(m_bases.getPlayerStartingBaseLocation(Players::Self)->getPosition(), pylon.getPosition());
			if (aNum > 50) {
				isDead = false;
			}
		}
		if (isDead) {
			isPylonBuilt = false;
		}
	}

	if (aProbe.isIdle()) {
		m_workers.finishedWithWorker(aProbe);
	}
	


	// This is there is no base and we dont have a scout scoot
	/*
	test = m_bases.getOccupiedBaseLocations(Players::Enemy);
	if (test.empty()) {
		std::cout << "NO BASE" << std::endl;
	}

	if (!test.empty()) {
		std::cout << "BASE" << std::endl;
	}
	*/
	if (stalkers.size() > 0 && !isPylonBuilt) {
		aProbe.move(Util::GetTilePosition(warpTo));
		float isCloseTox = aProbe.getPosition().x - warpTobase->getPosition().x;
		float isCloseToy = aProbe.getPosition().y - warpTobase->getPosition().y;

		if (isCloseTox < 1 && isCloseTox > -1 && isCloseToy < 1 && isCloseToy > -1) {
			aProbe.build(UnitType(sc2::UNIT_TYPEID::PROTOSS_PYLON, *this), Util::GetTilePosition(warpTo));
		}

		for (auto & pylon : pylons) {
			float isCloseTopx = pylon.getPosition().x - warpTobase->getPosition().x;
			float isCloseTopy = pylon.getPosition().y - warpTobase->getPosition().y;
			if (isCloseTopx < 1 && isCloseTopx > -1 && isCloseTopy < 1 && isCloseTopy > -1) {
				isPylonBuilt = true;
				const static bool pylonBuilt = true;
			}
		}
	}

	//expand base if running low on resources, only once atm
	if (!expanded) {
		for (auto & mineral : m_bases.getPlayerStartingBaseLocation(Players::Self)->getMinerals()) {
			if (mineral.getUnitPtr()->mineral_contents < 100) {
				m_gameCommander.expandBase();
				expanded = true;
				break;
			}
		}
	}

	// Try to do warpgate shit
	/*for (auto & warpg : warpgates) {
		if (warpg.getPlayer() == Players::Self) {
			warpg.train(UnitType(sc2::UNIT_TYPEID::PROTOSS_STALKER, *this));
		}
	}*/

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
int CCBot::GetSupplyRemaining() const
{
#ifdef SC2API
	return Observation()->GetFoodCap() - Observation()->GetFoodUsed();
#else
	return BWAPI::Broodwar->self()->gas();
#endif
}
int CCBot::GetTotalSupply() const
{
#ifdef SC2API
	return Observation()->GetFoodCap();
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

CCTilePosition CCBot::GetWalkableTile() {
	auto closest = enemybase->getClosestTiles();
	
	CCTilePosition tileOld = CCTilePosition(0,0);
	for (int i = 0; i < closest.size(); ++i) {

		CCTilePosition tile = closest[i];
		if (m_map.isConnected(tile, Util::GetTilePosition(m_bases.getPlayerStartingBaseLocation(Players::Self)->getPosition()))) {
			if (m_map.isWalkable(tile.x, tile.y) && m_map.isWalkable(tile.x, tile.y)) {
				if ((prevTile != tile) && m_map.isPowered(tile.x, tile.y)) {
					tileOld = tile;

					prevTile = tile;
					return tile;
				}
			}
		}
		

	}
	std::cout << "couldnt find a postition" << std::endl;
	return tileOld;
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