// local includes
#include "reportView.h"

ReportView::ReportView( QWidget *parent, const char *name )
    : QicsTable( 0, parent, name )
{
    //setReadOnly( true );
    setRowHeaderUsesModel( true );
    setColumnHeaderUsesModel( true );
}

#include "reportView.moc"
