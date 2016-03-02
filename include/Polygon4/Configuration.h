/*
 * Polygon-4 Engine
 * Copyright (C) 2015 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>
#include <string>
#include <set>

#include <Polygon4/DataManager/Types.h>

namespace polygon4
{

class DLL_EXPORT Configuration : public detail::Configuration
{
    using Base = detail::Configuration;

public:
    Configuration(const Base &);

    virtual void addObject(IObjectBase *) override final;

    virtual void addEquipment(detail::Equipment *) override final;
    virtual void addGlider(detail::Glider *) override final;
    virtual void addGood(detail::Good *) override final;
    virtual void addModificator(detail::Modificator *) override final;
    virtual void addProjectile(detail::Projectile *) override final;
    virtual void addWeapon(detail::Weapon *) override final;
};

} // namespace polygon4
