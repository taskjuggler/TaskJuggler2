#ifndef _KTJTASKREPORT_H_
#define _KTJTASKREPORT_H_

#include "ktjReport.h"

/**
 * Report containing Tasks as primary rows
 * and allocated Resources as secondary rows.
 *
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class KTJTaskReport: public KTJReport
{
public:
    KTJTaskReport( Project * proj );
    virtual ~KTJTaskReport();
    virtual QString name() const;
    virtual QString description() const;
    virtual QicsDataModelDefault * generate();

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
