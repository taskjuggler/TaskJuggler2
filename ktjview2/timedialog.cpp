/*
 * timedialog.h - TaskJuggler Viewer time dialog
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "timedialog.h"

#include <qhbox.h>
#include <qvbox.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <qlabel.h>
#include <qdatetime.h>


TimeDialog::TimeDialog( QWidget *parentWidget, const QDateTime & start, const QDateTime & end )
    : KDialogBase( parentWidget, "TimeDialog", true,
                   i18n("Setup Time Frame"),
                   Ok|Cancel, Ok )
{
    QVBox *vb = makeVBoxMainWidget();

    QHBox *hb = new QHBox( vb );

    (void) new QLabel( i18n("Start Time: "), hb );
    m_dateStart = new KDateTimeWidget( start, hb );

    hb = new QHBox( vb );
    (void) new QLabel( i18n("End Time: "), hb );
    m_dateEnd = new KDateTimeWidget( end, hb );
}

#include "timedialog.moc"
