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


#ifndef _QicsSort_H
#define _QicsSort_H 1







class QicsSort {
public:
	virtual ~QicsSort();

	
	virtual int compare(char *a, char *b) = 0;

	
	void sort(char *a, int n);

protected:
	
	QicsSort(unsigned int _elementWidth);

private:
	/// the size in bytes of the elements to sort.
	unsigned int	elementWidth;

	
	void sortsection(char *a, char *endptr);

	
	inline void swap(char *i, char *j)
	{
		register char *ri, *rj;
		int n = elementWidth;
		ri = i;
		rj = j;
		do {
			register char c = *ri;
			*ri++ = *rj;
			*rj++ = c;
		} while(--n);
	}

	
	void threewayswap(char *i, char *j, char *k);

};

#endif /* _QicsSort_H */
