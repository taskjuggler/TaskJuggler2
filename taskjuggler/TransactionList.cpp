/*
 * TransactionList.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#include "TransactionList.h"

int
TransactionList::compareItems(QCollection::Item i1, QCollection::Item i2)
{
    Transaction* t1 = static_cast<Transaction*>(i1);
    Transaction* t2 = static_cast<Transaction*>(i2);

    if (t1->date == t2->date)
        return 0;
    else
        return t2->date - t1->date;
}

