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
void Unit::morphWarpGate() const
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
	if (m_unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_WARPGATE) {
		for (auto & ability : m_bot->Query()->GetAbilitiesForUnit(m_unit).abilities) {
			if (ability.ability_id == sc2::ABILITY_ID::TRAINWARP_STALKER) {
				return false;
			}
		}
		return true;
	}
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

	if (type.is(sc2::UNIT_TYPEID::PROTOSS_STALKER) && m_unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_WARPGATE) {
		float furthest = 0;
		Unit create;
		for (auto unit : m_bot->GetUnits()) {
			if (unit.getPlayer() == Players::Self && unit.getType().is(sc2::UNIT_TYPEID::PROTOSS_PYLON)) {
				float temp = Util::Dist(m_unit->pos, unit.getPosition());
				if (temp > furthest) {
					furthest = temp;
					create = unit;
				}
			}
		}

		std::vector <sc2::Point2D> possible;

		float xCenter = create.getPosition().x, yCenter = create.getPosition().y;
		for (auto x =  xCenter- 7; x <= xCenter; x++) {
			for (auto y = yCenter - 7; y <= yCenter; y++) {
				if ((x - xCenter)*(x - xCenter) + (y - yCenter)*(y - yCenter) <= 49) {
					bool valid[4] = {true, true, true, true};
					float xSym = xCenter - (x - xCenter);
					float ySym = yCenter - (y - yCenter);
					for (auto unit : m_bot->GetUnits()) {
						CCPosition temp = unit.getPosition();
						if (temp == CCPosition(x, y) || !m_bot->Map().isWalkable(int(x), int(y))) {
							valid[0] = false;;
						}
						if (temp == CCPosition(xSym, y) || !m_bot->Map().isWalkable(int(xSym), int(y))) {
							valid[1] = false;
						}
						if (temp == CCPosition(x, ySym) || !m_bot->Map().isWalkable(int(x), int(ySym))) {
							valid[2] = false;
						}
						if (temp == CCPosition(xSym, ySym) || !m_bot->Map().isWalkable(int(xSym), int(ySym))) {
							valid[3] = false;
						}
					}
					
					if (valid[0] && m_bot->Map().isConnected(sc2::Point2D(create.getPosition()), sc2::Point2D(CCPosition(x, y)))) {
						possible.push_back(sc2::Point2D(CCPosition(x, y)));
					}
					if (valid[1] && m_bot->Map().isConnected(sc2::Point2D(create.getPosition()), sc2::Point2D(CCPosition(xSym, y)))) {
						possible.push_back(sc2::Point2D(CCPosition(xSym, y)));
					}
					if (valid[2] && m_bot->Map().isConnected(sc2::Point2D(create.getPosition()), sc2::Point2D(CCPosition(x, ySym)))) {
						possible.push_back(sc2::Point2D(CCPosition(x, ySym)));
					}
					if (valid[3] && m_bot->Map().isConnected(sc2::Point2D(create.getPosition()), sc2::Point2D(CCPosition(xSym, ySym)))) {
						possible.push_back(sc2::Point2D(CCPosition(xSym, ySym)));
					}				
				}
			}
		}
		for (auto pos : possible) {
			m_bot->Actions()->UnitCommand(m_unit, 1414, pos);
			if (m_bot->GetUnit(m_unit->tag).isTraining()) {
				break;
			}
		}
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