/*
 * Account.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Account.h"
#include "Interval.h"
#include "Task.h"
#include "Project.h"
#include "CustomAttributeDefinition.h"

Account::Account(Project* p, const QString& i, const QString& n, Account* pr,
                 AccountType at) : CoreAttributes(p, i, n, pr), acctType(at)
{
    p->addAccount(this);
    kotrusId = "";
}

Account::~Account()
{
    project->deleteAccount(this);
}

void
Account::inheritValues()
{
    if (parent)
    {
        // Inherit inheritable custom attributes
        inheritCustomAttributes(project->getAccountAttributeDict());
    }
}

double
Account::getVolume(int sc, const Interval& period) const
{
    double volume = 0.0;
    // Add plan credits for all tasks that should be credited to this account.
    for (TaskListIterator tli(project->getTaskListIterator()); *tli != 0; ++tli)
        if ((*tli)->getAccount() == this)
            volume += (*tli)->getCredits(sc, period, acctType, 0, FALSE);

    // Add all transactions that are registered within the period.
    for (TransactionListIterator tli(transactions); *tli != 0; ++tli)
        if (period.contains((*tli)->getDate()))
            volume += (*tli)->getAmount();
    
    // Add volume of all sub-accounts.
    for (AccountListIterator ali(*sub); *ali != 0; ++ali)
        volume += (*ali)->getVolume(sc, period);

    return volume;
}

double
Account::getBalance(int /*sc*/, time_t /* date */) const
{
    return 0.0;
}

void 
Account::credit(Transaction* t)
{
    transactions.inSort(t);
}
