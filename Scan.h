# pragma once
#include "Iterator.h"
#include <fstream>

class ScanPlan : public Plan
{
	friend class ScanIterator;
public:
	ScanPlan (char const * const name, RowCount const count, int order);
	~ScanPlan ();
	Iterator * init () const;
    int order;

private:
	RowCount const _count;
}; // class ScanPlan

class ScanIterator : public Iterator
{
public:
	ScanIterator (ScanPlan const * const plan, int sortOrder);
	~ScanIterator ();
	bool next (Row & row);
	void free (Row & row);
    int generate_rand_int();
    int sortOrder; // -1 if desc sort input, 0 for neutral, 1 for asc sort input
    std::ifstream inFileSpecial;
	string _currLine;
private:
	ScanPlan const * const _plan;
	RowCount _count;
}; // class ScanIterator
