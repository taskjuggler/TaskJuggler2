#ifndef _REPORTVIEW_H_
#define _REPORTVIEW_H_

#include "qicstable/QicsTable.h"
#include "qicstable/QicsDataModel.h"

class ReportView: public QicsTable
{
    Q_OBJECT
public:
    /**
     * Constructor
     */
    ReportView( QWidget *parent, const char *name = 0 );

    /**
     * Clears the table and the selections
     */
    void clear();

    void setRowHeader( QicsDataModelColumn & header );

    void setColumnHeader( QicsDataModelRow & header );
};

#endif
