#include "selectDialog.h"

#include "TaskList.h"
#include "Task.h"
#include "ResourceList.h"
#include "Resource.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>

#include <klocale.h>
#include <klistview.h>

SelectDialog::SelectDialog( TaskListIterator it, bool multi, QWidget* parent, const char* name )
    : KDialogBase( parent, name, true, i18n( "Tasks" ), Ok|Cancel, Ok ), m_multi( multi )
{
    init();
    lbSelect->setText( i18n( "Select tasks:" ) );
}

SelectDialog::SelectDialog( ResourceListIterator it, bool multi, QWidget* parent, const char* name )
    : KDialogBase( parent, name, true, i18n( "Resources" ), Ok|Cancel, Ok ), m_multi( multi )
{
    init();
    lbSelect->setText( i18n( "Select resources:" ) );

    // fill the listview with resources
    for ( ; *it != 0; ++it )
    {
        Resource* res = (*it);

        if ( m_multi )
        {
            QCheckListItem * item = new QCheckListItem( lvContents, res->getId(), QCheckListItem::CheckBox );
            item->setText( 1, res->getName() );
        }
        else
            ( void ) new KListViewItem( lvContents, res->getId(), res->getName() );
    }
}

SelectDialog::~SelectDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void SelectDialog::init()
{
    QVBox * layout = makeVBoxMainWidget();

    lbSelect = new QLabel( i18n( "Select:" ), layout, "lbSelect" );

    lvContents = new KListView( layout, "lvContents" );
    lvContents->addColumn( i18n( "ID" ) );
    lvContents->addColumn( i18n( "Name" ) );
    lvContents->setAllColumnsShowFocus( TRUE );
    lvContents->setShowSortIndicator( TRUE );
    lvContents->setResizeMode( KListView::LastColumn );
    lvContents->setItemsMovable( FALSE );
    lvContents->hideColumn( 0 );
    resize( QSize(492, 394).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

QStringList SelectDialog::resultList() const
{
    QStringList result;

    QListViewItemIterator::IteratorFlag flags;
    if ( m_multi )
        flags = QListViewItemIterator::Checked;
    else
        flags = QListViewItemIterator::Selected;

    QListViewItemIterator it( lvContents, flags );
    while ( it.current() )
    {
        if ( m_multi )
            result.append( static_cast<QCheckListItem *>( *it )->text() );
        else
            result.append( ( *it )->text( 0 ) );

        ++it;
    }

    return result;
}

#include "selectDialog.moc"

