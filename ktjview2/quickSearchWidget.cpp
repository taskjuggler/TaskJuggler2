#include "quickSearchWidget.h"
#include "klistviewsearchline.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qapplication.h>

#include <klistview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>
#include <ktoolbarbutton.h>

QuickSearchWidget::QuickSearchWidget( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    QHBoxLayout * lay = new QHBoxLayout( this );
    lay->setSpacing( KDialog::spacingHint() );

    m_clearButton = new KToolBarButton( QApplication::reverseLayout() ? "clear_left" : "locationbar_erase",
                                         0, this );

    connect( m_clearButton, SIGNAL( clicked() ), this, SLOT( reset() ) );
    lay->add( m_clearButton );

    m_searchLabel = new QLabel( i18n( "&Search:" ), this, "kde toolbar widget" );
    lay->add( m_searchLabel );

    m_searchLine = new KListViewSearchLine( this, 0, "search_line" );
    lay->addWidget( m_searchLine, 1 ); // adds the widget stretched

    m_searchLabel->setBuddy( m_searchLine );
}

void QuickSearchWidget::setListView( KListView * view )
{
    m_searchLine->setListView( view );
}

void QuickSearchWidget::reset()
{
    m_searchLine->clear();
}

#include "quickSearchWidget.moc"
