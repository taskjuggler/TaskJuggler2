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
#include <time.h>
#include <kdialogbase.h>
#include <kdatewidget.h>

/**
 *
 * @short TaskJuggler Gantt Viewer Header Widget
 * @author Klaas Freitag <freitag@suse.de>
 * @version 0.1
 */
class KDateWidget;

class TimeDialog : public KDialogBase
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    TimeDialog( QWidget *parentWidget, time_t start, time_t end  );

    QDate getStartDate() const
        { return m_dateStart->date(); }
    QDate getEndDate() const
        { return m_dateEnd->date(); }

private:
    KDateWidget *m_dateStart;
    KDateWidget *m_dateEnd;
};

#endif // KTJVIEWPART_H
