#include "ktjgantt.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <qsplitter.h>

#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>

#include "ktvtasktable.h"
#include "ktvtaskcanvasview.h"


KTJGantt::KTJGantt( QWidget *parentWidget, const char *)
   : QSplitter( parentWidget )
{
    // we need an instance
    m_table = new KTVTaskTable( this, "TABLE");
    m_canvas = new KTVTaskCanvasView( this, m_table, "CANVAS");

    connect( m_table, SIGNAL(itemHeightChanged(int)),
	     m_canvas->canvas(), SLOT  (slSetRowHeight(int) ));

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
}

KTJGantt::~KTJGantt()
{
}


void KTJGantt::showProject( Project *p )
{
   m_canvas->showProject( p );
   m_table->showProject( p );
}


#include "ktjgantt.moc"
