#include <klocale.h>

#include "ktvcanvasitem.h"

KTVCanvasItemBase::KTVCanvasItemBase()
   :QObject()
{
}


KTVCanvasItemTask::KTVCanvasItemTask( QCanvas *c )
{
   cRect = new QCanvasRectangle(c);

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


/* Milestone */
KTVCanvasItemMilestone::KTVCanvasItemMilestone( QCanvas *c )
{
   cCirc = new QCanvasEllipse(10, 10,c);

   cCirc->setBrush( blue );
}

void KTVCanvasItemMilestone::setSize( int , int h )
{
   if( cCirc )
      cCirc->setSize( h, h );
}

void KTVCanvasItemMilestone::move( double x, double y )
{
   if( cCirc )
      cCirc->move(x,y);
}

void KTVCanvasItemMilestone::moveBy( int dx, int dy)
{
   if( cCirc )
      cCirc->moveBy( dx, dy );
}

void KTVCanvasItemMilestone::hide()
{
   if( cCirc )
      cCirc->hide();
}

void KTVCanvasItemMilestone::show()
{
   if( cCirc )
      cCirc->show();
}

/* container */
KTVCanvasItemContainer::KTVCanvasItemContainer( QCanvas *c )
{
   cLine = new QCanvasLine(c);

   cLine->setPen( QColor(blue) );
}

void KTVCanvasItemContainer::setSize( int w, int h )
{
   if( !cLine ) return;

   cLine->setPoints( 0, 4, w, 4 );
}

void KTVCanvasItemContainer::move( double x, double y )
{
   if( cLine )
      cLine->move(x,y);
}

void KTVCanvasItemContainer::moveBy( int dx, int dy)
{
   if( cLine )
      cLine->moveBy( dx, dy );
}

void KTVCanvasItemContainer::hide()
{
   if( cLine )
      cLine->hide();
}

void KTVCanvasItemContainer::show()
{
   if( cLine )
      cLine->show();
}
