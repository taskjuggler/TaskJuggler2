/*
 * ktvconnector.h - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _KTVCONNECTOR_H
#define _KTVCONNECTOR_H

#include <qcanvas.h>
#include <qptrlist.h>
#include <qptrdict.h>
#include <qpoint.h>

class KTVConnector: public QCanvasPolygon
{
public:
    KTVConnector( QCanvas *,
                  const QPoint& from = QPoint(),
                  const QPoint& to   = QPoint() );

    void setConnectPoints( const QPoint&, const QPoint& );

    QPoint startPoint() const;
    QPoint endPoint() const;

private:
    static const int m_wingX = 4;
    static const int m_wingY = 8;
};


/*
 * List of Pointers to Connectors
 */
class KTVConnectorList: public QPtrDict<KTVConnector> // store connectors with Tasks as keys.
{
public:
    KTVConnectorList() {}
    // ~KTVConnectorList() {}
};

typedef QPtrDictIterator<KTVConnector> KTVConnectorListIterator;



#endif
