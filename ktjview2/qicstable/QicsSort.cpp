/**************************************************************************
**
** Copyright (C) 2002-2003 Integrated Computer Solutions, Inc.
** All rights reserved.
**
** This file is part of the QicsTable Product.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ics.com/qt/licenses/gpl/ for GPL licensing information.
**
** If you would like to use this software under a commericial license
** for the development of proprietary software, send email to sales@ics.com
** for details.
**
** Contact info@ics.com if any conditions of this licensing are
** not clear to you.
**
**************************************************************************/


/*
 * C++ wrapper around quicksort.   See QicsSort.h for an example
 * of how to use it.   The most compelling reason is that, by making
 * it a class, context information is available in the call to compare.
 * This allows several sorts to run at a time in different threads
 * without killing each other.
 */

#include <QicsSort.h>


QicsSort::QicsSort(unsigned int ewidth)
    :
	elementWidth(ewidth)
{
}

QicsSort::~QicsSort() { }

void QicsSort::sort(char *a, int n)
{
	sortsection(a, a + n*elementWidth);
}

void QicsSort::sortsection(char *base, char *endptr)
{
	char *i, *j;
	char *lp, *hp;
	int c;
	unsigned int	nbytes;

start:
	nbytes = endptr - base;
	if(nbytes <= elementWidth) return;

	// pick midpoint
	unsigned int midpoint = ((nbytes/(2*elementWidth))*elementWidth);
	hp = lp = base + midpoint;

	i = base;
	j = endptr - elementWidth;

	for(;;) {
		if(i < lp) {
			if((c = this->compare(i, lp)) == 0) {
				lp -= elementWidth;
				swap(i, lp);
				continue;
			}
			if(c < 0) {
				i += elementWidth;
				continue;
			}
		}

loop:
		if(j > hp) {
			if((c = this->compare(hp, j)) == 0) {
				hp += elementWidth;
				swap(hp, j);
				goto loop;
			}
			if(c > 0) {
				if(i == lp) {
					hp += elementWidth;
					threewayswap(i, hp, j);
					lp += elementWidth;
					i = lp;
					goto loop;
				}
				swap(i, j);
				j -= elementWidth;
				i += elementWidth;
				continue;
			}
			j -= elementWidth;
			goto loop;
		}


		if(i == lp) {
			if(lp-base >= endptr-hp) {
				sortsection(hp+elementWidth, endptr);
				endptr = lp;
			} else {
				sortsection(base, lp);
				base = hp+elementWidth;
			}
			goto start;
		}


		threewayswap(j, lp -= elementWidth, i);
		hp -= elementWidth;
		j = hp;
	}
}


/* 
 * Three way exchange: i <- k, k <- j, j <- i
 */
void QicsSort::threewayswap(char *i, char *j, char *k)
{
	register char *ri, *rj, *rk;
	unsigned int	n;

	n = elementWidth;
	ri = i;
	rj = j;
	rk = k;
	do {
		char c = *ri;
		*ri++ = *rk;
		*rk++ = *rj;
		*rj++ = c;
	} while(--n);
}
