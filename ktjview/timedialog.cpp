#include "timedialog.h"
#include "Utility.h"

#include <qhbox.h>
#include <qvbox.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <qlabel.h>
#include <qdatetime.h>


TimeDialog::TimeDialog( QWidget *parentWidget, time_t start, time_t end )
    : KDialogBase( parentWidget, "TimeDialog", true,
                   i18n("Setup time Frame"),
                   Ok|Apply|Close, Ok )
{
    QVBox *vb = makeVBoxMainWidget();

    QHBox *hb = new QHBox( vb );
    QDateTime dt;
    dt.setTime_t( start );

    (void) new QLabel( i18n("Display Start Time: "), hb );
    m_dateStart = new KDateWidget( dt.date(), hb );

    dt.setTime_t( end );

    hb = new QHBox( vb );
    (void) new QLabel( i18n("Display End Time: "), hb );
    m_dateEnd = new KDateWidget( dt.date(), hb );


}


