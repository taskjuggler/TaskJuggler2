#include <klocale.h>
#include <qfont.h>
#include "ktvcanvasitem.h"

KTVCanvasItemBase::KTVCanvasItemBase()
   :QObject()
{
    // Required to have reloading to work properly, but ATTENTION here:
    // ALWAYS leave one of the two lists with autodelete = false, because
    // one Item is put into both lists. Thus, deleting from both causes a
    // sigfault.
    m_conIn.setAutoDelete(true);
    m_conOut.setAutoDelete(false);
}

KTVCanvasItemBase::~KTVCanvasItemBase()
{
    m_conIn.clear();
    m_conOut.clear();
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

void KTVCanvasItemBase::moveBy( int, int )
{

}

void KTVCanvasItemBase::moveConnectorsBy( int dx, int dy )
{
   moveInConnectorsBy( dx, dy );
   moveOutConnectorsBy( dx, dy );
}

void KTVCanvasItemBase::moveInConnectorsBy( int dx, int dy )
{
   KTVConnectorListIterator it( m_conIn );
   KTVConnector *c = 0;

   // qDebug( "Moving In connectors by %d, %d!", dx, dy );
   while( (c = it.current()) != 0 )
   {
      ++it;
      /* Preserve the starting point */
      qDebug("This is c: %p", c );
      QPoint ps = c->startPoint();
      // qDebug( "Connector start point: %d, %d!", ps.x(), ps.y() );
      QPoint pe = c->endPoint();
      // qDebug( "Connector end  point: %d, %d!", pe.x(), pe.y() );
      pe.setX( pe.x()+dx );
      pe.setY( pe.y()+dy );
      // qDebug( "In connector end-Point is %dx%d", pe.x(), pe.y());
      c->setConnectPoints( ps, pe );
   }
}

void KTVCanvasItemBase::moveOutConnectorsBy( int dx, int dy )
{

   KTVConnectorListIterator it2( m_conOut );
   KTVConnector *c = 0;

   // qDebug( "Moving Out connectors by %d, %d!", dx, dy );
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

/* move in- and out-connectors to absolute position */
void KTVCanvasItemBase::moveConnectors( double x, double y )
{
   moveInConnectors(x,y);
   moveOutConnectors(x,y);
}
void KTVCanvasItemBase::moveInConnectors( double x, double y )
{
   KTVConnectorListIterator it( m_conIn );
   KTVConnector *c = 0;

   while( (c = it.current()) != 0 )
   {
      ++it;
      /* Preserve the starting point */
      QPoint ps = c->startPoint();
      QPoint pe = c->endPoint();
      pe.setX( int(x) );
      pe.setY( int(y) );
      c->setConnectPoints( ps, pe );
   }
}

void KTVCanvasItemBase::moveOutConnectors( double x, double y )
{

   KTVConnectorListIterator it2( m_conOut );
   KTVConnector *c = 0;

   while( (c = it2.current()) != 0 )
   {
      ++it2;
      QPoint ps = c->startPoint();
      QPoint pe = c->endPoint();
      ps.setX( int(x) );
      ps.setY( int(y) );
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
    :KTVCanvasItemBase(),
     m_cmplLine(0),
     m_TaskTextXOffset(10)
{
    /* Rectangle that shows the task */
    cRect = new QCanvasRectangle(c);
    m_height = 16;
    cRect->setBrush( QColor(0xDB, 0xE8, 0x4F));

    /* Line that shows the completeness */
    m_cmplLine = new QCanvasLine(c);
    m_cmplLine->hide();
    QPen pen( QColor(0xe6, 0x79, 0x00), 2 );
    // green: QColor(0x00, 0x7d, 0x00)
    // orange: QColor(0xe6, 0x79, 0x00);
    m_cmplLine->setPen(pen);

    /* Text */
    m_cText = new QCanvasText(c);
    m_cText->setFont( KTVCanvasItemBase::smFont );
    m_cText->hide();

    m_cText->setZ( 12.0 );
    m_cmplLine->setZ(11.0);
    cRect->setZ( 10.0 );
}

KTVCanvasItemTask::~KTVCanvasItemTask()
{
    delete cRect;
    delete m_cText;
    delete m_cmplLine;
}

void KTVCanvasItemTask::setComplete(double percent)
{
    if( percent > 0 && percent <= 100 )
    {
        int startX = x()+1;
        int pcent  = int(cRect->width() * percent/100.0) -1;
        int ty = y()+2;
        m_cmplLine->setPoints( startX, ty, startX+pcent, ty );
        m_cmplLine->show();
    }
    else
    {
        m_cmplLine->hide();
    }
}


void KTVCanvasItemTask::setSize( int w, int h )
{
   if( cRect )
   {
      int dx = w-cRect->width();
      /* TODO: Height adjusting */

      /* Move the outgoing connectors */
      moveOutConnectorsBy( dx, 0 );

      /* And now resize the box */
      cRect->setSize( w, h );

      if( m_task )
          setComplete( m_task->getComplete(0));
   }
   m_height = h;
}

void KTVCanvasItemTask::move( double x, double y )
{
    moveConnectors( x, y );

    if( cRect )
        cRect->move(x,y);
    if( m_cmplLine )
        m_cmplLine->move(x,y);
    if( m_cText )
    {
        QFontMetrics fm(m_cText->font());
        int fHeight=fm.height();

        int yOff = ( m_height-fHeight )/2;
        qDebug("The Y-Offset is %d", yOff );
        m_cText->move( x+m_TaskTextXOffset, y+yOff );
    }

}

void KTVCanvasItemTask::moveBy( int dx, int dy)
{
    // qDebug( "1. Moving in CanvasItemTask by %d, %d!", dx, dy );
   moveConnectorsBy( dx, dy );


    if( m_cmplLine )
        m_cmplLine->moveBy(dx,dy);
   if( cRect )
      cRect->moveBy( dx, dy );
   if( m_cText )
      m_cText->moveBy( dx, dy );
}

void KTVCanvasItemTask::hide()
{
   KTVCanvasItemBase::hide();
   if( m_cmplLine )
       m_cmplLine->hide();
   if( cRect )
      cRect->hide();
   if( m_cText )
      m_cText->hide();
}

void KTVCanvasItemTask::show()
{
   if( m_cmplLine )
       m_cmplLine->show();
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
      p.setY( r.y()); // +( r.height()/2));
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

KTVCanvasItemMilestone::~KTVCanvasItemMilestone()
{
    delete cPoly;

}

void KTVCanvasItemMilestone::setSize( int , int h )
{
   if(! cPoly ) return;

   int oldW = cPoly->boundingRect().width();

   QPointArray p(4);

   m_height = h;

   int diag=m_height/2;

   p.setPoint( 0, 0, 0 );
   p.setPoint( 1, diag, diag );
   p.setPoint( 2, 0, 2*diag );
   p.setPoint( 3, -1*diag, diag);

   cPoly->setPoints( p );

   /* move connectors, width and height are identical for milestones */
   moveOutConnectors( (m_height - oldW)/2, 0 );
}

void KTVCanvasItemMilestone::move( double x, double y )
{
   if( cPoly )
      cPoly->move(x,y);
   moveConnectors( x, y );
}

void KTVCanvasItemMilestone::moveBy( int dx, int dy)
{
   moveConnectorsBy( dx, dy );
   if( cPoly )
      cPoly->moveBy( dx, dy );
}

void KTVCanvasItemMilestone::hide()
{
   KTVCanvasItemBase::hide();
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

KTVCanvasItemContainer::~KTVCanvasItemContainer( )
{
    delete cPoly;
}

void KTVCanvasItemContainer::setSize( int w, int h  )
{
   QPointArray p(8);

   // w = w < h ? h : w;

   m_height = h;

   if( !cPoly ) return;

   int dx = w- (cPoly->boundingRect()).width();
   if( dx != 0 )
       moveOutConnectors( dx, 0 );

   p.setPoint(0, 0, 0);
   p.setPoint(1, 0, h);
   p.setPoint(2, h/2, h/2 );
   p.setPoint(3, h/2, h/2-2);

   p.setPoint(4, w-h/2, h/2-2);
   p.setPoint(5, w-h/2, h/2);
   p.setPoint(6, w, h );
   p.setPoint(7, w, 0 );

   cPoly->setPoints( p );
}

void KTVCanvasItemContainer::move( double x, double y )
{
   if( cPoly )
      cPoly->move(x,y);
   moveConnectors( x, y);
}

void KTVCanvasItemContainer::moveBy( int dx, int dy)
{
   moveConnectorsBy( dx, dy);

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

QFont KTVCanvasItemBase::smFont = QFont();
