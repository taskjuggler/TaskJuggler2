#ifndef _KTJTASKREPORT_H_
#define _KTJTASKREPORT_H_

#include "ktjReport.h"
#include "ktjReportView.h"

// TJ includes
#include "Task.h"
#include "Resource.h"

class KtjTaskReport: public KtjReport
{
public:
    KtjTaskReport( Project * proj, KtjReportView * view );
    virtual void generate();

protected:
    virtual int setupColumns();

private:
    void generatePrimaryRow( KtjReportView * parent, Task * task, int columns );
    void generateSecondaryRow( KListViewItem * parent, Task * task, Resource * res, int columns );
};

#endif
