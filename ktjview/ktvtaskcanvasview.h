#ifndef _KTVTASKCANVASVIEW_H
#define _KTVTASKCANVASVIEW_H


#include <klistview.h>
#include <Project.h>
#include <Task.h>
#include <qcanvas.h>

#include <qpixmap.h>

#include "ktvtaskcanvas.h"

class KTVTaskTable;
class KTVTaskCanvasItem;


class KTVTaskCanvasView: public QCanvasView
{
    Q_OBJECT
public:
    KTVTaskCanvasView( QWidget *parent, KTVTaskTable *tab=0, const char *name = 0 );
    virtual ~KTVTaskCanvasView(){ }
   void showProject( Project * );
   void contentsMousePressEvent( QMouseEvent* e );   
private:
   void addTask( Task *t );
   Project *m_pro;

   KTVTaskCanvas *m_canvas;
};


#endif

