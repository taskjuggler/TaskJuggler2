/*
 * TrasactionList.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _TransactionList_h_
#define _TransactionList_h_

#include <time.h>

#include <qstring.h>
#include <qptrlist.h>

/**
 * @short Stores all transaction related information.
 * @author Chris Schlaeger <cs@suse.de>
 */
class Transaction
{
    friend class TransactionList;
public:
    Transaction(time_t d, double a, const QString& descr)
        : date(d), amount(a), description(descr) { }
    ~Transaction() { }

    time_t getDate() { return date; }
    double getAmount() { return amount; }
    const QString& getDescription() { return description; }

private:
    Transaction() { }   // dont use this
    /// The moment when the transaction happened.
    time_t date;
    /// The amount deposited or withdrawn.
    double amount;
    /// A short description of the transaction purpose
    QString description;
} ;

/**
 * @short A list of transactions.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TransactionList : public QPtrList<Transaction>
{
public:
    TransactionList() { }
    virtual ~TransactionList() { }
protected:
    virtual int compareItems(QCollection::Item i1, QCollection::Item i2);
} ;

/**
 * @short Iterator for TransactionList objects.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TransactionListIterator : public QPtrListIterator<Transaction>
{
public:
    TransactionListIterator(const TransactionList& t) :
        QPtrListIterator<Transaction>(t) {}
    virtual ~TransactionListIterator() { }
} ;

#endif

