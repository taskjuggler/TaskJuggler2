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
   virtual void hide(){};
private:
   Task *m_task;
};

class KTVCanvasItemTask: public KTVCanvasItemBase
{
public:
   KTVCanvasItemTask( QCanvas* );

   void setSize( int, int );
   void move(double, double );
   void moveBy( int, int );
   void hide();
   void show();
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
private:
   QCanvasEllipse *cCirc;
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
private:
   QCanvasLine    *cLine;
};
#endif

