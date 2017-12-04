#include "Unit.h"
#include "CCBot.h"
#include <algorithm>

Unit::Unit()
    : m_bot(nullptr)
    , m_unit(nullptr)
    , m_unitID(0)
{

}

#ifdef SC2API
Unit::Unit(const sc2::Unit * unit, CCBot & bot)
    : m_bot(&bot)
    , m_unit(unit)
    , m_unitID(unit->tag)
    , m_unitType(unit->unit_type, bot)
{
    
}
const sc2::Unit * Unit::getUnitPtr() const
{
    return m_unit;
}

const sc2::UnitTypeID & Unit::getAPIUnitType() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
    return m_unit->unit_type;
}

#else
Unit::Unit(const BWAPI::Unit unit, CCBot & bot)
    : m_bot(&bot)
    , m_unit(unit)
    , m_unitID(unit->getID())
    , m_unitType(unit->getType(), bot)
{
    
}

const BWAPI::Unit Unit::getUnitPtr() const
{
    return m_unit;
}

const BWAPI::UnitType & Unit::getAPIUnitType() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
    return m_unit->getType();
}

#endif
bool Unit::operator < (const Unit & rhs) const
{
    return m_unit < rhs.m_unit;
}

bool Unit::operator == (const Unit & rhs) const
{
    return m_unit == rhs.m_unit;
}

const UnitType & Unit::getType() const
{
    return m_unitType;
}


CCPosition Unit::getPosition() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->pos;
#else
    return m_unit->getPosition();
#endif
}

CCTilePosition Unit::getTilePosition() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return Util::GetTilePosition(m_unit->pos);
#else
    return m_unit->getTilePosition();
#endif
}

CCHealth Unit::getHitPoints() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->health;
#else
    return m_unit->getHitPoints();
#endif
}

CCHealth Unit::getShields() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->shield;
#else
    return m_unit->getShields();
#endif
}

CCHealth Unit::getEnergy() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
	std::cout << "id: " << m_unitID << " has energy " << m_unitType.getName() << std::endl;
	
    return m_unit->energy;
#else
    return m_unit->getEnergy();
#endif
}
void Unit::morphWarpGate(const Unit & target) const
{
#ifdef SC2API
	m_bot->Actions()->UnitCommand(m_unit, 1518);
#endif
}


void Unit::chronoBoost(const Unit & target) const 
{
#ifdef SC2API
	if (m_unit->energy >= 50) {
		if (target.isTraining()) {
			m_bot->Actions()->UnitCommand(m_unit, 3755, target.getUnitPtr());
		}
	}
#endif
}
float Unit::getBuildPercentage() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->build_progress;
#else
    if (getType().isBuilding()) { return m_unit->getRemainingBuildTime() / (float)getType().getAPIUnitType().buildTime(); }
    else { return m_unit->getRemainingTrainTime() / (float)getType().getAPIUnitType().buildTime(); }
#endif
}

CCPlayer Unit::getPlayer() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    if (m_unit->alliance == sc2::Unit::Alliance::Self) { return 0; }
    else if (m_unit->alliance == sc2::Unit::Alliance::Enemy) { return 1; }
    else { return 2; }
#else
    if (m_unit->getPlayer() == BWAPI::Broodwar->self()) { return 0; }
    else if (m_unit->getPlayer() == BWAPI::Broodwar->enemy()) { return 1; }
    else { return 2; }
#endif
}

CCUnitID Unit::getID() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    CCUnitID id = m_unit->tag;
#else
    CCUnitID id = m_unit->getID();
#endif

    BOT_ASSERT(id == m_unitID, "Unit ID changed somehow");
    return id;
}

bool Unit::isCompleted() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->build_progress >= 1.0f;
#else
    return m_unit->isCompleted();
#endif
}

bool Unit::isTraining() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->orders.size() > 0;
#else
    return m_unit->isTraining();
#endif
}

bool Unit::isBeingConstructed() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return !isCompleted() && m_unit->build_progress > 0.0f;
#else
    return m_unit->isBeingConstructed();
#endif
}

int Unit::getWeaponCooldown() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return (int)m_unit->weapon_cooldown;
#else
    return std::max(m_unit->getGroundWeaponCooldown(), m_unit->getAirWeaponCooldown());
#endif
}

bool Unit::isCloaked() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->cloak;
#else
    return m_unit->isCloaked();
#endif
}

bool Unit::isFlying() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->is_flying;
#else
    return m_unit->isFlying();
#endif
}

bool Unit::isAlive() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->is_alive;
#else
    return m_unit->getHitPoints() > 0;
#endif
}

bool Unit::isPowered() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
	
    return m_unit->is_powered;
#else
    return m_unit->isPowered();
#endif
}

bool Unit::isIdle() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->orders.empty();
#else
    return m_unit->isIdle() && !m_unit->isMoving() && !m_unit->isGatheringGas() && !m_unit->isGatheringMinerals();
#endif
}

bool Unit::isBurrowed() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    return m_unit->is_burrowed;
#else
    return m_unit->isBurrowed();
#endif
}

bool Unit::isValid() const
{
    return m_unit != nullptr;
}

void Unit::stop() const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::STOP);
#else
    m_unit->stop();
#endif
}

void Unit::attackUnit(const Unit & target) const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
    BOT_ASSERT(target.isValid(), "Target is not valid");
#ifdef SC2API
    m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::ATTACK_ATTACK, target.getUnitPtr());
#else
    m_unit->attack(target.getUnitPtr());
#endif
}

void Unit::attackMove(const CCPosition & targetPosition) const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::ATTACK_ATTACK, targetPosition);
#else
    m_unit->attack(targetPosition);
#endif
}

void Unit::move(const CCPosition & targetPosition) const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
	if (m_unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_STALKER) {
		bool blink = false;
		for (auto & abilites : m_bot->Query()->GetAbilitiesForUnit(m_unit).abilities) {
			if (abilites.ability_id == sc2::ABILITY_ID::EFFECT_BLINK) {
				blink = true;
				break;
			}
		}

		if (blink) {
			m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::EFFECT_BLINK_STALKER, targetPosition);
		}
		else {
			m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::MOVE, targetPosition);
		}
	}
	else {
		m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::MOVE, targetPosition);
	}
#else
    m_unit->move(targetPosition);
#endif
}

void Unit::move(const CCTilePosition & targetPosition) const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
	if (m_unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_STALKER) {
		bool blink = false;
		for (auto & abilites : m_bot->Query()->GetAbilitiesForUnit(m_unit).abilities) {
			if (abilites.ability_id == sc2::ABILITY_ID::EFFECT_BLINK) {
				blink = true;
				break;
			}
		}

		if (blink) {
			m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::EFFECT_BLINK_STALKER, CCPosition((float)targetPosition.x, (float)targetPosition.y));
		}
		else {
			m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::MOVE, CCPosition((float)targetPosition.x, (float)targetPosition.y));
		}
	}
	else {
		m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::MOVE, CCPosition((float)targetPosition.x, (float)targetPosition.y));
	}
#else
    m_unit->move(CCPosition(targetPosition));
#endif
}

void Unit::rightClick(const Unit & target) const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    m_bot->Actions()->UnitCommand(m_unit, sc2::ABILITY_ID::SMART, target.getUnitPtr());
#else
    m_unit->rightClick(target.getUnitPtr());
#endif
}

void Unit::repair(const Unit & target) const
{
    rightClick(target);
}

void Unit::build(const UnitType & buildingType, CCTilePosition pos) const
{
    BOT_ASSERT(m_bot->Map().isConnected(getTilePosition(), pos), "Error: Build Position is not connected to worker");
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    m_bot->Actions()->UnitCommand(m_unit, m_bot->Data(buildingType).buildAbility, Util::GetPosition(pos));
#else
    m_unit->build(buildingType.getAPIUnitType(), pos);
#endif
}

void Unit::buildTarget(const UnitType & buildingType, const Unit & target) const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
    m_bot->Actions()->UnitCommand(m_unit, m_bot->Data(buildingType).buildAbility, target.getUnitPtr());
#else
    BOT_ASSERT(false, "buildTarget shouldn't be called for BWAPI bots");
#endif
}

void Unit::train(const UnitType & type) const
{
    BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
	std::vector<Unit> aList;
	std::vector<Unit> stalkers;
	std::vector<float> xList = { 0,1,2,3,4 };
	std::vector<float> yList = { 0,1,2,3,4 };
	const static CCPosition mybase = m_bot->GetStartLocation();
	aList.clear();

	for (auto & aUnit : m_bot->GetUnits()) {
		if (aUnit.getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_PYLON) {
			aList.push_back(aUnit);
		}
	}
	static CCPosition selected;
	static bool warping = false;


	if (m_unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_WARPGATE) {
		//std::cout << m_bot->Data(type).warpAbility.to_string << std::endl;
		if (warping) {
			selected.x;
			selected.y;
		}
		warping = true;

		if (warping) {
			float max = 0;
			const static BaseLocation *eBase = m_bot->Bases().getPlayerStartingBaseLocation(Players::Enemy);
			for (auto & pylon : aList) {
				if (max < Util::DistSq(mybase, pylon.getPosition())) {
					max = Util::DistSq(mybase, pylon.getPosition());
					selected = pylon.getPosition();
				}
			}
		}
		CCTilePosition tile(selected.x, selected.y);
		CCTilePosition spawnTile = m_bot->GetWalkableTile(tile);
		/*
		std::random_shuffle(xList.begin(), xList.end());
		std::random_shuffle(yList.begin(), yList.end());
		if (m_bot->Map().canBuildTypeAtPosition(int(xList[0]), int(yList[0]), type)) {
			selected.x += xList[0];
			selected.y += yList[0];
		}
		*/
		

		//m_bot->Map().
		UnitType pylon = UnitType::GetUnitTypeFromName("Pylon", *m_bot);
		/*for (float xFloat = 0;) {
			for (auto & yFloat : yList) {
				if (m_bot->Query()->Placement(m_bot->Data(pylon).buildAbility, CCPosition(selected.x + xFloat, selected.y + yFloat))) {
					selected.x += xFloat;
					selected.y += yFloat;
					break;
				}
				std::cout << "(" << selected.x << ", " << selected.y << ")" << std::endl;
			}
		}*/
		
		/*while (1) {
			float xFloat = selected.x + 3 * cos(rand());
			float yFloat = selected.y + 3 * sin(rand());
			if (m_bot->Query()->Placement(m_bot->Data(pylon).buildAbility, CCPosition(selected.x + xFloat, selected.y + yFloat))) {
				std::cout << "worked" << std::endl;
				selected.x += xFloat;
				selected.y += yFloat;
				break;
			}
			std::cout << "(" << selected.x << ", " << selected.y << ")" << std::endl;
			std::cout << "flaot s(" << xFloat << ", " << yFloat << ")" << std::endl;
		}*/
		
		//Util::Ds
		//std::cout << aListx << " : " << aListy << std::endl;
		//m_bot->Actions()->UnitCommand(aUnit.getUnitPtr(), m_bot->Data(type).warpAbility.IsValid, sc2::Point2D(aListx, aListy);)
		
		m_bot->Actions()->UnitCommand(m_unit, m_bot->Data(type).warpAbility, sc2::Point2D(spawnTile.x, spawnTile.y));
		//m_bot->Actions()->UnitCommand(m_unit, m_bot->Data(type).warpAbility, sc2::Point2D(selected.x, selected.y-4));
		//m_bot->Actions()->UnitCommand(m_unit, m_bot->Data(type).warpAbility, sc2::Point2D(selected.x-4, selected.y-4));//(sc2::ABILITY_ID::TRAINWARP_STALKER, sc2::Point2D(aList[0].getPosition()));
	}
	else {
		m_bot->Actions()->UnitCommand(m_unit, m_bot->Data(type).buildAbility);
	}
#else
    m_unit->train(type.getAPIUnitType());
#endif
}

void Unit::upgrade(const CCUpgrade & type) const {
	BOT_ASSERT(isValid(), "Unit is not valid");
#ifdef SC2API
	m_bot->Actions()->UnitCommand(m_unit, m_bot->Data(type).buildAbility);
#endif
}

bool Unit::isConstructing(const UnitType & type) const
{
#ifdef SC2API
    sc2::AbilityID buildAbility = m_bot->Data(type).buildAbility;
    return (getUnitPtr()->orders.size() > 0) && (getUnitPtr()->orders[0].ability_id == buildAbility);
#else
    return m_unit->isConstructing();
#endif
}