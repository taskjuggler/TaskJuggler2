#include "ktjReportView.h"
#include "ResourceItem.h"
#include "TaskItem.h"

#include <klocale.h>
#include <kglobal.h>

KtjReportView::KtjReportView( QWidget *parent, const char *name )
    : KListView( parent, name )
{
    setSortColumn( -1 );
    setSorting( -1 );
    setItemsMovable( false );
    setAcceptDrops( false );
    setAllColumnsShowFocus( true );
    setRootIsDecorated( true );
    setTooltipColumn( 0 );
    //setShowToolTips( false );
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

QString KtjReportView::tooltip( QListViewItem *item, int column ) const
{
    if ( static_cast<TaskItem *>( item ) )
    {
        TaskItem * tItem = static_cast<TaskItem *>( item );
        return i18n( "Start: %1\nEnd: %2" )
            .arg( KGlobal::locale()->formatDateTime( tItem->startDate() ) )
            .arg( KGlobal::locale()->formatDateTime( tItem->endDate() ) );
    }
#if 0
    else if ( static_cast<ResourceItem *>( item ) )
    {

    }
#endif
    else
        return KListView::tooltip( item, column );
}

bool KtjReportView::showTooltip( QListViewItem *, const QPoint &, int ) const
{
    return true;
}

#include "ktjReportView.moc"
