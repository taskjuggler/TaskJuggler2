#ifndef _SELECTDIALOG_H
#define _SELECTDIALOG_H

#include <qwidget.h>

#include <kdialogbase.h>

class QLabel;
class KListView;
class KDialogBase;
class TaskListIterator;
class ResourceListIterator;

class SelectDialog : public KDialogBase
{
    Q_OBJECT

public:
    SelectDialog( TaskListIterator it, bool multi = false, QWidget* parent = 0, const char* name = 0 );
    SelectDialog( ResourceListIterator it, bool multi = false, QWidget* parent = 0, const char* name = 0 );
    ~SelectDialog();

    /**
     * Contains the result list of task/resource IDs selected by the user
     */
    QStringList resultList() const;

private:
    /**
     * Setup the widgets
     */
    void init();

    QLabel* lbSelect;
    KListView* lvContents;

    /// whether multiple selection is allowed
    bool m_multi;
};

#endif
