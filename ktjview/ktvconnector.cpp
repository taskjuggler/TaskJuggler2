#include "ktvconnector.h"

/*
 * Connector
 */ 

KTVConnector::KTVConnector( QCanvas *c, 
			   const QPoint& from,
			   const QPoint& to ):
   QCanvasLine(c)
{
   QPointArray pa(2);

   setPen(QPen(black,2));
#if 0
   qDebug("Connecting from %d/%d to %d/%d", from.x(), from.y(),
	  to.x(), to.y() );
   pa.setPoint( 0, from );
   pa.setPoint( 1, to );
   
   setPoints( pa );
#endif
   setPoints( from.x(), from.y(), to.x(), to.y()); 
}


void KTVConnector::setConnectPoints( const QPoint& from, const QPoint& to )
{
   setPoints( from.x(), from.y(), to.x(), to.y()); 
}

