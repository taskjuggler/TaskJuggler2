#include "ktjReportView.h"

KtjReportView::KtjReportView( QWidget *parent, const char *name )
    : KListView( parent, name )
{
    setSortColumn( -1 );
    setSorting( -1 );
    setItemsMovable( false );
    setAcceptDrops( false );
    setAllColumnsShowFocus( true );
    setRootIsDecorated( true );
}

void KtjReportView::removeColumns()
{
    int cols = columns() - 1;
    for ( int i = cols; i >=0; i-- )
        removeColumn( i );
}

void KtjReportView::clear()
{
    removeColumns();
    KListView::clear();
}

#include "ktjReportView.moc"
