#ifndef _KTJREPORT_H_
#define _KTJREPORT_H_

#include "Project.h"
#include "qicstable/QicsDataModelDefault.h"

class KTJReport
{
public:
    KTJReport( Project * proj );
    virtual ~KTJReport();

    /**
     * Time scale for the horizontal line
     */
    enum Scale { SC_DAY = 0, SC_WEEK, SC_MONTH, SC_QUARTER };

    /**
     * Specify which information to draw in a cell
     */
    enum DisplayData
    {
        /// resource load / total load (where total == load + free)
        DIS_LOAD_AND_TOTAL = 0,
        /// resource load
        DIS_LOAD,
        /// available resource workload
        DIS_FREE
    };

    /**
     * @return the current scale
     */
    Scale scale() const;

    /**
     * Set the scale
     */
    void setScale( int sc );

    /**
     * @return the start date of the view
     */
    QDateTime startDate() const;

    /**
     * Set the start date of the view
     */
    void setStartDate( const QDateTime & date );

    /**
     * @return the end date of the view
     */
    QDateTime endDate() const;

    /**
     * Set the end date of the view
     */
    void setEndDate( const QDateTime & date );

    /**
     * @return the type of data to display
     * @see DisplayData
     */
    DisplayData displayData() const;

    /**
     * Set the type of data to display
     * @see DisplayData
     */
    void setDisplayData( int data );

    /**
     * Clear the report (model)
     */
    void clear();

    /**
     * Abstract method to generate the report,
     * implementers need to clear the data model first
     */
    virtual QicsDataModel generate() const = 0;

    /**
     * @return short unique name of the report
     */
    virtual QString name() const = 0;

    /**
     * @return verbose description of the report
     */
    virtual QString description() const = 0;

protected:
    /**
     * @return formatted @p date according to @p format and the current scale
     * (uses strftime)
     */
    QString formatDate( time_t date, QString format ) const;

    /**
     * @return the Interval which corresponds to column @p col according to the current Scale
     */
    Interval intervalForCol( int col ) const;

    /// our project
    Project * m_proj;
    /// (returned) data model
    QicsDataModelDefault * m_model;
    /// current time scale
    Scale m_scale;
    /// display params
    DisplayData m_display;
    /// project start, project end
    QDateTime m_start, m_end;
};

#endif
