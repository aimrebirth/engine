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

#include <Polygon4/BuildingMenu.h>

#include <algorithm>

#include <Polygon4/Engine.h>

namespace polygon4
{

InfoTreeItem::InfoTreeItem(const detail::IObjectBase *o)
{
    assign(o);
}

InfoTreeItem *InfoTreeItem::findChild(const detail::IObjectBase *o)
{
    auto i = std::find_if(children.begin(), children.end(),
        [o](const auto &e) {return e->object == o; });
    if (i != children.end())
        return i->get();
    return nullptr;
}

InfoTreeItem &InfoTreeItem::operator=(const detail::IObjectBase *o)
{
    assign(o);
    return *this;
}

void InfoTreeItem::assign(const detail::IObjectBase *o)
{
    if (!o)
        return;
    text = o->getName();
    object = (detail::IObjectBase *)o;
    switch (o->getType())
    {
    case detail::EObjectType::Message:
        text = ((detail::Message*)o)->title->string;
        break;
    }
}

BuildingMenu::BuildingMenu()
{
    auto &messages = getEngine()->getMessages();

#define SET_CHILD(v, e, m) *(v.children[InfoTreeItem::e]) = messages[#m]

    for (auto i = 0; i < InfoTreeItem::ThemesMax; i++)
        themes.children.emplace_back(std::make_shared<InfoTreeItem>());
    SET_CHILD(themes, ThemesId, INT_THEMES);

    for (auto i = 0; i < InfoTreeItem::JournalMax; i++)
        journal.children.emplace_back(std::make_shared<InfoTreeItem>());
    SET_CHILD(journal, JournalInProgress, INT_QUESTS_ACTIVE);
    SET_CHILD(journal, JournalCompleted, INT_QUESTS_COMPLETED);
    SET_CHILD(journal, JournalFailed, INT_QUESTS_FAILED);
    SET_CHILD(journal, JournalId, INT_JOURNAL);

    for (auto i = 0; i < InfoTreeItem::GliderMax; i++)
        glider.children.emplace_back(std::make_shared<InfoTreeItem>());
    SET_CHILD(glider, GliderGeneral, INT_PMENU_GLIDER_INFO);
    SET_CHILD(glider, GliderId, INT_PMENU_GLIDER_GLIDER);
    SET_CHILD(glider, GliderArmor, INT_PMENU_GLIDER_ARMOR);
    SET_CHILD(glider, GliderWeapons, INT_PMENU_GLIDER_WEAPONS);
    SET_CHILD(glider, GliderEquipment, INT_PMENU_GLIDER_EQUIPMENT);
    SET_CHILD(glider, GliderAmmo, INT_PMENU_GLIDER_AMMO);

    for (auto i = 0; i < InfoTreeItem::GliderStoreMax; i++)
        glider_store.children.emplace_back(std::make_shared<InfoTreeItem>());
    SET_CHILD(glider_store, GliderStoreId, INT_BASE_GLIDERS);
    SET_CHILD(glider_store, GliderStoreEquipment, INT_BASE_EQUIPMENT);
    SET_CHILD(glider_store, GliderStoreWeapons, INT_BASE_WEAPONS);
    SET_CHILD(glider_store, GliderStoreAmmo, INT_BASE_AMMO);
}

BuildingMenu::~BuildingMenu()
{
}

void BuildingMenu::SetCurrentBuilding(detail::ModificationMapBuilding *b)
{
    if (!b)
        return;
    building = b;
    themes.children[InfoTreeItem::ThemesId]->children.clear();
}

void BuildingMenu::update()
{
    updateJournal();
    updateGlider();
    updateGliderStore();
}

void BuildingMenu::updateJournal()
{
    auto p = mechanoid->getPlayer();
    if (!p)
        return;

    std::sort(p->records.begin(), p->records.end(),
        [](const auto &r1, const auto &r2)
    {
        if (r1->type == detail::JournalRecordType::InProgress &&
            r2->type == detail::JournalRecordType::Completed)
            return true;
        if (r2->type == detail::JournalRecordType::InProgress &&
            r1->type == detail::JournalRecordType::Completed)
            return false;
        return r1->time > r2->time;
    });

    journal.children[InfoTreeItem::JournalId]->children.clear();
    for (auto &r : p->records)
    {
        auto c = std::make_shared<InfoTreeItem>();
        c->object = r;
        journal.children[InfoTreeItem::JournalId]->children.push_back(c);
    }
}

void BuildingMenu::updateGlider()
{
    auto c = mechanoid->configuration;

    glider.children[InfoTreeItem::GliderId]->children.clear();
    glider.children[InfoTreeItem::GliderId]->children.push_back(std::make_shared<InfoTreeItem>(c->glider));

    glider.children[InfoTreeItem::GliderWeapons]->children.clear();
    for (auto &w : c->weapons)
    {
        glider.children[InfoTreeItem::GliderWeapons]->children.push_back(std::make_shared<InfoTreeItem>(w));
    }

    glider.children[InfoTreeItem::GliderArmor]->children.clear();
    glider.children[InfoTreeItem::GliderEquipment]->children.clear();
    for (auto &e : c->equipments)
    {
        switch (e->equipment->type)
        {
        case detail::EquipmentType::Armor:
        case detail::EquipmentType::Generator:
            glider.children[InfoTreeItem::GliderArmor]->children.push_back(std::make_shared<InfoTreeItem>(e));
            break;
        case detail::EquipmentType::Reactor:
        case detail::EquipmentType::Engine:
            glider.children[InfoTreeItem::GliderId]->children.push_back(std::make_shared<InfoTreeItem>(e));
            break;
        default:
            glider.children[InfoTreeItem::GliderEquipment]->children.push_back(std::make_shared<InfoTreeItem>(e));
            break;
        }
    }

    glider.children[InfoTreeItem::GliderAmmo]->children.clear();
    for (auto &p : c->projectiles)
    {
        glider.children[InfoTreeItem::GliderAmmo]->children.push_back(std::make_shared<InfoTreeItem>(p));
    }

    // maybe sort
    // armor: armor, generator
    // glider: glider, reactor, engine,
    // weapons: light, heavy
    // equ: ?
    // ammo: ?
}

void BuildingMenu::updateGliderStore()
{
    glider_store.children[InfoTreeItem::GliderStoreId]->children.clear();
    for (auto &g : building->gliders)
    {
        glider_store.children[InfoTreeItem::GliderStoreId]->children.push_back(std::make_shared<InfoTreeItem>(g));
    }

    glider_store.children[InfoTreeItem::GliderStoreWeapons]->children.clear();
    for (auto &w : building->weapons)
    {
        glider_store.children[InfoTreeItem::GliderStoreWeapons]->children.push_back(std::make_shared<InfoTreeItem>(w));
    }

    glider_store.children[InfoTreeItem::GliderStoreEquipment]->children.clear();
    for (auto &e : building->equipments)
    {
        glider_store.children[InfoTreeItem::GliderStoreEquipment]->children.push_back(std::make_shared<InfoTreeItem>(e));
    }

    glider_store.children[InfoTreeItem::GliderStoreAmmo]->children.clear();
    for (auto &p : building->projectiles)
    {
        glider_store.children[InfoTreeItem::GliderStoreAmmo]->children.push_back(std::make_shared<InfoTreeItem>(p));
    }

    // maybe sort
    // armor: armor, generator
    // glider: glider, reactor, engine,
    // weapons: light, heavy
    // equ: ?
    // ammo: ?
}

void BuildingMenu::addTheme(const detail::Message *m)
{
    auto c = themes.findChild(m);
    if (c)
        return;
    themes.children[InfoTreeItem::ThemesId]->children.push_back(std::make_shared<InfoTreeItem>(m));
    addMessage(m);
}

void BuildingMenu::addMessage(const detail::Message *m)
{
    if (!text.empty())
        text += "\n\n";
    printMessage(m);
}

void BuildingMenu::showMessage(const detail::Message *m)
{
    text.clear();
    printMessage(m);
}

void BuildingMenu::printMessage(const detail::Message *m)
{
    printText(m->title->string);
    printText(m->txt->string);
}

void BuildingMenu::addText(const String &t)
{
    if (!text.empty())
        text += "\n\n";
    printText(t);
}

void BuildingMenu::addText(const String &ti, const String &t)
{
    if (!text.empty())
        text += "\n\n";
    printText(ti);
    printText(t);
}

void BuildingMenu::showText(const String &t)
{
    text.clear();
    printText(t);
}

void BuildingMenu::showText(const String &ti, const String &t)
{
    text.clear();
    printText(ti);
    printText(t);
}

void BuildingMenu::clearText()
{
    text.clear();
}

void BuildingMenu::printText(const String &t)
{
    // TODO: format here!
    // maybe boost format
    text += t;
    text += "\n";
}

} // namespace polygon4
