#ifndef _KTJTASKREPORT_H_
#define _KTJTASKREPORT_H_

#include "ktjReport.h"

class KTJTaskReport: public KTJReport
{
public:
    KTJTaskReport( Project * proj );
    virtual QicsDataModel generate();
    virtual QString name() const;
    virtual QString description() const;
    virtual QicsDataModel generate() const;

private:
    /**
     * Generate column header
     * @return number of columns
     */
    int generateHeader();

    /**
     * Generate main task row
     */
    void generateRow( const Task * task );

    /**
     * Generate secondary resource row for task
     */
    void generateRow( const Task * task, const Resource * res );
};

#endif
