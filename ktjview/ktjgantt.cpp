#include "ktjgantt.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kaction.h>
#include <qsplitter.h>
#include <klocale.h>

#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>

#include "ktvtasktable.h"
#include "ktvtaskcanvasview.h"


KTJGantt::KTJGantt( QWidget *parentWidget, const char *)
   : QSplitter( parentWidget )
{


}

void KTJGantt::showProject( Project *p )
{
    // we need an instance
    m_table = new KTVTaskTable( this, "TABLE");
    m_canvas = new KTVTaskCanvasView( this, m_table, "CANVAS");

    /* synchron y scroll */
    connect( m_table, SIGNAL(scrolledBy(int,int)),
	     m_canvas, SLOT(scrollBy( int, int ) ));
    
    connect( m_table, SIGNAL(itemHeightChanged(int)),
	     m_canvas->canvas(), SLOT  (slSetRowHeight(int) ));

    connect( m_table, SIGNAL(heightChanged(int)),
	     m_canvas->canvas(), SLOT  (slHeightChanged(int) ));

    connect( m_table, SIGNAL(newTaskAdded(Task *, KTVTaskTableItem *)),
	     m_canvas->canvas(), SLOT(slNewTask(Task *, KTVTaskTableItem *) ));

    connect( m_table, SIGNAL(showTaskByItem(KTVTaskTableItem*)),
    	     m_canvas->canvas(), SLOT(slShowTask(KTVTaskTableItem*)) );

    connect( m_table, SIGNAL(hideTaskByItem(KTVTaskTableItem*)),
    	     m_canvas->canvas(), SLOT(slHideTask(KTVTaskTableItem*)) );

    connect( m_table, SIGNAL(moveMarker(int)),
    	     m_canvas->canvas(), SLOT(slShowMarker(int)) );

    connect( m_table, SIGNAL( needUpdate() ),
    	     m_canvas->canvas(), SLOT(update()) );

    connect( m_table, SIGNAL( moveItems( int, int )),
	     m_canvas->canvas(), SLOT( slMoveItems( int, int )));
    
    connect( m_table, SIGNAL( topOffsetChanged( int )),
	     m_canvas->canvas(), SLOT( slSetTopOffset( int )));

    QValueList<int> sizes;
    sizes.append( 200 );
    setSizes( sizes );
    
    setResizeMode( m_table, QSplitter::KeepSize );
    setResizeMode( m_canvas, QSplitter::Stretch );
    // notify the part that this is our internal widget

    /* Prepare the draw operation */
    m_canvas->showProject( p );
    /* the table creates all tasks in both the table and the canvas */
    m_table->showProject( p );
    /* finalise the canvas */
    m_canvas->finalise( p );
    
    m_table->show();
    m_canvas->show(); 
    update();
}

void KTJGantt::slZoomIn()
{
   m_canvas->zoomIn();
   update();
}

void KTJGantt::slZoomOut()
{
   m_canvas->zoomOut();
}

void KTJGantt::slZoomOriginal()
{
   m_canvas->zoomOriginal();
}


KTJGantt::~KTJGantt()
{
}



#include "ktjgantt.moc"
