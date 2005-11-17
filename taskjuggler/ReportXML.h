/*
 * Report.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Report_xml_h_
#define _Report_xml_h_

#include <stdio.h>
#include <time.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qtextstream.h>
#include <qdom.h>

#include "Report.h"

class Project;
class ExpressionTree;

class ReportXML : public Report
{
public:
   ReportXML(Project* p, const QString& f, const QString& df, int dl);
   virtual ~ReportXML() { }

   bool generate();

   static QDomElement createXMLElem( QDomDocument& doc, const QString& name,
                     const QString& val );
   
protected:
   ReportXML() { }
} ;

#endif
