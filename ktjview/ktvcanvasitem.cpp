#include <klocale.h>
#include <qfont.h>
#include "ktvcanvasitem.h"

KTVCanvasItemBase::KTVCanvasItemBase()
   :QObject(),
    m_cText(0)
   
{
}


void KTVCanvasItemBase::addConnectIn( KTVConnector *cIn, Task *t )
{
   if( cIn && t )
      m_conIn.insert( t, cIn );
}

void KTVCanvasItemBase::addConnectOut( KTVConnector *cOut, Task *t )
{
   if( cOut && t )
      m_conOut.insert( t, cOut );
}

void KTVCanvasItemBase::hide()
{
   KTVConnectorListIterator it( m_conIn );
   KTVConnector *c = 0;

   qDebug( "Hiding connectors!" );
   while( (c = it.current()) != 0 )
   {
      ++it;
      c->hide();
   }

   KTVConnectorListIterator it2( m_conOut );
   while( (c = it2.current()) != 0 )
   {
      ++it2;
      c->hide();
   }
}

void KTVCanvasItemBase::setSize( int, int )
{
   
}

void KTVCanvasItemBase::moveBy( int dx, int dy)
{
   KTVConnectorListIterator it( m_conIn );
   KTVConnector *c = 0;

   qDebug( "Moving connectors!" );
   while( (c = it.current()) != 0 )
   {
      ++it;
      /* Preserve the starting point */
      QPoint ps = c->startPoint();
      QPoint pe = c->endPoint();
      pe.setX( pe.x()+dx );
      pe.setY( pe.y()+dy );
      c->setConnectPoints( ps, pe );
   }

   KTVConnectorListIterator it2( m_conOut );
   while( (c = it2.current()) != 0 )
   {
      ++it2;
      QPoint ps = c->startPoint();
      QPoint pe = c->endPoint();
      ps.setX( ps.x()+dx );
      ps.setY( ps.y()+dy );
      c->setConnectPoints( ps, pe );
   }
}


/*
 * returns the connector coming in from Task t
 */
KTVConnector* KTVCanvasItemBase::connectorIn( Task *t )
{
   return m_conIn[ (void*) t ];
}

/*
 * Returns a connector going out to task t
 */
KTVConnector* KTVCanvasItemBase::connectorOut( Task *t )
{
   return m_conOut[ (void*) t ];
}


/* **********************************************************************
 * Task
 */

KTVCanvasItemTask::KTVCanvasItemTask( QCanvas *c )
   :KTVCanvasItemBase()
{
   cRect = new QCanvasRectangle(c);
   m_height = 12;
   cRect->setBrush( QColor(0xDB, 0xE8, 0x4F));

   /* Text */
   m_cText = new QCanvasText(c);
   m_cText->hide();

   m_cText->setZ( 12.0 );
   cRect->setZ( 10.0 );
}

void KTVCanvasItemTask::setSize( int w, int h )
{
   if( cRect )
   {
      int dx = w-cRect->width();
      /* TODO: Height adjusting */
      
      /* Move the outgoing connectors */
      KTVConnectorListIterator conIt( m_conOut );
      KTVConnector *c;
      while( (c = conIt.current()) != 0 )
      {
	 ++conIt;
	 QPoint ps = c->startPoint();
	 
	 ps.setX( ps.x()+dx );
	 // ps.setY( ps.y()+dy );
	 c->setConnectPoints( ps, c->endPoint() );
      }

      /* And now resize the box */
      cRect->setSize( w, h );
   }

}

void KTVCanvasItemTask::move( double x, double y )
{
   if( cRect )
      cRect->move(x,y);
   if( m_cText )
      m_cText->move( x+10, y );
}

void KTVCanvasItemTask::moveBy( int dx, int dy)
{
   qDebug( "Moving in CanvasItemTask!" );
   KTVCanvasItemBase::moveBy( dx, dy );

   
   if( cRect )
      cRect->moveBy( dx, dy );
   if( m_cText )
      m_cText->moveBy( dx, dy );
}

void KTVCanvasItemTask::hide()
{
   KTVCanvasItemBase::hide();
   if( cRect )
      cRect->hide();
   if( m_cText )
      m_cText->hide();
}

void KTVCanvasItemTask::show()
{
   if( cRect )
      cRect->show();
   if( m_cText )
      m_cText->show();
}

bool KTVCanvasItemTask::contains( QCanvasItem* ci )
{
   return( cRect == ci );
}

void KTVCanvasItemTask::setTask( Task *t )
{
   m_task = t;
   if( m_cText )
   {
      qDebug("### This is the Tasks Name: " + t->getName() );
      m_cText->setText( t->getName());
      m_cText->show();
   }
   else
   {
      qDebug("### No text defined!" );
   }
}

QPoint KTVCanvasItemTask::getConnectorIn() const
{
   QPoint p;
   
   if( cRect )
   {
      QRect r = cRect->rect();
      p.setX( r.x());
      p.setY( r.y()+( r.height()/2));
   }
   return p;
}

QPoint KTVCanvasItemTask::getConnectorOut() const
{
   QPoint p;
   
   if( cRect )
   {
      QRect r = cRect->rect();
      p.setX( r.right());
      p.setY( r.y()+( r.height()/2));
   }
   return p;
}

int KTVCanvasItemTask::y()
{
   return ( int( cRect->y() ));
}

int KTVCanvasItemTask::x()
{
   return ( int( cRect->x() ));
}

/* **********************************************************************
 * Milestone
 */
KTVCanvasItemMilestone::KTVCanvasItemMilestone( QCanvas *c )
   :KTVCanvasItemBase()
{
   cPoly = new QCanvasPolygon(c);

   cPoly->setBrush( blue );
   cPoly->setZ( 10.0 );
   
   setSize( 14, 14 );
}

void KTVCanvasItemMilestone::setSize( int , int h )
{
   if(! cPoly ) return;

   QPointArray p(4);

   m_height = h;
   
   int diag=m_height/2;

   p.setPoint( 0, 0, 0 );
   p.setPoint( 1, diag, diag );
   p.setPoint( 2, 0, 2*diag );
   p.setPoint( 3, -1*diag, diag);

   cPoly->setPoints( p );

}

void KTVCanvasItemMilestone::move( double x, double y )
{
   if( cPoly )
      cPoly->move(x,y);
}

void KTVCanvasItemMilestone::moveBy( int dx, int dy)
{
   KTVCanvasItemBase::moveBy( dx, dy );
   if( cPoly )
      cPoly->moveBy( dx, dy );
}

void KTVCanvasItemMilestone::hide()
{
   if( cPoly )
      cPoly->hide();
}

void KTVCanvasItemMilestone::show()
{
   if( cPoly )
      cPoly->show();
}

bool KTVCanvasItemMilestone::contains( QCanvasItem* ci )
{
   return( cPoly == ci );
}

int KTVCanvasItemMilestone::y()
{
   if( ! cPoly ) return 0;

   return ( int(cPoly->y()));
}

int KTVCanvasItemMilestone::x()
{
   if( ! cPoly ) return 0;

   return ( int(cPoly->x()));
}

QPoint KTVCanvasItemMilestone::getConnectorIn() const
{
   QPoint p;
   
   if( cPoly )
   {
      QRect r = cPoly->boundingRect();
      p.setX( r.x());
      p.setY( r.y()+( r.height()/2));
   }
   return p;
}

QPoint KTVCanvasItemMilestone::getConnectorOut() const
{
   QPoint p;
   
   if( cPoly )
   {
      QRect r = cPoly->boundingRect();
      p.setX( r.right());
      p.setY( r.y()+( r.height()/2));
   }
   return p;
}


/* **********************************************************************
 * container
 */
KTVCanvasItemContainer::KTVCanvasItemContainer( QCanvas *c )
   :KTVCanvasItemBase()
{
   cPoly = new QCanvasPolygon(c);
   m_height = 16;
   setSize( 0, m_height );
   cPoly->setBrush( blue );
   cPoly->setPen( QColor(blue) );
   cPoly->setZ( 10.0 );
}

void KTVCanvasItemContainer::setSize( int w, int h  )
{
   QPointArray p(8);

   w = w < h ? h : w;
   
   if( !cPoly ) return;

   p.setPoint(0, 0, 0);
   p.setPoint(1, 0, h);
   p.setPoint(2, h/2, h/2 );
   p.setPoint(3, h/2, h/2-2);
   
   p.setPoint(4, w-h/2, h/2-2);
   p.setPoint(5, w-h/2, h/2);
   p.setPoint(6, w, h );
   p.setPoint(7, w, 0 );
#if 0
   /* This is like
    *  +-----~-----+
    *  |+----~----+|
    *  ||         ||
    *  \/         \/
    * Do not forget to enhance the point-array to 10 !
    */  
   p.setPoint( 0, 0, 0 );
   p.setPoint( 1, 0, h/2 );
   p.setPoint( 2, h/2, h );
   p.setPoint( 3, h, h/2 );
   p.setPoint( 4, h, h/2-2 );

   p.setPoint( 5, w-h, h/2-2 );
   p.setPoint( 6, w-h, h/2 );
   p.setPoint( 7, w-h/2, h );
   p.setPoint( 8, w, h/2 );
   p.setPoint( 9, w, 0 );
#endif
   
   cPoly->setPoints( p );
}

void KTVCanvasItemContainer::move( double x, double y )
{
   if( cPoly )
      cPoly->move(x,y);
}

void KTVCanvasItemContainer::moveBy( int dx, int dy)
{
   if( cPoly )
      cPoly->moveBy( dx, dy );
}

void KTVCanvasItemContainer::hide()
{
   if( cPoly )
      cPoly->hide();
}

void KTVCanvasItemContainer::show()
{
   if( cPoly )
      cPoly->show();
}

bool KTVCanvasItemContainer::contains( QCanvasItem* ci )
{
   return( cPoly == ci );
}

int KTVCanvasItemContainer::y()
{
   if( ! cPoly ) return 0;

   return ( int(cPoly->y()));
}


int KTVCanvasItemContainer::x()
{
   if( ! cPoly ) return 0;

   return ( int(cPoly->x()));
}

QPoint KTVCanvasItemContainer::getConnectorIn() const
{
   QPoint p;
   
   if( cPoly )
   {
      QRect r = cPoly->boundingRect();
      
      p.setX( r.x());
      p.setY( r.y());
   }
   return p;
}

QPoint KTVCanvasItemContainer::getConnectorOut() const
{
   QPoint p;
   
   if( cPoly )
   {
      QRect r = cPoly->boundingRect();
      
      p.setX( r.right());
      p.setY( r.y()+( r.height()));
   }
   return p;
}

