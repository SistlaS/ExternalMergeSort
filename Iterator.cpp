#include "Iterator.h"

Row::Row ()
{
	TRACE (true);
} 

Row::~Row ()
{
	TRACE (true);
} 

Plan::Plan (char const * const name)
	: _name (name)
{
	TRACE (true);
}

Plan::~Plan ()
{
	TRACE (true);
} 

Iterator::Iterator () : _rows (0)
{
	TRACE (true);
} 

Iterator::~Iterator ()
{
	TRACE (true);
} 

void Iterator::run ()
{
	TRACE (true);

	for (Row row;  next (row);  free (row))
		++ _rows;

	traceprintf ("entire plan produced %lu rows\n",
			(unsigned long) _rows);
}
