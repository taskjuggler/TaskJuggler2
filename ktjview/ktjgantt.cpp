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
    m_table->setCanvasView( m_canvas );

    /* synchron y scroll */
    connect( m_canvas, SIGNAL(scrolledBy(int,int)),
	     m_table,    SLOT(scrollBy( int, int ) ));

    connect( m_table, SIGNAL(itemHeightChanged(int)),
	     m_canvas->canvas(), SLOT  (slSetRowHeight(int) ));

    connect( m_table, SIGNAL(heightChanged(int)),
	     m_canvas->canvas(), SLOT  (slHeightChanged(int) ));

    connect( m_table, SIGNAL(newTaskAdded(Task *, KTVTaskTableItem *)),
	     m_canvas->canvas(), SLOT(slNewTask(Task *, KTVTaskTableItem *) ));

    connect( m_table, SIGNAL(moveMarker(int)),
    	     m_canvas->canvas(), SLOT(slShowMarker(int)) );

    connect( m_table, SIGNAL( topOffsetChanged( int )),
	     m_canvas->canvas(), SLOT( slSetTopOffset( int )));

    connect( m_canvas, SIGNAL(canvasClicked(time_t)),
             this, SLOT(slCanvasClicked( time_t )));
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

void KTJGantt::slCanvasClicked( time_t t)
{
    emit statusBarChange( time2ISO( t ));
}

KTJGantt::~KTJGantt()
{
}

#include "ktjgantt.moc"
