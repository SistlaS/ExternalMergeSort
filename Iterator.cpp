#include "Iterator.h"
#include <iostream>
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
	// std::cout<<"Plan constructor called : " << name<<std::endl;
	TRACE (true);
	// std::cout<<"after trace " <<std::endl;
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

	for (Row row;  next (row);  free (row)){
		// std::cout<<"Row data : " <<row<<std::endl;
		++ _rows;
	}
	traceprintf ("entire plan produced %lu rows\n",
			(unsigned long) _rows);
}
