// local includes
#include "reportView.h"

// QicsTable includes
#include "qicstable/QicsColumnHeader.h"
#include "qicstable/QicsRowHeader.h"

ReportView::ReportView( QWidget *parent, const char *name )
    : QicsTable( 0, parent, name )
{
    setReadOnly( true );
}

void ReportView::clear()
{
    clearSelectionList();
}

void ReportView::setRowHeader( QicsDataModelColumn & header )
{
    rowHeaderRef().columnRef(0).setDataValues( header );
}

void ReportView::setColumnHeader( QicsDataModelRow & header )
{
    columnHeaderRef().rowRef(0).setDataValues( header );
}

#include "reportView.moc"
