/****************************************************************************
** Form interface generated from reading ui file './TemplateSelector.ui'
**
** Created: Sun Oct 30 15:59:26 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef TEMPLATESELECTOR_H
#define TEMPLATESELECTOR_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class KListView;
class QListViewItem;
class QPushButton;

class TemplateSelector : public QDialog
{
    Q_OBJECT

public:
    TemplateSelector( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~TemplateSelector();

    KListView* templateList;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

public slots:
    virtual void itemSelected();
    virtual void listDoubleClicked( QListViewItem * lvi );

protected:
    QGridLayout* TemplateSelectorLayout;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

};

#endif // TEMPLATESELECTOR_H
