/*
 * AccountList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "AccountList.h"
#include "Account.h"

Account*
AccountList::getAccount(const QString& id) const
{
    for (AccountListIterator ali(*this); *ali != 0; ++ali)
        if ((*ali)->getId() == id)
            return *ali;
    return 0;
}

bool
AccountList::isSupportedSortingCriteria(int sc)
{
    switch (sc)
    {
    case TreeMode:
        return TRUE;
    default:
        return CoreAttributesList::isSupportedSortingCriteria(sc);
    }
}

int
AccountList::compareItemsLevel(CoreAttributes* c1, CoreAttributes* c2,
                               int level)
{
    Account* a1 = static_cast<Account*>(c1);
    Account* a2 = static_cast<Account*>(c2);
    if (level > 2)
        return -1;

    switch (sorting[level])
    {
    case TreeMode:
    {
        /* Since we like to show all cost accounts first, we have add a
         * bit of extra code to the usual tree sort handling. */
        if (a1->getAcctType() == Cost &&
            a2->getAcctType() != Cost)
            return -1;
        if (a1->getAcctType() != Cost &&
            a2->getAcctType() == Cost)
            return 1;
        if (level == 0)
            return compareTreeItemsT(this, a1, a2);
        else
            return a1->getSequenceNo() == a2->getSequenceNo() ? 0 :
                a1->getSequenceNo() < a2->getSequenceNo() ? -1 : 1;
    }
    default:
        return CoreAttributesList::compareItemsLevel(a1, a2, level);
    }
}

