// local includes
#include "reportView.h"

#include "qicstable/QicsColumnHeader.h"
#include "qicstable/QicsRowHeader.h"

ReportView::ReportView( QWidget *parent, const char *name )
    : QicsTable( 0, parent, name )
{
    rowHeaderRef().setNumColumns( 1 );
    columnHeaderRef().setNumRows( 1 );
    //setRowHeaderUsesModel( true );
    //setColumnHeaderUsesModel( true );
    setReadOnly( true );
}

#include "reportView.moc"
