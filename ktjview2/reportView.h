#ifndef _REPORTVIEW_H_
#define _REPORTVIEW_H_

#include "qicstable/QicsTable.h"

class ReportView: public QicsTable
{
    Q_OBJECT
public:
    /**
     * Constructor
     */
    ReportView( QWidget *parent, const char *name = 0 );

    /**
     * Initialize the widget
     */
    void init();

    /**
     * Clears the table and the selections
     */
    void clear();
};

#endif
