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
#include "TableColumn.h"
#include "tjlib-internal.h"
#include "Account.h"
#include "AccountList.h"

/*
#include "TjMessageHandler.h"
#include "HTMLAccountReport.h"
#include "Project.h"
#include "Task.h"
#include "Account.h"
#include "Interval.h"
#include "Utility.h"
*/
HTMLAccountReportElement::HTMLAccountReportElement(Report* r,
                                                   const QString& df,
                                                   int dl) :
    HTMLReportElement(r, df, dl)
{
    columns.append(new TableColumn("no"));
    columns.append(new TableColumn("name"));
    columns.append(new TableColumn("total"));
}

HTMLAccountReportElement::~HTMLAccountReportElement()
{
}

#include "Project.h"
void
HTMLAccountReportElement::generate()
{
    generateTableHeader();

    s() << "<tbody>" << endl;

    AccountList filteredList;
    filterAccountList(filteredList, AllAccounts, hideAccount, rollUpAccount);
    maxDepthAccountList = filteredList.maxDepth();
    
    filterAccountList(filteredList, Cost, hideAccount, rollUpAccount);
    sortAccountList(filteredList);

    columnTotalsCosts = new QMap<QString, double>[scenarios.count()];
    columnTotalsRevenue = new QMap<QString, double>[scenarios.count()];
    columnTotals = columnTotalsCosts;
    
    int aNo = 1;
    for (AccountListIterator ali(filteredList); *ali != 0; ++ali, ++aNo)
    {
        generateFirstAccount(*ali, aNo);
        for (uint sc = 1; sc < scenarios.count(); ++sc)
            generateNextAccount(sc, *ali);
    }
    generateSummary(i18n("Subtotal Cost"), "headersmall");

    filterAccountList(filteredList, Revenue, hideAccount, rollUpAccount);
    sortAccountList(filteredList);

    columnTotals = columnTotalsRevenue;
    for (AccountListIterator ali(filteredList); *ali != 0; ++ali, ++aNo)
    {
        generateFirstAccount(*ali, aNo);
        for (uint sc = 1; sc < scenarios.count(); ++sc)
            generateNextAccount(sc, *ali);
    }
    generateSummary(i18n("Subtotal Revenue"), "headersmall");
    columnTotals = new QMap<QString, double>[scenarios.count()];
    for (uint sc = 0; sc < scenarios.count(); ++sc)
    {
        QMap<QString, double>::Iterator ctc;
        QMap<QString, double>::Iterator ctr;
        for (ctc = columnTotalsCosts[sc].begin(),
             ctr = columnTotalsRevenue[sc].begin();
             ctc != columnTotalsCosts[sc].end(); ++ctc, ++ctr)
        {
            columnTotals[sc][ctc.key()] = *ctr - *ctc;
        }
    }
    generateSummary(i18n("Total"), "default");
   
    delete [] columnTotalsCosts;
    columnTotalsCosts = 0;
    delete [] columnTotalsRevenue;
    columnTotalsRevenue = 0;
    delete [] columnTotals;
    columnTotals = 0;

    s() << "</tbody>" << endl;    
    s() << "</table>" << endl;
}

