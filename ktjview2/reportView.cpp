// local includes
#include "reportView.h"

// TJ includes
#include "TaskList.h"

ReportView::ReportView( QWidget *parent, const char *name )
    : QicsTable( 0, parent, name )
{
    setReadOnly( true );
}

#include "reportView.moc"
