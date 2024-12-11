#include "Iterator.h"

class WitnessPlan : public Plan
{
	friend class WitnessIterator;
public:
	WitnessPlan (char const * const name, Plan * const input);
	~WitnessPlan ();
	Iterator * init () const;
private:
	Plan * const _input;
}; // class WitnessPlan

class WitnessIterator : public Iterator
{
public:
	WitnessIterator (WitnessPlan const * const plan);
	~WitnessIterator ();
	bool next (Row & row);
	void free (Row & row);
	void computeXOR(vector<int> data);
	void countInversions(vector<int> data);
	uint64_t XOR;
	uint64_t _inversions;
	bool _hasPrevious;
	vector<int> _previous;

private:
	WitnessPlan const * const _plan;
	Iterator * const _input;
	RowCount _rows;
}; // class WitnessIterator
