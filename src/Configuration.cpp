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

#include <Polygon4/Configuration.h>

#include <Polygon4/Engine.h>

#include <tools/Logger.h>
DECLARE_STATIC_LOGGER(logger, "configuration");

namespace polygon4
{

Configuration::Configuration(const Base &rhs)
    : Base(rhs)
{
}

void Configuration::addObject(IObjectBase *o)
{
    using polygon4::detail::EObjectType;

    switch (o->getType())
    {
    case EObjectType::Equipment:
        addEquipment((detail::Equipment *)o);
        break;
    case EObjectType::Glider:
        addGlider((detail::Glider *)o);
        break;
    case EObjectType::Weapon:
        addWeapon((detail::Weapon *)o);
        break;
    case EObjectType::Modificator:
        addModificator((detail::Modificator *)o);
        break;
    case EObjectType::Good:
        addGood((detail::Good *)o);
        break;
    case EObjectType::Projectile:
        addProjectile((detail::Projectile *)o);
        break;
    default:
        return;
    }
}

void Configuration::addEquipment(detail::Equipment *o)
{
    auto i = std::find_if(equipments.begin(), equipments.end(),
        [o](const auto &e) { return e->equipment.get() == o; });
    if (i != equipments.end())
    {
        auto e = *i;
        if (e->quantity == e->equipment->max_count)
            return;
        e->quantity++;
        return;
    }
    auto s = getStorage();
    auto v = s->configurationEquipments.createAtEnd();
    v->configuration = this;
    v->equipment = o;
    equipments.push_back(v);
}

void Configuration::addGlider(detail::Glider *o)
{
    glider = o;
}

void Configuration::addGood(detail::Good *o)
{
    auto i = std::find_if(goods.begin(), goods.end(),
        [o](const auto &e) { return e->good.get() == o; });
    if (i != goods.end())
    {
        auto e = *i;
        if (e->quantity == e->good->max_count)
            return;
        e->quantity++;
        return;
    }
    auto s = getStorage();
    auto v = s->configurationGoods.createAtEnd();
    v->configuration = this;
    v->good = o;
    goods.push_back(v);
}

void Configuration::addModificator(detail::Modificator *o)
{
    auto i = std::find_if(modificators.begin(), modificators.end(),
        [o](const auto &e) { return e->modificator.get() == o; });
    if (i != modificators.end())
    {
        auto e = *i;
        if (e->quantity == e->modificator->max_count)
            return;
        e->quantity++;
        return;
    }
    auto s = getStorage();
    auto v = s->configurationModificators.createAtEnd();
    v->configuration = this;
    v->modificator = o;
    modificators.push_back(v);
}

void Configuration::addProjectile(detail::Projectile *o)
{
    auto s = getStorage();
    auto v = s->configurationProjectiles.createAtEnd();
    v->configuration = this;
    v->projectile = o;
    projectiles.push_back(v);
}

void Configuration::addWeapon(detail::Weapon *o)
{
    auto s = getStorage();
    auto v = s->configurationWeapons.createAtEnd();
    v->configuration = this;
    v->weapon = o;
    weapons.push_back(v);
}

} // namespace polygon4