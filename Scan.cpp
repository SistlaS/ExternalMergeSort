#include "Scan.h"

ScanPlan::ScanPlan (char const * const name, RowCount const count)
	: Plan (name), _count (count)
{
	TRACE (true);
} 

ScanPlan::~ScanPlan ()
{
	TRACE (true);
} 

Iterator * ScanPlan::init () const
{
	TRACE (true);
	return new ScanIterator (this);
} 

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (true);
}

ScanIterator::~ScanIterator ()
{
	TRACE (true);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} 

bool ScanIterator::next (Row & row)
{
	TRACE (true);

	if (_count >= _plan->_count)
		return false;

	++ _count;
	return true;
} 

void ScanIterator::free (Row & row)
{
	TRACE (true);
}
