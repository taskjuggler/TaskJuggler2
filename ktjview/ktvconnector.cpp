#include "ktvconnector.h"

/*
 * Connector
 */

KTVConnector::KTVConnector( QCanvas *c,
			   const QPoint& from,
			   const QPoint& to ):
   QCanvasLine(c)
{
   setPen(QPen(black,1));   // TODO configurable
   setPoints( from.x(), from.y(), to.x(), to.y());
   setZ( 1.0 );
}


void KTVConnector::setConnectPoints( const QPoint& from, const QPoint& to )
{
   setPoints( from.x(), from.y(), to.x(), to.y());
}

