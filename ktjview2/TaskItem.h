#ifndef _TASKITEM_H_
#define _TASKITEM_H_

#include <qdatetime.h>
#include <qstring.h>

#include <klistview.h>

/**
 * This class represents an item in the tasks listview.
 * @short Task item (listview)
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class TaskItem: public KListViewItem
{
public:
    /**
     * CTOR
     * @param parent Parent item (see KListView)
     * @param id ID of the corresponding task
     * @param start Task's start date
     * @param end Task's end date
     */
    TaskItem( KListView * parent, QString id, QDateTime start, QDateTime end ):
        KListViewItem( parent, id )
        {
            m_id = id;
            m_start = start;
            m_end = end;
        }
    /**
     * CTOR
     * @param parent Parent item (see KListViewItem)
     * @param id ID of the corresponding task
     * @param start Task's start date
     * @param end Task's end date
     */
    TaskItem( KListViewItem * parent, QString id, QDateTime start, QDateTime end ):
        KListViewItem( parent, id )
        {
            m_id = id;
            m_start = start;
            m_end = end;
        }

    /**
     * @return ID of the task
     */
    QString id() const { return m_id ;}

    /**
     * @return The task's start date
     */
    QDateTime startDate() const { return m_start; }

    /**
     * @return The task's end date
     */
    QDateTime endDate() const { return m_end; }

    /**
     * Reimplemented from QListViewItem to get correct sorting (dates, numbers, etc)
     */
    virtual int compare( QListViewItem * i, int col, bool ascending ) const;

private:
    QString m_id;
    QDateTime m_start, m_end;
};

#endif
