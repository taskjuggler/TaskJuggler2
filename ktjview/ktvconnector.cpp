#include "ktvconnector.h"

#define MIN_DX 3   /* minimum x component of a connector */
/*
 * Connector
 */

KTVConnector::KTVConnector( QCanvas *c,
                           const QPoint& from,
                           const QPoint& to ):
   QCanvasPolygon(c)
{
   setPen(QPen(black,1));   // TODO configurable
   setBrush(QBrush(black));

   setZ( 1.0 );

   setConnectPoints( from, to );
}


void KTVConnector::setConnectPoints( const QPoint& from, const QPoint& to )
{

    int dx = to.x()-from.x();
    int dy = to.y()-from.y();

    qDebug("From X: %d, To X: %d", from.x(), to.x() );

    if( from.x() < (to.x()- MIN_DX) )
    {
    	/* Calculate the points into the array by the following sceme:
    	 *
    	 *
    	 *    0-------------o1
    	 *    8----------- -|-o7
    	 *                  | |
    	 *                  | |
    	 *                  | |
    	 *                  | |
    	 *              3  2| |6  5
    	 *               \--+ +--/
    	 *                \     /
    	 *                 \   /
    	 *                  \ /
    	 *                   o
    	 *                   4
    	 *
    	 **/

	     QPointArray p(9);

	dx = dx > MIN_DX ? dx : MIN_DX;

	p.setPoint(0, 0, 0 );
	p.setPoint(1, dx, 0);
	p.setPoint(2, dx, dy-m_wingY );
	p.setPoint(3, dx-m_wingX, dy-m_wingY );
	p.setPoint(4, dx, dy);
	p.setPoint(5, dx+m_wingX, dy-m_wingY );
	p.setPoint(6, dx+1, dy-m_wingY );
	p.setPoint(7, dx+1, 1);
	p.setPoint(8, 0, 1);

	setPoints(p);
    }
    else if( to.x() <= from.x()+MIN_DX )
    {
	QPointArray p(13);
	int turnY = dy-m_wingY-MIN_DX;

	p.setPoint(0,  0, 0);
	p.setPoint(1,  2*MIN_DX, 0 );
	p.setPoint(2,  2*MIN_DX, turnY );
	p.setPoint(3,  dx, turnY );
	p.setPoint(4,  dx, dy-m_wingY );
	p.setPoint(5,  dx+m_wingX, dy-m_wingY );
	p.setPoint(6,  dx, dy );
	p.setPoint(7,  dx-m_wingX, dy-m_wingY );
	p.setPoint(8,  dx-1, dy-m_wingY );
	p.setPoint(9,  dx-1, turnY-1 );
	p.setPoint(10, 2*MIN_DX-1, turnY-1 );
	p.setPoint(11, 2*MIN_DX-1, 1 );
	p.setPoint(12, 0, 1 );

	setPoints(p);
    } else {
	qDebug("Danger: Setting no points!");
    }

    move(from.x(), from.y());
}

QPoint KTVConnector::startPoint() const
{
   return QPoint(int(x()), int(y())) + points()[0];
}

QPoint KTVConnector::endPoint() const
{
   return QPoint(int(x()), int(y())) + points()[4];
}

