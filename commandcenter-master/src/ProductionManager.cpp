#include "ProductionManager.h"
#include "Util.h"
#include "CCBot.h"

ProductionManager::ProductionManager(CCBot & bot)
    : m_bot             (bot)
    , m_buildingManager (bot)
    , m_queue           (bot)
	,doneQueue			(false)
	,prevSupply			(0)
	,prevProbe			(0)
	,prevPhoton			(0)
{

}

void ProductionManager::setBuildOrder(const BuildOrder & buildOrder)
{
    m_queue.clearAll();

    for (size_t i(0); i<buildOrder.size(); ++i)
    {
        m_queue.queueAsLowestPriority(buildOrder[i], true);
    }
}


void ProductionManager::onStart()
{
    m_buildingManager.onStart();
    setBuildOrder(m_bot.Strategy().getOpeningBookBuildOrder());
}

void ProductionManager::onFrame()
{
    // TODO: if nothing is currently building, get a new goal from the strategy manager
	if (doneQueue && (m_bot.GetSupplyRemaining() <= 7) && (prevSupply != m_bot.GetTotalSupply())) {
		prevSupply = m_bot.GetTotalSupply();
		//doneQueue = false;
		BuildOrder buildOrder;
		MetaType metaPylon("Pylon", m_bot);
		buildOrder.add(metaPylon);
		m_queue.queueAsHighestPriority(buildOrder[0], true);
	}
	int probes = 0;
	int photon = 0;
	if (doneQueue) {
		for (auto & unit : m_bot.GetUnits()) {
			if ((unit.getPlayer() == Players::Self) && unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_PROBE) {
				++probes;
			}
			if ((unit.getPlayer() == Players::Self) && unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON) {
				++photon;
			}
		}
	}
	if (doneQueue && (photon < 1) && (prevPhoton != photon)) {
		prevPhoton = photon;
		BuildOrder buildOrder;
		MetaType metaPhoton("PhotonCannon", m_bot);
		MetaType metaStalker("Stalker", m_bot);
		buildOrder.add(metaPhoton);
		buildOrder.add(metaStalker);
		m_queue.queueAsHighestPriority(buildOrder[1], true);
		m_queue.queueAsHighestPriority(buildOrder[0], true);
	}
	if (doneQueue && (probes < 20) && (prevProbe != probes)) {
		prevProbe = probes;
		BuildOrder buildOrder;
		MetaType metaProbe("Probe", m_bot);
		MetaType metaStalker("Stalker", m_bot);
		buildOrder.add(metaProbe);
		buildOrder.add(metaStalker);
		m_queue.queueAsHighestPriority(buildOrder[1], true);
		m_queue.queueAsHighestPriority(buildOrder[0], true);
	}


	if (m_queue.isEmpty()) {
		if (!doneQueue) {
			doneQueue = true;
			MetaType metaGround("ProtossGroundWeaponsLevel2", m_bot);
			MetaType metaArm("ProtossGroundArmorsLevel2", m_bot);
			BuildOrder buildOrder;
			MetaType metaStalker("Stalker", m_bot);

			buildOrder.add(metaStalker);
			buildOrder.add(metaGround);
			buildOrder.add(metaArm); 
			for (int i = 0; i < 10; ++i) {
				m_queue.queueAsLowestPriority(buildOrder[0], true);
			}
			m_queue.queueAsLowestPriority(buildOrder[1], true);
			for (int i = 0; i < 11; ++i) {
				m_queue.queueAsLowestPriority(buildOrder[0], true);
			}
			m_queue.queueAsLowestPriority(buildOrder[2], true);
		}
		else {
			BuildOrder buildOrder;
			MetaType metaStalker("Stalker", m_bot);

			buildOrder.add(metaStalker);
			m_queue.queueAsLowestPriority(buildOrder[0], true);
		}
	}
	// check the _queue for stuff we can build
	manageBuildOrderQueue();
    // TODO: detect if there's a build order deadlock once per second
    // TODO: triggers for game things like cloaked units etc

    m_buildingManager.onFrame();
    drawProductionInformation();
}

// on unit destroy
void ProductionManager::onUnitDestroy(const Unit & unit)
{
    // TODO: might have to re-do build order if a vital unit died
	std::cout << unit.getType().getName() << " died" << std::endl;
}

void ProductionManager::createNewBase() {
	BuildOrder buildOrder;
	MetaType metaNexus("Nexus", m_bot);
	MetaType metaProbe("Probe", m_bot);
	MetaType metaAssimilator("Assimilator", m_bot);
	MetaType metaStalker("Stalker", m_bot);
	buildOrder.add(metaNexus);
	m_queue.queueAsHighestPriority(buildOrder[0], true);
	buildOrder.add(metaAssimilator);
	m_queue.queueAsLowestPriority(buildOrder[1], false);
	int n = 7;
	for (int i = 2; i <= n; ++i) {
		buildOrder.add(metaProbe);
		buildOrder.add(metaStalker);
		m_queue.queueAsLowestPriority(buildOrder[i], false);
		++i;
		++n;
		m_queue.queueAsLowestPriority(buildOrder[i], true);
		if (i == n) {
			buildOrder.add(metaAssimilator);
			++i;
			m_queue.queueAsLowestPriority(buildOrder[i], false);
		}
	}
}
void ProductionManager::manageBuildOrderQueue()
{
    // if there is nothing in the queue, oh well
    if (m_queue.isEmpty())
    {
        return;
    }

    // the current item to be used
    BuildOrderItem & currentItem = m_queue.getHighestPriorityItem();

    // while there is still something left in the queue
    while (!m_queue.isEmpty())
    {
        // this is the unit which can produce the currentItem
		Unit producer = getProducer(currentItem.type);
		
        // check to see if we can make it right now
        bool canMake = canMakeNow(producer, currentItem.type);
		//std::cout << producer.getType().getName() << " check " << canMake << std::endl;
        // TODO: if it's a building and we can't make it yet, predict the worker movement to the location

        // if we can make the current item
        if (producer.isValid() && canMake)
        {
            // create it and remove it from the _queue
			//std::cout << producer.getType().getName() << " worked" << std::endl;
            create(producer, currentItem);
            m_queue.removeCurrentHighestPriorityItem();

            // don't actually loop around in here
            break;
        }
        // otherwise, if we can skip the current item
        else if (m_queue.canSkipItem())
        {
			//std::cout << producer.getType().getName() << " failed" << std::endl;
            // skip it
            m_queue.skipItem();

            // and get the next one
            currentItem = m_queue.getNextHighestPriorityItem();
        }
        else
        {
			//std::cout << producer.getType().getName() << " failed" << std::endl;
            // so break out
            break;
        }
    }
}

Unit ProductionManager::getProducer(const MetaType & type, CCPosition closestTo)
{
    // get all the types of units that cna build this type
    std::vector<UnitType> producerTypes = m_bot.Data(type).whatBuilds;
	if ((type.getName() == "Stalker")) {
		producerTypes.push_back(UnitType(sc2::UNIT_TYPEID::PROTOSS_WARPGATE, m_bot));
	}

    // make a set of all candidate producers
    std::vector<Unit> candidateProducers;
    for (auto unit : m_bot.UnitInfo().getUnits(Players::Self))
    {
        // reasons a unit can not train the desired type
        if (std::find(producerTypes.begin(), producerTypes.end(), unit.getType()) == producerTypes.end()) { continue; }
        if (!unit.isCompleted()) { continue; }
        if (m_bot.Data(unit).isBuilding && unit.isTraining()) { continue; }
        if (unit.isFlying()) { continue; }
		if (unit.getPlayer() != Players::Self) { continue; }
        // TODO: if unit is not powered continue
        // TODO: if the type is an addon, some special cases
        // TODO: if the type requires an addon and the producer doesn't have one
        // if we haven't cut it, add it to the set of candidates
        candidateProducers.push_back(unit);
    }

    return getClosestUnitToPosition(candidateProducers, closestTo);
}

Unit ProductionManager::getClosestUnitToPosition(const std::vector<Unit> & units, CCPosition closestTo)
{
    if (units.size() == 0)
    {
        return Unit();
    }

    // if we don't care where the unit is return the first one we have
    if (closestTo.x == 0 && closestTo.y == 0)
    {
        return units[0];
    }

    Unit closestUnit;
    double minDist = std::numeric_limits<double>::max();

    for (auto & unit : units)
    {
        double distance = Util::Dist(unit, closestTo);
        if (!closestUnit.isValid() || distance < minDist)
        {
            closestUnit = unit;
            minDist = distance;
        }
    }

    return closestUnit;
}

// this function will check to see if all preconditions are met and then create a unit
void ProductionManager::create(const Unit & producer, BuildOrderItem & item)
{
    if (!producer.isValid())
    {
        return;
    }
    // if we're dealing with a building
    // TODO: deal with morphed buildings & addons
	if (m_bot.Data(item.type).isBuilding)
	{
		// send the building task to the building manager
		m_buildingManager.addBuildingTask(item.type.getUnitType(), Util::GetTilePosition(m_bot.GetStartLocation()));
	}
    // if we're dealing with a non-building unit
    else if (item.type.isUnit())
    {
        producer.train(item.type.getUnitType());
    }
    else if (item.type.isUpgrade())
    {
        // TODO: UPGRADES
        //Micro::SmartAbility(producer, m_bot.Data(item.type.getUpgradeID()).buildAbility, m_bot);
		
		producer.upgrade(item.type.getUpgrade());
    }
}

bool ProductionManager::canMakeNow(const Unit & producer, const MetaType & type)
{
    if (!producer.isValid() || !meetsReservedResources(type))
    {
        return false;
    }
	
#ifdef SC2API
    sc2::AvailableAbilities available_abilities = m_bot.Query()->GetAbilitiesForUnit(producer.getUnitPtr());

    // quick check if the unit can't do anything it certainly can't build the thing we want
    if (available_abilities.abilities.empty())
    {
        return false;
    }
    else
    {
        // check to see if one of the unit's available abilities matches the build ability type
		sc2::AbilityID MetaTypeAbility = m_bot.Data(type).buildAbility;
		if (producer.getType().getAPIUnitType() == sc2::UNIT_TYPEID::PROTOSS_WARPGATE) {
			MetaTypeAbility = sc2::ABILITY_ID::TRAINWARP_STALKER;
		}
		if (MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL1 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL2 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL3) {
			MetaTypeAbility = sc2::ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONS;
		}
		else if (MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL1 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL2 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL3) {
			MetaTypeAbility = sc2::ABILITY_ID::RESEARCH_PROTOSSAIRARMOR;
		}
		else if (MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL2 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL3){
			MetaTypeAbility = sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS;
		}
		else if (MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL1 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL2 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL3) {
			MetaTypeAbility = sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR;
		}
		else if (MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL1 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL2 || MetaTypeAbility == sc2::ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL3) {
			MetaTypeAbility = sc2::ABILITY_ID::RESEARCH_PROTOSSSHIELDS;
		}

        for (const sc2::AvailableAbility & available_ability : available_abilities.abilities)
        {
            if (available_ability.ability_id == MetaTypeAbility)
            {
                return true;
            }
        }
    }

    return false;
#else
    bool canMake = meetsReservedResources(type);
	if (canMake)
	{
		if (type.isUnit())
		{
			canMake = BWAPI::Broodwar->canMake(type.getUnitType().getAPIUnitType(), producer.getUnitPtr());
		}
		else if (type.isTech())
		{
			canMake = BWAPI::Broodwar->canResearch(type.getTechType(), producer.getUnitPtr());
		}
		else if (type.isUpgrade())
		{
			canMake = BWAPI::Broodwar->canUpgrade(type.getUpgrade(), producer.getUnitPtr());
		}
		else
		{	
			BOT_ASSERT(false, "Unknown type");
		}
	}

	return canMake;
#endif
}

bool ProductionManager::detectBuildOrderDeadlock()
{
    // TODO: detect build order deadlocks here
    return false;
}

int ProductionManager::getFreeMinerals()
{
    return m_bot.GetMinerals() - m_buildingManager.getReservedMinerals();
}

int ProductionManager::getFreeGas()
{
    return m_bot.GetGas() - m_buildingManager.getReservedGas();
}

// return whether or not we meet resources, including building reserves
bool ProductionManager::meetsReservedResources(const MetaType & type)
{
    // return whether or not we meet the resources
    return (m_bot.Data(type).mineralCost <= getFreeMinerals()) && (m_bot.Data(type).gasCost <= getFreeGas());
}

void ProductionManager::drawProductionInformation()
{
    if (!m_bot.Config().DrawProductionInfo)
    {
        return;
    }

    std::stringstream ss;
    ss << "Production Information\n\n";

    for (auto & unit : m_bot.UnitInfo().getUnits(Players::Self))
    {
        if (unit.isBeingConstructed())
        {
            //ss << sc2::UnitTypeToName(unit.unit_type) << " " << unit.build_progress << "\n";
        }
    }

    ss << m_queue.getQueueInformation();

    m_bot.Map().drawTextScreen(0.01f, 0.01f, ss.str(), CCColor(255, 255, 0));
}
