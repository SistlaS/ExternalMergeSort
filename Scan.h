#pragma once
#include "Iterator.h"

class ScanPlan : public Plan
{
	friend class ScanIterator;
public:
	ScanPlan (char const * const filename, RowCount const count);
	~ScanPlan ();
	Iterator * init () const;
private:
	RowCount const _count;
}; // class ScanPlan

class ScanIterator : public Iterator
{
public:
	ScanIterator (ScanPlan const * const plan);
	~ScanIterator ();
	bool next (Row & row);
	void free (Row & row);
    int generate_rand_int(int idx);
    bool generateData(char const * const fileName,int numRecords);
private:
	ScanPlan const * const _plan;
	RowCount _count;
}; // class ScanIterator
