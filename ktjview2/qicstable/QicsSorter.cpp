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


#include <qobject.h>
#include <qglobal.h>

#include <QicsDataModel.h>
#include <QicsSorter.h>
#include <QicsSort.h>
#include <QicsDataItem.h>

///////////////////////////////////////////////////////////////////////////

QicsSorter::QicsSorter(Qics::QicsIndexType _type, QicsDataModel *model) :
    QObject(),
    myType(_type),
    myDM(model)
{
    if (myDM)
    {
	if(myType == RowIndex) {
	    connect(myDM, SIGNAL(rowsInserted(int, int)),
		    this, SLOT(insertElements(int, int)));
	    connect(myDM, SIGNAL(rowsAdded(int)),
		    this, SLOT(appendElements(int)));
	    connect(myDM, SIGNAL(rowsDeleted(int, int)),
		    this, SLOT(deleteElements(int, int)));
	} else {
	    connect(myDM, SIGNAL(columnsInserted(int, int)),
		    this, SLOT(insertElements(int, int)));
	    connect(myDM, SIGNAL(columnsAdded(int)),
		    this, SLOT(appendElements(int)));
	    connect(myDM, SIGNAL(columnsDeleted(int, int)),
		    this, SLOT(deleteElements(int, int)));
	}
    }
}

QicsSorter::~QicsSorter()
{
}


void QicsSorter::expandTo(int newsize)
{
#if (((QT_VERSION >> 16) == 3) && (((QT_VERSION >> 8) & 0xFF) < 1))
    order.resize(newsize);
#else
    order.resize(newsize, QGArray::SpeedOptim);
#endif
}

void QicsSorter::integrityCheck(void)
{
    QMemArray<int> used;
    used.fill(0, order.size());

    for(int i = 0; i < static_cast<int> (order.size()); i++) {
	int j = order[i];
	assert(j >= 0);
	assert(j < static_cast<int> (order.size()));
	assert(used[j] == 0);
	used[j] = 1;
    }
}


/*! internal slot
 */
void
QicsSorter::appendElements(int how_many)
{
    // If we haven't yet reordered anything, we don't need to do anything
    // here.

    if (order.isEmpty())
	return;

    /* The logic of appending a row or column is pretty
     * clear.   We added something to the data model,
     * and we should open up space at the end of the view.
     * Towards that end, we simply alloc more space int
     * the order vector and stuff in new indices.  This
     * will put the new rows at the end of the view.
     */
    
    int cur_size = order.size();

    expandTo(order.size() + how_many);

    for(int i = 0; i < how_many; i++)
    {
	order[cur_size] = cur_size++;
    }

    flushModelToVisualMap();
}

/*! internal slot
 */
void
QicsSorter::insertElements(int how_many, int start)
{
    /* guard ourselves */
    assert(how_many > 0);

    // If we haven't yet reordered anything, we don't need to do anything
    // here.

    if (order.isEmpty())
	return;

    assert(start >= 0 && start < static_cast<int> (order.size()));

    QMemArray<int> visChange(order.size());

    /* what should happen when you insert things in the middle
     * of a reorders set is not obvious.   We start with some points
     * 
     * - We will expand norder by how_many.
     * - Viewers will probably expect the new rows to appear next
     *   to each other
     */

    int cur_size = static_cast<int> (order.size());
    expandTo(order.size() + how_many);

    /* incoming data is in model units.
     * - all indices >= start must be increased by how_many
     * - there should be how_many slots opened up at start,
     * - these will contain the indices start up to start+how_many
     * Doing it in three passes makes it pretty obvious.
     */

    // fix all the existing indices
    int i;
    int	insertPoint = -1;
    for(i = 0; i < cur_size; i++)
    {
	int x = order[i];
	if(x == start) insertPoint = i;
	if(x >= start) order[i] += how_many;
	visChange[i] = order[i];
    }

    assert(insertPoint >= 0);
    // open up the space
    for(i = cur_size-1; static_cast<int> (i) >= insertPoint; i--)
    {
	order[i + how_many] = order[i];
    }

    // create the new ones
    for(i = 0; static_cast<int> (i) < how_many; i++)
    {
	order[insertPoint+i] = start+i;
    }

    integrityCheck();
    flushModelToVisualMap();

    emit orderChanged(myType, visChange);
}

/*! internal slot
 */
void
QicsSorter::deleteElements(int how_many, int start)
{
    // If we haven't yet reordered anything, we don't need to do anything
    // here.

    if (order.isEmpty())
	return;

    /* incoming data is in model units.   We have to
     * delete from the ordering vector all positions x, where
     * start <= x <start+how_many.  Now, we can't have gaps
     * in the indices, so any positions y
     * (start+how_many < y) must be reduced by how_many,
     *
     * At the same time we build the visual reorder vector.
     * which maps old visual locations to new ones.  If
     * we did not delete anything then visChange[i] == i,
     * for all i.  If delete the fifth slot, then v0 = 0 ...
     * v3==3, v4=-1, v5=4.
     *
     */

    QMemArray<int> visChange(order.size());

    int ndone = 0;
    for(int i = 0; i < static_cast<int> (order.size()); i++)
    {
	int x = order[i];
	if(start <= x && x < start+how_many)
	{
	    // This slot is deleted
	    visChange[i] = -1;
	    continue;
	}

	if(x >= start+how_many) x--;
	visChange[i] = ndone;
	order[ndone++] = x;
    }
    assert(ndone == (static_cast<int> (order.size()) - how_many));
    order.resize(ndone);

    integrityCheck();
    flushModelToVisualMap();

    emit orderChanged(myType, visChange);
}


void
QicsSorter::fillVisualToModelMap(void)
{
    int orderAllocated = 0;

    if (myDM)
    {
	orderAllocated = ((myType == RowIndex) ? (myDM->lastRow() + 1) :
			  (myDM->lastColumn() + 1));
    }

    order.resize(orderAllocated);

    for(int i = 0; i < static_cast<int> (order.size()); i++)
    {
	order[i] = i;
    }
}

/* We lazy evaluate this map because we don't need it all the time.
 */
void
QicsSorter::fillModelToVisualMap(void)
{
    modelToVisualMap.resize(order.size());
    for(int i = 0; i < static_cast<int> (order.size()); i++)
	modelToVisualMap[order[i]] = i;
}

void
QicsSorter::flushModelToVisualMap(void)
{
    modelToVisualMap.resize(0);
}

int
QicsSorter::modelToVisual(int x)
{
    if (!order.isEmpty() && (x >= 0) && (x < static_cast<int> (order.size())))
    {
	if(modelToVisualMap.isEmpty())
	    fillModelToVisualMap();
	return modelToVisualMap[x];
    }
    else
	return x;
}

int
QicsSorter::visualToModel(int x)
{
    if (!order.isEmpty() && (x >= 0) && (x < static_cast<int> (order.size())))
	return order[x];
    else
	return x;
}

class QicsSorter_sortHelper : public QicsSort {
public:
	QicsSorter_sortHelper(QicsSorter *,
				int _thing,
				DataItemComparator func,
				Qics::QicsSortOrder so);
	int compare(char *x, char *b);

private:
        QicsSorter		*self;
	int			thingToSort;
	DataItemComparator	compareFunc;
	int			ordFlip;
};


QicsSorter_sortHelper::QicsSorter_sortHelper(
		QicsSorter *_self,
		int _thing,
		DataItemComparator _func,
		Qics::QicsSortOrder sortOrder)
    :
	QicsSort(sizeof(int)),
	self(_self),
	thingToSort(_thing),
	compareFunc(_func)
{
	ordFlip = 1;
	if(sortOrder == Qics::Descending) ordFlip = -1;
}

int
QicsSorter_sortHelper::compare(char *_a, char *_b)
{
	int *ia = (int *) _a;
	int *ib = (int *) _b;

	if (!self->myDM)
	{
	    // This is kind of meaningless, but...
	    return (ia < ib ? -1 : (ia > ib ? 1 : 0));
	}

	const QicsDataItem *a;
	const QicsDataItem *b;
	int retval = 0;

	if (self->myType == Qics::RowIndex)
	{
	    a = self->myDM->item(*ia, thingToSort);
	    if (a)
		a = a->clone();

	    b = self->myDM->item(*ib, thingToSort);
	    if (b)
		b = b->clone();
	}
	else
	{
	    a = self->myDM->item(thingToSort, *ia);
	    if (a)
		a = a->clone();

	    b = self->myDM->item(thingToSort, *ib);
	    if (b)
		b = b->clone();
	}

	// sometimes the cells are null
	if (a == 0)
	{
	    if (b == 0)
		retval = 0;
	    else
		retval = ordFlip * -1;
	}
	else if (b == 0)
	    retval = ordFlip;
	else if (compareFunc)
	    retval = ordFlip * compareFunc(a, b);
	else
	    retval = ordFlip * (a->compareTo(*b));

	delete a;
	delete b;

	return retval;
}

void
QicsSorter::sort(int row_or_column, QicsSortOrder sort_order,
		 int from, int to, DataItemComparator func)
{
    int i;

    // If we haven't moved or sorted yet, we need to fill this map.
    if (order.isEmpty())
	fillVisualToModelMap();

    QicsSorter_sortHelper *sh =
	new QicsSorter_sortHelper(this,row_or_column,func,sort_order);

    if(from < 0)
	from = 0;
    if(to < 0 || to >= static_cast<int> (order.size()))
	to = order.size() - 1;

    // save off a copy of the order so we can build the change map
    QMemArray<int> visChange = order.copy();

    sh->sort((char *)(order.data()+from), to-from+1);
    delete sh;
    flushModelToVisualMap();

    /* Now we compute the mapping of old visual to new visual.
     * For old visual i, visChange[i] is the model index,
     * modelToVisualMap of that gives the current visual.
     */
    fillModelToVisualMap();

    for(i = 0; i < static_cast<int> (order.size()); i++)
    {
	visChange[i] = modelToVisualMap[visChange[i]];
    }

    emit orderChanged(myType, visChange);
}


/*
 * moveItems could seem tricky, but the algorithm is
 * mostly straightforward
 * - build a new empty ordering vector
 *   initialize the slots to something "unused" (-1)
 * - compute a new target by subtracting the number of items
 *   the target from the 
 * - grab the data from the slots indicated move list and
 *   copy it into the new vector at the desired target,
 *   set the original slots to "unused"
 * - copy the remaining slots into the new vector,
 *   carefully pouring around the "used" slots
 */

void
QicsSorter::moveItems(int target, const QMemArray<int> &itms)
{
#if notdef
    qDebug("We are moving to %d:", target);
    for(int i = 0; i < static_cast<int> (itms.size()); i++)
	qDebug(" %d", itms[i]);
    qDebug("\n");
#endif

    int i;

    // If we haven't moved or sorted yet, we need to fill this map.
    if (order.isEmpty())
	fillVisualToModelMap();

    QMemArray<int> visChange(order.size());

    // - build a new empty ordering vector
    // - initialize the slots to something "unused" (-1)
    QMemArray<int> neworder(order.size());
    neworder.fill(-1);

    // compute new target
    int newtarget = target;
    for(i = 0; i < static_cast<int> (itms.size()); i++) {
	if(itms[i] < target) newtarget--;
    }

    // - grab the data from the slots indicated move list and
    // - copy it into the new vector at the desired target,
    // - set the original slots to "unused"
    for(i = 0; i < static_cast<int>(itms.size()); i++) {
	neworder[newtarget+i] = order[itms[i]];
	visChange[itms[i]] = newtarget+i;
	order[itms[i]] = -1;
    }

    // - copy the remaining slots into the new vector,
    //   carefully pouring around the "used" slots
    int newi = 0;
    for(i = 0; i < static_cast<int> (order.size()); i++) {
	if(order[i] >= 0) {
	    while(neworder[newi] != -1) newi++;
	    visChange[i] = newi;
	    neworder[newi++] = order[i];
	}
    }

    order = neworder;

    integrityCheck();
    flushModelToVisualMap();

    emit orderChanged(myType, visChange);
}
