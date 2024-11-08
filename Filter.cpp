#include "Filter.h"
#include <string>

FilterPlan::FilterPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (true);
}

FilterPlan::~FilterPlan ()
{
	TRACE (true);
	delete _input;
}

Iterator * FilterPlan::init () const
{
	TRACE (true);
	return new FilterIterator (this);
}

FilterIterator::FilterIterator (FilterPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	TRACE (true);
}

FilterIterator::~FilterIterator ()
{
	TRACE (true);

	delete _input;

	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
}

bool FilterIterator::next (Row & row)
{
	TRACE (true);

	for (;;)
	{
		// return false if no more elements are present in the row
		if ( ! _input->next (row))  return false;

		++ _consumed;
		// if (_consumed % 2 != 0) // the fake filter predicate
		// 	break;
		if (row.isFiltered()) {
            ++_produced;  // Increment produced count
			_input->free (row);
            return true;  // Row is valid, return it
        }

		_input->free (row);
	}

	// ++ _produced;
	return false;
}

void FilterIterator::free (Row & row)
{
	TRACE (true);
	_input->free (row);
}
