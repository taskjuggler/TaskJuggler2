/*
 * Account.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Account_h_
#define _Account_h_

#include "AccountList.h"
#include "TransactionList.h"

class Interval;

/**
 * @short Stores all account related information.
 * @author Chris Schlaeger <cs@kde.org>
 */
class Account : public CoreAttributes
{
public:
    Account(Project* p, const QString& i, const QString& n, Account* pr,
            AccountType at, const QString& df = QString::null, uint dl = 0);
    virtual ~Account();

    virtual CAType getType() const { return CA_Account; }

    Account* getParent() const { return static_cast<Account*>(parent); }

    AccountListIterator getSubListIterator() const
    {
        return AccountListIterator(*sub);
    }

    void inheritValues();

    void setAcctType(AccountType at) { acctType = at; }
    AccountType getAcctType() const { return acctType; }

    void credit(Transaction* t);
    bool isGroup() const { return !sub->isEmpty(); }

    double getBalance(int sc, time_t d) const;
    double getVolume(int sc, const Interval& period) const;

private:
    TransactionList transactions;
    AccountType acctType;
} ;

#endif
