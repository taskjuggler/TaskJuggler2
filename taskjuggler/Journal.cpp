#include "Journal.h"

int
Journal::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
{
    return (static_cast<JournalEntry*>(item2))->getDate() -
        (static_cast<JournalEntry*>(item1))->getDate();
}

