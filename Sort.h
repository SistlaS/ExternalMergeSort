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
};

class SortIterator : public Iterator
{
public:
	SortIterator (SortPlan const * const plan);
	~SortIterator ();
	bool next (Row & row);
	void free (Row & row);
	void quickSort(std::vector<std::vector<std::string>> data);
	// void ssdSort();
	// void externalMerge();
	// void clearTempFile();
private:
	SortPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced;
	// int _recsize;
	// uint32_t runs;
};
