// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-

#ifndef _RESUSAGE_VIEW_H_
#define _RESUSAGE_VIEW_H_

#include <qtable.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "Resource.h"
#include "Project.h"
#include "ResourceList.h"
#include "Interval.h"

enum Scale { SC_DAY = 0, SC_WEEK, SC_MONTH, SC_QUARTER };

class ResUsageView: public QTable
{
    Q_OBJECT
public:
    ResUsageView( QWidget * parent = 0, const char * name = 0 );
    virtual ~ResUsageView();

    void assignResources( ResourceList reslist );
    void clear();

    Scale scale() const;
    void setScale( int sc );

    QDateTime startDate() const;
    void setStartDate( const QDateTime & date );

    QDateTime endDate() const;
    void setEndDate( const QDateTime & date );

protected:
    // @reimp
    virtual void paintCell( QPainter * p, int row, int col, const QRect & cr, bool selected, const QColorGroup & cg );

     // @reimp, noop
    virtual void resizeData( int len );

signals:
    /**
     * Emitted when the horizontal (time) scale changes
     * @p sc the new scale
     */
    void scaleChanged( Scale sc );

private slots:
    /**
     * Update the column headers with respect to the current scale
     */
    void updateColumns();

private:
    /**
     * @return the Interval which corresponds to column @p col according to the current Scale
     */
    Interval intervalForCol( int col ) const;

    /**
     * @return Resource which corresponds to @p row
     */
    Resource * resourceForRow( int row );

    /**
     * @return list of column labels, depending on the current scale
     */
    QStringList getColumnLabels() const;

    /**
     * @return formatted @p date according to @p format and the current scale
     * (uses strftime)
     */
    QString formatDate( const QDateTime & date, const char * format ) const;

    /// list of resources
    ResourceList m_resList;
    /// list of row labels <res ID, res name>
    QStringList m_rowLabels;
    // current time scale
    Scale m_scale;
    /// project start, project end
    QDateTime m_start, m_end;
};

#endif
