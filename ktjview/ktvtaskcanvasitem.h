#ifndef _KTVTASKCANVASITEM_H
#define _KTVTASKCANVASITEM_H


#include <qcanvas.h>
#include <Project.h>
#include <Task.h>

class KTVTaskCanvas;

class KTVTaskCanvasItem: public QCanvasItem
{
public:
   KTVTaskCanvasItem( QCanvas *parent = 0);
   Task *getTask() { return m_task; }


private:
   Task *m_task;
};


#endif

