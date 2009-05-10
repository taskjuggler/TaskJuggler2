/*
 * TableLineInfo.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TableLineInfo_h_
#define _TableLineInfo_h_

#include <qcolor.h>

class CoreAttributes;
class Task;
class Resource;
class Account;

class TableLineInfo
{
    friend class ReportElement;

public:
    TableLineInfo() :
        row(0),
        sc(0),
        ca1(0),
        ca2(0),
        task(0),
        resource(0),
        account(0),
        idxNo(0),
        bgCol(),
        boldText(false),
        fontFactor(100),
        specialName()
    { }

    ~TableLineInfo() { }

    int row;
    int sc;
    const CoreAttributes* ca1;
    const CoreAttributes* ca2;
    const Task* task;
    Resource* resource;
    const Account* account;
    uint idxNo;

    QColor bgCol;
    bool boldText;
    int fontFactor;
    QString specialName;
} ;

#endif

