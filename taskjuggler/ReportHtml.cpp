/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#include "taskjuggler.h"
#include "Project.h"
#include "Report.h"
#include "Utility.h"
#include "MacroTable.h"

#define KW(a) a

/* The following encoding table was copied from the Qt library sources since
 * this information is not available over the public API. */

struct Entity 
{
	const char * name;
	Q_UINT16 code;
};

static const Entity entitylist [] = 
{
    { "AElig", 0x00c6 },
    { "Aacute", 0x00c1 },
    { "Acirc", 0x00c2 },
    { "Agrave", 0x00c0 },
    { "Alpha", 0x0391 },
    { "AMP", 38 },
    { "Aring", 0x00c5 },
    { "Atilde", 0x00c3 },
    { "Auml", 0x00c4 },
    { "Beta", 0x0392 },
    { "Ccedil", 0x00c7 },
    { "Chi", 0x03a7 },
    { "Dagger", 0x2021 },
    { "Delta", 0x0394 },
    { "ETH", 0x00d0 },
    { "Eacute", 0x00c9 },
    { "Ecirc", 0x00ca },
    { "Egrave", 0x00c8 },
    { "Epsilon", 0x0395 },
    { "Eta", 0x0397 },
    { "Euml", 0x00cb },
    { "Gamma", 0x0393 },
    { "GT", 62 },
    { "Iacute", 0x00cd },
    { "Icirc", 0x00ce },
    { "Igrave", 0x00cc },
    { "Iota", 0x0399 },
    { "Iuml", 0x00cf },
    { "Kappa", 0x039a },
    { "Lambda", 0x039b },
    { "LT", 60 },
    { "Mu", 0x039c },
    { "Ntilde", 0x00d1 },
    { "Nu", 0x039d },
    { "OElig", 0x0152 },
    { "Oacute", 0x00d3 },
    { "Ocirc", 0x00d4 },
    { "Ograve", 0x00d2 },
    { "Omega", 0x03a9 },
    { "Omicron", 0x039f },
    { "Oslash", 0x00d8 },
    { "Otilde", 0x00d5 },
    { "Ouml", 0x00d6 },
    { "Phi", 0x03a6 },
    { "Pi", 0x03a0 },
    { "Prime", 0x2033 },
    { "Psi", 0x03a8 },
    { "QUOT", 34 },
    { "Rho", 0x03a1 },
    { "Scaron", 0x0160 },
    { "Sigma", 0x03a3 },
    { "THORN", 0x00de },
    { "Tau", 0x03a4 },
    { "Theta", 0x0398 },
    { "Uacute", 0x00da },
    { "Ucirc", 0x00db },
    { "Ugrave", 0x00d9 },
    { "Upsilon", 0x03a5 },
    { "Uuml", 0x00dc },
    { "Xi", 0x039e },
    { "Yacute", 0x00dd },
    { "Yuml", 0x0178 },
    { "Zeta", 0x0396 },
    { "aacute", 0x00e1 },
    { "acirc", 0x00e2 },
    { "acute", 0x00b4 },
    { "aelig", 0x00e6 },
    { "agrave", 0x00e0 },
    { "alefsym", 0x2135 },
    { "alpha", 0x03b1 },
    { "amp", 38 },
    { "and", 0x22a5 },
    { "ang", 0x2220 },
    { "apos", 0x0027 },
    { "aring", 0x00e5 },
    { "asymp", 0x2248 },
    { "atilde", 0x00e3 },
    { "auml", 0x00e4 },
    { "bdquo", 0x201e },
    { "beta", 0x03b2 },
    { "brvbar", 0x00a6 },
    { "bull", 0x2022 },
    { "cap", 0x2229 },
    { "ccedil", 0x00e7 },
    { "cedil", 0x00b8 },
    { "cent", 0x00a2 },
    { "chi", 0x03c7 },
    { "circ", 0x02c6 },
    { "clubs", 0x2663 },
    { "cong", 0x2245 },
    { "copy", 0x00a9 },
    { "crarr", 0x21b5 },
    { "cup", 0x222a },
    { "curren", 0x00a4 },
    { "dArr", 0x21d3 },
    { "dagger", 0x2020 },
    { "darr", 0x2193 },
    { "deg", 0x00b0 },
    { "delta", 0x03b4 },
    { "diams", 0x2666 },
    { "divide", 0x00f7 },
    { "eacute", 0x00e9 },
    { "ecirc", 0x00ea },
    { "egrave", 0x00e8 },
    { "empty", 0x2205 },
    { "emsp", 0x2003 },
    { "ensp", 0x2002 },
    { "epsilon", 0x03b5 },
    { "equiv", 0x2261 },
    { "eta", 0x03b7 },
    { "eth", 0x00f0 },
    { "euml", 0x00eb },
    { "euro", 0x20ac },
    { "exist", 0x2203 },
    { "fnof", 0x0192 },
    { "forall", 0x2200 },
    { "frac12", 0x00bd },
    { "frac14", 0x00bc },
    { "frac34", 0x00be },
    { "frasl", 0x2044 },
    { "gamma", 0x03b3 },
    { "ge", 0x2265 },
    { "gt", 62 },
    { "hArr", 0x21d4 },
    { "harr", 0x2194 },
    { "hearts", 0x2665 },
    { "hellip", 0x2026 },
    { "iacute", 0x00ed },
    { "icirc", 0x00ee },
    { "iexcl", 0x00a1 },
    { "igrave", 0x00ec },
    { "image", 0x2111 },
    { "infin", 0x221e },
    { "int", 0x222b },
    { "iota", 0x03b9 },
    { "iquest", 0x00bf },
    { "isin", 0x2208 },
    { "iuml", 0x00ef },
    { "kappa", 0x03ba },
    { "lArr", 0x21d0 },
    { "lambda", 0x03bb },
    { "lang", 0x2329 },
    { "laquo", 0x00ab },
    { "larr", 0x2190 },
    { "lceil", 0x2308 },
    { "ldquo", 0x201c },
    { "le", 0x2264 },
    { "lfloor", 0x230a },
    { "lowast", 0x2217 },
    { "loz", 0x25ca },
    { "lrm", 0x200e },
    { "lsaquo", 0x2039 },
    { "lsquo", 0x2018 },
    { "lt", 60 },
    { "macr", 0x00af },
    { "mdash", 0x2014 },
    { "micro", 0x00b5 },
    { "middot", 0x00b7 },
    { "minus", 0x2212 },
    { "mu", 0x03bc },
    { "nabla", 0x2207 },
    { "nbsp", 0x00a0 },
    { "ndash", 0x2013 },
    { "ne", 0x2260 },
    { "ni", 0x220b },
    { "not", 0x00ac },
    { "notin", 0x2209 },
    { "nsub", 0x2284 },
    { "ntilde", 0x00f1 },
    { "nu", 0x03bd },
    { "oacute", 0x00f3 },
    { "ocirc", 0x00f4 },
    { "oelig", 0x0153 },
    { "ograve", 0x00f2 },
    { "oline", 0x203e },
    { "omega", 0x03c9 },
    { "omicron", 0x03bf },
    { "oplus", 0x2295 },
    { "or", 0x22a6 },
    { "ordf", 0x00aa },
    { "ordm", 0x00ba },
    { "oslash", 0x00f8 },
    { "otilde", 0x00f5 },
    { "otimes", 0x2297 },
    { "ouml", 0x00f6 },
    { "para", 0x00b6 },
    { "part", 0x2202 },
    { "percnt", 0x0025 },
    { "permil", 0x2030 },
    { "perp", 0x22a5 },
    { "phi", 0x03c6 },
    { "pi", 0x03c0 },
    { "piv", 0x03d6 },
    { "plusmn", 0x00b1 },
    { "pound", 0x00a3 },
    { "prime", 0x2032 },
    { "prod", 0x220f },
    { "prop", 0x221d },
    { "psi", 0x03c8 },
    { "quot", 34 },
    { "rArr", 0x21d2 },
    { "radic", 0x221a },
    { "rang", 0x232a },
    { "raquo", 0x00bb },
    { "rarr", 0x2192 },
    { "rceil", 0x2309 },
    { "rdquo", 0x201d },
    { "real", 0x211c },
    { "reg", 0x00ae },
    { "rfloor", 0x230b },
    { "rho", 0x03c1 },
    { "rlm", 0x200f },
    { "rsaquo", 0x203a },
    { "rsquo", 0x2019 },
    { "sbquo", 0x201a },
    { "scaron", 0x0161 },
    { "sdot", 0x22c5 },
    { "sect", 0x00a7 },
    { "shy", 0x00ad },
    { "sigma", 0x03c3 },
    { "sigmaf", 0x03c2 },
    { "sim", 0x223c },
    { "spades", 0x2660 },
    { "sub", 0x2282 },
    { "sube", 0x2286 },
    { "sum", 0x2211 },
    { "sup1", 0x00b9 },
    { "sup2", 0x00b2 },
    { "sup3", 0x00b3 },
    { "sup", 0x2283 },
    { "supe", 0x2287 },
    { "szlig", 0x00df },
    { "tau", 0x03c4 },
    { "there4", 0x2234 },
    { "theta", 0x03b8 },
    { "thetasym", 0x03d1 },
    { "thinsp", 0x2009 },
    { "thorn", 0x00fe },
    { "tilde", 0x02dc },
    { "times", 0x00d7 },
    { "trade", 0x2122 },
    { "uArr", 0x21d1 },
    { "uacute", 0x00fa },
    { "uarr", 0x2191 },
    { "ucirc", 0x00fb },
    { "ugrave", 0x00f9 },
    { "uml", 0x00a8 },
    { "upsih", 0x03d2 },
    { "upsilon", 0x03c5 },
    { "uuml", 0x00fc },
    { "weierp", 0x2118 },
    { "xi", 0x03be },
    { "yacute", 0x00fd },
    { "yen", 0x00a5 },
    { "yuml", 0x00ff },
    { "zeta", 0x03b6 },
    { "zwj", 0x200d },
    { "zwnj", 0x200c },
    { "", 0x0000 }
};

static QMap<Q_UINT16, QCString> *HtmlMap = 0;

ReportHtml::ReportHtml(Project* p, const QString& f, time_t s, time_t e,
					   const QString& df, int dl) :
   Report(p, f, s, e, df, dl)
{
	colDefault = 0xf3ebae;
	colDefaultLight = 0xfffadd;
	colWeekend = 0xffec80;
	colVacation = 0xfffc60;
	colAvailable = 0xa4ff8d;
	colBooked = 0xff5a5d;
	colBookedLight = 0xffbfbf;
	colHeader = 0xa5c2ff;
	colMilestone = 0xff2a2a;
	colCompleted = 0x87ff75;
	colCompletedLight = 0xa1ff9a;
	colToday = 0xa387ff;

	barLabels = BLT_LOAD;

	registerUrl(KW("dayheader"));
	registerUrl(KW("monthheader"));
	registerUrl(KW("resourcename"));
	registerUrl(KW("taskname"));
	registerUrl(KW("weekheader"));
	registerUrl(KW("yearheader"));
}

void
ReportHtml::generatePlanTask(Task* t, Resource* r, uint no)
{
	s << "<tr valign=\"middle\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("seqno"))
			textTwoRows((r == 0 ? QString().sprintf("%d.", t->getSequenceNo()) :
						 QString("")), r != 0, "");
		else if (*it == KW("no"))
			textTwoRows((r == 0 ? QString().sprintf("%d.", no) :
						 QString("")), r != 0, "");
		else if (*it == KW("index"))
			textTwoRows((r == 0 ? QString().sprintf("%d.", t->getIndex()) :
						 QString("")), r != 0, "");
		else if (*it == KW("id"))
			textTwoRows(htmlFilter(t->getId()), r != 0, "left");
		else if (*it == KW("name"))
			taskName(t, r, r == 0);
		else if (*it == KW("start"))
			s << "<td class=\""
			  << (t->isStartOk(Task::Plan) ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" style=\"text-align:left white-space:nowrap\">"
			  << time2user(t->getStart(Task::Plan), timeFormat)
			  << "</td>" << endl;
		else if (*it == KW("end"))
			s << "<td class=\""
			  << (t->isEndOk(Task::Plan) ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" style=\"text-align:left white-space:nowrap\">"
			  << time2user(t->getEnd(Task::Plan) + 1, timeFormat)
			  << "</td>" << endl;
		else if (*it == KW("minstart"))
			textTwoRows(time2user(t->getMinStart(), timeFormat), r != 0, "");
		else if (*it == KW("maxstart"))
			textTwoRows(time2user(t->getMaxStart(), timeFormat),
					   	r != 0, "");
		else if (*it == KW("minend"))
			textTwoRows(time2user(t->getMinEnd(), timeFormat), r != 0, "");
		else if (*it == KW("maxend"))
			textTwoRows(time2user(t->getMaxEnd(), timeFormat), r != 0, "");
		else if (*it == KW("startbuffer"))
			textTwoRows(QString().sprintf
						("%3.0f", t->getStartBuffer(Task::Plan)), r != 0, 
						"right");
		else if (*it == KW("endbuffer"))
			textTwoRows(QString().sprintf
						("%3.0f", t->getEndBuffer(Task::Plan)), r != 0, 
						"right");
		else if (*it == KW("startbufferend"))
			textOneRow(time2user(t->getStartBufferEnd(Task::Plan) + 1,
								 timeFormat), r != 0, "left");
		else if (*it == KW("endbufferstart"))
			textOneRow(time2user(t->getEndBufferStart(Task::Plan), timeFormat),
					   r != 0, "left");
		else if (*it == KW("duration"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(t->getCalcDuration(Task::Plan))
			  << "</td>" << endl;
		}
		else if (*it == KW("effort"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(t->getLoad(Task::Plan, Interval(start, end), r))
			  << "</td>" << endl;
		}
		else if (*it == KW("projectid"))
			textTwoRows(t->getProjectId() + " (" +
						project->getIdIndex(t->getProjectId()) + ")", r != 0,
						"left");
		else if (*it == KW("resources"))
			planResources(t, r != 0);
		else if (*it == KW("responsible"))
			if (t->getResponsible())
				textTwoRows(htmlFilter(t->getResponsible()->getName()), r != 0,
							"left");
			else
				textTwoRows("&nbsp", r != 0, "left");
		else if (*it == KW("responsibilities"))
			emptyPlan(r != 0);
		else if (*it == KW("depends"))
			generateDepends(t, r != 0);
		else if (*it == KW("follows"))
			generateFollows(t, r != 0);
		else if (*it == KW("schedule"))
			emptyPlan(r != 0);
		else if (*it == KW("mineffort"))
			emptyPlan(r != 0);
		else if (*it == KW("maxeffort"))
			emptyPlan(r != 0);
		else if (*it == KW("rate"))
			emptyPlan(r != 0);
		else if (*it == KW("kotrusid"))
			emptyPlan(r != 0);
		else if (*it == KW("note"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" rowspan=\""
			  << (showActual ? "2" : "1")
			  << "\" style=\"text-align:left\">"
			  << "<span style=\"font-size:100%\">";
			if (t->getNote().isEmpty())
				s << "&nbsp;";
			else
				s << htmlFilter(t->getNote());
			s << "</span></td>" << endl;
		}
		else if (*it == KW("costs"))
			textOneRow(
				QString().sprintf("%.*f", project->getCurrencyDigits(),
								  t->getCredits(Task::Plan,
												Interval(start, end), r)),
				r != 0,
				"right");
		else if (*it == KW("priority"))
			textTwoRows(QString().sprintf("%d", t->getPriority()), r != 0,
						"right");
		else if (*it == KW("flags"))
			flagList(t, r);
		else if (*it == KW("daily"))
			dailyTaskPlan(t, r);
		else if (*it == KW("weekly"))
			weeklyTaskPlan(t, r);
		else if (*it == KW("monthly"))
			monthlyTaskPlan(t, r);
		else
			qFatal("generatePlanTask: Unknown Column %s",
				   (*it).latin1());
	}
	s << "</tr>" << endl;
}

void
ReportHtml::generateActualTask(Task* t, Resource* r)
{
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin();
		 it != columns.end();
		 ++it )
	{
		if (*it == KW("start"))
		{
			s << "<td class=\""
			  << (t->isStartOk(Task::Actual) ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" style=\"text-align:left white-space:nowrap\">"
			  << time2user(t->getStart(Task::Actual), timeFormat)
			  << "</td>" << endl;
		}
		else if (*it == KW("end"))
		{
			s << "<td class=\""
			  << (t->isEndOk(Task::Actual) ?
				  (r == 0 ? "default" : "defaultlight") : "milestone")
			  << "\" style=\"white-space:nowrap\">"
			  << time2user(t->getEnd(Task::Actual) + 1, timeFormat)
			  << "</td>" << endl;
		}
		else if (*it == KW("startbufferend"))
			textOneRow(time2user(t->getStartBufferEnd(Task::Actual) + 1, 
								 timeFormat), r != 0, "left");
		else if (*it == KW("endbufferstart"))
			textOneRow(time2user(t->getEndBufferStart(Task::Actual), 
								 timeFormat), r != 0, "left");
		else if (*it == KW("duration"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(t->getCalcDuration(Task::Actual))
			  << "</td>" << endl;
		}
		else if (*it == KW("effort"))
		{
			s << "<td class=\""
			  << (r == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(t->getLoad(Task::Actual, Interval(start, end), r))
			  << "</td>" << endl;
		}
		else if (*it == KW("resources"))
			actualResources(t, r != 0);
		else if (*it == "costs")
			textOneRow(
				QString().sprintf("%.*f", project->getCurrencyDigits(),
								  t->getCredits(Task::Actual,
												Interval(start, end), r)),
				r != 0, "right");
		if (*it == KW("daily"))
			dailyTaskActual(t, r);
		else if (*it == KW("weekly"))
			weeklyTaskActual(t, r);
		else if (*it == KW("monthly"))
			monthlyTaskActual(t, r);
	}
	s << "</tr>" << endl;
}

void
ReportHtml::generatePlanResource(Resource* r, Task* t, uint no)
{
	s << "<tr valign=\"middle\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("seqno"))
			textTwoRows((t == 0 ? QString().sprintf("%d.", r->getSequenceNo()) :
						 QString("")), t != 0, "");
		else if (*it == KW("no"))
			textTwoRows((t == 0 ? QString().sprintf("%d.", no) :
						 QString("")), t != 0, "");
		else if (*it == KW("index"))
			textTwoRows((t == 0 ? QString().sprintf("%d.", r->getIndex()) :
						 QString("")), t != 0, "");
		else if (*it == KW("id"))
			textTwoRows(htmlFilter(r->getId()), t != 0, "left");
		else if (*it == KW("name"))
			resourceName(r, t, FALSE);
		else if (*it == KW("start"))
			emptyPlan(t != 0);
		else if (*it == KW("end"))
			emptyPlan(t != 0);
		else if (*it == KW("minstart"))
			emptyPlan(t != 0);
		else if (*it == KW("maxstart"))
			emptyPlan(t != 0);
		else if (*it == KW("minend"))
			emptyPlan(t != 0);
		else if (*it == KW("maxend"))
			emptyPlan(t != 0);
		else if (*it == KW("startbuffer"))
			emptyPlan(t != 0);
		else if (*it == KW("endbuffer"))
			emptyPlan(t != 0);
		else if (*it == KW("startbufferend"))
			emptyPlan(t != 0);
		else if (*it == KW("endbufferstart"))
			emptyPlan(t != 0);
		else if (*it == KW("duration"))
			emptyPlan(t != 0);
		else if (*it == KW("effort"))
		{
			s << "<td class=\""
			  << (t == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(r->getLoad(Task::Plan, Interval(start, end), t))
			  << "</td>" << endl;
		}
		else if (*it == KW("projectid"))
			emptyPlan(t != 0);
		else if (*it == KW("resources"))
			emptyPlan(t != 0);
		else if (*it == KW("responsible"))
			emptyPlan(t != 0);
		else if (*it == KW("responsibilities"))
			generateResponsibilities(r, t != 0);
		else if (*it == KW("depends"))
			emptyPlan(t != 0);
		else if (*it == KW("follows"))
			emptyPlan(t != 0);
		else if (*it == KW("schedule"))
			generateSchedule(Task::Plan, r, t);
		else if (*it == KW("mineffort"))
			textTwoRows(QString().sprintf("%.2f", r->getMinEffort()), t != 0,
						"right");
		else if (*it == KW("maxeffort"))
			textTwoRows(QString().sprintf("%.2f", r->getMaxEffort()), t != 0,
						"right");
		else if (*it == KW("rate"))
			textTwoRows(QString().sprintf("%.*f", project->getCurrencyDigits(),
										  r->getRate()), t != 0,
						"right");
		else if (*it == KW("kotrusid"))
			textTwoRows(r->getKotrusId(), t != 0, "left");
		else if (*it == KW("note"))
			emptyPlan(t != 0);
		else if (*it == KW("costs"))
			textOneRow(
				QString().sprintf("%.*f", project->getCurrencyDigits(),
								  r->getCredits(Task::Plan,
											   	Interval(start, end), t)),
				t != 0, "right");
		else if (*it == KW("priority"))
			emptyPlan(t != 0);
		else if (*it == KW("flags"))
			flagList(r, t);
		else if (*it == KW("daily"))
			dailyResourcePlan(r, t);
		else if (*it == KW("weekly"))
			weeklyResourcePlan(r, t);
		else if (*it == KW("monthly"))
			monthlyResourcePlan(r, t);
		else
			qFatal("generatePlanResource: Unknown Column %s",
				   (*it).latin1());
	}
	s << "</tr>" << endl;
}

void
ReportHtml::generateActualResource(Resource* r, Task* t)
{
	s << "<tr valign=\"middle\">";
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("effort"))
		{
			s << "<td class=\""
			  << (t == 0 ? "default" : "defaultlight")
			  << "\" style=\"text-align:right white-space:nowrap\">"
			  << scaledLoad(r->getLoad(Task::Actual, Interval(start, end), t))
			  << "</td>" << endl;
		}
		else if (*it == KW("schedule"))
			generateSchedule(Task::Actual, r, t);
		else if (*it == KW("costs"))
			textOneRow(
				QString().sprintf("%.*f", project->getCurrencyDigits(),
								  r->getCredits(Task::Actual,
												Interval(start, end), t)),
				t != 0,
				"right");
		else if (*it == KW("daily"))
			dailyResourceActual(r, t);
		else if (*it == KW("weekly"))
			weeklyResourceActual(r, t);
		else if (*it == KW("monthly"))
			monthlyResourceActual(r, t);
	}
	s << "</tr>" << endl;
}

void
ReportHtml::reportHTMLHeader()
{
	s << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">"
		<< endl
		<< "<!-- Generated by TaskJuggler v"VERSION" -->" << endl
		<< "<!-- For details about TaskJuggler see "
		<< TJURL << " -->" << endl
		<< "<html>" << endl
		<< "<head>" << endl
		<< "<title>Task Report</title>" << endl
		<< "<style type=\"text/css\"><!--" << endl;
	if (rawStyleSheet.isEmpty())
	{
		s.reset();
		s.setf(QTextStream::hex);
		s << ".default { background-color:#" << colDefault
			<< "; font-size:70%; text-align:center }" << endl
			<< ".defaultlight { background-color:#" << colDefaultLight
			<< "; font-size:70%; text-align:center }" << endl
			<< ".task { background-color:#" << colDefault
			<< "; font-size:100%; text-align:left }" << endl
			<< ".tasklight { background-color:#" << colDefaultLight
			<< "; font-size:100%; text-align:left }" << endl
			<< ".available { background-color:#" << colAvailable
			<< "; font-size:70%; text-align:center }" << endl
			<< ".vacation { background-color:#" << colVacation
			<< "; font-size:70%; text-align:center }" << endl
			<< ".weekend { background-color:#" << colWeekend
			<< "; font-size:70%; text-align:center }" << endl
			<< ".milestone { background-color:#" << colMilestone
			<< "; font-size:70%; text-align:center }" << endl
			<< ".booked { background-color:#" << colBooked
			<< "; font-size:70%; text-align:center }" << endl
			<< ".bookedlight { background-color:#" << colBookedLight
			<< "; font-size:70%; text-align:center }" << endl
			<< ".headersmall { background-color:#" << colHeader
			<< "; font-size:70%; text-align:center }" << endl
			<< ".headerbig { background-color:#" << colHeader
			<< "; font-size:110%; font-weight:bold; text-align:center }" << endl
			<< ".completed { background-color:#" << colCompleted
			<< "; font-size:70%; text-align:center }" << endl
			<< ".completedlight { background-color:#" << colCompletedLight
			<< "; font-size:70%; text-align:center }" << endl
			<< ".today { background-color:#" << colToday
			<< "; font-size:70%; text-align:center }" << endl;
	}
	else
		s << rawStyleSheet << endl;
	s << "--></style>" << endl;
	s << "</head>" << endl
	  << "<body>" << endl;

	if (!headline.isEmpty())
		s << "<h1>" << htmlFilter(headline) << "</h1>" << endl;
	if (!caption.isEmpty())
		s << "<p>" << htmlFilter(caption) << "</p>" << endl;
	if (!rawHead.isEmpty())
		s << rawHead << endl;
	s << "<table align=\"center\" cellpadding=\"1\">\n" << endl;
}

void
ReportHtml::reportHTMLFooter()
{
	s << "</table>" << endl;
	if (!rawTail.isEmpty())
		s << rawTail << endl;

	s << "<p><span style=\"font-size:0.7em\">";
	if (!project->getCopyright().isEmpty())
		s << htmlFilter(project->getCopyright()) << " - ";
	s << "Version " << htmlFilter(project->getVersion())
	  << " - Created with <a HREF=\"" << TJURL <<
	  "\">TaskJuggler v"
	  << VERSION << "</a></span></p>" << endl << "</body>\n";
}

bool
ReportHtml::generateTableHeader()
{
	// Header line 1
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("seqno"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Seq. No.</td>";
		else if (*it == KW("no"))
			s << "<td class=\"headerbig\" rowspan=\"2\">No.</td>";
		else if (*it == KW("index"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Index No.</td>";
		else if (*it == KW("id"))
			s << "<td class=\"headerbig\" rowspan=\"2\">ID</td>";
		else if (*it == KW("name"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Name</td>";
		else if (*it == KW("start"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Start</td>";
		else if (*it == KW("end"))
			s << "<td class=\"headerbig\" rowspan=\"2\">End</td>";
		else if (*it == KW("minstart"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. Start</td>";
		else if (*it == KW("maxstart"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. Start</td>";
		else if (*it == KW("minend"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. End</td>";
		else if (*it == KW("maxend"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. End</td>";
		else if (*it == KW("startbufferend"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Start Buf. End</td>";
		else if (*it == KW("endbufferstart"))
			s << "<td class=\"headerbig\" rowspan=\"2\">End Buf. Start</td>";
		else if (*it == KW("startbuffer"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Start Buf.</td>";
		else if (*it == KW("endbuffer"))
			s << "<td class=\"headerbig\" rowspan=\"2\">End Buf.</td>";
		else if (*it == KW("duration"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Duration</td>";
		else if (*it == KW("effort"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Effort</td>";
		else if (*it == KW("projectid"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Project ID</td>";
		else if (*it == KW("resources"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Resources</td>";
		else if (*it == KW("responsible"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Responsible</td>";
		else if (*it == KW("responsibilities"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Responsibilities</td>";
		else if (*it == KW("depends"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Dependencies</td>";
		else if (*it == KW("follows"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Followers</td>";
		else if (*it == KW("schedule"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Schedule</td>";
		else if (*it == KW("mineffort"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Min. Effort</td>";
		else if (*it == KW("maxeffort"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Max. Effort</td>";
		else if (*it == KW("flags"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Flags</td>";
		else if (*it == KW("rate"))
		{
			s << "<td class=\"headerbig\" rowspan=\"2\">Rate";
			if (!project->getCurrency().isEmpty())
				s << " " << htmlFilter(project->getCurrency());
			s << "</td>";
		}
		else if (*it == KW("kotrusid"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Kotrus ID</td>";
		else if (*it == KW("note"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Note</td>";
		else if (*it == KW("costs"))
		{
			s << "<td class=\"headerbig\" rowspan=\"2\">Costs";
			if (!project->getCurrency().isEmpty())
				s << " " << htmlFilter(project->getCurrency());
			s << "</td>";
		}
		else if (*it == KW("priority"))
			s << "<td class=\"headerbig\" rowspan=\"2\">Priority</td>";
		else if (*it == KW("daily"))
			htmlDailyHeaderMonths();
		else if (*it == KW("weekly"))
			htmlWeeklyHeaderMonths();
		else if (*it == KW("monthly"))
			htmlMonthlyHeaderYears();
		else
		{
			qWarning("Unknown Column '%s' for HTML Task Report\n",
					(*it).latin1());
			return FALSE;
		}
	}
	s << "</tr>" << endl;

	// Header line 2
	bool td = FALSE;
	s << "<tr>" << endl;
	for (QStringList::Iterator it = columns.begin(); it != columns.end();
		 ++it )
	{
		if (*it == KW("daily"))
		{
			td = TRUE;
			htmlDailyHeaderDays();
		}
		else if (*it == KW("weekly"))
		{
			td = TRUE;
			htmlWeeklyHeaderWeeks();
		}
		else if (*it == KW("monthly"))
		{
			td = TRUE;
			htmlMonthlyHeaderMonths();
		}
	}
	if (!td)
		s << "<td>&nbsp;</td>";
	s << "</tr>\n" << endl;

	return TRUE;
}

void
ReportHtml::htmlDailyHeaderDays(bool highlightNow)
{
	// Generates the 2nd header line for daily calendar views.
	for (time_t day = midnight(start); day < end; day = sameTimeNextDay(day))
	{
		int dom = dayOfMonth(day);
		s << "<td class=\"" <<
			(highlightNow && isSameDay(project->getNow(), day) ?
			 "today" : isWeekend(day) ? "weekend" : "headersmall")
		  << "\"><span style=\"font-size:0.8em\">&nbsp;";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d", dom),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(day)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(day)),
							  defFileName, defFileLine));
		if (dom < 10)
			s << "&nbsp;";
		s << generateUrl(KW("dayheader"), QString().sprintf("%d", dom));
		s << "</span></td>";
	}
}

void
ReportHtml::htmlDailyHeaderMonths()
{
	// Generates the 1st header line for daily calendar views.
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t day = midnight(start); day < end;
		 day = beginOfMonth(sameTimeNextMonth(day)))
	{
		int left = daysLeftInMonth(day);
		if (left > daysBetween(day, end))
			left = daysBetween(day, end);
		s << "<td class=\"headerbig\" colspan=\""
			<< QString().sprintf("%d", left) << "\">"; 
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(day)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(day)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(day)),
							  defFileName, defFileLine));
		s << generateUrl(KW("monthheader"), monthAndYear(day)); 
		s << "</td>" << endl;
	}
}

void
ReportHtml::htmlWeeklyHeaderWeeks(bool highlightNow)
{
	// Generates the 2nd header line for weekly calendar views.
	for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
		 week = sameTimeNextWeek(week))
	{
		int woy = weekOfYear(week, weekStartsMonday);
		s << "<td class=\"" <<
			(highlightNow && isSameWeek(project->getNow(), week,
										weekStartsMonday) ?
			 "today" : "headersmall")
		  << "\"><span style=\"font-size:0.8em\">&nbsp;";
		if (woy < 10)
			s << "&nbsp;";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(woy)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(woy)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(woy)),
							  defFileName, defFileLine));
		s << generateUrl(KW("weekheader"), QString().sprintf("%d", woy));
		s << "</span></td>";
	}
}

void
ReportHtml::htmlWeeklyHeaderMonths()
{
	static const char* mnames[] =
   	{
	   	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
   	};
	// Generates the 1st header line for weekly calendar views.
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t week = beginOfWeek(start, weekStartsMonday); week < end; )
	{
		int currMonth = monthOfWeek(week, weekStartsMonday);
		int left;
		time_t wi = sameTimeNextWeek(week);
		for (left = 1 ; wi < end &&
			 monthOfWeek(wi, weekStartsMonday) == currMonth;
			 wi = sameTimeNextWeek(wi))
			left++;
			 
		s << "<td class=\"headerbig\" colspan=\""
		  << QString().sprintf("%d", left) << "\">";
		mt.clear();
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfWeek(week,
															weekStartsMonday)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", yearOfWeek(week,
															weekStartsMonday)),
							  defFileName, defFileLine));
		s << generateUrl(KW("monthheader"), 
						 QString("%1 %2").
						 arg(mnames[monthOfWeek(week, weekStartsMonday) - 1]).
						 arg(yearOfWeek(week, weekStartsMonday)));
		s << "</td>" << endl;
		week = wi;
	}
}

void
ReportHtml::htmlMonthlyHeaderMonths(bool highlightNow)
{
	// Generates 2nd header line of monthly calendar view.
	static const char* mnames[] =
   	{
	   	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
   	};

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		int moy = monthOfYear(month);
		s << "<td class=\"" <<
			(highlightNow && isSameMonth(project->getNow(), month) ?
			 "today" : "headersmall")
		  << "\"><span style=\"font-size:0.8em\">&nbsp;";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(moy)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(moy)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", year(moy)),
							  defFileName, defFileLine));
		s << generateUrl(KW("monthheader"), mnames[moy - 1]);
		s << "</span></td>";
	}
}

void
ReportHtml::htmlMonthlyHeaderYears()
{
	// Generates 1st header line of monthly calendar view.
	if (!hidePlan && showActual)
		s << "<td class=\"headerbig\" rowspan=\"2\">&nbsp;</td>";

	for (time_t year = midnight(start); year < end;
		 year = beginOfYear(sameTimeNextYear(year)))
	{
		int left = monthLeftInYear(year);
		if (left > monthsBetween(year, end))
			left = monthsBetween(year, end);
		s << "<td class=\"headerbig\" colspan=\""
		  << QString().sprintf("%d", left) << "\">";
		mt.clear();
		mt.addMacro(new Macro(KW("day"), QString().sprintf("%02d",
														   dayOfMonth(year)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("month"),
							  QString().sprintf("%02d", monthOfYear(year)),
							  defFileName, defFileLine));
		mt.addMacro(new Macro(KW("year"),
							  QString().sprintf("%04d", ::year(year)),
							  defFileName, defFileLine));
		s << generateUrl(KW("yearheader"),
						 QString().sprintf("%d", ::year(year)));
		s << "</td>" << endl;
	}
}

void
ReportHtml::emptyPlan(bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\">&nbsp;</td>";
}

void
ReportHtml::emptyActual(bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\">&nbsp;</td>";
}

void
ReportHtml::textOneRow(const QString& text, bool light, const QString& align)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default") << "\"";
	if (!align.isEmpty())
		s << " style=\"text-align:" << align << " white-space:nowrap\"";
	s << ">" << text << "</td>";
}

void
ReportHtml::textTwoRows(const QString& text, bool light, const QString& align)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1") << "\"";
	if (!align.isEmpty())
		s << " style=\"text-align:" << align << "; white-space:nowrap\"";
	s << ">" << text << "</td>";
}

void
ReportHtml::dailyResourcePlan(Resource* r, Task* t)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = r->getLoad(Task::Plan, Interval(day).firstDay(), t);
		QString bgCol = 
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" : "bookedlight") :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) || r->hasVacationDay(day) ?
			"vacation" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::dailyResourceActual(Resource* r, Task* t)
{
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = r->getLoad(Task::Actual, Interval(day).firstDay(), t);
		QString bgCol = 
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(Task::Plan, sameTimeNextDay(day) - 1) ?
			  "completedlight" : "bookedlight")) :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) || r->hasVacationDay(day) ?
			"vacation" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void 
ReportHtml::dailyTaskPlan(Task* t, Resource* r)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = t->getLoad(Task::Plan, Interval(day).firstDay(), r);
		QString bgCol = 
			t->isActive(Task::Plan, Interval(day).firstDay()) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isBuffer(Task::Plan, Interval(day).firstDay()) ?
			  "booked" : "bookedlight")) :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) ? "vacation" :
			(r == 0 ? "default" : "defaultlight");
		if (showPIDs)
		{
			QString pids = r->getProjectIDs(Task::Plan,
										   	Interval(day).firstDay(), t);
			reportPIDs(pids, bgCol, !r->isGroup());
		}
		else
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::dailyTaskActual(Task* t, Resource* r)
{
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t day = midnight(start); day < end;
		 day = sameTimeNextDay(day))
	{
		double load = t->getLoad(Task::Actual, Interval(day).firstDay(), r);
		QString bgCol = 
			t->isActive(Task::Actual, Interval(day).firstDay()) ? 
			(t->isCompleted(Task::Plan, sameTimeNextDay(day) - 1) ?
			 (r == 0 ? "completed" : "completedlight") :
			 t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isBuffer(Task::Actual, Interval(day).firstDay())
			  ? "booked" : "bookedlight")) :
			isSameDay(project->getNow(), day) ? "today" :
			isWeekend(day) ? "weekend" :
			project->isVacation(day) ? "vacation" :
			(r == 0 ? "default" : "defaultlight");
		if (showPIDs)
		{
			QString pids = r->getProjectIDs(Task::Actual,
											Interval(day).firstDay(), t);
			reportPIDs(pids, bgCol, !r->isGroup());
		}
		else
			reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::weeklyResourcePlan(Resource* r, Task* t)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load =
		   	r->getLoad(Task::Plan, 
					   Interval(week).firstWeek(weekStartsMonday), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" : "bookedlight") :
			isSameWeek(project->getNow(), week, weekStartsMonday) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		if (showPIDs)
		{
			QString pids =
			   	r->getProjectIDs(Task::Plan, Interval(week).
								 firstWeek(weekStartsMonday), t);
			reportPIDs(pids, bgCol, !r->isGroup());
		}
		else
			reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::weeklyResourceActual(Resource* r, Task* t)
{
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = 
			r->getLoad(Task::Actual, 
					   Interval(week).firstWeek(weekStartsMonday), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(Task::Plan, sameTimeNextWeek(week) - 1) ?
			  "completedlight" : "bookedlight")) :
			isSameWeek(project->getNow(), week, weekStartsMonday) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		if (showPIDs)
		{
			QString pids = r->getProjectIDs(Task::Actual, Interval(week).
											firstWeek(weekStartsMonday), t);
			reportPIDs(pids, bgCol, !r->isGroup());
		}
		else
			reportLoad(load, bgCol, !r->isGroup());
	}
}

void 
ReportHtml::weeklyTaskPlan(Task* t, Resource* r)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = t->getLoad(Task::Plan, Interval(week).
								 firstWeek(weekStartsMonday), r);
		QString bgCol = 
			t->isActive(Task::Plan, 
						Interval(week).firstWeek(weekStartsMonday)) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isBuffer(Task::Plan, Interval(week).
									 firstWeek(weekStartsMonday))
			  ? "booked" : "bookedlight")) :
			isSameWeek(project->getNow(), week, weekStartsMonday) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::weeklyTaskActual(Task* t, Resource* r)
{
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t week = beginOfWeek(start, weekStartsMonday); week < end;
		 week = sameTimeNextWeek(week))
	{
		double load = t->getLoad(Task::Actual, Interval(week).
								 firstWeek(weekStartsMonday), r);
		QString bgCol = 
			t->isActive(Task::Actual,
						Interval(week).firstWeek(weekStartsMonday)) ?
			(t->isCompleted(Task::Plan, sameTimeNextWeek(week) - 1) ?
			 (r == 0 ? "completed" : "completedlight") :
			 t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isBuffer(Task::Actual, Interval(week).
										   firstWeek(weekStartsMonday))
			  ? "booked" : "bookedlight")) :
			isSameWeek(project->getNow(), week, weekStartsMonday) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::monthlyResourcePlan(Resource* r, Task* t)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = r->getLoad(Task::Plan, Interval(month).firstMonth(), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" : "bookedlight") :
			isSameMonth(project->getNow(), month) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::monthlyResourceActual(Resource* r, Task* t)
{
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = r->getLoad(Task::Actual, Interval(month).firstMonth(), t);
		QString bgCol =
			load > r->getMinEffort() * r->getEfficiency() ?
			(t == 0 ? "booked" :
			 (t->isCompleted(Task::Plan, sameTimeNextMonth(month) - 1) ?
			  "completedlight" : "bookedlight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(t == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !r->isGroup());
	}
}

void
ReportHtml::monthlyTaskPlan(Task* t, Resource* r)
{
	if (hidePlan)
		return;

	if (showActual)
		s << "<td class=\"headersmall\">Plan</td>" << endl;

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = t->getLoad(Task::Plan, Interval(month).firstMonth(), r);
		QString bgCol = 
			t->isActive(Task::Plan, Interval(month).firstMonth()) ?
			(t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isBuffer(Task::Plan, Interval(month).firstMonth())
			  ? "booked" : "bookedlight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::monthlyTaskActual(Task* t, Resource* r)
{
	if (!hidePlan)
		s << "<td class=\"headersmall\">Actual</td>" << endl;

	for (time_t month = beginOfMonth(start); month < end;
		 month = sameTimeNextMonth(month))
	{
		double load = t->getLoad(Task::Actual, Interval(month).firstMonth(), r);
		QString bgCol = 
			t->isActive(Task::Actual, Interval(month).firstMonth()) ?
			(t->isCompleted(Task::Plan, sameTimeNextMonth(month) - 1) ?
			 (r == 0 ? "completed" : "completedlight"):
			 t->isMilestone() ? "milestone" :
			 (r == 0 && !t->isBuffer(Task::Actual, 
									 Interval(month).firstMonth())
			  ? "booked" : "bookedlight")) :
			isSameMonth(project->getNow(), month) ? "today" :
			(r == 0 ? "default" : "defaultlight");
		reportLoad(load, bgCol, !t->isContainer());
	}
}

void
ReportHtml::taskName(Task* t, Resource* r, bool big)
{
	QString spaces;
	int fontSize = big ? 100 : 90; 
	if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
		for (Resource* rp = r ; rp != 0; rp = rp->getParent())
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";

	mt.clear();
	mt.addMacro(new Macro(KW("taskid"), t->getId(), defFileName,
						  defFileLine));

	if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
	{
		for (uint i = 0; i < t->treeLevel(); i++)
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
		fontSize = fontSize + 5 * (maxDepthTaskList - 1 - t->treeLevel()); 
		s << "<td class=\""
		  << (r == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" style=\"white-space:nowrap\">"
		  << spaces
		  << "<span style=\"font-size:" 
		  << QString().sprintf("%d", fontSize) << "%\">";
		if (r == 0)
			s << "<a name=\"task_" << t->getId() << "\"></a>";
		s << generateUrl(KW("taskname"), t->getName());
		s << "</span></td>" << endl;
	}
	else
	{
		s << "<td class=\""
		  << (r == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" style=\"white-space:nowrap\">"
		  << spaces;
		if (r == 0)
			s << "<a name=\"task_" << t->getFullId() << "\"></a>";
		s << generateUrl(KW("taskname"), t->getName());
		s << "</td>" << endl;
	}
}

void
ReportHtml::resourceName(Resource* r, Task* t, bool big)
{
	QString spaces;
	int fontSize = big ? 100 : 90;
	if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
		for (Task* tp = t; tp != 0; tp = tp->getParent())
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";

	mt.clear();
	mt.addMacro(new Macro(KW("resourceid"), r->getId(), defFileName,
						  defFileLine));
	
	if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
	{
		for (uint i = 0; i < r->treeLevel(); i++)
			spaces += "&nbsp;&nbsp;&nbsp;&nbsp;";
		fontSize = fontSize + 5 * (maxDepthResourceList - 1 - r->treeLevel());
		s << "<td class=\""
		  << (t == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" style=\"white-space:nowrap\">"
		  << spaces
		  << "<span style=\"font-size:" 
		  << QString().sprintf("%d", fontSize) << "%\">";
		if (t == 0)
			s << "<a name=\"resource_" << r->getId() << "\"></a>";
		s << generateUrl(KW("resourcename"), r->getName());
		s << "</span></td>" << endl;
	}
	else
	{
		s << "<td class=\""
		  << (t == 0 ? "task" : "tasklight") << "\" rowspan=\""
		  << (showActual ? "2" : "1") << "\" style=\"white-space:nowrap\">"
		  << spaces;
		if (t == 0)
			s << "<a name=\"resource_" << r->getId() << "\"></a>";
		s << generateUrl(KW("resourcename"), r->getName());
		s << "</td>" << endl;
	}
}

void
ReportHtml::planResources(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" style=\"text-align:left\">"
	  << "<span style=\"font-size:100%\">";
	bool first = TRUE;
	QPtrList<Resource> planResources = t->getBookedResources(Task::Plan);
	for (Resource* r = planResources.first(); r != 0;
		 r = planResources.next())
	{
		if (!first)
			s << ", ";
					
		s << htmlFilter(r->getName());
		first = FALSE;
	}
	s << "</span></td>" << endl;
}

void
ReportHtml::actualResources(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" style=\"text-align:left\">"
	  << "<span style=\"font-size:100%\">";
	bool first = TRUE;
	QPtrList<Resource> actualResources = t->getBookedResources(Task::Actual);
	for (Resource* r = actualResources.first(); r != 0;
		 r = actualResources.next())
	{
		if (!first)
			s << ", ";
					
		s << htmlFilter(r->getName());
		first = FALSE;
	}
	s << "</span></td>" << endl;
}

void
ReportHtml::generateDepends(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\"><span style=\"font-size:100%\">";
	bool first = TRUE;
	for (Task* d = t->firstPrevious(); d != 0;
		 d = t->nextPrevious())
	{
		if (!first)
			s << ", ";
		else
			first = FALSE;
		s << d->getId();
	}
	s << "</span</td>" << endl;
}

void
ReportHtml::generateFollows(Task* t, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\">"
		"<span style=\"font-size:100%\">";
	bool first = TRUE;
	for (Task* d = t->firstFollower(); d != 0;
		 d = t->nextFollower())
	{
		if (!first)
			s << ", ";
		s << d->getId();
		first = FALSE;
	}
	s << "</span</td>" << endl;
}

void
ReportHtml::generateResponsibilities(Resource*r, bool light)
{
	s << "<td class=\""
	  << (light ? "defaultlight" : "default")
	  << "\" rowspan=\""
	  << (showActual ? "2" : "1")
	  << "\" style=\"text-align:left\">"
		"<span style=\"font-size:100%\">";
	bool first = TRUE;
	TaskList tl = project->getTaskList();
	for (Task* t = tl.first(); t != 0; t = tl.next())
	{
		if (t->getResponsible() == r)
		{
			if (!first)
				s << ", ";
			s << t->getId();
			first = FALSE;
		}
	}
	s << "</span></td>" << endl;
}

void
ReportHtml::generateSchedule(int sc, Resource* r, Task* t)
{
	s << "<td class=\""
	  << (t == 0 ? "default" : "defaultlight") 
	  << "\" style=\"text-align:left\">";

	if (r)
	{
		BookingList jobs = r->getJobs(sc);
		jobs.setAutoDelete(TRUE);
		time_t prevTime = 0;
		Interval reportPeriod(start, end);
		s << "<table style=\"width:150px; font-size:100%; "
		   "text-align:left\"><tr><th style=\"width:35%\"></th>"
		   "<th style=\"width:65%\"></th></tr>" << endl;
		for (Booking* b = jobs.first(); b != 0; b = jobs.next())
		{
			if ((t == 0 || t == b->getTask()) && 
				reportPeriod.overlaps(Interval(b->getStart(), b->getEnd())))
			{
				/* If the reporting interval is not more than a day, we
				 * do not print the day since this information is most
				 * likely given by the context of the report. */
				if (!isSameDay(prevTime, b->getStart()) &&
					!isSameDay(start, end - 1))
				{
					s << "<tr><td colspan=\"2\" style=\"font-size:120%\">"
						<< time2weekday(b->getStart()) << ", "
						<< time2date(b->getStart()) << "</td></tr>" << endl;
				}
				s << "<tr><td>";
				Interval workPeriod(b->getStart(), b->getEnd());
				workPeriod.overlap(reportPeriod);
				s << time2user(workPeriod.getStart(), shortTimeFormat)
				   	<< "&nbsp;-&nbsp;"
					<< time2user(workPeriod.getEnd() + 1, shortTimeFormat);
				s << "</td><td>";
				if (t == 0)
					s << " " << htmlFilter(b->getTask()->getName());
				s << "</td>" << endl;
				prevTime = b->getStart();
				s << "</tr>" << endl;
			}
		}
		s << "</table>" << endl;
	}
	else
		s << "&nbsp;";

	s << "</td>" << endl;
}

void
ReportHtml::flagList(CoreAttributes* c1, CoreAttributes* c2)
{
	FlagList allFlags = c1->getFlagList();
	QString flagStr;
	for (QStringList::Iterator it = allFlags.begin();
		 it != allFlags.end(); ++it)
	{
		if (it != allFlags.begin())
			flagStr += ", ";
		flagStr += htmlFilter(*it);
	}
	if (flagStr.isEmpty())
		flagStr = "&nbsp";
	textTwoRows(flagStr, c2 != 0, "left");
}

QString
ReportHtml::htmlFilter(const QString& s)
{
    if (!HtmlMap)
   	{
		HtmlMap = new QMap<Q_UINT16, QCString>;

		const Entity* ent = entitylist;
		while(ent->code)
	   	{
			HtmlMap->insert(ent->code, ent->name);
			ent++;
		}
    }
	
	QString out;
	bool parTags = FALSE;
	for (uint i = 0; i < s.length(); i++)
	{
		QString repl;
		if (HtmlMap->find(s[i]) != HtmlMap->end())
			repl = QString("&") + QString(*(HtmlMap->find(s[i]))) + ";";
		else if (s.mid(i, 2) == "\n\n")
		{
			repl = "</p><p>";
			parTags = TRUE;
			i++;
		}

		if (repl.isEmpty())
			out += s[i];
		else
			out += repl;
	}

	return parTags ? QString("<p>") + out + "</p>" : out;
}

void
ReportHtml::reportLoad(double load, const QString& bgCol, bool bold)
{
	if (load > 0.0 && barLabels != BLT_EMPTY)
	{
		s << "<td class=\""
		  << bgCol << "\">";
		if (bold)
			s << "<b>";
		s << scaledLoad(load);
		if (bold)
			s << "</b>";
		s << "</td>" << endl;
	}
	else
		s << "<td class=\""
		  << bgCol << "\">&nbsp;</td>" << endl;
}

void
ReportHtml::reportPIDs(const QString& pids, const QString bgCol, bool bold)
{
	s << "<td class=\""
	  << bgCol << "\" style=\"white-space:nowrap\">";
	if (bold)
		s << "<b>";
	s << pids;
	if (bold)
		s << "</b>";
	s << "</td>" << endl;
}

bool
ReportHtml::setUrl(const QString& key, const QString& url)
{
	if (urls.find(key) == urls.end())
		return FALSE;

	urls[key] = url;
	return TRUE;
}

const QString*
ReportHtml::getUrl(const QString& key) const
{
	if (urls.find(key) == urls.end() || urls[key] == "")
		return 0;
	return &urls[key];
}

QString
ReportHtml::generateUrl(const QString& key, const QString& txt)
{
	if (getUrl(key))
	{
		mt.setLocation(defFileName, defFileLine);
		return QString("<a href=\"") + mt.expand(*getUrl(key))
			+ "\">" + htmlFilter(txt) + "</a>";
	}
	else
		return htmlFilter(txt);
}

