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
#include "TableLineInfo.h"
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
    
    /* Generate table of cost accounts. */
    filterAccountList(filteredList, Cost, hideAccount, rollUpAccount);
    sortAccountList(filteredList);

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
            tli.bgCol = colors.getColor("default").dark(100 + sc * 10);
            generateLine(&tli, sc == 0 ? 6 : 7);
        }
    }
  
    /* Generate summary line for cost accounts. */
    tli.boldText = TRUE; 
    tli.specialName = i18n("Total Costs");
    for (uint sc = 0; sc < scenarios.count(); ++sc)
    {
        tli.row = sc;
        tli.idxNo = 0;
        tli.sc = scenarios[sc];
        tli.bgCol = colors.getColor("header").dark(100 + sc * 10);
        generateLine(&tli, sc == 0 ? 8 : 9);
    }
    
    for (QPtrListIterator<TableColumnInfo> ci(columns); *ci != 0; ++ci)
    {
        (*ci)->addSumToMemory(TRUE);
        (*ci)->clearSum();
    }

    /* Generate table of revenue accounts. */
    filterAccountList(filteredList, Revenue, hideAccount, rollUpAccount);
    sortAccountList(filteredList);

    tli.boldText = FALSE;
    tli.specialName = QString::null;
    for (AccountListIterator ali(filteredList); *ali != 0; ++ali, ++aNo)
    {
        tli.ca1 = tli.account = *ali;
        for (uint sc = 0; sc < scenarios.count(); ++sc)
        {
            tli.row = sc;
            tli.idxNo = aNo;
            tli.sc = scenarios[sc];
            tli.bgCol = colors.getColor("default").dark(100 + sc * 10);
            generateLine(&tli, sc == 0 ? 6 : 7);
        }
    }
    
    /* Generate summary line for revenue accounts. */
    tli.boldText = TRUE;
    tli.specialName = i18n("Total Revenues");
    for (uint sc = 0; sc < scenarios.count(); ++sc)
    {
        tli.row = sc;
        tli.idxNo = 0;
        tli.sc = scenarios[sc];
        tli.bgCol = colors.getColor("header").dark(100 + sc * 10);
        generateLine(&tli, sc == 0 ? 8 : 9);
    }

    for (QPtrListIterator<TableColumnInfo> ci(columns); *ci != 0; ++ci)
    {
        (*ci)->addSumToMemory(FALSE);
        (*ci)->recallMemory();
    }

    /* Generate total summary line. */    
    tli.specialName = i18n("Total");
    for (uint sc = 0; sc < scenarios.count(); ++sc)
    {
        tli.row = sc;
        tli.idxNo = 0;
        tli.sc = scenarios[sc];
        tli.bgCol = colors.getColor("default").dark(100 + sc * 10); 
        generateLine(&tli, sc == 0 ? 8 : 9);
    }
   
    s() << "</tbody>" << endl;    
    s() << "</table>" << endl;
}

