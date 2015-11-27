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

#include <Polygon4/Modification.h>

#include <Polygon4/DataManager/Types.h>
#include <Polygon4/Engine.h>
#include <Polygon4/Settings.h>

#include "Script.h"

#include <tools/Logger.h>
DECLARE_STATIC_LOGGER(logger, "mods");

namespace polygon4
{

Modification::Modification(const Base &rhs)
    : Base(rhs)
{
}

bool Modification::newGame()
{
    if (directory.empty())
    {
        LOG_ERROR(logger, "Game directory is not set!");
        return false;
    }
    if (script_language.empty())
    {
        LOG_ERROR(logger, "Script language is not set!");
        return false;
    }
    if (script_main.empty())
    {
        LOG_ERROR(logger, "Main script name is not set!");
        return false;
    }

    const auto &path = getSettings().modsDir;
    try
    {
        auto script_name = getScriptName(path + directory, script_main);
        auto script = Script::createScript(script_name, script_language);

        auto &pmap = player_mechanoid->map;
        auto i = std::find_if(maps.begin(), maps.end(), [&pmap](const auto &map)
        {
            return map->map == pmap;
        });
        if (i == maps.end())
        {
            LOG_ERROR(logger, "Cannot find map: " << pmap->resource.toString());
            return false;
        }

        if (!pmap->loadLevel())
            return false;
        current_map = pmap;

        gEngine->HideMainMenu();
        gEngine->LoadLevelObjects = [this, pmap]()
        {
            pmap->loadObjects();
            spawnMechanoids();
        };

        // action will proceed on UE4 side
    }
    catch (std::exception &e)
    {
        LOG_ERROR(logger, "Cannot start game: " << e.what());
        return false;
    }
    return true;
}

bool Modification::loadGame(const String &filename)
{
    return false;
}

bool Modification::operator<(const Modification &rhs) const
{
    return directory < rhs.directory;
}

void Modification::spawnMechanoids()
{
    std::unordered_map<detail::Mechanoid *, detail::Player *> mplayers;
    for (auto &p : players)
    {
        if (p->mechanoid)
            mplayers[p->mechanoid] = p.second.get();
        // for now we allow only one local player
        break;
    }
    for (auto &m : mechanoids)
    {
        if (m->map == *current_map)
            m->spawn(mplayers.count(m.second.get()));
    }
}

} // namespace polygon4
