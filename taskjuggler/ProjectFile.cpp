/*
 * ProjectFile.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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

#include "ProjectFile.h"
#include "debug.h"
#include "taskjuggler.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Project.h"
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
#include "HTMLTaskReport.h"
#include "ExportReport.h"
#include "TableColumnInfo.h"
#include "ReportXML.h"
#include "ReferenceAttribute.h"
#include "TextAttribute.h"

// Dummy marco to mark all keywords of taskjuggler syntax
#define KW(a) a

ProjectFile::ProjectFile(Project* p)
{
    proj = p;

    openFiles.setAutoDelete(TRUE);
}

bool
ProjectFile::open(const QString& file, const QString& parentPath,
                  const QString& taskPrefix)
{
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
        qFatal("Cannot open '%s'", absFileName.latin1());
        return FALSE;
    }

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
            else if (token == "now")
            {
                errorMessage
                    (i18n("WARNING: 'now' is no longer a property. It's now "
                          " an optional project attribute. Please fix your "
                          "project file."));
                if (nextToken(token) != DATE)
                {
                    errorMessage(i18n("Date expected"));
                    return FALSE;
                }
                proj->setNow(date2time(token));
                break;
            }
            else if (token == KW("mineffort"))
            {
                if (nextToken(token) != REAL)
                {
                    errorMessage(i18n("Real value exptected"));
                    return FALSE;
                }
                proj->setMinEffort(token.toDouble());
                break;
            }
            else if (token == KW("maxeffort"))
            {
                if (nextToken(token) != REAL)
                {
                    errorMessage(i18n("Real value exptected"));
                    return FALSE;
                }
                proj->setMaxEffort(token.toDouble());
                break;
            }
            else if (token == KW("rate"))
            {
                if (nextToken(token) != REAL)
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
                    (i18n("WARNING: 'currency' is no longer a property. It's "
                          "now an optional project attribute. Please fix "
                          "your project file."));
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                proj->setCurrency(token);
                break;
            }
            else if (token == KW("currencydigits"))
            {
                errorMessage
                    (i18n("WARNING: 'currencydigits' has been deprecated. "
                          "Please use 'currencyformat' instead."));
                if (nextToken(token) != INTEGER)
                {
                    errorMessage(i18n("Integer value expected"));
                    return FALSE;
                }
                proj->setCurrencyDigits(token.toInt());
                break;
            }
            else if (token == "timingresolution")
            {
                errorMessage
                    (i18n("WARNING: 'timingresolution' is no longer a "
                          "property. It's now an optional project attribute. "
                          "Please fix your project file."));
                ulong resolution;
                if (!readTimeValue(resolution))
                    return FALSE;
                if (proj->resourceCount() > 0)
                {
                    errorMessage
                        (i18n("The timing resolution cannot be changed after "
                              "resources have been declared."));
                    return FALSE;
                }
                if (resolution < 60 * 5)
                {
                    errorMessage(i18n("timing resolution must be at least 5 "
                                      "min"));
                    return FALSE;
                }
                proj->setScheduleGranularity(resolution);
                break;
            }
            else if (token == KW("workinghours"))
            {
                errorMessage
                    (i18n("WARNING: 'workinghours' is no longer a property. "
                          "It's now an optional project attribute. Please fix "
                          "your project file."));
                int dow;
                QPtrList<Interval>* l = new QPtrList<Interval>();
                if (!readWorkingHours(dow, l))
                    return FALSE;

                proj->setWorkingHours(dow, l);
                break;
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
            else if (token == "xmltaskreport")
            {
                errorMessage
                    (i18n("WARNING: The keyword 'xmltaskreport' is "
                          "deprecated. Please use the keyword 'xmlreport' "
                          "instead."));
                if(!readXMLReport())
                    return FALSE;
                break;
            }
            else if (token == KW("xmlreport"))
            {
               if(!readXMLReport())
                  return FALSE;
               break;
            }
            else if (token == "icalreport" )
            {
#ifdef HAVE_ICAL
#ifdef HAVE_KDE
               if( !readICalTaskReport())
#else
                  errorMessage(i18n( "TaskJuggler was built without KDE support -> no ICal support!" ));
#endif
                  return FALSE;
               break;
#else
               errorMessage(i18n( "TaskJuggler was built without ICal-Support, sorry." ));
               break;
#endif
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
            else if (token == KW("htmlstatusreport"))
            {
                if (!readHTMLStatusReport())
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
    }

    return TRUE;
}

bool
ProjectFile::readProject()
{
    QString token;

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
    if (nextToken(token) != DATE)
    {
        errorMessage(i18n("Start date expected"));
        return FALSE;
    }
    start = date2time(token);
    if (nextToken(token) != DATE)
    {
        errorMessage(i18n("End date expected"));
        return FALSE;
    }
    end = date2time(token);
    if (end <= start)
    {
        errorMessage(i18n("End date must be larger then start date"));
        return FALSE;
    }
    proj->setStart(start);
    proj->setEnd(end - 1);

    TokenType tt;
    if ((tt = nextToken(token)) == LCBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) != ID && tt != RCBRACE)
            {
                errorMessage(i18n("Attribute ID expected"));
                return FALSE;
            }
            if (tt == RCBRACE)
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
                if (nextToken(token) != DATE)
                {
                    errorMessage(i18n("Date expected"));
                    return FALSE;
                }
                proj->setNow(date2time(token));
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
                proj->setScheduleGranularity(resolution);
            }
            else if (token == KW("timezone"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("Timezone name expected"));
                    return FALSE;
                }
                setTimezone(token);
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
    if (nextToken(token) != LCBRACE)
    {
        errorMessage(i18n("'{' expected."));
        return FALSE;
    }
    TokenType tt;
    for ( ; ; )
    {
        QString attrID;
        if ((tt = nextToken(attrID)) == RCBRACE)
            break;
        else if (tt != ID)
        {
            errorMessage(i18n("Attribute ID expected."));
            return FALSE;
        }
        QString attrType;
        nextToken(attrType);
        Project::CustomAttributeType cat;
        if (attrType == "reference")
            cat = Project::CAT_Reference;
        else if (attrType == "text")
            cat = Project::CAT_Text;
        else
        {
            errorMessage(i18n("'%1' is not a known attribute type. Please use "
                              "'reference' or 'text'.").arg(attrType));
            return FALSE;
        }
        bool ok = FALSE;
        if (property == "task")
            ok = proj->addTaskAttribute(attrID, cat);
        else if (property == "resource")
            ok = proj->addResourceAttribute(attrID, cat);
        if (!ok)
        {
            errorMessage(i18n("The custom attribute '%1' has already been "
                         "declared for the property '%2'.")
                .arg(attrID).arg(property));
            return FALSE;
        }
    }
   
    return TRUE; 
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
    QString token;
    TokenType tt;

    QString taskPrefix = getTaskPrefix();
    /* The nextToken() call may yield an EndOfFile and shift file scope to
     * parent file. So we have to save the path of the current file to pass it
     * later to open(). */
    QString parentPath = openFiles.last()->getPath();
    
    if ((tt = nextToken(token)) == LCBRACE)
    {
        while ((tt = nextToken(token)) != RCBRACE)
        {
            if (tt == ID && token == KW("taskprefix"))
            {
                if (nextToken(token) != ID || tt == ABSOLUTE_ID)
                {
                    errorMessage(i18n("String expected"));
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
ProjectFile::readTask(Task* parent)
{
    TokenType tt;
    QString token;

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

    if ((tt = nextToken(token)) != LCBRACE)
    {
        errorMessage(i18n("{ expected"));
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
    
    // We need to check that the task id has not been declared before.
    if (proj->getTask(id))
    {
        errorMessage(i18n("Task %1 has already been declared")
                     .arg(id));
        return FALSE;
    }

    Task* task = new Task(proj, id, name, parent, getFile(), getLine());

    if (!readTaskBody(task))
        return FALSE;
    
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
    if (nextToken(token) != LCBRACE)
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
    
    for (bool done = false ; !done; )
    {
        switch (tt = nextToken(token))
        {
        case ID:
            if (proj->getTaskAttributeType(token) == Project::CAT_Reference)
            {
                QString ref, label;
                if (!readReference(ref, label))
                    return FALSE;
                ReferenceAttribute* ra = new ReferenceAttribute(ref, label);
                task->addCustomAttribute(token, ra);
            }
            else if (proj->getTaskAttributeType(token) == Project::CAT_Text)
            {
                QString text;
                if (nextToken(text) == STRING)
                {
                    TextAttribute* ra = new TextAttribute(text);
                    task->addCustomAttribute(token, ra);
                }
                else
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
            }
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
            else if (token == KW("statusnote"))
            {
                if ((tt = nextToken(token)) == STRING)
                    task->setStatusNote(Task::Plan, token);
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
            else if (token == KW("start"))
            {
                time_t val;
                if (!readDate(val, 0))
                    return FALSE;
                task->setStart(Task::Plan, val);
                task->setScheduling(Task::ASAP);
            }
            else if (token == KW("end"))
            {
                time_t val;
                if (!readDate(val, 1))
                    return FALSE;
                task->setEnd(Task::Plan, val);
                task->setScheduling(Task::ALAP);
            }
            else if (token == KW("actualstart"))
            {
                time_t val;
                if (!readDate(val, 0))
                    return FALSE;
                task->setStart(Task::Actual, val);
                task->setScheduling(Task::ASAP);
            }
            else if (token == KW("actualend"))
            {
                time_t val;
                if (!readDate(val, 1))
                    return FALSE;
                task->setEnd(Task::Actual, val);
                task->setScheduling(Task::ALAP);
            }
            else if (token == KW("minstart"))
            {
                time_t val;
                if (!readDate(val, 0))
                    return FALSE;
                task->setMinStart(val);
            }
            else if (token == KW("maxstart"))
            {
                time_t val;
                if (!readDate(val, 0))
                    return FALSE;
                task->setMaxStart(val);
            }
            else if (token == KW("minend"))
            {
                time_t val;
                if (!readDate(val, 0))
                    return FALSE;
                task->setMinEnd(val);
            }
            else if (token == KW("maxend"))
            {
                time_t val;
                if (!readDate(val, 0))
                    return FALSE;
                task->setMaxEnd(val);
            }
            else if (token == KW("length"))
            {
                double d;
                if (!readPlanTimeFrame(d, TRUE))
                    return FALSE;
                task->setLength(Task::Plan, d);
            }
            else if (token == KW("effort"))
            {
                double d;
                if (!readPlanTimeFrame(d, TRUE))
                    return FALSE;
                task->setEffort(Task::Plan, d);
            }
            else if (token == KW("duration"))
            {
                double d;
                if (!readPlanTimeFrame(d, FALSE))
                    return FALSE;
                task->setDuration(Task::Plan, d);
            }
            else if (token == KW("actuallength"))
            {
                double d;
                if (!readPlanTimeFrame(d, TRUE))
                    return FALSE;
                task->setLength(Task::Actual, d);
            }
            else if (token == KW("actualeffort"))
            {
                double d;
                if (!readPlanTimeFrame(d, TRUE))
                    return FALSE;
                task->setEffort(Task::Actual, d);
            }
            else if (token == KW("actualduration"))
            {
                double d;
                if (!readPlanTimeFrame(d, FALSE))
                    return FALSE;
                task->setDuration(Task::Actual, d);
            }
            else if (token == KW("planscheduled"))
                task->setScheduled(Task::Plan, TRUE);
            else if (token == KW("actualscheduled"))
                task->setScheduled(Task::Actual, TRUE);
            else if (token == KW("complete"))
            {
                if (nextToken(token) != INTEGER)
                {
                    errorMessage(i18n("Integer value expected"));
                    return FALSE;
                }
                int complete = token.toInt();
                if (complete < 0 || complete > 100)
                {
                    errorMessage(i18n("Value of complete must be between 0 "
                                      "and 100"));
                    return FALSE;
                }
                task->setComplete(Task::Plan, complete);
            }
            else if (token == KW("startbuffer"))
            {
                double value;
                if (!readPercent(value))
                    return FALSE;
                task->setStartBuffer(Task::Plan, value);
            }
            else if (token == KW("endbuffer"))
            {
                double value;
                if (!readPercent(value))
                    return FALSE;
                task->setEndBuffer(Task::Plan, value);
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
                    task->addDepends(id);
                    task->setScheduling(Task::ASAP);
                    if ((tt = nextToken(token)) != COMMA)
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
                    task->addPrecedes(id);
                    task->setScheduling(Task::ALAP);
                    if ((tt = nextToken(token)) != COMMA)
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
            else if (token == KW("startcredit"))
            {
                if (nextToken(token) != REAL)
                {
                    errorMessage(i18n("Real value expected"));
                    return FALSE;
                }
                task->setStartCredit(Task::Plan, token.toDouble());
            }
            else if (token == KW("endcredit"))
            {
                if (nextToken(token) != REAL)
                {
                    errorMessage(i18n("Real value expected"));
                    return FALSE;
                }
                task->setEndCredit(Task::Plan, token.toDouble());
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
            else if (token == "include")
            {
                errorMessage
                    (i18n("WARNING: The 'include' attribute is no longer "
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
        case RCBRACE:
            done = true;
            break;
        default:
            errorMessage(i18n("Syntax Error at '%1'").arg(token));
            return FALSE;
        }
    }

    return TRUE;
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
    QString start;
    if ((tt = nextToken(start)) != DATE)
    {
        errorMessage(i18n("Date expected"));
        return FALSE;
    }
    QString token;
    if ((tt = nextToken(token)) != MINUS)
    {
        // vacation e. g. 2001-11-28
        returnToken(tt, token);
        from = date2time(start);
        to = sameTimeNextDay(date2time(start)) - 1;
    }
    else
    {
        // vacation e. g. 2001-11-28 - 2001-11-30
        QString end;
        if ((tt = nextToken(end)) != DATE)
        {
            errorMessage(i18n("Date expected"));
            return FALSE;
        }
        from = date2time(start);
        if (date2time(start) > date2time(end))
        {
            errorMessage(i18n("Vacation must start before end"));
            return FALSE;
        }
        to = date2time(end) - 1;
    }
    return TRUE;
}

bool
ProjectFile::readDate(time_t& val, time_t correction)
{
    QString token;
    
    if (nextToken(token) != DATE)
    {
        errorMessage(i18n("Date expected"));
        return FALSE;
    }
    
    val = date2time(token) - correction;
    if (val < proj->getStart() ||
        val > proj->getEnd())
    {
        errorMessage(i18n("Date %1 is outside of project time frame "
                          "(%2 - %3")
                     .arg(time2tjp(val))
                     .arg(time2tjp(proj->getStart()))
                     .arg(time2tjp(proj->getEnd())));
        return FALSE;
    }
    return TRUE;
}

bool
ProjectFile::readRealFormat(RealFormat* format)
{
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
    if ((tt = nextToken(token)) == LCBRACE)
    {
        while ((tt = nextToken(token)) != RCBRACE)
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

    if (proj->getResource(id))
    {
        errorMessage(i18n("Resource %1 has already been defined")
                     .arg(id));
        return FALSE;
    }

    Resource* r = new Resource(proj, id, name, parent);

    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LCBRACE)
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
    if (nextToken(token) != LCBRACE)
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

    while ((tt = nextToken(token)) != RCBRACE)
    {
        if (tt != ID)
        {
            errorMessage(i18n("Unknown attribute '%1'").arg(token));
            return FALSE;
        }
        if (token == KW("resource"))
        {
            if (!readResource(r))
                return FALSE;
        }
        else if (token == KW("mineffort"))
        {
            if (nextToken(token) != REAL)
            {
                errorMessage(i18n("Real value exptected"));
                return FALSE;
            }
            r->setMinEffort(token.toDouble());
        }
        else if (token == KW("maxeffort"))
        {
            if (nextToken(token) != REAL)
            {
                errorMessage(i18n("Real value exptected"));
                return FALSE;
            }
            r->setMaxEffort(token.toDouble());
        }
        else if (token == KW("efficiency"))
        {
            if (nextToken(token) != REAL)
            {
                errorMessage(i18n("Read value expected"));
                return FALSE;
            }
            r->setEfficiency(token.toDouble());
        }
        else if (token == KW("rate"))
        {
            if (nextToken(token) != REAL)
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
        else if (token == KW("planbooking"))
        {
            Booking* b;
            if ((b = readBooking()) == 0)
                return FALSE;
            if (!r->addBooking(Task::Plan, b))
            {
                errorMessage(i18n("Resource %1 cannot be allocated during this "
                                  "period").arg(r->getId()));
                return FALSE;
            }
        }
        else if (token == KW("actualbooking"))
        {
            Booking* b;
            if ((b = readBooking()) == 0)
                return FALSE;
            if (!r->addBooking(Task::Actual, b))
            {
                errorMessage(i18n("Resource %1 cannot be allocated during this "
                                  "period").arg(r->getId()));
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
        else if (token == KW("include"))
        {
            if (!readInclude())
                return FALSE;
            break;
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

    Shift* s = new Shift(proj, id, name, parent);

    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LCBRACE)
    {
        // read optional attributes
        while ((tt = nextToken(token)) != RCBRACE)
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
                if (!readInclude())
                    return FALSE;
                break;
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
ProjectFile::readBooking()
{
    QString token;

    if (nextToken(token) != DATE)
    {
        errorMessage(i18n("Start date expected"));
        return 0;
    }
    time_t start = date2time(token);
    if (start < proj->getStart() || start >= proj->getEnd())
    {
        errorMessage(i18n("Start date must be within the project timeframe"));
        return 0;
    }
    
    if (nextToken(token) != DATE)
    {
        errorMessage(i18n("End date expected"));
        return 0;
    }
    time_t end = date2time(token) - 1;
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

    Task* task;
    TokenType tt;
    if (((tt = nextToken(token)) != ID && tt != ABSOLUTE_ID) ||
        (task = proj->getTask(getTaskPrefix() + token)) == 0)
    {
        errorMessage(i18n("Task ID expected"));
        return 0;
    }
    return new Booking(Interval(start, end), task);
}

bool
ProjectFile::readAccount(Account* parent)
{
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
        if (nextToken(at) != ID && (at != KW("cost") ||
                                    at != KW("revenue")))
        {
            errorMessage(i18n("Account type 'cost' or 'revenue' expected"));
            return FALSE;
        }
        acctType = at == KW("cost") ? Cost : Revenue;
    }
    else
        acctType = parent->getAcctType();

    Account* a = new Account(proj, id, name, parent, acctType);

    TokenType tt;
    QString token;
    if ((tt = nextToken(token)) == LCBRACE)
    {
        bool hasSubAccounts = FALSE;
        bool cantBeParent = FALSE;
        // read optional attributes
        while ((tt = nextToken(token)) != RCBRACE)
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
    QString token;

    if (nextToken(token) != DATE)
    {
        errorMessage(i18n("Date expected"));
        return FALSE;
    }
    time_t date = date2time(token);

    QString description;
    if (nextToken(description) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }

    if (nextToken(token) != REAL)
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
        errorMessage(i18n("Resource ID expected"));
        return FALSE;
    }
    Allocation* a = new Allocation(r);
    QString token;
    TokenType tt;
    if ((tt = nextToken(token)) == LCBRACE)
    {
        while ((tt = nextToken(token)) != RCBRACE)
        {
            if (tt != ID)
            {
                errorMessage(i18n("Unknown attribute '%1'")
                             .arg(token));
                return FALSE;
            }
            if (token == KW("load"))
            {
                if (nextToken(token) != REAL)
                {
                    errorMessage(i18n("Real value expected"));
                    return FALSE;
                }
                double load = token.toDouble();
                if (load < 0.01)
                {
                    errorMessage(i18n("Value must be at least 0.01"));
                    return FALSE;
                }
                a->setLoad((int) (100 * load));
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
ProjectFile::readPlanTimeFrame(double& value, bool workingDays)
{
    QString val;
    TokenType tt;
    if ((tt = nextToken(val)) != REAL && tt != INTEGER)
    {
        errorMessage(i18n("Real value expected"));
        return FALSE;
    }
    QString unit;
    if (nextToken(unit) != ID)
    {
        errorMessage(i18n("Unit expected"));
        return FALSE;
    }
    if (unit == KW("min"))
        value = val.toDouble() / (proj->getDailyWorkingHours() * 60);
    else if (unit == KW("h"))
        value = val.toDouble() / proj->getDailyWorkingHours();
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

#ifdef HAVE_ICAL
#ifdef HAVE_KDE
bool
ProjectFile::readICalTaskReport()
{
   QString token;
   if (nextToken(token) != STRING)
   {
      errorMessage(i18n("File name expected"));
      return FALSE;
   }
   ReportICal *rep = new ReportICal( proj, token, proj->getStart(), proj->getEnd());
   proj->addICalReport( rep );

   return( true );
}
#endif
#endif

bool
ProjectFile::readXMLReport()
{
   QString token;
   if (nextToken(token) != STRING)
   {
      errorMessage(i18n("File name expected"));
      return FALSE;
   }
   ReportXML *rep = new ReportXML(proj, token, getFile(), getLine());
   proj->addXMLReport( rep );

   return( true );
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
    if ((tt = nextToken(token)) == LCBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RCBRACE)
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
            else if (token == KW("scenarios"))
            {
                tab->clearScenarios();
                for ( ; ; )
                {
                    QString scId;
                    if ((tt = nextToken(scId)) != ID)
                    {
                        errorMessage(i18n("Scenario ID expected"));
                        return FALSE;
                    }
                    if (proj->getScenarioIndex(scId) == -1)
                    {
                        errorMessage(i18n("Unknown scenario %1")
                                     .arg(scId));
                        return FALSE;
                    }
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
                if (nextToken(token) != DATE)
                {
                    errorMessage(i18n("Date expected"));
                    return FALSE;
                }
                tab->setStart(date2time(token));
            }
            else if (token == KW("end"))
            {
                if (nextToken(token) != DATE)
                {
                    errorMessage(i18n("Date expected"));
                    return FALSE;
                }
                tab->setEnd(date2time(token) - 1);
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
            else if (token == KW("rawstylesheet"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return FALSE;
                }
                errorMessage("WARNING: 'rawstylesheet' is no longer supported. "
                             "Please use 'rawhead' instead.");
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
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                tab->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
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
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                tab->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
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
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                tab->setHideAccount(et);
            }
            else if (token == KW("rollupaccount"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                tab->setRollUpAccount(et);
            }
            else if (token == KW("sortaccounts"))
            {
                if (!readSorting(tab, 2))
                    return FALSE;
            }
            else if (token == KW("url"))
            {
                if (!readHtmlUrl(tab))
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
    
    if (reportType == KW("htmltaskreport"))
        proj->addHTMLTaskReport((HTMLTaskReport*) report);
    else if (reportType == KW("htmlresourcereport"))
        proj->addHTMLResourceReport((HTMLResourceReport*) report);
    else if (reportType == KW("htmlweeklycalendar"))
        proj->addHTMLWeeklyCalendar((HTMLWeeklyCalendar*) report);
    else if (reportType == KW("htmlaccountreport"))
        proj->addHTMLAccountReport((HTMLAccountReport*) report);

    return TRUE;
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
    if ((tt = nextToken(token)) == LCBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RCBRACE)
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
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    proj->addHTMLStatusReport(report);

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
    
    ExportReport* report;
    report = new ExportReport(proj, token, getFile(), getLine());
        
    TokenType tt;
    if ((tt = nextToken(token)) == LCBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RCBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                return FALSE;
            }

            if (token == KW("hidetask"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                report->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                report->setRollUpTask(et);
            }
            else if (token == KW("hideresource"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                report->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
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
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return FALSE;
            }
        }
    }
    else
        returnToken(tt, token);

    proj->addExportReport(report);

    return TRUE;
}

bool
ProjectFile::readReportElement(ReportElement* el)
{
    QString token;
    TokenType tt;
    if ((tt = nextToken(token)) == LCBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RCBRACE)
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
                    if (proj->getScenarioIndex(scId) == -1)
                    {
                        errorMessage(i18n("Unknown scenario %1")
                                     .arg(scId));
                        return FALSE;
                    }
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
                if (nextToken(token) != DATE)
                {
                    errorMessage(i18n("Date expected"));
                    return FALSE;
                }
                el->setStart(date2time(token));
            }
            else if (token == KW("end"))
            {
                if (nextToken(token) != DATE)
                {
                    errorMessage(i18n("Date expected"));
                    return FALSE;
                }
                el->setEnd(date2time(token) - 1);
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
                    (i18n("WARNING: The keyword 'showactual' has been "
                          "deprecated. Please use the keyword 'scenarios' "
                          "instead."));
            }
            else if (token == KW("showprojectids"))
            {
                el->setShowPIDs(TRUE);
            }
            else if (token == KW("hidetask"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                el->setHideTask(et);
            }
            else if (token == KW("rolluptask"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
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
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                el->setHideResource(et);
            }
            else if (token == KW("rollupresource"))
            {
                Operation* op;
                if ((op = readLogicalExpression()) == 0)
                    return FALSE;
                ExpressionTree* et = new ExpressionTree(op);
                el->setRollUpResource(et);
            }
            else if (token == KW("sortresources"))
            {
                if (!readSorting(el, 1))
                    return FALSE;
            }
            else if (token == KW("url"))
            {
                if (!readHtmlUrl(el))
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

bool
ProjectFile::readHtmlUrl(ReportElement* tab)
{
    QString key;
    QString url;

    if (nextToken(key) != ID)
    {
        errorMessage(i18n("URL ID expected"));
        return FALSE;
    }
    if (nextToken(url) != STRING)
    {
        errorMessage(i18n("String expected"));
        return FALSE;
    }
    if (!tab->setUrl(key, url))
    {
        errorMessage(i18n("Unknown URL ID '%1'").arg(key));
        return FALSE;
    }
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
        if ((tt = nextToken(lookAhead)) == LBRACE)
        {
            if (ExpressionTree::isFunction(token))
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
            if (proj->isAllowedFlag(token) ||
                proj->getTask(token) ||
                proj->getResource(token) ||
                proj->getAccount(token) ||
                (proj->getScenarioIndex(token) > 0) ||
                proj->isValidId(token))
            {
                op = new Operation(Operation::Id, token);
            }
            else
            {
                errorMessage(i18n("Flag or ID '%1' is unknown.").
                             arg(token));
                return 0;
            }
        }
    }
    else if (tt == STRING)
    {
        op = new Operation(Operation::String, token);
    }
    else if (tt == DATE)
    {
        time_t date;
        if ((date = date2time(token)) == 0)
        {
            errorMessage("%s", getUtilityError().latin1());
            return 0;
        }
        else
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
    else if (tt == LBRACE)
    {
        if ((op = readLogicalExpression()) == 0)
        {
            if (DEBUGEX(5))
                qDebug("exit after ()");
            return 0;
        }
        if ((tt = nextToken(token)) != RBRACE)
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
    for (int i = 0; i < ExpressionTree::arguments(name); i++)
    {
        if (DEBUGEX(5))
            qDebug("Reading function '%s' arg %d", name.latin1(), i);
        Operation* op;
        if ((op = readLogicalExpression()) == 0)
            return 0;
        args.append(op);
        if ((i < ExpressionTree::arguments(name) - 1) &&
            nextToken(token) != COMMA)
        {
            errorMessage(i18n("Comma expected. "
                              "Function '%1' needs %2 arguments.")
                         .arg(name).arg(ExpressionTree::arguments(name)));
            return 0;
        }
    }
    if ((tt = nextToken(token)) != RBRACE)
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

    if ((tt = nextToken(token)) == LCBRACE)
    {
        for ( ; ; )
        {
            if ((tt = nextToken(token)) == RCBRACE)
                break;
            else if (tt != ID)
            {
                errorMessage(i18n("Attribute ID or '}' expected"));
                return 0;
            }

            if (token == KW("title"))
            {
                if (nextToken(token) != STRING)
                {
                    errorMessage(i18n("String expected"));
                    return 0;
                }
                tci->setTitle(token);
            }
            else
            {
                errorMessage(i18n("Illegal attribute"));
                return 0;
            }
        }
    }
    else
        returnToken(tt, token);

    return tci;
}

bool
ProjectFile::readSortingMode(int& sorting)
{
    QString token;

    nextToken(token);
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
    else if (token == KW("startup") || token == KW("planstartup"))
        sorting = CoreAttributesList::StartUp;
    else if (token == KW("startdown") || token == KW("planstartdown"))
        sorting = CoreAttributesList::StartDown;
    else if (token == KW("endup") || token == KW("planendup"))
        sorting = CoreAttributesList::EndUp;
    else if (token == KW("enddown") || token == KW("planenddown"))
        sorting = CoreAttributesList::EndDown;
    else if (token == KW("actualstartup"))
        sorting = CoreAttributesList::StartUp + 0xFFFF;
    else if (token == KW("actualstartdown"))
        sorting = CoreAttributesList::StartDown + 0xFFFF;
    else if (token == KW("actualendup"))
        sorting = CoreAttributesList::EndUp + 0xFFFF;
    else if (token == KW("actualenddown"))
        sorting = CoreAttributesList::EndDown + 0xFFFF;
    else if (token == KW("planstatusup"))
        sorting = CoreAttributesList::StatusUp;
    else if (token == KW("planstatusdown"))
        sorting = CoreAttributesList::StatusDown;
    else if (token == KW("plancompletedup"))
        sorting = CoreAttributesList::CompletedUp;
    else if (token == KW("plancompleteddown"))
        sorting = CoreAttributesList::CompletedDown;
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
    else
    {
        errorMessage(i18n("Sorting criteria expected"));
        return FALSE;
    }

    return TRUE;
}

time_t
ProjectFile::date2time(const QString& date)
{
    time_t res;
    if ((res = ::date2time(date)) == 0)
        errorMessage(i18n(getUtilityError()));
    return res;
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
