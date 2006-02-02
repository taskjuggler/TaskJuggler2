/*
 * ProjectFile.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>

#include <qtextstream.h>
#include <qregexp.h>
#include <qapplication.h>

#include "ProjectFile.h"
#include "debug.h"
#include "taskjuggler.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Scenario.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "Token.h"
#include "ExpressionTree.h"
#include "Operation.h"
#include "Allocation.h"
#include "Booking.h"
#include "kotrus.h"
#include "HTMLTaskReport.h"
#include "HTMLTaskReportElement.h"
#include "HTMLResourceReport.h"
#include "HTMLResourceReportElement.h"
#include "HTMLAccountReport.h"
#include "HTMLAccountReportElement.h"
#include "HTMLWeeklyCalendar.h"
#include "HTMLWeeklyCalendarElement.h"
#include "HTMLStatusReport.h"
#include "HTMLReportElement.h"
#include "QtReport.h"
#include "QtReportElement.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"
#include "QtResourceReport.h"
#include "QtResourceReportElement.h"
//#include "QtAccountReport.h"
//#include "QtAccountReportElement.h"
#include "CSVTaskReport.h"
#include "CSVTaskReportElement.h"
#include "CSVResourceReport.h"
#include "CSVResourceReportElement.h"
#include "CSVAccountReport.h"
#include "CSVAccountReportElement.h"
#include "CSVReport.h"
#include "XMLReport.h"
#ifdef HAVE_KDE
#include "ICalReport.h"
#endif
#include "ExportReport.h"
#include "TableColumnInfo.h"
#include "ReportXML.h"
#include "ReferenceAttribute.h"
#include "TextAttribute.h"
#include "CustomAttributeDefinition.h"
#include "UsageLimits.h"
#include "ExpressionFunctionTable.h"
#include "JournalEntry.h"

// Dummy marco to mark all keywords of taskjuggler syntax
#define KW(a) a

ProjectFile::ProjectFile(Project* p)
{
    proj = p;

    openFiles.setAutoDelete(TRUE);
}

bool
ProjectFile::open(const QString& file, const QString& parentPath,
                  const QString& taskPrefix, bool masterfile)
{
    if (masterfile)
    {
        proj->setProgressBar(0, 100);
        masterFile = file;
    }

    QString absFileName = file;
    if (DEBUGPF(10))
        qDebug("Requesting to open file %s", file.latin1());
    if (absFileName[0] != '/')
        absFileName = parentPath + absFileName;

    if (DEBUGPF(10))
        qDebug("File name before compression: %s", absFileName.latin1());
    int end = 0;
    while ((end = absFileName.find("/../", end)) >= 0)
    {
        int start = absFileName.findRev('/', end - 1);
        if (start < 0)
            start = 0;
        else
            start++;    // move after '/'
        if (start < end && absFileName.mid(start, end - start) != "..")
            absFileName.remove(start, end + strlen("/../") - start);
        end = start - 1;
    }
    if (DEBUGPF(10))
        qDebug("File name after compression: %s", absFileName.latin1());

    // Make sure that we include each file only once.
    if (includedFiles.findIndex(absFileName) != -1)
    {
        if (DEBUGPF(2))
            qDebug("Ignoring already read file %s",
                     absFileName.latin1());
        return TRUE;
    }

    FileInfo* fi = new FileInfo(this, absFileName, taskPrefix);

    if (!fi->open())
    {
        errorMessage(i18n("Cannot read file '%1'").arg(absFileName));
        return FALSE;
    }

    // Register source file
    proj->addSourceFile(absFileName);
    proj->setProgressInfo(i18n("Parsing %1...").arg(absFileName));

    if (DEBUGPF(2))
        qDebug("Reading %s", absFileName.latin1());

    openFiles.append(fi);
    includedFiles.append(absFileName);
    return TRUE;
}

bool
ProjectFile::close()
{
    bool error = FALSE;

    FileInfo* fi = openFiles.getLast();

    if (!fi->close())
        error = TRUE;
    if (DEBUGPF(2))
        qDebug("Finished file %s", fi->getFile().latin1());
    openFiles.removeLast();

    if (openFiles.isEmpty())
    {
        proj->setProgressInfo(i18n("Parsing completed"));
    }
    else
        proj->setProgressInfo(i18n("Parsing %1...")
                              .arg(openFiles.getLast()->getFile()));

    return error;
}

bool
ProjectFile::parse()
{
    TokenType tt;
    QString token;

    for ( ; ; )
    {
        switch (tt = nextToken(token))
        {
        case EndOfFile:
            return TRUE;
        case ID:
            // Only macro and include are allowed prior to the project header.
            if (proj->getEnd() == 0 &&
                token != KW("project") && token != KW("macro") &&
                token != KW("include"))
            {
                errorMessage
                    (i18n("The project properties must be defined prior to any "
                          "account, shift, task or resource."));
                return FALSE;
            }

            if (token == KW("task"))
            {
                if (!readTask(0))
                    return FALSE;
                break;
            }
            else if (token == KW("account"))
            {
                if (!readAccount(0))
                    return FALSE;
                break;
            }
            else if (token == KW("resource"))
            {
                if (!readResource(0))
                    return FALSE;
                break;
            }
            else if (token == KW("shift"))
            {
                if (!readShift(0))
                    return FALSE;
                break;
            }
            else if (token == KW("vacation"))
            {
                time_t from, to;
                QString name;
                if (!readVacation(from, to, TRUE, &name))
                    return FALSE;
                proj->addVacation(name, Interval(from, to));
                break;
            }
            else if (token == KW("priority"))
            {
                int priority;
                if (!readPriority(priority))
                    return FALSE;
                proj->setPriority(priority);
                break;
            }
            else if (token == KW("mineffort"))
            {
                TokenType tt;
                if ((tt = nextToken(token)) != REAL && tt != INTEGER)
                {
                    errorMessage(i18n("Real value exptected"));
                    return FALSE;
                }
                proj->setMinEffort(token.toDouble());
                break;
            }
            else if (token == KW("maxeffort"))
            {
                TokenType tt;
                if ((tt = nextToken(token)) != REAL && tt != INTEGER)
                {
                    errorMessage(i18n("Real value exptected"));
                    return FALSE;
                }
                UsageLimits* limits = new UsageLimits;
                limits->setDailyMax
                    ((uint) ((token.toDouble() *
                              proj->getDailyWorkingHours() * 3600) /
                             proj->getScheduleGranularity()));
                proj->setResourceLimits(limits);
                break;
            }
            else if (token == KW("limits"))
            {
                UsageLimits* limits;
                if ((limits = readLimits()) == 0)
                    return FALSE;
                proj->setResourceLimits(limits);
                break;
            }
            else if (token == KW("rate"))
            {
                TokenType tt;
                if ((tt = nextToken(token)) != REAL && tt != INTEGER)
                {
                    errorMessage(i18n("Real value exptected"));
                    return FALSE;
                }
                proj->setRate(token.toDouble());
                break;
            }
            else if (token == KW("currency"))
            {
                errorMessage
                    (i18n("ERROR: 'currency' is no longer a property. It's "
                          "now an optional project attribute. Please fix "
                          "your project file."));
                return FALSE;
            }
            else if (token == KW("currencydigits"))
            {
                errorMessage
                    (i18n("ERROR: 'currencydigits' has been deprecated. "
                          "Please use 'currencyformat' instead."));
                return FALSE;
            }
            else if (token == "timingresolution")
            {
                errorMessage
                    (i18n("ERROR: 'timingresolution' is no longer a "
                          "property. It's now an optional project attribute. "
                          "Please fix your project file."));
                return FALSE;
            }
            else if (token == KW("workinghours"))
            {
                errorMessage
                    (i18n("ERROR: 'workinghours' is no longer a property. "
                          "It's now an optional project attribute. Please fix "
                          "your project file."));
                return FALSE;
            }
            else if (token == KW("copyright"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                proj->setCopyright(token);
                break;
            }
            else if (token == KW("include"))
            {
                if (!readInclude())
                    return FALSE;
                break;
            }
            else if (token == KW("macro"))
            {
                QString id;
                if (nextToken(id) != ID)
                {
                    errorMessage(i18n("Macro ID expected"));
                    return FALSE;
                }
                QString file = openFiles.last()->getFile();
                uint line = openFiles.last()->getLine();
                if (nextToken(token) != MacroBody)
                {
                    errorMessage(i18n("Macro body expected"));
                    return FALSE;
                }
                Macro* macro = new Macro(id, token, file, line);
                if (!macros.addMacro(macro))
                {
                    errorMessage(i18n("Macro has been defined already"));
                    delete macro;
                    return FALSE;
                }
                break;
            }
            else if (token == KW("flags"))
            {
                for ( ; ; )
                {
                    QString flag;
                    if (nextToken(flag) != ID)
                    {
                        errorMessage(i18n("flag ID expected"));
                        return FALSE;
                    }

                    /* Flags can be declared multiple times, but we
                     * register a flag only once. */
                    if (!proj->isAllowedFlag(flag))
                        proj->addAllowedFlag(flag);

                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
                break;
            }
            else if (token == KW("project"))
            {
                if (!readProject())
                    return FALSE;
                break;
            }
            else if (token == KW("projectid"))
            {
                for ( ; ; )
                {
                    QString id;
                    if (nextToken(id) != ID)
                    {
                        errorMessage(i18n("Project ID expected"));
                        return FALSE;
                    }

                    if (!proj->addId(id))
                    {
                        errorMessage
                            (i18n("Project ID %1 has already been registered")
                             .arg(id));
                        return FALSE;
                    }

                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
                break;
            }
            else if (token == KW("projectids"))
            {
                for ( ; ; )
                {
                    QString id;
                    if (nextToken(id) != ID)
                    {
                        errorMessage(i18n("Project ID expected"));
                        return FALSE;
                    }

                    proj->addId(id, FALSE);

                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
                break;
            }
            else if (token == "xmltaskreport")
            {
                errorMessage
                    (i18n("ERROR: The keyword 'xmltaskreport' is "
                          "deprecated. Please use the keyword 'xmlreport' "
                          "instead."));
                return FALSE;
            }
            else if (token == KW("xmlreport"))
            {
               if(!readXMLReport())
                  return FALSE;
               break;
            }
            else if (token == KW("icalreport"))
            {
               if( !readICalTaskReport())
                  return FALSE;
               break;
            }
            else if (token == KW("htmltaskreport") ||
                     token == KW("htmlresourcereport") ||
                     token == KW("htmlweeklycalendar") ||
                     token == KW("htmlaccountreport"))
            {
               if (!readHTMLReport(token))
                   return FALSE;
               break;
            }
            else if (token == KW("taskreport") ||
                     token == KW("resourcereport") ||
                     token == KW("accountreport"))
            {
                if (!readReport(token))
                    return FALSE;
                break;
            }
            else if (token == KW("htmlstatusreport"))
            {
                if (!readHTMLStatusReport())
                    return FALSE;
                break;
            }
            else if (token == KW("csvtaskreport") ||
                     token == KW("csvresourcereport") ||
                     token == KW("csvaccountreport"))
            {
                if (!readCSVReport(token))
                    return FALSE;
                break;
            }
            else if (token == KW("export"))
            {
                if (!readExportReport())
                    return FALSE;
                break;
            }
            else if (token == KW("kotrusmode"))
            {
                if (nextToken(token) != STRING ||
                    (token != KW("db") && token != KW("xml") &&
                     token != KW("nokotrus")))
                {
                    errorMessage(i18n("Unknown kotrus mode"));
                    return FALSE;
                }
                if (token != KW("nokotrus"))
                {
                    Kotrus* kotrus = new Kotrus();
                    kotrus->setKotrusMode(token);
                    proj->setKotrus(kotrus);
                }
                break;
            }
            else if (token == KW("supplement"))
            {
                if (nextToken(token) != ID ||
                    (token != KW("task") && (token != KW("resource"))))
                {
                    errorMessage(i18n("'task' or 'resource' expected"));
                    return FALSE;
                }
                if ((token == "task" && !readTaskSupplement("")) ||
                    (token == "resource" && !readResourceSupplement()))
                    return FALSE;
                break;
            }
            // break missing on purpose!
        default:
            errorMessage(i18n("Syntax Error at '%1'!").arg(token));
            return FALSE;
        }
        qApp->processEvents();
    }

    return TRUE;
}

bool
ProjectFile::readProject()
{
    QString token;

    if (!proj->getProjectIdList().isEmpty())
    {
        errorMessage
            (i18n("Illegal redefinition of project property. It can only "
                  "be defined once."));
        return FALSE;
    }

    if (proj->accountCount() > 0 || proj->resourceCount() > 0 ||
        proj->shiftCount() > 0 || proj->taskCount() > 0)
    {
        errorMessage
            (i18n("The project properties must be defined prior to any "
                  "account, shift, task or resource."));
        return FALSE;
    }

    if (nextToken(token) != ID)
    {
        errorMessage(i18n("Project ID expected"));
        return FALSE;
    }
    if (!proj->addId(token))
    {
        errorMessage
            (i18n("Project ID %1 has already been registered")
             .arg(token));
        return FALSE;
    }
    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("Project name expected"));
        return FALSE;
    }
    proj->setName(token);
    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("Version string expected"));
        return FALSE;
    }
    proj->setVersion(token);
    time_t start, end;
    if (!readDate(start, 0, FALSE))
        return FALSE;
    if (!readDate(end, 0, FALSE))
        return FALSE;
    if (end <= start)
    {
        errorMessage(i18n("End date must be larger than start date"));
        return FALSE;
    }
    proj->setStart(start);
    proj->setEnd(end - 1);

    TokenType tt;
    bool scenariosDefined = FALSE;
    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) != ID && tt != RBRACE)
            {
                errorMessage(i18n("Attribute ID expected"));
                return FALSE;
            }
            if (tt == RBRACE)
                break;
            if (token == KW("workinghours"))
            {
                int dow;
                QPtrList<Interval>* l = new QPtrList<Interval>();
                if (!readWorkingHours(dow, l))
                    return FALSE;

                proj->setWorkingHours(dow, l);
            }
            else if (token == KW("dailyworkinghours"))
            {
                if ((tt = nextToken(token)) != REAL && tt != INTEGER)
                {
                    errorMessage(i18n("Real number expected"));
                    return FALSE;
                }
                proj->setDailyWorkingHours(token.toDouble());
            }
            else if (token == KW("yearlyworkingdays"))
            {
                if ((tt = nextToken(token)) != REAL && tt != INTEGER)
                {
                    errorMessage(i18n("Real number expected"));
                    return FALSE;
                }
                proj->setYearlyWorkingDays(token.toDouble());
            }
            else if (token == KW("now"))
            {
                time_t now;
                if (!readDate(now, 0))
                    return FALSE;
                proj->setNow(now);
            }
            else if (token == KW("timingresolution"))
            {
                ulong resolution;
                if (!readTimeValue(resolution))
                    return FALSE;
                if (resolution < 60 * 5)
                {
                    errorMessage(i18n("timing resolution must be at least 5 "
                                      "min"));
                    return FALSE;
                }
                if (resolution >
                    (ulong) ((proj->getEnd() - proj->getStart()) / 24))
                {
                    errorMessage(i18n("timing resolution must be at least "
                                      "24 times smaller than the project "
                                      "duration"));
                    return FALSE;
                }
                proj->setScheduleGranularity(resolution);
            }
            else if (token == KW("timezone"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Timezone name expected"));
                    return FALSE;
                }
                proj->setTimeZone(token);
            }
            else if (token == KW("timeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    return FALSE;
                }
                proj->setTimeFormat(token);
            }
            else if (token == KW("shorttimeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    return FALSE;
                }
                proj->setShortTimeFormat(token);
            }
            else if (token == KW("numberformat"))
            {
                RealFormat format;
                if (!readRealFormat(&format))
                    return FALSE;
                proj->setNumberFormat(format);
            }
            else if (token == KW("currencyformat"))
            {
                RealFormat format;
                if (!readRealFormat(&format))
                    return FALSE;
                proj->setCurrencyFormat(format);
            }
            else if (token == KW("currency"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                proj->setCurrency(token);
            }
            else if (token == KW("weekstartsmonday"))
            {
                proj->setWeekStartsMonday(TRUE);
            }
            else if (token == KW("weekstartssunday"))
            {
                proj->setWeekStartsMonday(FALSE);
            }
            else if (token == KW("extend"))
            {
                if (!readExtend())
                    return FALSE;
            }
            else if (token == KW("scenario"))
            {
                if (scenariosDefined)
                {
                    errorMessage("There can only be one top-level scenario. "
                                 "All other scenarios must be nested into "
                                 "the top-level scenario.");
                    return FALSE;
                }
                if (!readScenario(0))
                    return FALSE;
                scenariosDefined = TRUE;
            }
            else if (token == KW("allowredefinitions"))
            {
                proj->setAllowRedefinitions(TRUE);
            }
            else if (token == KW("journalentry"))
            {
                JournalEntry* entry;
                if ((entry = readJournalEntry()) == 0)
                    return FALSE;

                proj->addJournalEntry(entry);
            }
            else if (token == KW("include"))
            {
                if (!readInclude())
                    return FALSE;
            }
            else
            {
                errorMessage(i18n("Unknown attribute %1").arg(token));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    return TRUE;
}

bool
ProjectFile::readExtend()
{
    QString property;

    if (nextToken(property) != ID ||
        (property != "task" && property != "resource"))
    {
        errorMessage(i18n("'%1' is not a property. Please use 'task' or "
                          "'resource'.").arg(property));
        return FALSE;
    }
    QString token;
    if (nextToken(token) != LBRACE)
    {
        errorMessage(i18n("'{' expected."));
        return FALSE;
    }
    TokenType tt;
    for ( ; ; )
    {
        QString attrType;
        if ((tt = nextToken(attrType)) == RBRACE)
            break;
        else if (tt != ID ||
                 (attrType != KW("reference") && attrType != KW("text")))
        {
            errorMessage(i18n("'%1' is not a known custom attribute type. "
                              "Please use 'reference' or 'text'.")
                         .arg(attrType));
            return FALSE;
        }
        QString attrID;
        if (nextToken(attrID) != ID)
        {
            errorMessage(i18n("Attribute ID expected."));
            return FALSE;
        }
        if (attrID[0].upper() != attrID[0])
        {
            errorMessage(i18n("User defined attribute IDs must start with "
                              "a capital letter to avoid conflicts with "
                              "future TaskJuggler keywords."));
            return FALSE;
        }
        QString attrName;
        if (nextToken(attrName) != STRING)
        {
            errorMessage(i18n("String expected"));
            return FALSE;
        }
        CustomAttributeType cat = CAT_Undefined;
        if (attrType == KW("reference"))
            cat = CAT_Reference;
        else if (attrType == KW("text"))
            cat = CAT_Text;
        bool ok = FALSE;
        CustomAttributeDefinition* ca =
            new CustomAttributeDefinition(attrName, cat);
        if (property == "task")
            ok = proj->addTaskAttribute(attrID, ca);
        else if (property == "resource")
            ok = proj->addResourceAttribute(attrID, ca);
        if (!ok)
        {
            errorMessage(i18n("The custom attribute '%1' has already been "
                         "declared for the property '%2'.")
                .arg(attrID).arg(property));
            return FALSE;
        }

        if ((tt = nextToken(token)) != LBRACE)
        {
            returnToken(tt, token);
            continue;
        }
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID exprected."));
                return FALSE;
            }
            if (token == KW("inherit"))
                ca->setInherit(TRUE);
            else
            {
                errorMessage(i18n("Attribute ID expected."));
                return FALSE;
            }
        }
    }

    return TRUE;
}

bool
ProjectFile::readScenario(Scenario* parent)
{
    QString id;
    if (nextToken(id) != ID)
    {
        errorMessage(i18n("Scenario ID expected. '%1' is not a scenario "
                          "id.").arg(id));
        return FALSE;
    }
    QString name;
    if (nextToken(name) != STRING)
    {
        errorMessage(i18n("Scenario name expected. '%1' is not a valid "
                          "scenario name.").arg(name));
        return FALSE;
    }

    /* If the scenario has no parent, we delete the old top-level scenario and
     * all included scenarios. */
    if (!parent)
        delete proj->getScenario(0);

    Scenario* scenario = new Scenario(proj, id, name, parent);
    QString token;
    TokenType tt;
    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            tt = nextToken(token);
            if (tt == RBRACE)
            {
                return TRUE;
            }
            else if (token == KW("scenario"))
            {
                if (!readScenario(scenario))
                    return FALSE;
            }
            else if (token == KW("disabled"))
            {
                scenario->setEnabled(false);
            }
            else if (token == KW("enabled"))
            {
                scenario->setEnabled(true);
            }
            else if (token == KW("projection"))
            {
                if (!readProjection(scenario))
                    return false;
            }
            else if (token == KW("baseline"))
            {
                scenario->setProjectionMode(false);
                /* baseline mode implies sloppy bookings. */
                scenario->setStrictBookings(false);
            }
            else if (token == KW("minslackrate"))
            {
                if ((tt = nextToken(token)) != REAL && tt != INTEGER)
                {
                    errorMessage(i18n("Real value expected"));
                    return -1;
                }
                double rate = token.toDouble();
                if (rate < 0.0 || rate > 100.0)
                {
                    errorMessage(i18n("Slack rate must be between 0 and 100"));
                    return -1;
                }
                scenario->setMinSlackRate(rate);
            }
            else
            {
                errorMessage(i18n("Unknown scenario attribute '%1'")
                             .arg(token));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    return TRUE;
}

bool
ProjectFile::readProjection(Scenario* scenario)
{
    TokenType tt;
    QString token;

    scenario->setProjectionMode(true);

    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            tt = nextToken(token);
            if (tt == RBRACE)
            {
                return true;
            }
            if (token == KW("strict"))
            {
                scenario->setStrictBookings(true);
            }
            else if (token == KW("sloppy"))
            {
                scenario->setStrictBookings(false);
            }
            else
            {
                errorMessage(i18n("Unknown projection attribute '%1'")
                             .arg(token));
                return false;
            }
        }
    }
    else
        returnToken(tt, token);

    return true;
}

TokenType
ProjectFile::nextToken(QString& buf)
{
    if (openFiles.isEmpty())
        return EndOfFile;

    TokenType tt;
    while ((tt = openFiles.last()->nextToken(buf)) == EndOfFile)
    {
        close();
        if (openFiles.isEmpty())
            return EndOfFile;
    }

    return tt;
}

const QString&
ProjectFile::getTaskPrefix()
{
    if (openFiles.isEmpty())
        return QString::null;

    return openFiles.last()->getTaskPrefix();
}

bool
ProjectFile::generateMakeDepList(const QString& fileName, bool append) const
{
    QTextStream* f;
    FILE* fh;
    if (fileName.isEmpty())
    {
        f = new QTextStream(stdout, IO_WriteOnly);
        fh = stdout;
    }
    else
    {
        if ((fh = fopen(fileName, append ? "a" : "w")) == 0)
            return FALSE;
        f = new QTextStream(fh, append ? IO_Append : IO_WriteOnly);
    }
    *f << masterFile << ": \\" << endl;
    bool first = TRUE;
    for (QStringList::ConstIterator it = includedFiles.begin();
         it != includedFiles.end(); ++it)
    {
        if (first)
            first = FALSE;
        else
            *f << " \\" << endl;
        *f << "  " << *it;
    }

    if (!fileName.isEmpty())
        fclose(fh);
    delete f;

    return TRUE;
}

void
ProjectFile::errorMessage(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);

    if (openFiles.isEmpty())
        TJMH.errorMessage
            (i18n("Unexpected end of file found. Probably a missing '}'."));
    else
        openFiles.last()->errorMessageVA(msg, ap);
    va_end(ap);
}

bool
ProjectFile::readInclude()
{
    QString fileName;

    if (nextToken(fileName) != STRING)
    {
        errorMessage(i18n("File name expected"));
        return FALSE;
    }
    if (fileName.right(4) != ".tji" &&
        fileName.right(5) != ".tjsp")
    {
        errorMessage(i18n("ERROR: The include file '%1' should have a "
                          "'.tji' extension.").arg(fileName));
        return FALSE;
    }

    QString token;
    TokenType tt;

    QString taskPrefix = getTaskPrefix();
    /* The nextToken() call may yield an EndOfFile and shift file scope to
     * parent file. So we have to save the path of the current file to pass it
     * later to open(). */
    QString parentPath = openFiles.last()->getPath();

    if ((tt = nextToken(token)) == LBRACE)
    {
        while ((tt = nextToken(token)) != RBRACE)
        {
            if (tt == ID && token == KW("taskprefix"))
            {
                if ((tt = nextToken(token)) != ID && tt != ABSOLUTE_ID)
                {
                    errorMessage(i18n("Task ID expected"));
                    return FALSE;
                }
                if (!proj->getTask(getTaskPrefix() + token))
                {
                    errorMessage(i18n("Task prefix must be a known task"));
                    return FALSE;
                }
                taskPrefix = getTaskPrefix() + token + ".";
            }
            else
            {
                errorMessage(i18n("Invalid optional attribute \'%1\'")
                             .arg(token));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    if (!open(fileName, parentPath, taskPrefix))
        return FALSE;

    return TRUE;
}

bool
ProjectFile::readCustomAttribute(CoreAttributes* property, const QString& id,
                                 CustomAttributeType type)
{
    if (type == CAT_Reference)
    {
        QString ref, label;
        if (!readReference(ref, label))
            return FALSE;
        ReferenceAttribute* ra = new ReferenceAttribute(ref, label);
        property->addCustomAttribute(id, ra);
    }
    else if (type == CAT_Text)
    {
        QString text;
        if (nextToken(text) == STRING)
        {
            TextAttribute* ra = new TextAttribute(text);
            property->addCustomAttribute(id, ra);
        }
        else
        {
            errorMessage(i18n("String expected"));
            return FALSE;
        }
    }
    else
        qFatal("ProjectFile::readCustomAttribute(): unknown type");

    return TRUE;
}

bool
ProjectFile::readTask(Task* parent)
{
    TokenType tt;
    QString token;

    QString definitionFile = getFile();
    uint definitionLine = getLine();

    QString id;
    if ((tt = nextToken(id)) != ID &&
        (tt != ABSOLUTE_ID) && (tt != RELATIVE_ID))
    {
        errorMessage(i18n("ID expected"));
        return FALSE;
    }

    if (tt == RELATIVE_ID)
    {
        /* If a relative ID has been specified the task is declared out of
         * it's actual scope. So we have to set 'parent' to point to the
         * correct parent task. */
        do
        {
            if (id[0] == '!')
            {
                if (parent != 0)
                    parent = parent->getParent();
                else
                {
                    errorMessage(i18n("Invalid relative task ID '%1'")
                                 .arg(id));
                    return FALSE;
                }
                id = id.right(id.length() - 1);
            }
            else if (id.find('.') >= 0)
            {
                QString tn = (parent ? parent->getId() + "." : QString())
                    + id.left(id.find('.'));
                bool found = FALSE;
                for (TaskListIterator tli(parent ?
                                          parent->getSubListIterator() :
                                          proj->getTaskListIterator());
                     *tli != 0; ++tli)
                    if ((*tli)->getId() == tn)
                    {
                        parent = *tli;
                        id = id.right(id.length() - id.find('.') - 1);
                        found = TRUE;
                        break;
                    }
                if (!found)
                {
                    errorMessage(i18n("Task '%1' unknown").arg(tn));
                    return FALSE;
                }
            }
        } while (id[0] == '!' || id.find('.') >= 0);
    }
    else if (tt == ABSOLUTE_ID)
    {
        QString path = getTaskPrefix() + id.left(id.findRev('.', -1));
        if ((parent = proj->getTask(path)) == 0)
        {
            errorMessage(i18n("Task '%1' has not been defined")
                         .arg(path));
            return FALSE;
        }
        id = id.right(id.length() - id.findRev('.', -1) - 1);
    }

    QString name;
    if ((tt = nextToken(name)) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }

    /*
     * If a pointer to a parent task was given, the id of the parent task is
     * used as a prefix to the ID of the task. Toplevel task may be prefixed
     * by a task prefix as specified by when including a project file.
     */
    if (parent)
        id = parent->getId() + "." + id;
    else
    {
        QString tp = getTaskPrefix();
        if (!tp.isEmpty())
        {
            // strip trailing '.'
            tp = tp.left(tp.length() - 1);
            parent = proj->getTask(tp);
            id = tp + "." + id;
        }
    }

    Task* task;
    // We need to check that the task id has not been declared before.
    if ((task = proj->getTask(id)) != 0)
    {
        if (proj->getAllowRedefinitions())
        {
            if (task->getName() != name)
            {
                errorMessage(i18n
                         ("Redefinition of task '%1' with different name '%2'. "
                          "Previous name was '%3'.")
                     .arg(id).arg(name).arg(task->getName()));
                return FALSE;
            }
        }
        else
        {
            errorMessage(i18n("Task %1 has already been declared")
                         .arg(id));
            return FALSE;
        }
    }
    else
    {
        task = new Task(proj, id, name, parent, definitionFile,
                        definitionLine);
        task->inheritValues();
    }

    if ((tt = nextToken(token)) == LBRACE)
    {
        if (!readTaskBody(task))
            return FALSE;
    }
    else
        returnToken(tt, token);

    if (task->getName().isEmpty())
    {
        errorMessage(i18n("No name specified for task '%1'").arg(id));
        return FALSE;
    }

    return TRUE;
}

bool
ProjectFile::readTaskSupplement(QString prefix)
{
    QString token;
    TokenType tt;
    Task* task;

    /* When supplement is used within a task declaration, the prefix is the id
     * of the parent task. If it's empty, then we need to use the prefix for
     * the current file. The parent task id has no trailing dot, so we have to
     * append it. */
    if (prefix.isEmpty())
        prefix = getTaskPrefix();
    else
        prefix += ".";

    if (((tt = nextToken(token)) != ID && tt != ABSOLUTE_ID) ||
        ((task = proj->getTask(prefix.isEmpty() ?
                               token : prefix + token)) == 0))
    {
        errorMessage(i18n("Task '%1' has not been defined yet")
            .arg(prefix.isEmpty() ? token : prefix + token));
        return FALSE;
    }
    if (nextToken(token) != LBRACE)
    {
        errorMessage(i18n("'}' expected"));
        return FALSE;
    }
    return readTaskBody(task);
}

bool
ProjectFile::readTaskBody(Task* task)
{
    QString token;
    TokenType tt;
    int res = 0;

    for (bool done = false ; !done; )
    {
        QString next;
        TokenType nextTT;
        switch (tt = nextToken(token))
        {
        case ID:
            if ((nextTT = nextToken(next)) == COLON)
            {
                int sc;
                if ((sc = proj->getScenarioIndex(token)) < 1)
                {
                    errorMessage(i18n("Scenario ID expected. '%1' is not "
                                      "a defined scenario.").arg(token));
                    return FALSE;
                }
                tt = nextToken(token);
                if (readTaskScenarioAttribute(token, task, sc - 1, TRUE) < 1)
                    return FALSE;
                continue;
            }
            else
                returnToken(nextTT, next);
            if (proj->getTaskAttribute(token))
            {
                if (!readCustomAttribute(task, token,
                                         proj->getTaskAttribute(token)->
                                         getType()))
                    return FALSE;
            }
            else if ((res = readTaskScenarioAttribute(token, task, 0, FALSE))
                     > 0)
                ;   // intentionally empty statement
            else if (res < 0)
                return -1;
            else if (token == KW("task"))
            {
                if (!readTask(task))
                    return FALSE;
            }
            else if (token == KW("note"))
            {
                if ((tt = nextToken(token)) == STRING)
                    task->setNote(token);
                else
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
            }
            else if (token == KW("milestone"))
            {
                task->setMilestone();
            }
            else if (token == "actualstart")
            {
                errorMessage(i18n("ERROR: 'actualstart' has been "
                                  "deprecated. Please use 'actual:start' "
                                  "instead."));
                return FALSE;
            }
            else if (token == "actualend")
            {
                errorMessage(i18n("ERROR: 'actualend' has been "
                                  "deprecated. Please use 'actual:end' "
                                  "instead."));
                return FALSE;
            }
            else if (token == "actuallength")
            {
                errorMessage(i18n("ERROR: 'actuallength' has been "
                                  "deprecated. Please use 'actual:length' "
                                  "instead."));
                return FALSE;
            }
            else if (token == "actualeffort")
            {
                errorMessage(i18n("ERROR: 'actualeffort' has been "
                                  "deprecated. Please use 'actual:effort' "
                                  "instead."));
                return FALSE;
            }
            else if (token == "actualduration")
            {
                errorMessage(i18n("ERROR: 'actualduration' has been "
                                  "deprecated. Please use 'actual:duration' "
                                  "instead."));
                return FALSE;
            }
            else if (token == "planscheduled")
            {
                errorMessage(i18n("ERROR: 'planscheduled' has been "
                                  "deprecated. Please use 'plan:scheduled' "
                                  "instead."));
                return FALSE;
            }
            else if (token == "actualscheduled")
            {
                errorMessage(i18n("ERROR: 'actualscheduled' has been "
                                  "deprecated. Please use 'actual:scheduled' "
                                  "instead."));
                return FALSE;
            }
            else if (token == KW("responsible"))
            {
                Resource* r;
                if (nextToken(token) != ID ||
                    (r = proj->getResource(token)) == 0)
                {
                    errorMessage(i18n("Resource ID expected"));
                    return FALSE;
                }
                task->setResponsible(r);
            }
            else if (token == KW("shift"))
            {
                time_t from, to;
                from = proj->getStart();
                to = proj->getEnd();
                Shift* s;
                if ((s = readShiftSelection(from, to)) == 0)
                    return FALSE;
                if (!task->addShift(Interval(from, to), s))
                {
                    errorMessage(i18n("Shift intervals may not overlap"));
                    return FALSE;
                }
            }
            else if (token == KW("allocate"))
            {
                do
                {
                    if (!readAllocate(task))
                        return FALSE;
                } while ((tt = nextToken(token)) == COMMA);
                returnToken(tt, token);
            }
            else if (token == KW("depends"))
            {
                if (task->isContainer())
                {
                    errorMessage
                        (i18n("Dependencies of container tasks must be "
                              "specified prior to any sub tasks."));
                    return FALSE;
                }
                for ( ; ; )
                {
                    QString id;
                    if ((tt = nextToken(id)) != ID &&
                        tt != RELATIVE_ID && tt != ABSOLUTE_ID)
                    {
                        errorMessage(i18n("Task ID expected"));
                        return FALSE;
                    }
                    if (tt == ABSOLUTE_ID || tt == ID)
                        id = getTaskPrefix() + id;
                    TaskDependency* td = task->addDepends(id);
                    task->setScheduling(Task::ASAP);
                    if ((tt = nextToken(token)) == LBRACE)
                    {
                        if (!readTaskDepOptions(td))
                            return FALSE;
                        tt = nextToken(token);
                    }
                    if (tt != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("precedes") || token == "preceeds")
            {
                if (task->isContainer())
                {
                    errorMessage
                        (i18n("Dependencies of container tasks must be "
                              "specified prior to any sub tasks."));
                    return FALSE;
                }
                for ( ; ; )
                {
                    QString id;
                    if ((tt = nextToken(id)) != ID &&
                        tt != RELATIVE_ID && tt != ABSOLUTE_ID)
                    {
                        errorMessage(i18n("Task ID expected"));
                        return FALSE;
                    }
                    if (tt == ABSOLUTE_ID || tt == ID)
                        id = getTaskPrefix() + id;
                    TaskDependency* td = task->addPrecedes(id);
                    task->setScheduling(Task::ALAP);
                    if ((tt = nextToken(token)) == LBRACE)
                    {
                        if (!readTaskDepOptions(td))
                            return FALSE;
                        tt = nextToken(token);
                    }
                    if (tt != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("scheduling"))
            {
                nextToken(token);
                if (token == KW("asap"))
                    task->setScheduling(Task::ASAP);
                else if (token == KW("alap"))
                    task->setScheduling(Task::ALAP);
                else
                {
                    errorMessage(i18n("Unknown scheduling policy"));
                    return FALSE;
                }
            }
            else if (token == KW("flags"))
            {
                for ( ; ; )
                {
                    QString flag;
                    if (nextToken(flag) != ID || !proj->isAllowedFlag(flag))
                    {
                        errorMessage(i18n("Flag unknown"));
                        return FALSE;
                    }
                    task->addFlag(flag);
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("priority"))
            {
                int priority;
                if (!readPriority(priority))
                    return FALSE;
                task->setPriority(priority);
            }
            else if (token == KW("account"))
            {
                QString account;
                if (nextToken(account) != ID ||
                    proj->getAccount(account) == 0)
                {
                    errorMessage(i18n("Account ID expected"));
                    return FALSE;
                }
                task->setAccount(proj->getAccount(account));
            }
            else if (token == KW("projectid"))
            {
                if (nextToken(token) != ID ||
                    !proj->isValidId(token))
                {
                    errorMessage(i18n("Project ID expected"));
                    return FALSE;
                }
                task->setProjectId(token);
            }
            else if (token == KW("reference"))
            {
                QString ref, label;
                if (!readReference(ref, label))
                    return FALSE;
                task->setReference(ref, label);
            }
            else if (token == KW("supplement"))
            {
                if (nextToken(token) != ID || (token != KW("task")))
                {
                    errorMessage(i18n("'task' expected"));
                    return FALSE;
                }
                if ((token == "task" &&
                     !readTaskSupplement(task->getId())))
                    return FALSE;
                break;
            }
            else if (token == KW("journalentry"))
            {
                JournalEntry* entry;
                if ((entry = readJournalEntry()) == 0)
                    return FALSE;

                task->addJournalEntry(entry);
                break;
            }
            else if (token == "include")
            {
                errorMessage
                    (i18n("ERROR: The 'include' attribute is no longer "
                          "supported within tasks since it caused ambiguoties "
                          "between flag declaration and flag attributes. "
                          "Please use the 'taskprefix' attribute of 'include' "
                          "ouside of tasks instead."));
                return FALSE;
            }
            else
            {
                errorMessage(i18n("Illegal task attribute '%1'").arg(token));
                return FALSE;
            }
            break;
        case RBRACE:
            done = true;
            break;
        default:
            errorMessage(i18n("Task attribute expected. '%1' is no "
                              "known task attribute.").arg(token));
            return FALSE;
        }
    }

    return TRUE;
}

int
ProjectFile::readTaskScenarioAttribute(const QString attribute, Task* task,
                                       int sc, bool enforce)
{
    if (attribute == KW("length"))
    {
        double d;
        if (!readTimeFrame(d, TRUE))
            return -1;
        task->setLength(sc, d);
    }
    else if (attribute == KW("effort"))
    {
        double d;
        if (!readTimeFrame(d, TRUE))
            return -1;
        task->setEffort(sc, d);
    }
    else if (attribute == KW("duration"))
    {
        double d;
        if (!readTimeFrame(d, FALSE))
            return -1;
        task->setDuration(sc, d);
    }
    else if (attribute == KW("start"))
    {
        time_t val;
        if (!readDate(val, 0))
            return -1;
        task->setSpecifiedStart(sc, val);
        task->setScheduling(Task::ASAP);
    }
    else if (attribute == KW("end"))
    {
        time_t val;
        if (!readDate(val, 1))
            return -1;
        task->setSpecifiedEnd(sc, val);
        task->setScheduling(Task::ALAP);
    }
    else if (attribute == KW("minstart"))
    {
        time_t val;
        if (!readDate(val, 0))
            return -1;
        task->setMinStart(sc, val);
    }
    else if (attribute == KW("maxstart"))
    {
        time_t val;
        if (!readDate(val, 0))
            return -1;
        task->setMaxStart(sc, val);
    }
    else if (attribute == KW("minend"))
    {
        time_t val;
        if (!readDate(val, 1))
            return -1;
        task->setMinEnd(sc, val);
    }
    else if (attribute == KW("maxend"))
    {
        time_t val;
        if (!readDate(val, 1))
            return -1;
        task->setMaxEnd(sc, val);
    }
    else if (attribute == KW("scheduled"))
        task->setSpecifiedScheduled(sc, TRUE);
    else if (attribute == KW("startbuffer"))
    {
        double value;
        if (!readPercent(value))
            return -1;
        task->setStartBuffer(sc, value);
    }
    else if (attribute == KW("endbuffer"))
    {
        double value;
        if (!readPercent(value))
            return -1;
        task->setEndBuffer(sc, value);
    }
    else if (attribute == KW("complete"))
    {
        QString token;
        if (nextToken(token) != INTEGER)
        {
            errorMessage(i18n("Integer value expected"));
            return -1;
        }
        int complete = token.toInt();
        if (complete < 0 || complete > 100)
        {
            errorMessage(i18n("Value of complete must be between 0 "
                              "and 100"));
            return -1;
        }
        task->setComplete(sc, complete);
    }
    else if (attribute == KW("statusnote"))
    {
        QString token;
        if (nextToken(token) == STRING)
            task->setStatusNote(sc, token);
        else
        {
            errorMessage(i18n("String expected"));
            return -1;
        }
    }
    else if (attribute == KW("startcredit"))
    {
        QString token;
        TokenType tt;
        if ((tt =nextToken(token)) != REAL && tt != INTEGER)
        {
            errorMessage(i18n("Real value expected"));
            return -1;
        }
        task->setStartCredit(sc, token.toDouble());
    }
    else if (attribute == KW("endcredit"))
    {
        QString token;
        TokenType tt;
        if ((tt = nextToken(token)) != REAL && tt != INTEGER)
        {
            errorMessage(i18n("Real value expected"));
            return -1;
        }
        task->setEndCredit(sc, token.toDouble());
    }
    else if (enforce)
    {
        // Only if the enforce flag is set an unknown attribute is an error.
        errorMessage(i18n("Scenario specific task attribute expected."));
        return -1;
    }
    else
        return 0;

    return 1;
}

JournalEntry*
ProjectFile::readJournalEntry()
{
    time_t date;
    if (!readDate(date, 0, FALSE))
        return 0;

    QString text;
    if (nextToken(text) != STRING)
    {
        errorMessage(i18n("String expected"));
        return 0;
    }
    return new JournalEntry(date, text);
}

bool
ProjectFile::readVacation(time_t& from, time_t& to, bool readName,
                          QString* n)
{
    TokenType tt;
    if (readName)
    {
        if ((tt = nextToken(*n)) != STRING)
        {
            errorMessage(i18n("String expected"));
            return FALSE;
        }
    }
    if (!readDate(from, 0, FALSE))
        return FALSE;
    QString token;
    if ((tt = nextToken(token)) != MINUS)
    {
        // vacation e. g. 2001-11-28
        returnToken(tt, token);
        to = sameTimeNextDay(from) - 1;
    }
    else
    {
        // vacation e. g. 2001-11-28 - 2001-11-30
        if (!readDate(to, 1, FALSE))
            return FALSE;
        if (from > to)
        {
            errorMessage(i18n("Vacation must start before end"));
            return FALSE;
        }
    }
    return TRUE;
}

bool
ProjectFile::readDate(time_t& val, time_t correction, bool checkPrjInterval)
{
    QString token;

    if (nextToken(token) != DATE)
    {
        errorMessage(i18n("Date expected"));
        return false;
    }

    if (!date2time(token, val))
        return false;

    val -= correction;

    if (checkPrjInterval)
    {
        if (val + correction < proj->getStart() ||
            val > proj->getEnd())
        {
            errorMessage(i18n("Date %1 is outside of project time frame "
                              "(%2 - %3")
                         .arg(time2tjp(val))
                         .arg(time2tjp(proj->getStart()))
                         .arg(time2tjp(proj->getEnd())));
            return false;
        }
    }

    return true;
}

bool
ProjectFile::readRealFormat(RealFormat* format)
{
    // E.g. "(" ")" "," "." 3
    QString token;
    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }
    format->setSignPrefix(token);

    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }
    format->setSignSuffix(token);

    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }
    format->setThousandSep(token);

    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }
    format->setFractionSep(token);

    if (nextToken(token) != INTEGER || token.toInt() < 0 || token.toInt() > 5)
    {
        errorMessage(i18n("Number between 0 and 5 expected"));
        return FALSE;
    }
    format->setFracDigits(token.toInt());

    return TRUE;
}

bool
ProjectFile::readReference(QString& ref, QString& label)
{

    if (nextToken(ref) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }
    label = ref;

    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LBRACE)
    {
        while ((tt = nextToken(token)) != RBRACE)
        {
            if (tt == ID && token == KW("label"))
            {
                if (nextToken(label) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
            }
            else
            {
                errorMessage(i18n("ID or '}' expected"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    return TRUE;
}

bool
ProjectFile::readPercent(double& value)
{
    QString token;
    TokenType tt;

    if ((tt = nextToken(token)) != INTEGER && tt != REAL)
    {
        errorMessage(i18n("Number expected"));
        return FALSE;
    }
    value = token.toDouble();
    if (value < 0.0 || value > 100.0)
    {
        errorMessage(i18n("Value must be between 0 and 100"));
        return FALSE;
    }
    return TRUE;
}

bool
ProjectFile::readResource(Resource* parent)
{
    QString definitionFile = getFile();
    uint definitionLine = getLine();

    // Syntax: 'resource id "name" { ... }
    QString id;
    if (nextToken(id) != ID)
    {
        errorMessage(i18n("ID expected"));
        return FALSE;
    }
    QString name;
    if (nextToken(name) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }

    Resource* r;
    if ((r = proj->getResource(id)) != 0)
    {
        if (proj->getAllowRedefinitions())
        {
            if (r->getName() != name)
            {
                errorMessage(i18n
                         ("Redefinition of resource '%1' with different "
                          "name '%2'. Previous name was '%3'.")
                     .arg(id).arg(name).arg(r->getName()));
                return FALSE;
            }
        }
        else
        {
            errorMessage(i18n("Resource %1 has already been defined")
                         .arg(id));
            return FALSE;
        }
    }
    else
    {
        r = new Resource(proj, id, name, parent, definitionFile,
                         definitionLine);
        r->inheritValues();
    }

    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LBRACE)
    {
        // read optional attributes
        if (!readResourceBody(r))
            return FALSE;
    }
    else
        returnToken(tt, token);

    return TRUE;
}

bool
ProjectFile::readResourceSupplement()
{
    QString token;
    Resource* r;
    if (nextToken(token) != ID || (r = proj->getResource(token)) == 0)
    {
        errorMessage(i18n("Already defined resource ID expected"));
        return FALSE;
    }
    if (nextToken(token) != LBRACE)
    {
        errorMessage(i18n("'{' expected"));
        return FALSE;
    }
    return readResourceBody(r);
}

bool
ProjectFile::readResourceBody(Resource* r)
{
    QString token;
    TokenType tt;

    while ((tt = nextToken(token)) != RBRACE)
    {
        QString next;
        TokenType nextTT;
        if (tt != ID)
        {
            errorMessage(i18n("Unknown attribute '%1'").arg(token));
            return FALSE;
        }
        int sc = 0;
        if ((nextTT = nextToken(next)) == COLON)
        {
            if ((sc = proj->getScenarioIndex(token) - 1) < 0)
            {
                errorMessage(i18n("Scenario ID expected. '%1' is not "
                                  "a defined scenario.").arg(token));
                return FALSE;
            }
            tt = nextToken(token);
            // Now make sure that the attribute is really scenario specific.
            if (token != "booking")
            {
                errorMessage(i18n("Scenario specific resource attribute "
                                  "expected."));
            }
        }
        else
            returnToken(nextTT, next);

        if (proj->getResourceAttribute(token))
        {
            if (!readCustomAttribute(r, token,
                                     proj->getResourceAttribute(token)->
                                     getType()))
                return FALSE;
        }
        else if (token == KW("booking"))
        {
            Booking* b;
            int sloppy;
            if ((b = readBooking(sc, sloppy)) == 0)
                return FALSE;
            if (!r->addBooking(sc, b, sloppy))
                return FALSE;
        }
        else if (token == KW("resource"))
        {
            if (!readResource(r))
                return FALSE;
        }
        else if (token == KW("mineffort"))
        {
            TokenType tt;
            if ((tt = nextToken(token)) != REAL && tt != INTEGER)
            {
                errorMessage(i18n("Real value exptected"));
                return FALSE;
            }
            r->setMinEffort(token.toDouble());
        }
        else if (token == KW("maxeffort"))
        {
            TokenType tt;
            if ((tt = nextToken(token)) != REAL && tt != INTEGER)
            {
                errorMessage(i18n("Real value exptected"));
                return FALSE;
            }
            UsageLimits* limits = new UsageLimits;
            limits->setDailyMax
                ((uint) ((token.toDouble() *
                          proj->getDailyWorkingHours() * 3600) /
                         proj->getScheduleGranularity()));
            r->setLimits(limits);
        }
        else if (token == KW("limits"))
        {
            UsageLimits* limits;
            if ((limits = readLimits()) == 0)
                return FALSE;
            r->setLimits(limits);
        }
        else if (token == KW("efficiency"))
        {
            TokenType tt;
            if ((tt = nextToken(token)) != REAL && tt != INTEGER)
            {
                errorMessage(i18n("Read value expected"));
                return FALSE;
            }
            r->setEfficiency(token.toDouble());
        }
        else if (token == KW("rate"))
        {
            TokenType tt;
            if ((tt = nextToken(token)) != REAL && tt != INTEGER)
            {
                errorMessage(i18n("Real value exptected"));
                return FALSE;
            }
            r->setRate(token.toDouble());
        }
        else if (token == KW("kotrusid"))
        {
            if (nextToken(token) != STRING)
            {
                errorMessage(i18n("String expected"));
                return FALSE;
            }
            r->setKotrusId(token);
        }
        else if (token == KW("vacation"))
        {
            time_t from, to;
            if (!readVacation(from, to))
                return FALSE;
            r->addVacation(new Interval(from, to));
        }
        else if (token == KW("workinghours"))
        {
            int dow;
            QPtrList<Interval>* l = new QPtrList<Interval>();
            if (!readWorkingHours(dow, l))
                return FALSE;

            r->setWorkingHours(dow, l);
        }
        else if (token == KW("shift"))
        {
            time_t from, to;
            from = proj->getStart();
            to = proj->getEnd();
            Shift* s;
            if ((s = readShiftSelection(from, to)) == 0)
                return FALSE;
            if (!r->addShift(Interval(from, to), s))
            {
                errorMessage(i18n("Shift interval overlaps with other"));
                return FALSE;
            }
        }
        else if (token == KW("flags"))
        {
            for ( ; ; )
            {
                QString flag;
                if (nextToken(flag) != ID || !proj->isAllowedFlag(flag))
                {
                    errorMessage(i18n("flag unknown"));
                    return FALSE;
                }
                r->addFlag(flag);
                if ((tt = nextToken(token)) != COMMA)
                {
                    returnToken(tt, token);
                    break;
                }
            }
        }
        else if (token == KW("journalentry"))
        {
            JournalEntry* entry;
            if ((entry = readJournalEntry()) == 0)
                return FALSE;

            r->addJournalEntry(entry);
        }
        else if (token == KW("include"))
        {
            errorMessage
                (i18n("ERROR: The 'include' attribute is no longer "
                      "supported within resources since it caused ambiguoties "
                      "between flag declaration and flag attributes."));
            return FALSE;
        }
        else
        {
            errorMessage(i18n("Unknown attribute '%1'").arg(token));
            return FALSE;
        }
    }

    return TRUE;
}

bool
ProjectFile::readShift(Shift* parent)
{
    QString definitionFile = getFile();
    uint definitionLine = getLine();

    // Syntax: 'shift id "name" { ... }
    QString id;
    if (nextToken(id) != ID)
    {
        errorMessage(i18n("ID expected"));
        return FALSE;
    }
    QString name;
    if (nextToken(name) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }

    if (proj->getShift(id))
    {
        errorMessage(i18n("Shift %1 has already been defined")
                     .arg(id));
        return FALSE;
    }

    Shift* s = new Shift(proj, id, name, parent, definitionFile,
                         definitionLine);
    s->inheritValues();

    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LBRACE)
    {
        // read optional attributes
        while ((tt = nextToken(token)) != RBRACE)
        {
            if (tt != ID)
            {
                errorMessage(i18n("Unknown attribute '%1'").arg(token));
                return FALSE;
            }
            if (token == KW("shift"))
            {
                if (!readShift(s))
                    return FALSE;
            }
            else if (token == KW("workinghours"))
            {
                int dow;
                QPtrList<Interval>* l = new QPtrList<Interval>();
                if (!readWorkingHours(dow, l))
                    return FALSE;

                s->setWorkingHours(dow, l);
            }
            else if (token == KW("include"))
            {
                errorMessage
                    (i18n("ERROR: The 'include' attribute is no longer "
                          "supported within shifts since it caused ambiguoties "
                          "between flag declaration and flag attributes."));
                return FALSE;
            }
            else
            {
                errorMessage(i18n("Unknown attribute '%1'").arg(token));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    return TRUE;
}

Shift*
ProjectFile::readShiftSelection(time_t& from, time_t& to)
{
    // Syntax: ID [from [- to]]
    QString id;
    if (nextToken(id) != ID)
    {
        errorMessage(i18n("Shift ID expected"));
        return 0;
    }
    Shift* s = 0;
    if ((s = proj->getShift(id)) == 0)
    {
        errorMessage(i18n("Unknown shift"));
        return 0;
    }
    QString token;
    TokenType tt;
    // Clumsy look-ahead
    tt = nextToken(token);
    returnToken(tt, token);
    if (tt == DATE)
        if (!readVacation(from, to))
            return 0;

    return s;
}

Booking*
ProjectFile::readBooking(int sc, int& sloppy)
{
    time_t start;
    if (!readDate(start, 0))
        return 0;
    if (start < proj->getStart() || start >= proj->getEnd())
    {
        errorMessage(i18n("Start date must be within the project timeframe"));
        return 0;
    }

    time_t end;
    if (!readDate(end, 1))
        return 0;
    if (end <= proj->getStart() || end > proj->getEnd())
    {
        errorMessage(i18n("End date must be within the project timeframe"));
        return 0;
    }
    if (start >= end)
    {
        errorMessage(i18n("End date must be after start date"));
        return 0;
    }

    if (proj->getScenario(sc)->getProjectionMode() && end > proj->getNow())
    {
        errorMessage(i18n("In projection Mode all bookings must end prior "
                          "to the current or 'now' date."));
        return 0;
    }

    Task* task;
    QString token;
    TokenType tt;
    if (((tt = nextToken(token)) != ID && tt != ABSOLUTE_ID) ||
        (task = proj->getTask(getTaskPrefix() + token)) == 0)
    {
        errorMessage(i18n("Task ID expected"));
        return 0;
    }

    sloppy = 0;
    if ((tt = nextToken(token)) == LBRACE)
    {
        while ((tt = nextToken(token)) != RBRACE)
        {
            if (token == "sloppy")
            {
                if (nextToken(token) != INTEGER ||
                    token.toInt() < 0 || token.toInt() > 3)
                {
                    errorMessage(i18n("Number between 0 and 3 expected"));
                    return 0;
                }
                sloppy = token.toInt();
            }
            else
            {
                errorMessage(i18n("Attribute ID expected"));
                return 0;
            }
        }
    }
    else
        returnToken(tt, token);

    return new Booking(Interval(start, end), task);
}

bool
ProjectFile::readAccount(Account* parent)
{
    QString definitionFile = getFile();
    uint definitionLine = getLine();

    // Syntax: 'account id "name" { ... }
    QString id;
    if (nextToken(id) != ID)
    {
        errorMessage(i18n("ID expected"));
        return FALSE;
    }

    if (proj->getAccount(id))
    {
        errorMessage(i18n("Account %1 has already been defined")
                     .arg(id));
        return FALSE;
    }

    QString name;
    if (nextToken(name) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }
    AccountType acctType;
    if (parent == 0)
    {
        /* Only accounts with no parent can have a type specifier. All
         * sub accounts inherit the type of the parent. */
        QString at;
        if (nextToken(at) != ID || (at != KW("cost") &&
                                    at != KW("revenue")))
        {
            errorMessage(i18n("Account type 'cost' or 'revenue' expected"));
            return FALSE;
        }
        acctType = at == KW("cost") ? Cost : Revenue;
    }
    else
        acctType = parent->getAcctType();

    Account* a;
    if ((a = proj->getAccount(id)) != 0)
    {
        if (proj->getAllowRedefinitions())
        {
            if (a->getName() != name)
            {
                errorMessage(i18n
                         ("Redefinition of account '%1' with different "
                          "name '%2'. Previous name was '%3'.")
                     .arg(id).arg(name).arg(a->getName()));
                return FALSE;
            }
        }
        else
        {
            errorMessage(i18n("Account '%1' has already been defined")
                         .arg(id));
            return FALSE;
        }
    }
    else
    {
        a = new Account(proj, id, name, parent, acctType, definitionFile,
                        definitionLine);
        a->inheritValues();
    }

    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LBRACE)
    {
        bool hasSubAccounts = FALSE;
        bool cantBeParent = FALSE;
        // read optional attributes
        while ((tt = nextToken(token)) != RBRACE)
        {
            if (tt != ID)
            {
                errorMessage(i18n("Unknown attribute '%1'")
                             .arg(token));
                return FALSE;
            }
            if (token == KW("account") && !cantBeParent)
            {
                if (!readAccount(a))
                    return FALSE;
                hasSubAccounts = TRUE;
            }
            else if (token == KW("credit"))
            {
                if (!readCredit(a))
                    return FALSE;
            }
            else if (token == KW("kotrusid") && !hasSubAccounts)
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                a->setKotrusId(token);
                cantBeParent = TRUE;
            }
            else if (token == KW("include"))
            {
                if (!readInclude())
                    return FALSE;
            }
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    return TRUE;
}

bool
ProjectFile::readCredit(Account* a)
{
    time_t date;
    if (!readDate(date, 0))
        return FALSE;

    QString description;
    if (nextToken(description) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }

    QString token;
    TokenType tt;
    if ((tt = nextToken(token)) != REAL && tt != INTEGER)
    {
        errorMessage(i18n("Real value expected"));
        return FALSE;
    }
    Transaction* t = new Transaction(date, token.toDouble(), description);
    a->credit(t);

    return TRUE;
}

bool
ProjectFile::readAllocate(Task* t)
{
    QString id;
    Resource* r;
    if (nextToken(id) != ID || (r = proj->getResource(id)) == 0)
    {
        errorMessage(i18n("Resource ID '%1' is unknown").arg(id));
        return FALSE;
    }
    Allocation* a = new Allocation();
    a->addCandidate(r);

    QString token;
    TokenType tt;
    if ((tt = nextToken(token)) == LBRACE)
    {
        while ((tt = nextToken(token)) != RBRACE)
        {
            if (tt != ID)
            {
                errorMessage(i18n("Unknown attribute '%1'")
                             .arg(token));
                return FALSE;
            }
            if (token == KW("load"))
            {
                TokenType tt;
                if ((tt = nextToken(token)) != REAL && tt != INTEGER)
                {
                    errorMessage(i18n("Real value expected"));
                    return FALSE;
                }
                UsageLimits* limits = new UsageLimits;
                limits->setDailyMax
                    ((uint) ((token.toDouble() *
                              proj->getDailyWorkingHours() * 3600) /
                             proj->getScheduleGranularity()));
                if (limits->getDailyMax() == 0)
                {
                    errorMessage(i18n("Value must be at least %f")
                                 .arg(proj->convertToDailyLoad
                                      (proj->getScheduleGranularity())));
                    delete limits;
                    return FALSE;
                }
                a->setLimits(limits);
            }
            else if (token == KW("limits"))
            {
                UsageLimits* limits;
                if ((limits = readLimits()) == 0)
                    return FALSE;
                a->setLimits(limits);
            }
            else if (token == KW("shift"))
            {
                time_t from = proj->getStart();
                time_t to = proj->getEnd();
                Shift* s;
                if ((s = readShiftSelection(from, to)) == 0)
                    return FALSE;
                if (!a->addShift(Interval(from, to), s))
                {
                    errorMessage(i18n("Shift intervals may not overlap"));
                    return FALSE;
                }
            }
            else if (token == KW("persistent"))
            {
                a->setPersistent(TRUE);
            }
            else if (token == KW("mandatory"))
            {
                a->setMandatory(TRUE);
            }
            else if (token == KW("alternative"))
            {
                do
                {
                    Resource* r;
                    if ((tt = nextToken(token)) != ID ||
                        (r = proj->getResource(token)) == 0)
                    {
                        errorMessage(i18n("Resource ID expected"));
                        return FALSE;
                    }
                    a->addCandidate(r);
                } while ((tt = nextToken(token)) == COMMA);
                returnToken(tt, token);
            }
            else if (token == KW("select"))
            {
                if (nextToken(token) != ID || !a->setSelectionMode(token))
                {
                    errorMessage(i18n("Invalid selction mode"));
                    return FALSE;
                }
            }
            else
            {
                errorMessage(i18n("Unknown attribute '%1'")
                             .arg(token));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);
    t->addAllocation(a);

    return TRUE;
}

UsageLimits*
ProjectFile::readLimits()
{
    UsageLimits* limits = new UsageLimits;

    QString token;
    if (nextToken(token) != LBRACE)
    {
        errorMessage(i18n("'{' expected"));
        delete limits;
        return 0;
    }
    TokenType tt;
    while ((tt = nextToken(token)) == ID)
    {
        double val;
        if (!readTimeFrame(val, TRUE))
        {
            delete limits;
            return 0;
        }
        uint uval = (uint) ((val * proj->getDailyWorkingHours() * 3600) /
                            proj->getScheduleGranularity());
        if (uval == 0)
        {
            errorMessage(i18n("Value must be larger than scheduling "
                              "granularity"));
            delete limits;
            return 0;
        }
        if (token == KW("dailymax"))
            limits->setDailyMax(uval);
        else if (token == KW("weeklymax"))
            limits->setWeeklyMax(uval);
        else if (token == KW("monthlymax"))
            limits->setMonthlyMax(uval);
        else
        {
            errorMessage(i18n("Unknown limit type '%1'").arg(token));
            delete limits;
            return 0;
        }
    }
    if (tt != RBRACE)
    {
        errorMessage(i18n("'}' expected"));
        delete limits;
        return 0;
    }

    return limits;
}

bool
ProjectFile::readTimeValue(ulong& value)
{
    QString val;
    if (nextToken(val) != INTEGER)
    {
        errorMessage(i18n("Integer value expected"));
        return FALSE;
    }
    QString unit;
    if (nextToken(unit) != ID)
    {
        errorMessage(i18n("Unit expected"));
        return FALSE;
    }
    if (unit == KW("min"))
        value = val.toULong() * 60;
    else if (unit == KW("h"))
        value = val.toULong() * (60 * 60);
    else if (unit == KW("d"))
        value = val.toULong() * (60 * 60 * 24);
    else if (unit == KW("w"))
        value = val.toULong() * (60 * 60 * 24 * 7);
    else if (unit == KW("m"))
        value = val.toULong() * (60 * 60 * 24 * 30);
    else if (unit == KW("y"))
        value = val.toULong() * (60 * 60 * 24 * 356);
    else
    {
        errorMessage(i18n("Unit expected"));
        return FALSE;
    }
    return TRUE;
}

bool
ProjectFile::readTimeFrame(double& value, bool workingDays, bool allowZero)
{
    QString val;
    TokenType tt;
    if ((tt = nextToken(val)) != REAL && tt != INTEGER)
    {
        errorMessage(i18n("Real value expected"));
        return FALSE;
    }
    if (allowZero)
    {
        if (val.toDouble() < 0.0)
        {
            errorMessage(i18n("Value must not be negative."));
            return FALSE;
        }
    }
    else
    {
        if (val.toDouble() <= 0.0)
        {
            errorMessage(i18n("Value must be greater than 0."));
            return FALSE;
        }
    }

    QString unit;
    if (nextToken(unit) != ID)
    {
        errorMessage(i18n("Unit expected"));
        return FALSE;
    }
    if (unit == KW("min"))
        value = val.toDouble() /
            (workingDays ? proj->getDailyWorkingHours() * 60 : 24 * 60);
    else if (unit == KW("h"))
        value = val.toDouble() /
            (workingDays ? proj->getDailyWorkingHours() : 24);
    else if (unit == KW("d"))
        value = val.toDouble();
    else if (unit == KW("w"))
        value = val.toDouble() *
            (workingDays ? proj->getWeeklyWorkingDays() : 7);
    else if (unit == KW("m"))
        value = val.toDouble() *
            (workingDays ? proj->getMonthlyWorkingDays() : 30.4167);
    else if (unit == KW("y"))
        value = val.toDouble() *
            (workingDays ? proj->getYearlyWorkingDays() : 365);
    else
    {
        errorMessage(i18n("Unit expected"));
        return FALSE;
    }

    return TRUE;
}

bool
ProjectFile::readWorkingHours(int& dayOfWeek, QPtrList<Interval>* l)
{
    l->setAutoDelete(TRUE);
    QString day;
    if (nextToken(day) != ID)
    {
        errorMessage(i18n("Weekday expected"));
        return FALSE;
    }
    const char* days[] = { KW("sun"), KW("mon"), KW("tue"), KW("wed"),
        KW("thu"), KW("fri"), KW("sat") };
    for (dayOfWeek = 0; dayOfWeek < 7; dayOfWeek++)
        if (days[dayOfWeek] == day)
            break;
    if (dayOfWeek == 7)
    {
        errorMessage(i18n("Weekday expected"));
        return FALSE;
    }

    QString token;
    TokenType tt;
    if ((tt = nextToken(token)) == ID && token == KW("off"))
        return TRUE;
    else
        returnToken(tt, token);

    for ( ; ; )
    {
        QString start;
        if (nextToken(start) != HOUR)
        {
            errorMessage(i18n("Start time as HH:MM expected"));
            return FALSE;
        }
        QString token;
        if (nextToken(token) != MINUS)
        {
            errorMessage(i18n("'-' expected"));
            return FALSE;
        }
        QString end;
        if (nextToken(end) != HOUR)
        {
            errorMessage(i18n("End time as HH:MM expected"));
            return FALSE;
        }
        time_t st, et;
        if ((st = hhmm2time(start)) < 0)
            return FALSE;
        if ((et = hhmm2time(end)) < 0)
            return FALSE;
        if (et <= st)
        {
            errorMessage(i18n("End time must be larger than start time"));
            return FALSE;
        }
        Interval* iv = new Interval(st, et - 1);
        for (QPtrListIterator<Interval> ili(*l); *ili != 0; ++ili)
            if (iv->overlaps(**ili))
            {
                errorMessage(i18n("Working hour intervals may not overlap"));
                return FALSE;
            }
        l->append(iv);
        TokenType tt;
        if ((tt = nextToken(token)) != COMMA)
        {
            returnToken(tt, token);
            break;
        }
    }
    return TRUE;
}

bool
ProjectFile::readPriority(int& priority)
{
    QString token;

    if (nextToken(token) != INTEGER)
    {
        errorMessage(i18n("Integer value expected"));
        return FALSE;
    }
    priority = token.toInt();
    if (priority < 1 || priority > 1000)
    {
        errorMessage(i18n("Priority value must be between 1 and 1000"));
        return FALSE;
    }
    return TRUE;
}

bool
ProjectFile::readICalTaskReport()
{
#ifndef HAVE_KDE
    errorMessage(i18n("The program was compiled without KDE support. "
                      "Therefor ICal support has been disabled."));
    return FALSE;
#else
    QString fileName;
    if (nextToken(fileName) != STRING)
    {
        errorMessage(i18n("File name expected"));
        return FALSE;
    }
    if (fileName.right(4) != ".ics" && fileName.right(4) != ".ICS")
    {
        errorMessage(i18n("File name '%1' for ICal files must end with "
                          "\".ics\" extension.").arg(fileName));
        return FALSE;
    }

    ICalReport* report;
    report = new ICalReport(proj, fileName, getFile(), getLine());
    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LBRACE)
    {
        while ((tt = nextToken(token)) != RBRACE)
        {
            if (token == KW("hidetask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setRollUpTask(et);
            }
            else if (token == KW("hideresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setRollUpResource(et);
            }
            else if (token == KW("scenario"))
            {
                report->clearScenarios();
                QString scId;
                if ((tt = nextToken(scId)) != ID)
                {
                    errorMessage(i18n("Scenario ID expected"));
                    return FALSE;
                }
                int scIdx;
                if ((scIdx = proj->getScenarioIndex(scId)) == -1)
                {
                    errorMessage(i18n("Unknown scenario %1")
                                 .arg(scId));
                    return FALSE;
                }
                if (proj->getScenario(scIdx - 1)->getEnabled())
                    report->addScenario(proj->getScenarioIndex(scId) - 1);
            }
            else
            {
                errorMessage(i18n("Illegal attribute '%1'").arg(token));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    proj->addReport(report);

    return(TRUE);
#endif
}

bool
ProjectFile::readXMLReport()
{
    QString fileName;
    if (nextToken(fileName) != STRING)
    {
        errorMessage(i18n("File name expected"));
        return FALSE;
    }
    /* We don't know yet what version of the report the user wants, so we
     * generate data structures for both reports. */
    int version = 1;

    // Data structure for version 1 format.
    ReportXML *rep = new ReportXML(proj, fileName, getFile(), getLine());
    // Data structure for version 2 format.
    XMLReport* report;
    report = new XMLReport(proj, fileName, getFile(), getLine());
    report->setMasterFile(TRUE);
    report->addTaskAttribute("all");
    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LBRACE)
    {
        while ((tt = nextToken(token)) != RBRACE)
        {
            if (token == KW("version"))
            {
                if (nextToken(token) != INTEGER ||
                    token.toInt() < 1 || token.toInt() > 2)
                {
                    errorMessage("Currently only version 1 and 2 are "
                                 "supported.");
                    return FALSE;
                }
                version = token.toInt();
            }
            else if (token == KW("hidetask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setRollUpTask(et);
            }
            else if (token == KW("hideresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setRollUpResource(et);
            }
            else if (token == KW("scenarios"))
            {
                report->clearScenarios();
                for ( ; ; )
                {
                    QString scId;
                    if ((tt = nextToken(scId)) != ID)
                    {
                        errorMessage(i18n("Scenario ID expected"));
                        return FALSE;
                    }
                    int scIdx;
                    if ((scIdx = proj->getScenarioIndex(scId)) == -1)
                    {
                        errorMessage(i18n("Unknown scenario %1")
                                     .arg(scId));
                        return FALSE;
                    }
                    if (proj->getScenario(scIdx - 1)->getEnabled())
                        report->addScenario(proj->getScenarioIndex(scId) - 1);
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("notimestamp"))
            {
                report->setTimeStamp(FALSE);
            }
            else
            {
                errorMessage(i18n("Illegal attribute '%1'").arg(token));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    if (version == 1)
    {
        delete report;
        proj->addXMLReport(rep);
    }
    else
    {
        delete rep;
        proj->addReport(report);
    }

    return(TRUE);
}

bool
ProjectFile::checkReportInterval(ReportElement* tab)
{
    if (tab->getEnd() < tab->getStart())
    {
        errorMessage(i18n("End date must be later than start date"));
        return FALSE;
    }
    if (proj->getStart() > tab->getStart() ||
        tab->getStart() > proj->getEnd())
    {
        errorMessage(i18n("Start date must be within the project time "
                          "frame"));
        return FALSE;
    }
    if (proj->getStart() > tab->getEnd() ||
        tab->getEnd() > proj->getEnd())
    {
        errorMessage(i18n("End date must be within the project time frame"));
        return FALSE;
    }

    return TRUE;
}

bool
ProjectFile::checkReportInterval(HTMLReport* report)
{
    if (report->getEnd() < report->getStart())
    {
        errorMessage(i18n("End date must be later than start date"));
        return FALSE;
    }
    if (proj->getStart() > report->getStart() ||
        report->getStart() > proj->getEnd())
    {
        errorMessage(i18n("Start date must be within the project time "
                          "frame"));
        return FALSE;
    }
    if (proj->getStart() > report->getEnd() ||
        report->getEnd() > proj->getEnd())
    {
        errorMessage(i18n("End date must be within the project time frame"));
        return FALSE;
    }

    return TRUE;
}

bool
ProjectFile::readReport(const QString& reportType)
{
    QString token;
    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("Report name expected"));
        return FALSE;
    }

    Report* report = 0;
    ReportElement* tab;
    if (reportType == KW("taskreport"))
    {
        report = new QtTaskReport(proj, token, getFile(), getLine());
        tab = ((QtTaskReport*) report)->getTable();
    }
    else if (reportType == KW("resourcereport"))
    {
        report = new QtResourceReport(proj, token, getFile(), getLine());
        tab = ((QtResourceReport*) report)->getTable();
    }
/*    else if (reportType == KW("accountreport"))
    {
        report = new QtReport(proj, token, getFile(), getLine());
        tab = ((QtReportElement*) report)->getTable();
    }*/
    else
    {
        errorMessage(i18n("Report type %1 not yet supported!")
                     .arg(reportType));
        return FALSE;
    }

    TokenType tt;
    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                return FALSE;
            }
            if (token == KW("columns"))
            {
                tab->clearColumns();
                for ( ; ; )
                {
                    TableColumnInfo* tci;
                    if ((tci = readColumn(proj->getMaxScenarios(),
                                          tab)) == 0)
                        return FALSE;
                    tab->addColumn(tci);
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("scenario"))
            {
                QString scId;
                if ((tt = nextToken(scId)) != ID)
                {
                    errorMessage(i18n("Scenario ID expected"));
                    return FALSE;
                }
                int scIdx;
                if ((scIdx = proj->getScenarioIndex(scId)) == -1)
                {
                    errorMessage(i18n("Unknown scenario '%1'")
                                 .arg(scId));
                    return FALSE;
                }
                if (proj->getScenario(scIdx - 1)->getEnabled())
                {
                    tab->clearScenarios();
                    tab->addScenario(proj->getScenarioIndex(scId) - 1);
                }
            }
            else if (token == KW("start"))
            {
                time_t start;
                if (!readDate(start, 0))
                    return FALSE;
                tab->setStart(start);
            }
            else if (token == KW("end"))
            {
                time_t end;
                if (!readDate(end, 1))
                    return FALSE;
                tab->setEnd(end);
            }
            else if (token == KW("headline"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String exptected"));
                    return FALSE;
                }
                tab->setHeadline(token);
            }
            else if (token == KW("caption"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String exptected"));
                    return FALSE;
                }
                tab->setCaption(token);
            }
            else if (token == KW("showprojectids"))
            {
                tab->setShowPIDs(TRUE);
            }
            else if (token == KW("hidetask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpTask(et);
            }
            else if (token == KW("sorttasks"))
            {
                if (!readSorting(tab, 0))
                    return FALSE;
            }
            else if (token == KW("hideresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpResource(et);
            }
            else if (token == KW("sortresources"))
            {
                if (!readSorting(tab, 1))
                    return FALSE;
            }
            else if (token == KW("hideaccount"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideAccount(et);
            }
            else if (token == KW("rollupaccount"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpAccount(et);
            }
            else if (token == KW("sortaccounts"))
            {
                if (!readSorting(tab, 2))
                    return FALSE;
            }
            else if (token == KW("loadunit"))
            {
                if (nextToken(token) != ID || !tab->setLoadUnit(token))
                {
                    errorMessage(i18n("Illegal load unit"));
                    return FALSE;
                }
            }
            else if (token == KW("timeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    return FALSE;
                }
                tab->setTimeFormat(token);
            }
            else if (token == KW("shorttimeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    return FALSE;
                }
                tab->setShortTimeFormat(token);
            }
            else if (token == KW("taskroot"))
            {
                if ((tt = nextToken(token)) == ID ||
                    tt == ABSOLUTE_ID)
                {
                    if (!proj->getTask(token))
                    {
                        errorMessage(i18n("taskroot must be a known task"));
                        return FALSE;
                    }
                    tab->setTaskRoot(token + ".");
                }
                else
                {
                    errorMessage(i18n("Task ID expected"));
                    return FALSE;
                }
            }
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    if (!checkReportInterval(tab))
        return FALSE;

    proj->addReport(report);

    return TRUE;
}

bool
ProjectFile::readHTMLReport(const QString& reportType)
{
    QString token;
    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("File name expected"));
        return FALSE;
    }

    HTMLReport* report = 0;
    HTMLReportElement* tab = 0;
    if (reportType == KW("htmltaskreport"))
    {
        report = new HTMLTaskReport(proj, token, getFile(), getLine());
        tab = ((HTMLTaskReport*) report)->getTable();
    }
    else if (reportType == KW("htmlresourcereport"))
    {
        report = new HTMLResourceReport(proj, token, getFile(), getLine());
        tab = ((HTMLResourceReport*) report)->getTable();
    }
    else if (reportType == KW("htmlweeklycalendar"))
    {
        report = new HTMLWeeklyCalendar(proj, token, getFile(), getLine());
        tab = ((HTMLWeeklyCalendar*) report)->getTable();
    }
    else if (reportType == KW("htmlaccountreport"))
    {
        report = new HTMLAccountReport(proj, token, getFile(), getLine());
        tab = ((HTMLAccountReport*) report)->getTable();
    }
    else
    {
        qFatal("readHTMLReport: bad report type");
        return FALSE;   // Just to please the compiler.
    }

    TokenType tt;
    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                goto exit_error;
            }
            if (token == KW("columns"))
            {
                tab->clearColumns();
                for ( ; ; )
                {
                    TableColumnInfo* tci;
                    if ((tci = readColumn(proj->getMaxScenarios(), tab)) == 0)
                        goto exit_error;
                    tab->addColumn(tci);
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("scenarios"))
            {
                tab->clearScenarios();
                for ( ; ; )
                {
                    QString scId;
                    if ((tt = nextToken(scId)) != ID)
                    {
                        errorMessage(i18n("Scenario ID expected"));
                        goto exit_error;
                    }
                    int scIdx;
                    if ((scIdx = proj->getScenarioIndex(scId)) == -1)
                    {
                        errorMessage(i18n("Unknown scenario '%1'")
                                     .arg(scId));
                        goto exit_error;
                    }
                    if (proj->getScenario(scIdx - 1)->getEnabled())
                        tab->addScenario(proj->getScenarioIndex(scId) - 1);
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("start"))
            {
                time_t start;
                if (!readDate(start, 0))
                    goto exit_error;
                tab->setStart(start);
            }
            else if (token == KW("end"))
            {
                time_t end;
                if (!readDate(end, 1))
                    goto exit_error;
                tab->setEnd(end);
            }
            else if (token == KW("headline"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String exptected"));
                    goto exit_error;
                }
                tab->setHeadline(token);
            }
            else if (token == KW("caption"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String exptected"));
                    goto exit_error;
                }
                tab->setCaption(token);
            }
            else if (token == KW("rawhead"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                tab->setRawHead(token);
            }
            else if (token == KW("rawtail"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                tab->setRawTail(token);
            }
            else if (token == KW("rawstylesheet"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                report->setRawStyleSheet(token);
            }
            else if (token == KW("showprojectids"))
            {
                tab->setShowPIDs(TRUE);
            }
            else if (token == KW("accumulate"))
            {
                tab->setAccumulate(TRUE);
            }
            else if (token == KW("hidetask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    goto exit_error;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    goto exit_error;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpTask(et);
            }
            else if (token == KW("sorttasks"))
            {
                if (!readSorting(tab, 0))
                    goto exit_error;
            }
            else if (token == KW("hideresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    goto exit_error;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    goto exit_error;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpResource(et);
            }
            else if (token == KW("sortresources"))
            {
                if (!readSorting(tab, 1))
                    goto exit_error;
            }
            else if (token == KW("hideaccount"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    goto exit_error;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideAccount(et);
            }
            else if (token == KW("rollupaccount"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    goto exit_error;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpAccount(et);
            }
            else if (token == KW("sortaccounts"))
            {
                if (!readSorting(tab, 2))
                    goto exit_error;
            }
            else if (token == "url")
            {
                errorMessage(i18n("The 'url' attribute is no longer "
                                  "supported. It has been replaced by the "
                                  "much more flexible column URLs. Please "
                                  "refer to the TaskJuggler manual to get "
                                  "more information about optional column "
                                  "attributes."));
                goto exit_error;
            }
            else if (token == KW("loadunit"))
            {
                if (nextToken(token) != ID || !tab->setLoadUnit(token))
                {
                    errorMessage(i18n("Illegal load unit"));
                    goto exit_error;
                }
            }
            else if (token == KW("timeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    goto exit_error;
                }
                tab->setTimeFormat(token);
            }
            else if (token == KW("shorttimeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    goto exit_error;
                }
                tab->setShortTimeFormat(token);
            }
            else if (token == KW("barlabels"))
            {
                if (nextToken(token) != ID)
                {
                    errorMessage(i18n("Bar label mode expected"));
                    goto exit_error;
                }
                if (token == KW("empty"))
                    tab->setBarLabels(HTMLReportElement::BLT_EMPTY);
                else if (token == KW("load"))
                    tab->setBarLabels(HTMLReportElement::BLT_LOAD);
                else
                {
                    errorMessage(i18n("Unknown bar label mode '%1'")
                                 .arg(token));
                    goto exit_error;
                }
            }
            else if (token == KW("notimestamp"))
            {
                report->setTimeStamp(FALSE);
            }
            else
            {
                errorMessage(i18n("Illegal attribute"));
                goto exit_error;
            }
        }
    }
    else
        returnToken(tt, token);

    if (!checkReportInterval(tab))
        goto exit_error;

    proj->addReport(report);

    return TRUE;

exit_error:
    delete report;
    return FALSE;
}

bool
ProjectFile::readHTMLStatusReport()
{
    QString token;
    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("File name expected"));
        return FALSE;
    }

    HTMLStatusReport* report;
    report = new HTMLStatusReport(proj, token, getFile(), getLine());

    TokenType tt;
    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                return FALSE;
            }
            if (token == KW("table"))
            {
                if (nextToken(token) != INTEGER || token.toInt() < 1 ||
                    token.toInt() > 4)
                {
                    errorMessage(i18n("Number between 1 and 4 expected"));
                    return FALSE;
                }
                HTMLReportElement* tab = report->getTable(token.toInt() - 1);
                if (!readReportElement(tab))
                    return FALSE;
            }
            else if (token == KW("headline"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String exptected"));
                    return FALSE;
                }
                report->setHeadline(token);
            }
            else if (token == KW("caption"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String exptected"));
                    return FALSE;
                }
                report->setCaption(token);
            }
            else if (token == KW("rawhead"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                report->setRawHead(token);
            }
            else if (token == KW("rawtail"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                report->setRawTail(token);
            }
            else if (token == KW("rawstylesheet"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                report->setRawStyleSheet(token);
            }
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    proj->addReport(report);

    return TRUE;
}

bool
ProjectFile::readCSVReport(const QString& reportType)
{
    QString token;
    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("File name expected"));
        return FALSE;
    }

    CSVReport* report = 0;
    CSVReportElement* tab = 0;
    if (reportType == KW("csvtaskreport"))
    {
        report = new CSVTaskReport(proj, token, getFile(), getLine());
        tab = ((CSVTaskReport*) report)->getTable();
    }
    else if (reportType == KW("csvresourcereport"))
    {
        report = new CSVResourceReport(proj, token, getFile(), getLine());
        tab = ((CSVResourceReport*) report)->getTable();
    }
    else if (reportType == KW("csvaccountreport"))
    {
        report = new CSVAccountReport(proj, token, getFile(), getLine());
        tab = ((CSVAccountReport*) report)->getTable();
    }
    else
    {
        qFatal("readCSVReport: bad report type");
        return FALSE;   // Just to please the compiler.
    }

    TokenType tt;
    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                return FALSE;
            }
            if (token == KW("columns"))
            {
                tab->clearColumns();
                for ( ; ; )
                {
                    TableColumnInfo* tci;
                    if ((tci = readColumn(proj->getMaxScenarios(), tab)) == 0)
                        return FALSE;
                    tab->addColumn(tci);
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("scenario"))
            {
                tab->clearScenarios();
                QString scId;
                if ((tt = nextToken(scId)) != ID)
                {
                    errorMessage(i18n("Scenario ID expected"));
                    return FALSE;
                }
                if (proj->getScenarioIndex(scId) == -1)
                {
                    errorMessage(i18n("Unknown scenario '%1'")
                                 .arg(scId));
                    return FALSE;
                }
                tab->addScenario(proj->getScenarioIndex(scId) - 1);
            }
            else if (token == KW("start"))
            {
                time_t start;
                if (!readDate(start, 0))
                    return FALSE;
                tab->setStart(start);
            }
            else if (token == KW("end"))
            {
                time_t end;
                if (!readDate(end, 1))
                    return FALSE;
                tab->setEnd(end);
            }
            else if (token == KW("rawhead"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                tab->setRawHead(token);
            }
            else if (token == KW("rawtail"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                tab->setRawTail(token);
            }
            else if (token == KW("showprojectids"))
            {
                tab->setShowPIDs(TRUE);
            }
            else if (token == KW("accumulate"))
            {
                tab->setAccumulate(TRUE);
            }
            else if (token == KW("hidetask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpTask(et);
            }
            else if (token == KW("sorttasks"))
            {
                if (!readSorting(tab, 0))
                    return FALSE;
            }
            else if (token == KW("hideresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpResource(et);
            }
            else if (token == KW("sortresources"))
            {
                if (!readSorting(tab, 1))
                    return FALSE;
            }
            else if (token == KW("hideaccount"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setHideAccount(et);
            }
            else if (token == KW("rollupaccount"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tab->setRollUpAccount(et);
            }
            else if (token == KW("sortaccounts"))
            {
                if (!readSorting(tab, 2))
                    return FALSE;
            }
            else if (token == KW("loadunit"))
            {
                if (nextToken(token) != ID || !tab->setLoadUnit(token))
                {
                    errorMessage(i18n("Illegal load unit"));
                    return FALSE;
                }
            }
            else if (token == KW("timeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    return FALSE;
                }
                tab->setTimeFormat(token);
            }
            else if (token == KW("shorttimeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    return FALSE;
                }
                tab->setShortTimeFormat(token);
            }
            else if (token == KW("barlabels"))
            {
                if (nextToken(token) != ID)
                {
                    errorMessage(i18n("Bar label mode expected"));
                    return FALSE;
                }
                if (token == KW("empty"))
                    tab->setBarLabels(HTMLReportElement::BLT_EMPTY);
                else if (token == KW("load"))
                    tab->setBarLabels(HTMLReportElement::BLT_LOAD);
                else
                {
                    errorMessage(i18n("Unknown bar label mode '%1'")
                                 .arg(token));
                    return FALSE;
                }
            }
            else if (token == KW("notimestamp"))
            {
                report->setTimeStamp(FALSE);
            }
            else if (token == KW("separator"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                tab->setFieldSeparator(token);
            }
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    if (!checkReportInterval(tab))
        return FALSE;

    proj->addReport(report);

    return TRUE;
}

bool
ProjectFile::readExportReport()
{
    QString token;
    if (nextToken(token) != STRING)
    {
        errorMessage(i18n("File name expected"));
        return FALSE;
    }

    if (token.right(4) != ".tjp" && token.right(4) != ".tji")
    {
        errorMessage(i18n("Illegal extension for export file name. "
                          "Please use '.tjp' for standalone projects and "
                          "'.tji' for sub projects."));
        return FALSE;
    }

    ExportReport* report;
    report = new ExportReport(proj, token, getFile(), getLine());

    if (token.right(4) == ".tjp")
    {
        report->setMasterFile(TRUE);
        report->setListShifts(TRUE);
        report->setListResources(TRUE);
    }
    else
    {
        report->setListShifts(FALSE);
        report->setListResources(FALSE);
    }


    TokenType tt;
    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                return FALSE;
            }

            if (token == KW("hidetask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setRollUpTask(et);
            }
            else if (token == KW("hideresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                report->setRollUpResource(et);
            }
            else if (token == KW("taskroot"))
            {
                if ((tt = nextToken(token)) == ID ||
                    tt == ABSOLUTE_ID)
                {
                    if (!proj->getTask(token))
                    {
                        errorMessage(i18n("taskroot must be a known task"));
                        return FALSE;
                    }
                    report->setTaskRoot(token + ".");
                }
                else
                {
                    errorMessage(i18n("Task ID expected"));
                    return FALSE;
                }
            }
            else if (token == KW("taskattributes"))
            {
                for ( ; ; )
                {
                    QString ta;
                    if (nextToken(ta) != ID ||
                        !report->addTaskAttribute(ta))
                    {
                        errorMessage(i18n("task attribute expected"));
                        return FALSE;
                    }

                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("scenarios"))
            {
                report->clearScenarios();
                for ( ; ; )
                {
                    QString scId;
                    if ((tt = nextToken(scId)) != ID)
                    {
                        errorMessage(i18n("Scenario ID expected"));
                        return FALSE;
                    }
                    int scIdx;
                    if ((scIdx = proj->getScenarioIndex(scId)) == -1)
                    {
                        errorMessage(i18n("Unknown scenario %1")
                                     .arg(scId));
                        return FALSE;
                    }
                    if (proj->getScenario(scIdx - 1)->getEnabled())
                        report->addScenario(proj->getScenarioIndex(scId) - 1);
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("start"))
            {
                time_t start;
                if (!readDate(start, 0))
                    return FALSE;
                report->setStart(start);
            }
            else if (token == KW("end"))
            {
                time_t end;
                if (!readDate(end, 1))
                    return FALSE;
                report->setEnd(end);
            }
            else if (token == KW("properties"))
            {
                report->resetContentFlags();
                for ( ; ; )
                {
                    if ((tt = nextToken(token)) != ID)
                    {
                        errorMessage(i18n("Property name expected"));
                        return FALSE;
                    }
                    if (token == KW("all"))
                    {
                        report->setListShifts(TRUE);
                        report->setListTasks(TRUE);
                        report->setListResources(TRUE);
                        report->setListBookings(TRUE);
                    }
                    else if (token == KW("shifts"))
                        report->setListShifts(TRUE);
                    else if (token == KW("tasks"))
                        report->setListTasks(TRUE);
                    else if (token == KW("resources"))
                        report->setListResources(TRUE);
                    else if (token == KW("bookings"))
                        report->setListBookings(TRUE);
                    else
                    {
                        errorMessage(i18n("Unknown property %1").arg(token));
                        return FALSE;
                    }
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("notimestamp"))
            {
                report->setTimeStamp(FALSE);
            }
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    proj->addReport(report);

    return TRUE;
}

bool
ProjectFile::readReportElement(ReportElement* el)
{
    QString token;
    TokenType tt;
    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                return FALSE;
            }

            if (token == KW("columns"))
            {
                el->clearColumns();
                for ( ; ; )
                {
                    QString col;
                    if ((tt = nextToken(col)) != ID)
                    {
                        errorMessage(i18n("Column ID expected"));
                        return FALSE;
                    }
                    el->addColumn(new TableColumnInfo(proj->getMaxScenarios(),
                                                      col));
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("scenarios"))
            {
                el->clearScenarios();
                for ( ; ; )
                {
                    QString scId;
                    if ((tt = nextToken(scId)) != ID)
                    {
                        errorMessage(i18n("Scenario ID expected"));
                        return FALSE;
                    }
                    int scIdx;
                    if ((scIdx = proj->getScenarioIndex(scId)) == -1)
                    {
                        errorMessage(i18n("Unknown scenario %1")
                                     .arg(scId));
                        return FALSE;
                    }
                    if (proj->getScenario(scIdx - 1)->getEnabled())
                        el->addScenario(proj->getScenarioIndex(scId) - 1);
                    if ((tt = nextToken(token)) != COMMA)
                    {
                        returnToken(tt, token);
                        break;
                    }
                }
            }
            else if (token == KW("start"))
            {
                time_t start;
                if (!readDate(start, 0))
                    return FALSE;
                el->setStart(start);
            }
            else if (token == KW("end"))
            {
                time_t end;
                if (!readDate(end, 1))
                    return FALSE;
                el->setEnd(end);
            }
            else if (token == KW("headline"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String exptected"));
                    return FALSE;
                }
                el->setHeadline(token);
            }
            else if (token == KW("caption"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String exptected"));
                    return FALSE;
                }
                el->setCaption(token);
            }
            else if (token == KW("rawhead"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                el->setRawHead(token);
            }
            else if (token == KW("rawtail"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                el->setRawTail(token);
            }
            else if (token == "showactual")
            {
                errorMessage
                    (i18n("ERROR: The keyword 'showactual' has been "
                          "deprecated. Please use the keyword 'scenarios' "
                          "instead."));
                return FALSE;
            }
            else if (token == KW("showprojectids"))
            {
                el->setShowPIDs(TRUE);
            }
            else if (token == KW("hidetask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                el->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                el->setRollUpTask(et);
            }
            else if (token == KW("sorttasks"))
            {
                if (!readSorting(el, 0))
                    return FALSE;
            }
            else if (token == KW("hideresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                el->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                el->setRollUpResource(et);
            }
            else if (token == KW("sortresources"))
            {
                if (!readSorting(el, 1))
                    return FALSE;
            }
            else if (token == KW("url"))
            {
                errorMessage(i18n("The 'url' attribute is no longer "
                                  "supported. It has been replaced by the "
                                  "much more flexible column URLs. Please "
                                  "refer to the TaskJuggler manual to get "
                                  "more information about optional column "
                                  "attributes."));
                return FALSE;
            }
            else if (token == KW("loadunit"))
            {
                if (nextToken(token) != ID || !el->setLoadUnit(token))
                {
                    errorMessage(i18n("Illegal load unit"));
                    return FALSE;
                }
            }
            else if (token == KW("timeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    return FALSE;
                }
                el->setTimeFormat(token);
            }
            else if (token == KW("shorttimeformat"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Time format string expected"));
                    return FALSE;
                }
                el->setShortTimeFormat(token);
            }
#if 0
            else if (token == KW("barlabels"))
            {
                if (nextToken(token) != ID)
                {
                    errorMessage(i18n("Bar label mode expected"));
                    return FALSE;
                }
                if (token == KW("empty"))
                    el->setBarLabels(ReportHtml::BLT_EMPTY);
                else if (token == KW("load"))
                    el->setBarLabels(ReportHtml::BLT_LOAD);
                else
                {
                    errorMessage(i18n("Unknown bar label mode '%1'")
                                 .arg(token));
                    return FALSE;
                }
            }
#endif
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    return TRUE;
}

Operation*
ProjectFile::readLogicalExpression(int precedence)
{
    Operation* op;
    QString token;
    TokenType tt;

    tt = nextToken(token);
    if (DEBUGEX(5))
        qDebug("readLogicalExpression(%d): %s", precedence, token.latin1());
    if (tt == ID || tt == ABSOLUTE_ID)
    {
        QString lookAhead;
        if ((tt = nextToken(lookAhead)) == LBRACKET)
        {
            if (EFT.isKnownFunction(token))
            {
                if ((op = readFunctionCall(token)) == 0)
                {
                    if (DEBUGEX(5))
                        qDebug("exit after function call");
                    return 0;
                }
            }
            else
            {
                errorMessage(i18n("Function '%1' is not defined").arg(token));
                return 0;
            }
        }
        else
        {
            returnToken(tt, lookAhead);
            op = new Operation(Operation::Id, token);
        }
    }
    else if (tt == STRING)
    {
        op = new Operation(Operation::String, token);
    }
    else if (tt == DATE)
    {
        time_t date;
        if (!date2time(token, date))
            return 0;
        op = new Operation(Operation::Date, date);
    }
    else if (tt == INTEGER)
    {
        op = new Operation(token.toLong());
    }
    else if (tt == TILDE)
    {
        if ((op = readLogicalExpression(1)) == 0)
        {
            if (DEBUGEX(5))
                qDebug("exit after NOT");
            return 0;
        }
        op = new Operation(op, Operation::Not);
    }
    else if (tt == LBRACKET)
    {
        if ((op = readLogicalExpression()) == 0)
        {
            if (DEBUGEX(5))
                qDebug("exit after ()");
            return 0;
        }
        if ((tt = nextToken(token)) != RBRACKET)
        {
            errorMessage(i18n("')' expected"));
            return 0;
        }
    }
    else
    {
        errorMessage(i18n("Logical expression expected"));
        return 0;
    }

    if (precedence < 1)
    {
        tt = nextToken(token);
        if (DEBUGEX(5))
            qDebug("Second operator %s", token.latin1());
        if (tt == AND)
        {
            Operation* op2 = readLogicalExpression();
            op = new Operation(op, Operation::And, op2);
        }
        else if (tt == OR)
        {
            Operation* op2 = readLogicalExpression();
            op = new Operation(op, Operation::Or, op2);
        }
        else if (tt == GREATER)
        {
            Operation* op2 = readLogicalExpression();
            op = new Operation(op, Operation::Greater, op2);
        }
        else if (tt == SMALLER)
        {
            Operation* op2 = readLogicalExpression();
            op = new Operation(op, Operation::Smaller, op2);
        }
        else if (tt == EQUAL)
        {
            Operation* op2 = readLogicalExpression();
            op = new Operation(op, Operation::Equal, op2);
        }
        else if (tt == GREATEROREQUAL)
        {
            Operation* op2 = readLogicalExpression();
            op = new Operation(op, Operation::GreaterOrEqual, op2);
        }
        else if (tt == SMALLEROREQUAL)
        {
            Operation* op2 = readLogicalExpression();
            op = new Operation(op, Operation::SmallerOrEqual, op2);
        }
        else
            returnToken(tt, token);
     }

    if (DEBUGEX(5))
        qDebug("exit default");
    return op;
}

Operation*
ProjectFile::readFunctionCall(const QString& name)
{
    QString token;
    TokenType tt;

    QPtrList<Operation> args;
    for (int i = 0; i < EFT.getArgumentCount(name); i++)
    {
        if (DEBUGEX(5))
            qDebug("Reading function '%s' arg %d", name.latin1(), i);
        Operation* op;
        if ((op = readLogicalExpression()) == 0)
            return 0;
        args.append(op);
        if ((i < EFT.getArgumentCount(name) - 1) &&
            nextToken(token) != COMMA)
        {
            errorMessage(i18n("Comma expected. "
                              "Function '%1' needs %2 arguments.")
                         .arg(name).arg(EFT.getArgumentCount(name)));
            return 0;
        }
    }
    if ((tt = nextToken(token)) != RBRACKET)
    {
        errorMessage(i18n("')' expected"));
        return 0;
    }
    Operation** argsArr = new Operation*[args.count()];
    int i = 0;
    for (QPtrListIterator<Operation> oli(args); *oli != 0; ++oli)
        argsArr[i++] = *oli;
    if (DEBUGEX(5))
        qDebug("function '%s' done", name.latin1());
    return new Operation(name, argsArr, args.count());
}

bool
ProjectFile::readSorting(Report* report, int which)
{
    TokenType tt;
    QString token;

    int i = 0;
    do
    {
        int sorting;
        if (!readSortingMode(sorting))
            return FALSE;

        bool ok = TRUE;
        switch (which)
        {
            case 0:
                ok = report->setTaskSorting(sorting, i);
                break;
            case 1:
                ok = report->setResourceSorting(sorting, i);
                break;
            case 2:
                ok = report->setAccountSorting(sorting, i);
                break;
            default:
                qFatal("readSorting: Unknown sorting attribute");
                return FALSE;
        }
        if (!ok)
        {
            errorMessage
                (i18n("This sorting criteria is not supported for the list "
                      "or it is used at the wrong position."));
            return FALSE;
        }
        tt = nextToken(token);
    } while (++i < CoreAttributesList::maxSortingLevel && tt == COMMA);

    returnToken(tt, token);

    return TRUE;
}

bool
ProjectFile::readSorting(ReportElement* tab, int which)
{
    TokenType tt;
    QString token;

    int i = 0;
    do
    {
        int sorting;
        if (!readSortingMode(sorting))
            return FALSE;

        bool ok = TRUE;
        switch (which)
        {
            case 0:
                ok = tab->setTaskSorting(sorting, i);
                break;
            case 1:
                ok = tab->setResourceSorting(sorting, i);
                break;
            case 2:
                ok = tab->setAccountSorting(sorting, i);
                break;
            default:
                qFatal("readSorting: Unknown sorting attribute");
                return FALSE;
        }
        if (!ok)
        {
            errorMessage
                (i18n("This sorting criteria is not supported for the list "
                      "or it is used at the wrong position."));
            return FALSE;
        }
        tt = nextToken(token);
    } while (++i < CoreAttributesList::maxSortingLevel && tt == COMMA);

    returnToken(tt, token);

    return TRUE;
}

TableColumnInfo*
ProjectFile::readColumn(uint maxScenarios, ReportElement* tab)
{
    QString token;
    TokenType tt;
    if ((tt = nextToken(token)) != ID)
    {
        errorMessage(i18n("Column ID expected"));
        return 0;
    }
    if (!tab->isSupportedColumn(token))
    {
        errorMessage(i18n("Unknown colon ID '%1'. Supported IDs are: %2.")
                     .arg(token).arg(tab->getSupportedColumnList().join(", ")));
        return 0;
    }
    TableColumnInfo* tci = new TableColumnInfo(maxScenarios, token);

    if ((tt = nextToken(token)) == LBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                goto exit_error;
            }

            if (token == KW("title"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                tci->setTitle(token);
            }
            else if (token == KW("titleurl"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                tci->setTitleURL(token);
            }
            else if (token == KW("subtitle"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                tci->setSubTitle(token);
            }
            else if (token == KW("subtitleurl"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                tci->setSubTitleURL(token);
            }
            else if (token == KW("celltext"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                tci->setCellText(token);
            }
            else if (token == KW("cellurl"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    goto exit_error;
                }
                tci->setCellURL(token);
            }
            else if (token == KW("hidecelltext"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    goto exit_error;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tci->setHideCellText(et);
            }
            else if (token == KW("hidecellurl"))
            {
                Operation* op;
                QString fileName = openFiles.last()->getFile();
                int lineNo = openFiles.last()->getLine();
                if ((op = readLogicalExpression()) == 0)
                    goto exit_error;
                ExpressionTree* et = new ExpressionTree(op);
                et->setDefLocation(fileName, lineNo);
                tci->setHideCellURL(et);
            }
            else
            {
                errorMessage(i18n("Illegal attribute"));
                goto exit_error;
            }
        }
    }
    else
        returnToken(tt, token);

    return tci;

exit_error:
    delete tci;
    return 0;
}

bool
ProjectFile::readSortingMode(int& sorting)
{
    QString token;

    nextToken(token);
    QString laToken;
    TokenType tt;
    if ((tt = nextToken(laToken)) == COLON)
    {
        /* Scenario specific task properties */
        int scenarioIdx = proj->getScenarioIndex(token);
        if (scenarioIdx <= 0)
        {
            errorMessage
                (i18n("Unknown scenario '%s'").arg(token));
            return FALSE;
        }
        nextToken(token);

        if (token == KW("startup"))
            sorting = CoreAttributesList::StartUp;
        else if (token == KW("startdown"))
            sorting = CoreAttributesList::StartDown;
        else if (token == KW("endup"))
            sorting = CoreAttributesList::EndUp;
        else if (token == KW("enddown"))
            sorting = CoreAttributesList::EndDown;
        else if (token == KW("statusup"))
            sorting = CoreAttributesList::StatusUp;
        else if (token == KW("statusdown"))
            sorting = CoreAttributesList::StatusDown;
        else if (token == KW("completedup"))
            sorting = CoreAttributesList::CompletedUp;
        else if (token == KW("completeddown"))
            sorting = CoreAttributesList::CompletedDown;
        else if (token == KW("criticalnessup"))
            sorting = CoreAttributesList::CriticalnessUp;
        else if (token == KW("criticalnessdown"))
            sorting = CoreAttributesList::CriticalnessDown;
        else if (token == KW("pathcriticalnessup"))
            sorting = CoreAttributesList::PathCriticalnessUp;
        else if (token == KW("pathcriticalnessdown"))
            sorting = CoreAttributesList::PathCriticalnessDown;
        else
        {
            errorMessage(i18n("Sorting criteria expected"));
            return FALSE;
        }
        sorting += (scenarioIdx - 1) << 16;
    }
    else
    {
        returnToken(tt, laToken);

        bool deprecatedWarning = FALSE;

        if (token == KW("tree"))
            sorting = CoreAttributesList::TreeMode;
        else if (token == KW("sequenceup"))
            sorting = CoreAttributesList::SequenceUp;
        else if (token == KW("sequencedown"))
            sorting = CoreAttributesList::SequenceDown;
        else if (token == KW("indexup"))
            sorting = CoreAttributesList::IndexUp;
        else if (token == KW("indexdown"))
            sorting = CoreAttributesList::IndexDown;
        else if (token == KW("idup"))
            sorting = CoreAttributesList::IdUp;
        else if (token == KW("iddown"))
            sorting = CoreAttributesList::IdDown;
        else if (token == KW("fullnameup"))
            sorting = CoreAttributesList::FullNameUp;
        else if (token == KW("fullnamedown"))
            sorting = CoreAttributesList::FullNameDown;
        else if (token == KW("nameup"))
            sorting = CoreAttributesList::NameUp;
        else if (token == KW("namedown"))
            sorting = CoreAttributesList::NameDown;
        else if (token == KW("priorityup"))
            sorting = CoreAttributesList::PrioUp;
        else if (token == KW("prioritydown"))
            sorting = CoreAttributesList::PrioDown;
        else if (token == KW("responsibleup"))
            sorting = CoreAttributesList::ResponsibleUp;
        else if (token == KW("responsibledown"))
            sorting = CoreAttributesList::ResponsibleDown;
        else if (token == KW("mineffortup"))
            sorting = CoreAttributesList::MinEffortUp;
        else if (token == KW("mineffortdown"))
            sorting = CoreAttributesList::MinEffortDown;
        else if (token == KW("maxeffortup"))
            sorting = CoreAttributesList::MaxEffortUp;
        else if (token == KW("maxeffortdown"))
            sorting = CoreAttributesList::MaxEffortDown;
        else if (token == KW("rateup"))
            sorting = CoreAttributesList::RateUp;
        else if (token == KW("ratedown"))
            sorting = CoreAttributesList::RateDown;
        else if (token == KW("kotrusidup"))
            sorting = CoreAttributesList::KotrusIdUp;
        else if (token == KW("kotrusiddown"))
            sorting = CoreAttributesList::KotrusIdDown;
        else if (token == KW("startup"))
            sorting = CoreAttributesList::StartUp;
        else if (token == KW("startdown"))
            sorting = CoreAttributesList::StartDown;
        else if (token == KW("endup"))
            sorting = CoreAttributesList::EndUp;
        else if (token == KW("enddown"))
            sorting = CoreAttributesList::EndDown;
        else if (token == KW("statusup"))
            sorting = CoreAttributesList::StatusUp;
        else if (token == KW("statusdown"))
            sorting = CoreAttributesList::StatusDown;
        else if (token == KW("completedup"))
            sorting = CoreAttributesList::CompletedUp;
        else if (token == KW("completeddown"))
            sorting = CoreAttributesList::CompletedDown;
        else if (token == "planstartup")
        {
            sorting = CoreAttributesList::StartUp;
            deprecatedWarning = TRUE;
        }
        else if (token == "planstartdown")
        {
            sorting = CoreAttributesList::StartDown;
            deprecatedWarning = TRUE;
        }
        else if (token == "planendup")
        {
            sorting = CoreAttributesList::EndUp;
            deprecatedWarning = TRUE;
        }
        else if (token == "planenddown")
        {
            sorting = CoreAttributesList::EndDown;
            deprecatedWarning = TRUE;
        }
        else if (token == "actualstartup")
        {
            sorting = CoreAttributesList::StartUp + 0x1FFFF;
            deprecatedWarning = TRUE;
        }
        else if (token == "actualstartdown")
        {
            sorting = CoreAttributesList::StartDown + 0x1FFFF;
            deprecatedWarning = TRUE;
        }
        else if (token == "actualendup")
        {
            sorting = CoreAttributesList::EndUp + 0x1FFFF;
            deprecatedWarning = TRUE;
        }
        else if (token == "actualenddown")
        {
            sorting = CoreAttributesList::EndDown + 0x1FFFF;
            deprecatedWarning = TRUE;
        }
        else if (token == "planstatusup")
        {
            sorting = CoreAttributesList::StatusUp;
            deprecatedWarning = TRUE;
        }
        else if (token == "planstatusdown")
        {
            sorting = CoreAttributesList::StatusDown;
            deprecatedWarning = TRUE;
        }
        else if (token == "plancompletedup")
        {
            sorting = CoreAttributesList::CompletedUp;
            deprecatedWarning = TRUE;
        }
        else if (token == "plancompleteddown")
        {
            sorting = CoreAttributesList::CompletedDown;
            deprecatedWarning = TRUE;
        }
        else
        {
            errorMessage(i18n("Sorting criteria expected"));
            return FALSE;
        }

        if (deprecatedWarning)
        {
            errorMessage
                (i18n("ERROR: Concatenating the scenario name and the "
                      "sorting criteria has been deprecated. Please separate "
                      "them by a colon. E. g. 'plan:start', 'actual:end'"));
            return FALSE;
        }
    }

    return TRUE;
}

bool
ProjectFile::readTaskDepOptions(TaskDependency* td)
{
    QString token;
    TokenType tt;

    for ( ; ; )
    {
        if ((tt = nextToken(token)) == RBRACE)
            break;
        else if (tt != ID)
        {
            errorMessage(i18n("Attribute ID or '}' expected"));
            return FALSE;
        }

        int scenarioIdx = 0;
        TokenType nextTT;
        QString next;
        if ((nextTT = nextToken(next)) == COLON)
        {
            if ((scenarioIdx = proj->getScenarioIndex(token) - 1) < 0)
            {
                errorMessage(i18n("Scenario ID expected. '%1' is not "
                                  "a defined scenario.").arg(token));
                return FALSE;
            }
            tt = nextToken(token);
        }
        else
            returnToken(nextTT, next);

        if (token == KW("gapduration"))
        {
            double d;
            if (!readTimeFrame(d, FALSE, scenarioIdx > 0))
                return FALSE;
            /* Set the duration and round it down to be a multiple of the
             * schedule granularity. */
            td->setGapDuration(scenarioIdx, (((long) (d * 60 * 60 * 24)) /
                                proj->getScheduleGranularity()) *
                               proj->getScheduleGranularity());
        }
        else if (token == KW("gaplength"))
        {
            double d;
            if (!readTimeFrame(d, FALSE, scenarioIdx > 0))
                return FALSE;
            /* Set the length and round it down to be a multiple of the
             * schedule granularity. */
            td->setGapLength(scenarioIdx, (((long) (d * 60 * 60 *
                                       proj->getDailyWorkingHours())) /
                              proj->getScheduleGranularity()) *
                             proj->getScheduleGranularity());
        }
        else
        {
            errorMessage(i18n("Illegal dependency attribute"));
            return FALSE;
        }
    }

    return TRUE;
}

bool
ProjectFile::date2time(const QString& date, time_t& val)
{
    // Make sure that we are within the UNIX lifetime (kindof).
    int year = date.left(4).toInt();
    if (year < 1971)
    {
        errorMessage(i18n("Date must be larger than 1971-01-01"));
        return false;
    }
    if (year > 2035)
    {
        errorMessage(i18n("Date must be smaller than 2035-01-01"));
        return false;
    }

    if ((val = ::date2time(date)) == 0)
        errorMessage(i18n(getUtilityError()));
    return true;
}

int
ProjectFile::hhmm2time(const QString& hhmm)
{
    int hour = hhmm.left(hhmm.find(':')).toInt();
    if (hour > 24)
    {
        errorMessage(i18n("Hour must be in the range of 0 - 24"));
        return -1;
    }
    int min = hhmm.mid(hhmm.find(':') + 1).toInt();
    if (min > 59)
    {
        errorMessage(i18n("Minutes must be in the range of 0 - 59"));
        return -1;
    }
    if (hour == 24 && min != 0)
    {
        errorMessage(i18n("Maximum time is 24:00"));
        return -1;
    }
    return hour * 60 * 60 + min * 60;
}
