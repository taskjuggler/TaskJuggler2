#ifndef _KTVCANVASITEM_H
#define _KTVCANVASITEM_H


#include <qcanvas.h>
#include <Project.h>
#include <Task.h>



class KTVCanvasItemBase: public QObject
{
public:
   KTVCanvasItemBase( );
   void  setTask( Task *t ){ m_task = t; }
   Task* getTask()         { return m_task; }

   virtual void setSize( int, int ){}
   virtual void move( double, double ){}
   virtual void moveBy( int, int ){}
   virtual void show(){}
   virtual void hide(){}
   virtual bool contains( QCanvasItem* ){ return false; }
   virtual int  height( ) const { return m_height; }
   
   static void slSetRowHeight( int _h ) { m_rowHeight = _h; }
   
protected:
   Task      *m_task;
   int        m_height;
   static int m_rowHeight;
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
private:
   QCanvasRectangle *cRect;
};

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
private:
   QCanvasPolygon *cPoly;
   
};

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
private:
   QCanvasPolygon    *cPoly;
};
#endif

