/*
 * ktvtasktableitem.h - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _KTVTASKTABLEITEM_H
#define _KTVTASKTABLEITEM_H


#include <klistview.h>
#include <Project.h>
#include <Task.h>

class KTVTaskTable;

class KTVTaskTableItem: public KListViewItem
{
public:
    KTVTaskTableItem( KTVTaskTable *parent, int height=20 );
    KTVTaskTableItem( KTVTaskTableItem *parent, Task*, int height=20 );
   int compare( QListViewItem *i, int col, bool ascending ) const;

   Task *getTask() { return m_task; }

    void setup() {} // required to reimplement here to make setHeight working!
private:
   int compareNumeric( long other, long me );
   int compareTimes( time_t other, time_t me );
   Task *m_task;
};


#endif

