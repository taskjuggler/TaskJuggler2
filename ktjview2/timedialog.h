/*
 * timedialog.h - TaskJuggler Viewer time Dialog
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
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

/**
 *
 * @short TaskJuggler Gantt Viewer Header Widget
 * @author Klaas Freitag <freitag@suse.de>
 * @version 0.1
 */
class QDateTime;

class TimeDialog : public KDialogBase
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    TimeDialog( QWidget *parentWidget, const QDateTime & start, const QDateTime & end );

    QDateTime getStartDate() const
        { return m_dateStart->dateTime(); }
    QDateTime getEndDate() const
        { return m_dateEnd->dateTime(); }

private:
    KDateTimeWidget *m_dateStart;
    KDateTimeWidget *m_dateEnd;
};

#endif // KTJVIEWPART_H
