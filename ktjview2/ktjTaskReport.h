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
     * Generate main @p task row
     */
    void generateRow( const Task * task, int columns );

    /**
     * Generate secondary resource row for @p task
     */
    void generateRow( const Task * task, const Resource * res, int columns );
};

#endif
