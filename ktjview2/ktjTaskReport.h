#ifndef _KTJTASKREPORT_H_
#define _KTJTASKREPORT_H_

#include "ktjReport.h"
#include "ktjReportView.h"

// TJ includes
#include "Task.h"
#include "Resource.h"

/**
 * @short Task Coverage report
 *
 * This report show tasks and their allocated resources in a hierarchical view
 * together with load
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class KtjTaskReport: public KtjReport
{
public:
    /**
     * Constructor
     * @param proj The project to work on
     * @param view View (KListView based) where to display data
     */
    KtjTaskReport( Project * proj, KtjReportView * view );
    virtual void generate();

protected:
    virtual int setupColumns();

private:
    /**
     * Generate primary row with tasks
     * @param parent view where to attach this item
     * @param task Task to work on
     * @param columns number of columns
     */
    void generatePrimaryRow( KtjReportView * parent, Task * task, int columns );

    /**
     * Generate primary row with tasks
     * @param parent parent item where to attach this item
     * @param task Task to work on
     * @param columns number of columns
     */
    void generateSecondaryRow( KListViewItem * parent, Task * task, Resource * res, int columns );
};

#endif
