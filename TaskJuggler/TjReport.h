/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TjReport_h_
#define _TjReport_h_

#include <qwidget.h>

class TjReport : public QWidget
{
public:
    TjReport(QWidget* p, const QString& n) :
        QWidget(p, n) { }
    virtual ~TjReport() { }

private:
    TjReport() { }
} ;

#endif

