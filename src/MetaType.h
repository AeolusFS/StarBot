#pragma once

#include "Common.h"
#include "UnitType.h"

namespace MetaTypes
{
    enum { Unit, Upgrade, Buff, Tech, Ability, None };
}

class CCBot;
class MetaType
{
    size_t          m_type;
    std::string     m_name;
    CCRace          m_race;
    UnitType        m_unitType;
    CCUpgrade       m_upgrade;

#ifndef SC2API
    BWAPI::TechType m_tech;
#endif
    
public:

    MetaType();
    MetaType(const std::string & name, CCBot & bot);
    MetaType(const UnitType & unitType);
    MetaType(const CCUpgrade & upgradeType);

    bool    isUnit()        const;
    bool    isUpgrade()     const;
    bool    isTech()        const;

    const size_t &          getMetaType()  const;
    const std::string &     getName()       const;
    const CCRace &          getRace()       const;
    const UnitType &        getUnitType() const;
    const CCUpgrade &       getUpgrade()  const;

#ifndef SC2API
    MetaType(const BWAPI::TechType & tech);
    const BWAPI::TechType & getTechType() const;
#endif
};

