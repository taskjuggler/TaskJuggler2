/*
 * timedialog.h - TaskJuggler Viewer time dialog
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 * Copyright (c) 2004 by Lukas Tinkl <lukas.tinkl@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef TIMEDIALOG_H
#define TIMEDIALOG_H

#include <kdialogbase.h>
#include <kdatetimewidget.h>
#include <kdatewidget.h>

class QDateTime;
class QDate;

/**
 * @short TaskJuggler Gantt Viewer time dialog
 * @author Klaas Freitag <freitag@suse.de>
 */
class TimeDialog : public KDialogBase
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    TimeDialog( QWidget *parentWidget, const QDateTime & start, const QDateTime & end );

    /**
     * @return the selected start date and time
     */
    QDateTime getStartDate() const
        { return m_dateStart->dateTime(); }
    /**
     * @return the selected end date and time
     */
    QDateTime getEndDate() const
        { return m_dateEnd->dateTime(); }

private:
    KDateTimeWidget *m_dateStart;
    KDateTimeWidget *m_dateEnd;
};


/**
 * @short TaskJuggler Gantt Viewer date dialog
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class DateDialog: public KDialogBase
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    DateDialog( QWidget *parentWidget, const QDate & date );

    /**
     * @return the selected date
     */
    QDate getDate() const
        { return m_date->date(); }
private:
    KDateWidget *m_date;
};

#endif
