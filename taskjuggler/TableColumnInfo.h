/*
 * TableColumnInfo.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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
class ExpressionTree;

/**
 * @short A column of a report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class TableColumnInfo
{
public:
    TableColumnInfo(uint sc, const QString& n) :
        name(n),
        maxScenarios(sc),
        title(),
        titleURL(),
        subTitle(),
        subTitleURL(),
        cellText(),
        cellURL(),
        hideCellText(0),
        hideCellURL(0),
        sum(0),
        memory(0),
        subColumns(0)
    {
        clearSum();
        clearMemory();
    }

    ~TableColumnInfo();

    const QString& getName() const { return name; }

    void setTitle(const QString& t) { title = t; }
    const QString& getTitle() const { return title; }

    void setTitleURL(const QString& t) { titleURL = t; }
    const QString& getTitleURL() const { return titleURL; }

    void setSubTitle(const QString& t) { subTitle = t; }
    const QString& getSubTitle() const { return subTitle; }

    void setSubTitleURL(const QString& t) { subTitleURL = t; }
    const QString& getSubTitleURL() const { return subTitleURL; }

    void setCellText(const QString& t) { cellText = t; }
    const QString& getCellText() const { return cellText; }

    void setCellURL(const QString& t) { cellURL = t; }
    const QString& getCellURL() const { return cellURL; }

    void setHideCellText(ExpressionTree* et) { hideCellText = et; }
    ExpressionTree* getHideCellText() const { return hideCellText; }

    void setHideCellURL(ExpressionTree* et) { hideCellURL = et; }
    ExpressionTree* getHideCellURL() const { return hideCellURL; }

    void clearSum();
    void clearMemory();
    void addToSum(uint sc, const QString& key, double val);
    double getSum(uint sc, const QString& key) const { return sum[sc][key]; }
    QMap<QString, double>* getSum() { return sum; }
    void addSumToMemory(bool subtract);
    void negateMemory();
    void recallMemory();

    void increaseSubColumns() { ++subColumns; }
    uint getSubColumns() const { return subColumns; }

protected:
    QString name;
    uint maxScenarios;
    QString title;
    QString titleURL;
    QString subTitle;
    QString subTitleURL;
    QString cellText;
    QString cellURL;
    ExpressionTree* hideCellText;
    ExpressionTree* hideCellURL;

    QMap<QString, double>* sum;
    QMap<QString, double>* memory;

    uint subColumns;

private:
    TableColumnInfo() { }
} ;

#endif

