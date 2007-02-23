/*
 * QtTaskReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "QtTaskReportElement.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "Project.h"

QtTaskReportElement::QtTaskReportElement(Report* r, const QString& df,
                                             int dl) :
    QtReportElement(r, df, dl)
{
    int sc = r->getProject()->getMaxScenarios();
    columns.append(new TableColumnInfo(sc, "start"));
    columns.append(new TableColumnInfo(sc, "end"));

    // show all tasks
    setHideTask(new ExpressionTree(new Operation(0)));
    // show all resources
    setHideResource(new ExpressionTree(new Operation(0)));

    taskSortCriteria[0] = CoreAttributesList::TreeMode;
    taskSortCriteria[1] = CoreAttributesList::StartUp;
    taskSortCriteria[2] = CoreAttributesList::EndUp;
    resourceSortCriteria[0] = CoreAttributesList::TreeMode;
}

QtTaskReportElement::~QtTaskReportElement()
{
}
