/*
 * CoreAttributes.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "CoreAttributes.h"
#include "CoreAttributesList.h"
#include "CustomAttributeDefinition.h"
#include "TextAttribute.h"
#include "ReferenceAttribute.h"

CoreAttributes::CoreAttributes(Project* p, const QString& i,
                               const QString& n,
                               CoreAttributes* parent_) :
        project(p), id(i), name(n), parent(parent_)
{
    index = -1;
    sub = new CoreAttributesList();
    if (parent_)
        parent_->sub->append(this);
}

CoreAttributes::~CoreAttributes()
{
    while (!sub->isEmpty())
        delete sub->getFirst();
    if (parent)
        parent->sub->removeRef(this);
    delete sub;
}

uint
CoreAttributes::treeLevel() const
{
    uint tl = 0;
    for (CoreAttributes* c = parent; c; c = c->parent)
        tl++;
    return tl;
}

CoreAttributesList
CoreAttributes::getSubList() const
{
   return *sub;
}

CoreAttributesListIterator
CoreAttributes::getSubListIterator() const
{
    return CoreAttributesListIterator(*sub);
}

bool
CoreAttributes::hasSubs() const
{ 
    return !sub->isEmpty(); 
}

void
CoreAttributes::setHierarchNo(uint no)
{
    hierarchNo = no;
    uint hNo = 1;
    for (CoreAttributesListIterator it(*sub); *it; ++it)
        (*it)->setHierarchNo(hNo++);
}

QString
CoreAttributes::getHierarchNo() const
{
    QString text;
    const CoreAttributes* ca = this;
    do
    {
        if (!text.isEmpty())
            text = "." + text;
        text = QString("%1").arg(ca->hierarchNo) + text;
        ca = ca->getParent();
    }
    while (ca);
    return text;
}

void
CoreAttributes::setHierarchIndex(uint no)
{
    if (no == 0)
    {
        hierarchIndex = 0;
        return;
    }
    /* If there is no parent, we take the passed number. */
    if (!parent)
    {
        hierarchIndex = no;
        return;
    }

    /* Find the highest hierarchIndex of all childs of this CAs parent. */
    uint max = 0;
    for (CoreAttributesListIterator it(*parent->sub); *it; ++it)
        if ((*it)->hierarchIndex > max)
            max = (*it)->hierarchIndex;

    /* The index is then the highest found + 1. */
    hierarchIndex = max + 1;
}

QString
CoreAttributes::getHierarchIndex() const
{
    QString text;
    const CoreAttributes* ca = this;
    do
    {
        if (!text.isEmpty())
            text = "." + text;
        text = QString("%1").arg(ca->hierarchIndex) + text;
        ca = ca->getParent();
    }
    while (ca);
    return text;
}

void
CoreAttributes::getFullName(QString& fullName) const
{
    fullName = QString::null;
    for (const CoreAttributes* c = this; c != 0; c = c->parent)
        fullName = c->name + "." + fullName;
    // Remove trailing dot.
    fullName.remove(fullName.length() - 1, 1);
}

QString
CoreAttributes::getFullId() const
{
    QString fullID = id;
    for (const CoreAttributes* c = parent; c != 0; c = c->parent)
        fullID = c->id + "." + fullID;
    return fullID;
}

bool
CoreAttributes::hasSameAncestor(const CoreAttributes* c) const
{
    if (c == 0)
        return FALSE;

    CoreAttributes const* p1;
    for (p1 = this; p1->parent; p1 = p1->parent)
        ;
    CoreAttributes const* p2;
    for (p2 = c; p2->parent; p2 = p2->parent)
        ;
    return p1 == p2;
}

bool
CoreAttributes::isDescendentOf(const CoreAttributes* c) const
{
    if (c == 0)
        return FALSE;

    for (CoreAttributes const* p = this; p; p = p->parent)
        if (p == c)
            return TRUE;

    return FALSE;
}

bool
CoreAttributes::isParentOf(const CoreAttributes* c) const
{
    if (!c)
        return FALSE;

    for (CoreAttributes const* p = c->parent; p; p = p->parent)
        if (p == this)
            return TRUE;

    return FALSE;
}

bool
CoreAttributes::isLeaf() const 
{ 
    return sub->isEmpty(); 
}

void
CoreAttributes::addCustomAttribute(const QString& id, CustomAttribute* ca)
{
    customAttributes.insert(id, ca);
}

const CustomAttribute*
CoreAttributes::getCustomAttribute(const QString& id) const
{
    return customAttributes[id];
}

void
CoreAttributes::inheritCustomAttributes
(const QDict<const CustomAttributeDefinition>& dict)
{
    QDictIterator<const CustomAttributeDefinition> cadi(dict);
    for ( ; cadi.current(); ++cadi)
    {
        const CustomAttribute* custAttr;
        if (cadi.current()->getInherit() &&
                (custAttr = parent->getCustomAttribute(cadi.currentKey())))
        {
            switch (custAttr->getType())
            {
            case CAT_Text:
                addCustomAttribute(cadi.currentKey(), new TextAttribute
                                   (*((TextAttribute*) custAttr)));
                break;
            case CAT_Reference:
                addCustomAttribute(cadi.currentKey(), new ReferenceAttribute
                                   (*((ReferenceAttribute*) custAttr)));
                break;
            default:
                qFatal("CoreAttributes::inheritCustomAttributes: "
                       "Unknown CAT %d", custAttr->getType());
                break;
            }
        }
    }
}

