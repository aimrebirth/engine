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

#include "ScriptAPI.h"

#include <chrono>

#include <Polygon4/Actions.h>
#include <Polygon4/BuildingMenu.h>
#include <Polygon4/Engine.h>
#include <Polygon4/Mechanoid.h>
#include <Polygon4/Modification.h>

#include "Script.h"

#include <tools/Logger.h>
DECLARE_STATIC_LOGGER(logger, "script_api");

namespace polygon4
{

namespace script
{

String &getScreenText()
{
    auto bm = getEngine()->getBuildingMenu();
    return bm->getText();
}

polygon4::detail::Message *get_message_by_id(const std::string &message_id)
{
    auto &v = getEngine()->getMessages();
    auto i = v.find(message_id);
    if (i == v.end())
    {
        LOG_ERROR(logger, "Message '" << message_id << "' was not found");
        return nullptr;
    }
    return (polygon4::detail::Message*)i->second;
}

void ScriptData::AddItem(const std::string &oname, int quantity)
{
    LOG_TRACE(logger, "AddItem(" << oname << ")");

    auto &objs = getEngine()->getItems();
    auto i = objs.find(oname);
    if (i == objs.end())
    {
        LOG_ERROR(logger, "Item '" << oname << "' was not found");
        return;
    }
    auto o = i->second;
    auto conf = player->mechanoid->getConfiguration();
    conf->addItem(o, quantity);
    BM_TEXT_ADD_ITEM(o, quantity);
}

void ScriptData::AddMoney(float amount)
{
    LOG_TRACE(logger, "AddMoney(" << amount << ")");

    SetMoney(GetMoney() + amount);
    BM_TEXT_ADD_MONEY(amount);
}

bool ScriptData::HasMoney(float amount) const
{
    LOG_TRACE(logger, "HasMoney(" << amount << ")");

    return GetMoney() >= amount;
}

float ScriptData::GetMoney() const
{
    return player->mechanoid->getMoney();
}

void ScriptData::SetMoney(float m)
{
    return player->mechanoid->setMoney(m);
}

void ScriptData::AddRating(float amount, RatingType type)
{
    LOG_TRACE(logger, "AddRating(" << amount << "), type: " << static_cast<int>(type));

    SetRating(GetRating(type) + amount);
}

bool ScriptData::HasRating(float amount, RatingType type) const
{
    LOG_TRACE(logger, "HasRating(" << amount << "), type: " << static_cast<int>(type));

    return GetRating(type) >= amount;
}

float ScriptData::GetRating(RatingType type) const
{
    LOG_TRACE(logger, "GetRating(), type: " << static_cast<int>(type));

    return player->mechanoid->getRating((polygon4::detail::RatingType)type);
}

void ScriptData::SetRating(float amount, RatingType type)
{
    LOG_TRACE(logger, "SetRating(), type: " << static_cast<int>(type));

    return player->mechanoid->setRating(amount, (polygon4::detail::RatingType)type);
}

int ScriptData::GetRatingLevel(RatingType type) const
{
    LOG_TRACE(logger, "GetRatingLevel(), type: " << static_cast<int>(type));

    return player->mechanoid->getRatingLevel((polygon4::detail::RatingType)type);
}

bool ScriptData::HasRatingLevel(int level, RatingType type) const
{
    LOG_TRACE(logger, "HasRatingLevel(" << level << "), type: " << static_cast<int>(type));

    return GetRatingLevel(type) >= level;
}

void ScriptData::SetRatingLevel(int level, RatingType type) const
{
    LOG_TRACE(logger, "SetRatingLevel(" << level << "), type: " << static_cast<int>(type));

    player->mechanoid->setRatingLevel(level, (polygon4::detail::RatingType)type);
}

void ScriptData::AddJournalRecord(const std::string &message_id, JournalRecord type)
{
    LOG_TRACE(logger, "AddJournalRecord(" << message_id << ")");

    auto m = get_message_by_id(message_id);
    if (!m)
        return;

    if (player->records.count(message_id) != 0)
    {
        auto &r = player->records[message_id];
        r->type = (detail::JournalRecordType)type;
        return;
    }

    auto s = player->getStorage();
    auto r = s->addJournalRecord(player);
    r->text_id = m->text_id;
    r->message = m;
    r->type = (detail::JournalRecordType)type;
    r->time = getEngine()->getSettings().playtime;
    player->records.insert_to_data(r);

    GET_BUILDING_MENU()->JournalRecordAdded();
}

void ScriptData::SetJournalRecordCompleted(const std::string &message_id)
{
    LOG_TRACE(logger, "SetJournalRecordCompleted(" << message_id << ")");

    auto m = get_message_by_id(message_id);
    if (!m)
        return;

    if (player->records.count(message_id) != 0)
    {
        auto &r = player->records[message_id];
        r->type = detail::JournalRecordType::Completed;
        return;
    }

    auto s = player->getStorage();
    auto r = s->addJournalRecord(player);
    r->text_id = m->text_id;
    r->message = m;
    r->type = detail::JournalRecordType::Completed;
    r->time = getEngine()->getSettings().playtime;
    player->records.insert_to_data(r);
}

int ScriptData::GetVar(const std::string &var)
{
    LOG_TRACE(logger, "GetVar(" << var << ")");

    auto v = std::find_if(player->variables.begin(), player->variables.end(), [&var](const auto &v)
    {
        return v->key == var;
    });
    if (v != player->variables.end())
    {
        LOG_TRACE(logger, "GetVar(val = " << (*v)->value_int << ")");
        return (*v)->value_int;
    }
    LOG_TRACE(logger, "GetVar(val = " << 0 << ")");
    return 0;
}

void ScriptData::SetVar(const std::string &var, int i)
{
    LOG_TRACE(logger, "SetVar(" << var << ", val = " << i << ")");

    auto v = std::find_if(player->variables.begin(), player->variables.end(), [&var](const auto &v)
    {
        return v->key == var;
    });
    if (v != player->variables.end())
    {
        (*v)->value_int = i;
        return;
    }

    auto sv = GET_STORAGE()->scriptVariables.createAtEnd();
    sv->player = player;
    sv->key = var;
    sv->value_int = i;
    player->variables.insert(sv);
}

void ScriptData::SetVar(const std::string &var, const std::string &val)
{
    LOG_TRACE(logger, "SetVar(" << var << ", val = " << val << ")");

    auto v = std::find_if(player->variables.begin(), player->variables.end(), [&var](const auto &v)
    {
        return v->key == var;
    });
    if (v != player->variables.end())
    {
        (*v)->value_text = val;
        return;
    }

    auto sv = GET_STORAGE()->scriptVariables.createAtEnd();
    sv->player = player;
    sv->key = var;
    sv->value_text = val;
    player->variables.insert(sv);
}

void ScriptData::UnsetVar(const std::string &var)
{
    LOG_TRACE(logger, "UnsetVar(" << var << ")");

    auto v = std::find_if(player->variables.begin(), player->variables.end(), [&var](const auto &v)
    {
        return v->key == var;
    });
    if (v == player->variables.end())
        return;
    player->variables.erase(v);
}

bool ScriptData::CheckVar(const std::string &var)
{
    LOG_TRACE(logger, "CheckVar(" << var << ")");

    auto v = std::find_if(player->variables.begin(), player->variables.end(), [&var](const auto &v)
    {
        return v->key == var;
    });
    if (v != player->variables.end())
    {
        LOG_TRACE(logger, "CheckVar(true)");
        return true;
    }
    LOG_TRACE(logger, "CheckVar(false)");
    return false;
}

bool ScriptData::RunOnce(const std::string &var)
{
    LOG_TRACE(logger, "RunOnce(" << var << ")");

    if (!CheckVar(var))
    {
        SetVar(var);
        return true;
    }
    return false;
}

std::string ScriptData::GetName() const
{
    if (player->mechanoid->name)
        return player->mechanoid->name->string.str().toString();
    return "";
}

void ScriptData::SetName(const std::string &name) const
{
    player->mechanoid->setName(name);
}

void AddText(const std::string &text)
{
    LOG_TRACE(logger, "AddText(" << text << ")");

    auto &t = getScreenText();
    t += String(text);
}

void ShowText(const std::string &text)
{
    LOG_TRACE(logger, "ShowText(" << text << ")");

    auto &t = getScreenText();
    t.clear();
    t += String(text);
}

void ClearText()
{
    LOG_TRACE(logger, "ClearText()");

    auto &t = getScreenText();
    t.clear();
}

void ClearThemes()
{
    LOG_TRACE(logger, "ClearThemes()");

    GET_BUILDING_MENU()->clearThemes();
}

void Clear()
{
    LOG_TRACE(logger, "Clear()");

    ClearText();
    ClearThemes();
}

static void AddMessage(const std::string &message_id, bool clear)
{
    if (clear)
        ClearText();
    GET_BUILDING_MENU()->addTheme(get_message_by_id(message_id));
}

void AddTheme(const std::string &message_id)
{
    LOG_TRACE(logger, "AddTheme(" << message_id << ")");

    AddMessage(message_id, false);
}

void AddMessage(const std::string &message_id)
{
    LOG_TRACE(logger, "AddMessage(" << message_id << ")");

    GET_BUILDING_MENU()->addMessage(get_message_by_id(message_id));
}

void ShowMessage(const std::string &message_id)
{
    LOG_TRACE(logger, "ShowMessage(" << message_id << ")");

    AddMessage(message_id, true);
}

ScreenText GetScreenText()
{
    LOG_TRACE(logger, "GetScreenText()");

    ScreenText text;
    text.screenText = &getScreenText();
    return text;
}

ScreenText ScreenText::operator+(const std::string &s)
{
    LOG_TRACE(logger, "ScreenText::operator+(" << s << ")");

    if (screenText)
        getScreenText() += String(s);
    return *this;
}

ScreenText ScreenText::__concat__(const std::string &s)
{
    return ScreenText::operator+(s);
}

void Log(const std::string &text)
{
    LOG_DEBUG("script_log", text);
}

} // namespace script

} // namespace polygon4
