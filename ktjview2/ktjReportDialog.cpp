#include "ktjReportDialog.h"
#include "reportSelectionWidget.h"

// Qt includes
#include <qcombobox.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>

// KDE includes
#include <klocale.h>
#include <kguiitem.h>

KtjReportDialog::KtjReportDialog( const QDateTime & start, const QDateTime & end, QWidget * parent, const char * name )
    : KDialogBase( parent, name, true, i18n( "Create Report" ), Ok|Cancel, Ok )
{
    m_base = new ReportSelectionWidget( this, "report_selection_widget" );
    setMainWidget( m_base );
    setStartDate( start );
    setEndDate( end );
    setButtonOK( KGuiItem( i18n( "Create" ), "ok",
                           i18n( "Create the report with the specified options" ) ) ); // FIXME doesn't set the tooltip
}

KtjReportDialog::~KtjReportDialog()
{
    delete m_base;
}

QDateTime KtjReportDialog::startDate() const
{
    return m_base->deStart->dateTime();
}

void KtjReportDialog::setStartDate( const QDateTime & date )
{
    m_base->deStart->setDateTime( date );
}

QDateTime KtjReportDialog::endDate() const
{
    return m_base->deEnd->dateTime();
}

void KtjReportDialog::setEndDate( const QDateTime & date )
{
    m_base->deEnd->setDateTime( date );
}

KtjReport::Scale KtjReportDialog::scale() const
{
    return static_cast<KtjReport::Scale>( m_base->cbScale->currentItem() );
}

#include "ktjReportDialog.moc"
