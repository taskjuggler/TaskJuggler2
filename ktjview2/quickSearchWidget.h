#ifndef _QUICK_SEARCH_WIDGET_H_
#define _QUICK_SEARCH_WIDGET_H_

#include <qwidget.h>

class QLabel;
class KListView;
class KListViewSearchLine;
class KToolBarButton;

class QuickSearchWidget: public QWidget
{
    Q_OBJECT
public:
    QuickSearchWidget( QWidget * parent, const char * name = 0 );
    ~QuickSearchWidget() { };

    void setListView( KListView * view );

public slots:
    void reset();

private:
    //QToolButton * m_clearButton;
    KToolBarButton * m_clearButton;
    QLabel * m_searchLabel;
    KListViewSearchLine * m_searchLine;
};


#endif
