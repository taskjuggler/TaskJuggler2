#ifndef _REPORTVIEW_H_
#define _REPORTVIEW_H_

#include "qicstable/QicsTable.h"

class ReportView: public QicsTable
{
    Q_OBJECT
public:
    ReportView( QWidget *parent, const char *name = 0 );
};

#endif
