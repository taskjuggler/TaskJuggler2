/*
 * AccountTreeIterator.h - AccountJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _AccountTreeIterator_h_
#define _AccountTreeIterator_h_

#include "CoreAttributesTreeIterator.h"

class AccountTreeIterator : public virtual CoreAttributesTreeIterator
{
public:
    AccountTreeIterator(Account* r, IterationMode m = leavesOnly)
        : CoreAttributesTreeIterator(r, m) { }
    virtual ~AccountTreeIterator() { }

    Account* operator*() { return static_cast<Account*>(current); }
    Account* operator++()
    {
        return static_cast<Account*>(CoreAttributesTreeIterator::operator++());
    }
} ;

#endif

