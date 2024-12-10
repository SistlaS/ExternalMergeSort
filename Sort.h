# pragma once
#include "Iterator.h"
#include <fstream>

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
    void ramExternalSort();
    void diskExternalSort();
    void mergeSort(bool isDiskSort, int numRuns);
    static RowCount _ramUsed, _ramBufferUsed, _bufferSpills;
private:
	SortPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced,_cacheUsed, _numRAMRuns;
    bool isLastBatch = false;
	ifstream outFile;
	string _currentLine;
	int currentRecordIndex = 0;
	
}; // class SortIterator

void insertCacheRunsInRAM(string cacheRun);
void spillRunToDisk(string cacheRun);

