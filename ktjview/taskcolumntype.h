/*
 * taskcolumntype.h - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _KTVTASKCOLTYPE_
#define _KTVTASKCOLTYPE_

#include <qobject.h>
#include <qstring.h>

#define COST     "Cost"
#define COMPLETE "Completed"
#define DEPEND   "Dependencies"
#define PRECEDES  "Precedes"
#define PREVIOUS  "Previous"
#define FOLLOWERS "Followers"

#define DURATION "Duration"
#define EFFORT    "Effort"
#define END       "End"
#define START     "Start"


typedef enum ColumnType { Int=0, String, Money, Resources, Tasks, Timespan, Time, TimeStamp };


class QString;

class TaskColumnType : public QObject
{
public:
    TaskColumnType( QString n, QString d, ColumnType t );

    QString getName() const { return m_name; }
    QString desc()    const { return m_desc; }
    ColumnType type() const { return m_type; }

    virtual QString toString(Task *t, int scen) const;

    void setVisible( bool b ) { m_show = b; }
    bool isVisible() {return m_show; }

    void setColumn(int c) { m_column = c; }
    int  getColumn() { return m_column; }

private:
    QString format (int) const;
    QString format( double ) const;
    QString format( TaskListIterator it ) const;
    QString format( time_t t ) const;
    ColumnType m_type;
    QString    m_name, m_desc;
    bool       m_show;
    int        m_column;
};



#endif
