#ifndef _RESOURCEITEM_H_
#define _RESOURCEITEM_H_

#include <qdatetime.h>
#include <qstring.h>

#include <klistview.h>

/**
 * This class represents an item in the tasks listview.
 * @short Task item (listview)
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class ResourceItem: public KListViewItem
{
public:
    /**
     * CTOR
     * @param parent Parent item (see KListView)
     * @param id ID of the corresponding resource
     */
    ResourceItem( KListView * parent, QString id ):
        KListViewItem( parent, id )
        {
            m_id = id;
        }
    /**
     * CTOR
     * @param parent Parent item (see KListViewItem)
     * @param id ID of the corresponding resource
     */
    ResourceItem( KListViewItem * parent, QString id ):
        KListViewItem( parent, id )
        {
            m_id = id;
        }

    /**
     * @return ID of the resource
     */
    QString id() const { return m_id ;}

    /**
     * Reimplemented from QListViewItem to get correct sorting (dates, numbers, etc)
     */
    virtual int compare( QListViewItem * i, int col, bool ascending ) const;

private:
    QString m_id;
};

#endif
