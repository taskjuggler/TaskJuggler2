#include <klocale.h>

#include "ktvcanvasitem.h"

KTVCanvasItemBase::KTVCanvasItemBase()
   :QObject()
{
}

/*
 *
 */


KTVCanvasItemTask::KTVCanvasItemTask( QCanvas *c )
{
   cRect = new QCanvasRectangle(c);
   m_height = 12;
   cRect->setBrush( red );
}

void KTVCanvasItemTask::setSize( int w, int h )
{
   if( cRect )
      cRect->setSize( w, h );
}

void KTVCanvasItemTask::move( double x, double y )
{
   if( cRect )
      cRect->move(x,y);
}

void KTVCanvasItemTask::moveBy( int dx, int dy)
{
   if( cRect )
      cRect->moveBy( dx, dy );
}

void KTVCanvasItemTask::hide()
{
   if( cRect )
      cRect->hide();
}

void KTVCanvasItemTask::show()
{
   if( cRect )
      cRect->show();
}

bool KTVCanvasItemTask::contains( QCanvasItem* ci )
{
   return( cRect == ci );
}


/* Milestone */
KTVCanvasItemMilestone::KTVCanvasItemMilestone( QCanvas *c )
{
   cPoly = new QCanvasPolygon(c);

   cPoly->setBrush( blue );
   
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

/* container */
KTVCanvasItemContainer::KTVCanvasItemContainer( QCanvas *c )
{
   cPoly = new QCanvasPolygon(c);
   m_height = 16;
   setSize( 0, m_height );
   cPoly->setBrush( blue );
   cPoly->setPen( QColor(blue) );
}

void KTVCanvasItemContainer::setSize( int w, int h  )
{
   QPointArray p(10);

   w = w < h ? h : w;
   
   if( !cPoly ) return;

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
