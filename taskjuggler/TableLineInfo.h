/*
 * TableLineInfo.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TableLineInfo_h_
#define _TableLineInfo_h_

class CoreAttributes;
class Task;
class Resource;

class TableLineInfo
{
    friend class ReportElement;

public:
    TableLineInfo(const CoreAttributes* c1, const CoreAttributes* c2, 
                  const Task* t, const Resource* r, uint n, int s) :
        ca1(c1), ca2(c2), task(t), resource(r), no(n), sc(s) { }
    ~TableLineInfo() { }

    const CoreAttributes* const ca1;
    const CoreAttributes* const ca2;
    const Task* const task;
    const Resource* const resource;
    const uint no;
    const int sc;

private:
    TableLineInfo() { }
} ;

#endif

