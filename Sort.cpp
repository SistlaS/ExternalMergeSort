#include "Sort.h"

SortPlan::SortPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (true);
} 

SortPlan::~SortPlan ()
{
	TRACE (true);
	delete _input;
}

Iterator * SortPlan::init () const
{
	TRACE (true);
	return new SortIterator (this);
}

SortIterator::SortIterator (SortPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	TRACE (true);

	for (Row row;  _input->next (row);  _input->free (row))
		++ _consumed;
	delete _input;

	traceprintf ("%s consumed %lu rows\n",
			_plan->_name,
			(unsigned long) (_consumed));
}

SortIterator::~SortIterator ()
{
	TRACE (true);

	traceprintf ("%s produced %lu of %lu rows\n",
			_plan->_name,
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
}

bool SortIterator::next (Row & row)
{
	TRACE (true);

	if (_produced >= _consumed)  return false;

	++ _produced;
	return true;
}

void SortIterator::free (Row & row)
{
	TRACE (true);
}
