#ifndef _KTJREPORTDIALOG_H_
#define _KTJREPORTDIALOG_H_

class ReportSelectionWidget;

class QDateTime;

#include "ktjReport.h"

#include <kdialogbase.h>

class KtjReportDialog: public KDialogBase
{
    Q_OBJECT
public:
    KtjReportDialog( const QDateTime & start, const QDateTime & end, QWidget * parent = 0, const char * name = 0 );
    ~KtjReportDialog();

    /**
     * @return the start date of the view
     */
    QDateTime startDate() const;

    /**
     * Set the start date of the view
     */
    void setStartDate( const QDateTime & date );

    /**
     * @return the end date of the view
     */
    QDateTime endDate() const;

    /**
     * Set the end date of the view
     */
    void setEndDate( const QDateTime & date );

    KtjReport::Scale scale() const;

private:
    ReportSelectionWidget * m_base;
};

#endif
