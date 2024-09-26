#include "Witness.h"

WitnessPlan::WitnessPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (true);
} 

WitnessPlan::~WitnessPlan ()
{
	TRACE (true);
	delete _input;
} 

Iterator * WitnessPlan::init () const
{
	TRACE (true);
	return new WitnessIterator (this);
} 

WitnessIterator::WitnessIterator (WitnessPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_rows (0)
{
	TRACE (true);
} 

WitnessIterator::~WitnessIterator ()
{
	TRACE (true);

	delete _input;

	traceprintf ("%s witnessed %lu rows\n",
			_plan->_name,
			(unsigned long) (_rows));
}

bool WitnessIterator::next (Row & row)
{
	TRACE (true);

	if ( ! _input->next (row))  return false;
	++ _rows;
	return true;
} 

void WitnessIterator::free (Row & row)
{
	TRACE (true);
	_input->free (row);
}
