/*
 * AccountList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
AccountList::compareItemsLevel(Account* a1, Account* a2, int level)
{
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

int
AccountList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
    Account* a1 = static_cast<Account*>(i1);
    Account* a2 = static_cast<Account*>(i2);

    int res;
    for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
        if ((res = compareItemsLevel(a1, a2, i)) != 0)
            return res;
    return res;
}

