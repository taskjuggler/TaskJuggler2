/*
 * HTMLIndexReportElement.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdlib.h>
#include <qdir.h>
#include <qfile.h>

#include "HTMLIndexReportElement.h"
#include "TableLineInfo.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Account.h"

HTMLIndexReportElement::HTMLIndexReportElement(Report* r,
                                                   const QString& df,
                                                   int dl) :
    HTMLReportElement(r, df, dl)
{
    columns.append(new TableColumnInfo(0, "Report"));
    columns.append(new TableColumnInfo(0, "Comment"));
}

bool
HTMLIndexReportElement::generateReportLine(Report* report, int level)
{
    // Deduce report file name relatively to index directory.
    QString absSourceFilePath = QFileInfo(defFileName).dir(true).canonicalPath();
    QString absReportFilePath = QFileInfo(report->getFullFileName()).dir(true).canonicalPath();
    QString reportRelPath = "";
    for (QString sourceFilePath = defFileName; // .left(defFileName.findRev(QDir().separator())-1);
        sourceFilePath.find('/') >= 0;
        sourceFilePath = sourceFilePath.left(sourceFilePath.findRev(QDir().separator())))
    {
        if (strncmp(sourceFilePath, absReportFilePath, sourceFilePath.length()) == 0)
        {
            reportRelPath += absReportFilePath.right(absReportFilePath.length() - sourceFilePath.length() - 1);
            break;
        }
        else
        {
            reportRelPath = reportRelPath + ".." + QDir().separator();
        }
    }

    QString reportRelFile = reportRelPath + QDir().separator() + QFileInfo(report->getFullFileName()).fileName();
    // Make thumbnail directory in case it does not exists yet

    // Generate thumbnail
    QFile reportQFile (report->getFullFileName() );
    s() << "<TR style=\"background-color:" << colors.getColorName("default") << "; \" ><TD>";
    s() << "</TD><TD><A HREF='" << reportRelFile << "'>";
    for (int i = 0 ; i < level; i++)
        s() << "<ul>";

    s() << "<strong>" << (report->getHeadline() == "" ?  QFileInfo(report->getFullFileName()).fileName() : report->getHeadline()) << "</strong>";
    for (int i = 0 ; i < level; i++)
        s() << "</ul>";

    s() << "</TD>";

    s() << "<TD><i> ( " << report->getType() << " ) </i></TD>";
    ElementHolder *elementHolder = dynamic_cast<ElementHolder*>(this);
    if (elementHolder && elementHolder->getTable()->getCaption() != "")
        s() << ": " << elementHolder->getTable()->getCaption();
    else if (report->getCaption() != "")
        s() << ": " << report->getCaption();

    s() << "</TR>\n";
    for (QPtrListIterator<Report> ri(report->getChildrenReportListIterator()); *ri; ++ri)
    {
        s() << "\n";
        generateReportLine(*ri, level + 1);
    }

    return true;
}

bool
HTMLIndexReportElement::generate()
{
    QString previousReportType;

    generateHeader();

    s() << "<table align=\"center\" cellpadding=\"2\" "
        << "style=\"background-color:" << colors.getColorName("default") << "\"";
    s() << ">" << endl;
    s() << " <thead>" << endl
        << "  <tr valign=\"middle\""
        << " style=\"background-color:" << colors.getColorName("header") << "; "
        << "font-size:110%; font-weight:bold; text-align:center\"";
    s() << ">" << endl;
    s() << "  </tr>" << endl;

    for (QPtrListIterator<Report> ri(report->getProject()->getReportListIterator()); *ri != 0; ++ri)
    {
        // We generate all but Qt*Reports. Those are for the GUI version.
        if (strncmp((*ri)->getType(), "Qt", 2) != 0
            && (*ri)->getParentReport() == 0
            && (*ri) != this->getReport())
        {
            generateReportLine(*ri);
        }
    }
    s() << " </thead>\n" << endl;
    s() << "</table>" << endl;

    return true;
}

