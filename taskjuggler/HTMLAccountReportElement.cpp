/*
 * HTMLAccountReportElement.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "HTMLAccountReportElement.h"
#include "TableColumnInfo.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Report.h"
#include "Account.h"
#include "AccountList.h"

HTMLAccountReportElement::HTMLAccountReportElement(Report* r,
                                                   const QString& df,
                                                   int dl) :
    HTMLReportElement(r, df, dl)
{
    uint sc = r->getProject()->getMaxScenarios();
    columns.append(new TableColumnInfo(sc, "no"));
    columns.append(new TableColumnInfo(sc, "name"));
    columns.append(new TableColumnInfo(sc, "total"));
}

HTMLAccountReportElement::~HTMLAccountReportElement()
{
}

void
HTMLAccountReportElement::generate()
{
    generateHeader();
    
    generateTableHeader();

    s() << "<tbody>" << endl;

    AccountList filteredList;
    filterAccountList(filteredList, AllAccounts, hideAccount, rollUpAccount);
    maxDepthAccountList = filteredList.maxDepth();
    
    filterAccountList(filteredList, Cost, hideAccount, rollUpAccount);
    sortAccountList(filteredList);

    int aNo = 1;
    for (AccountListIterator ali(filteredList); *ali != 0; ++ali, ++aNo)
    {
        generateFirstAccount(scenarios[0], *ali, aNo);
        for (uint sc = 1; sc < scenarios.count(); ++sc)
            generateNextAccount(sc, *ali);
    }
    
    generateSummaryFirst(scenarios[0], i18n("Subtotal Cost"), 
                         colors.getColorName("header"));
    for (uint sc = 1; sc < scenarios.count(); ++sc)
        generateSummaryNext(sc, colors.getColorName("header"));
    
    for (QPtrListIterator<TableColumnInfo> ci(columns); *ci != 0; ++ci)
    {
        (*ci)->addSumToMemory(TRUE);
        (*ci)->clearSum();
    }

    filterAccountList(filteredList, Revenue, hideAccount, rollUpAccount);
    sortAccountList(filteredList);

    for (AccountListIterator ali(filteredList); *ali != 0; ++ali, ++aNo)
    {
        generateFirstAccount(scenarios[0], *ali, aNo);
        for (uint sc = 1; sc < scenarios.count(); ++sc)
            generateNextAccount(sc, *ali);
    }
    
    generateSummaryFirst(scenarios[0], i18n("Subtotal Revenue"), 
                         colors.getColorName("header"));
    for (uint sc = 1; sc < scenarios.count(); ++sc)
        generateSummaryNext(sc, colors.getColorName("header"));

    for (QPtrListIterator<TableColumnInfo> ci(columns); *ci != 0; ++ci)
    {
        (*ci)->addSumToMemory(FALSE);
        (*ci)->recallMemory();
    }

    generateSummaryFirst(scenarios[0], i18n("Total"), 
                         colors.getColorName("default"));
    for (uint sc = 1; sc < scenarios.count(); ++sc)
        generateSummaryNext(sc, colors.getColorName("default"));
   
    s() << "</tbody>" << endl;    
    s() << "</table>" << endl;
}

