// local includes
#include "reportView.h"

// QicsTable includes
#include "qicstable/QicsColumnHeader.h"
#include "qicstable/QicsRowHeader.h"

ReportView::ReportView( QWidget *parent, const char *name )
    : QicsTable( 0, parent, name )
{
    setReadOnly( true );
    init();
}

void ReportView::init()
{
    rowHeaderRef().setNumColumns( 1 );
    columnHeaderRef().setNumRows( 1 );
    setRowHeaderUsesModel( true );
    setColumnHeaderUsesModel( true );
}

void ReportView::clear()
{
    clearSelectionList();
}

#include "reportView.moc"
