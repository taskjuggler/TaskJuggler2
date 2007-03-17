/*
 * CSVAccountReportElement.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "CSVAccountReportElement.h"
#include "TableColumnInfo.h"
#include "TableLineInfo.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Account.h"

CSVAccountReportElement::CSVAccountReportElement(Report* r,
                                                   const QString& df,
                                                   int dl) :
    CSVReportElement(r, df, dl)
{
    uint sc = r->getProject()->getMaxScenarios();
    columns.append(new TableColumnInfo(sc, "no"));
    columns.append(new TableColumnInfo(sc, "name"));
    columns.append(new TableColumnInfo(sc, "total"));
}

bool
CSVAccountReportElement::generate()
{
    generateHeader();

    generateTableHeader();

    AccountList filteredList;
    if (!filterAccountList(filteredList, AllAccounts, hideAccount,
                           rollUpAccount))
        return false;
    maxDepthAccountList = filteredList.maxDepth();

    /* Generate table of cost accounts. */
    if (!filterAccountList(filteredList, Cost, hideAccount, rollUpAccount))
        return false;
    sortAccountList(filteredList);
    maxDepthAccountList = filteredList.maxDepth();

    TableLineInfo tli;
    int aNo = 1;
    for (AccountListIterator ali(filteredList); *ali != 0; ++ali, ++aNo)
    {
        tli.ca1 = tli.account = *ali;
        for (uint sc = 0; sc < scenarios.count(); ++sc)
        {
            tli.row = sc;
            tli.idxNo = aNo;
            tli.sc = scenarios[sc];
            generateLine(&tli, sc == 0 ? 6 : 7);
        }
    }

    /* Generate summary line for cost accounts. */
    tli.boldText = true;
    tli.specialName = i18n("Total Costs");
    for (uint sc = 0; sc < scenarios.count(); ++sc)
    {
        tli.row = sc;
        tli.idxNo = 0;
        tli.sc = scenarios[sc];
        generateLine(&tli, sc == 0 ? 8 : 9);
    }

    for (QPtrListIterator<TableColumnInfo> ci(columns); *ci != 0; ++ci)
    {
        (*ci)->addSumToMemory(true);
        (*ci)->clearSum();
    }

    /* Generate table of revenue accounts. */
    if (!filterAccountList(filteredList, Revenue, hideAccount, rollUpAccount))
        return false;
    sortAccountList(filteredList);
    maxDepthAccountList = filteredList.maxDepth();

    tli.boldText = false;
    tli.specialName = QString::null;
    for (AccountListIterator ali(filteredList); *ali != 0; ++ali, ++aNo)
    {
        tli.ca1 = tli.account = *ali;
        for (uint sc = 0; sc < scenarios.count(); ++sc)
        {
            tli.row = sc;
            tli.idxNo = aNo;
            tli.sc = scenarios[sc];
            generateLine(&tli, sc == 0 ? 6 : 7);
        }
    }

    /* Generate summary line for revenue accounts. */
    tli.boldText = true;
    tli.specialName = i18n("Total Revenues");
    for (uint sc = 0; sc < scenarios.count(); ++sc)
    {
        tli.row = sc;
        tli.idxNo = 0;
        tli.sc = scenarios[sc];
        generateLine(&tli, sc == 0 ? 8 : 9);
    }

    for (QPtrListIterator<TableColumnInfo> ci(columns); *ci != 0; ++ci)
    {
        (*ci)->addSumToMemory(false);
        (*ci)->recallMemory();
    }

    /* Generate total summary line. */
    tli.specialName = i18n("Total");
    for (uint sc = 0; sc < scenarios.count(); ++sc)
    {
        tli.row = sc;
        tli.idxNo = 0;
        tli.sc = scenarios[sc];
        generateLine(&tli, sc == 0 ? 8 : 9);
    }

    return true;
}

