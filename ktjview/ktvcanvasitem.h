#ifndef _KTVCANVASITEM_H
#define _KTVCANVASITEM_H

#include <qpoint.h>
#include <qcanvas.h>
#include <Project.h>
#include <Task.h>

#include "ktvconnector.h"

class QFont;

/*
 * The base. Contains most of the functionality all others need.
 */ 
class KTVCanvasItemBase: public QObject
{
public:
   KTVCanvasItemBase( );
   void  setTask( Task *t ){ m_task = t; }
   Task* getTask()         { return m_task; }

   void setFont( const QFont& f)
      { if( m_cText ) m_cText->setFont( f ); }

   virtual int  y(){ return 0; }
   virtual void setSize( int, int ){}
   virtual void move( double, double ){}
   virtual void moveBy( int, int );
   virtual void show(){}
   virtual void hide();
   virtual bool contains( QCanvasItem* ){ return false; }
   virtual int  height( ) const { return m_height; }

   void addConnectIn( KTVConnector*, Task* );
   void addConnectOut( KTVConnector*, Task* );
   KTVConnector* connectorIn( Task* );
   KTVConnector* connectorOut( Task* );
   
   virtual QPoint getConnectorIn() const {return QPoint(); }
   virtual QPoint getConnectorOut() const {return QPoint(); }

   int               m_height;
protected:
   Task             *m_task;
   QCanvasText      *m_cText;

   
private:
   /* Connectorlists */
   KTVConnectorList  m_conIn;
   KTVConnectorList  m_conOut;
};

/*
 * List of Pointers to Canvas Items 
 */
class CanvasItemList: public QPtrList<KTVCanvasItemBase>
{
public:
   CanvasItemList() {}
   ~CanvasItemList() {}
};

typedef QPtrListIterator<KTVCanvasItemBase> CanvasItemListIterator;

/*
 * Task Item
 */
class KTVCanvasItemTask: public KTVCanvasItemBase
{
public:
   KTVCanvasItemTask( QCanvas* );

   void setSize( int, int );
   void move(double, double );
   void moveBy( int, int );
   void hide();
   void show();
   bool contains( QCanvasItem* );

   void setTask( Task* );

   int y();
   
   QPoint getConnectorIn() const;
   QPoint getConnectorOut() const;
private:
   QCanvasRectangle *cRect;
};

/*
 * Milestone-Item
 */
class KTVCanvasItemMilestone: public KTVCanvasItemBase
{
public:
   KTVCanvasItemMilestone( QCanvas* );

   void setSize( int, int );
   void move(double, double );
   void moveBy( int, int );
   void hide();
   void show();
   bool contains( QCanvasItem* );
   int y();
private:
   QCanvasPolygon *cPoly;
   
};

/*
 * Container-Item
 */
class  KTVCanvasItemContainer: public KTVCanvasItemBase
{
public:
   KTVCanvasItemContainer( QCanvas* );

   void setSize( int, int );
   void move(double, double );
   void moveBy( int, int );
   void hide();
   void show();
   bool contains( QCanvasItem* );
   int y();
private:
   QCanvasPolygon    *cPoly;
};

#endif
