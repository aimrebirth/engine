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

#include <Polygon4/Mechanoid.h>

#include <Polygon4/Engine.h>
#include <Polygon4/Configuration.h>

#include "Executor.h"
#include "Script.h"

#include <tools/Logger.h>
DECLARE_STATIC_LOGGER(logger, "mechanoid");

namespace polygon4
{

Mechanoid::Mechanoid(const Base &rhs)
    : Base(rhs)
{
}

detail::Configuration *Mechanoid::getConfiguration()
{
    if (!configuration)
    {
        configuration = getStorage()->configurations.createAtEnd(*initial_configuration);
        configuration->deepCopyFrom(*initial_configuration);
    }
    replace<Configuration>(configuration.get());
    return configuration;
}

void Mechanoid::enterBuilding(detail::MapBuilding *building)
{
    if (!building)
        return;
    auto mmb = building->getModificationMapBuilding();
    if (!mmb)
        return;
    if (!isPlayer())
    {
        // handle bot's building visit
        return;
    }

    // player is now officially in the building
    // we need to run scripts, setup texts etc.

    auto se = mmb->map->modification->getScriptEngine();
    path script_file = path("maps") / mmb->map->script_dir.toString() / mmb->script_name.toString();
    auto s = se->getScript(script_file.string());

    ScriptData data;
    data.scriptEngine = se;
    data.player = player;
    data.building_name = mmb->text_id;
    s->OnEnterBuilding(data);

    //mmb->map->;
    //set menu texts etc.

    // end of function
    getEngine()->SetBuildingMenuCurrentBuilding(mmb);
    getEngine()->ShowBuildingMenu();

    // do async save to not freeze the game
    // the ideal algorithm here is:
    // 1. freeze the game
    // 2. async({ save(); unfreeze(); });
    getTaskExecutor().push([]()
    {
        getEngine()->saveAuto();
    });
}

} // namespace polygon4
