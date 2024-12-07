# pragma once
#include "Iterator.h"

class SortPlan : public Plan
{
	friend class SortIterator;
public:
	SortPlan (char const * const name, Plan * const input);
	~SortPlan ();
	Iterator * init () const;
private:
	Plan * const _input;
}; // class SortPlan

class SortIterator : public Iterator
{
public:
	SortIterator (SortPlan const * const plan);
	~SortIterator ();
	bool next (Row & row);
	void free (Row & row);
    void generateCacheRuns(Row row, bool lastBatch);
    void insertCacheRunsInRAM(string cacheRun);
    void spillBufferToDisk();
private:
	SortPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced,_cacheUsed,_ramUsed, _ramBufferUsed;
    bool isLastBatch = false;
}; // class SortIterator
