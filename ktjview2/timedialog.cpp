#include "timedialog.h"

#include <qhbox.h>
#include <qvbox.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <qlabel.h>
#include <qdatetime.h>


TimeDialog::TimeDialog( QWidget *parentWidget, const QDateTime & start, const QDateTime & end )
    : KDialogBase( parentWidget, "TimeDialog", true,
                   i18n("Setup time Frame"),
                   Ok|Cancel, Ok )
{
    QVBox *vb = makeVBoxMainWidget();

    QHBox *hb = new QHBox( vb );

    (void) new QLabel( i18n("Display Start Time: "), hb );
    m_dateStart = new KDateTimeWidget( start, hb );

    hb = new QHBox( vb );
    (void) new QLabel( i18n("Display End Time: "), hb );
    m_dateEnd = new KDateTimeWidget( end, hb );
}

#include "timedialog.moc"
