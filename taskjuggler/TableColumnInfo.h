/*
 * TableColumnInfo.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TableColumnInfo_h_
#define _TableColumnInfo_h_

#include <qmap.h>

class QString;

/**
 * @short A column of a report.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TableColumnInfo
{
public:
    TableColumnInfo(uint sc, const QString& n) : name(n)
    {
        sum = memory = 0;
        maxScenarios = sc;
        clearSum();
        clearMemory();
    }
    ~TableColumnInfo()
    {
        delete [] sum;
        delete [] memory;
    }

    const QString& getName() const { return name; }

    void setTitle(const QString& t) { title = t; }
    const QString& getTitle() const { return title; }

    void setCellText(const QString& t) { cellText = t; }
    const QString& getCellText() const { return cellText; }

    void clearSum();
    void clearMemory();
    void addToSum(uint sc, const QString& key, double val);
    double getSum(uint sc, const QString& key) const { return sum[sc][key]; }
    QMap<QString, double>* getSum() { return sum; }
    void addSumToMemory(bool subtract);
    void negateMemory();
    void recallMemory();
    
protected:
    QString name;
    uint maxScenarios;
    QString title;
    QString cellText;
    
    QMap<QString, double>* sum;
    QMap<QString, double>* memory;

private:
    TableColumnInfo() { }
} ;

#endif

