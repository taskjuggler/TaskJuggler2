#ifndef _RU_FIND_DIALOG_H_
#define _RU_FIND_DIALOG_H_

class QSpacerItem;
class QButtonGroup;
class QLabel;
class QCheckBox;
class KLineEdit;
class QGridLayout;

#include <kdialogbase.h>

class ruFindDlg : public KDialogBase
{
    Q_OBJECT
public:
    ruFindDlg( QStringList data, QWidget* parent = 0, const char* name = 0 );
    ~ruFindDlg();

signals:
    /**
     * Signals that we found a matching entry, or -1 if none
     */
    void signalMatch( int );

protected slots:
    /**
     * Invoked when the 'Find' button is pressed, start the search
     */
    virtual void slotUser1();

private slots:
    /**
     * Enable/disable the 'Find' button depending on the text's length
     */
    void slotTextChanged( const QString & text );

private:
    /**
     * Start the search function
     */
    void startSearch();

    /**
     * Go to the next match (after first)
     */
    void findNext();

    QButtonGroup* grpResource;
    QLabel* lbName;
    QCheckBox* cbCaseSensitive;
    KLineEdit* leResource;
    QCheckBox* cbRegExp;
    QSpacerItem* spacer3;
    QGridLayout* grpResourceLayout;

    /// our data to search in
    QStringList m_data;
    /// the result list of column numbers
    QValueList<int> m_result;

    bool m_firstRun;
};

#endif
