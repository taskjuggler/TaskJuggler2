#ifndef _REPORTVIEW_H_
#define _REPORTVIEW_H_

#include "qicstable/QicsDataModel.h"
#include "qicstable/QicsTable.h"

#include "Project.h"

class ReportView: public QicsTable
{
    Q_OBJECT
public:
    ReportView( QWidget *parent, const char *name = 0 );
    ~ReportView();

    void createTaskReport( Project * proj );
    void createResourceReport( Project * proj );
private:
    QicsDataModelDefault * m_model;
};

#endif
