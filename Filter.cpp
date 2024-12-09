#include "Filter.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

FilterPlan::FilterPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (true);
} // FilterPlan::FilterPlan

FilterPlan::~FilterPlan ()
{
	TRACE (true);
	delete _input;
} // FilterPlan::~FilterPlan

Iterator * FilterPlan::init () const
{
	TRACE (true);
	return new FilterIterator (this);
} // FilterPlan::init

FilterIterator::FilterIterator (FilterPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	TRACE (true);
} // FilterIterator::FilterIterator

FilterIterator::~FilterIterator ()
{
	TRACE (true);

	delete _input;

	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
} // FilterIterator::~FilterIterator

bool FilterIterator::next (Row & row)
{
	TRACE (true);
    vector<string> temp;
	for (;;)
	{
		if ( ! _input->next (row))  return false;

		++ _consumed;
		if (_consumed % 2 != 0) // the fake filter predicate
			break;
        
        temp.push_back(row.row);
		_input->free (row);

        

	}

	++ _produced;
    std::ofstream outFile("input.txt", std::ios::app);
    // cout<<"*****************FILTERED ROWS :"<<_produced<<endl;
    if (!outFile) {
        std::cerr << "Error opening file for writing." << std::endl;
        return 1;
    }

    // Write each string from the vector to the file
    for (const auto& line : temp) {
        // cout<<line<<"--------"<<endl;
        outFile << line << std::endl;
    }

    // Close the file
    outFile.close();

    // std::cout << "File written successfully." << std::endl;
	
    return true;
} // FilterIterator::next

void FilterIterator::free (Row & row)
{
	TRACE (true);
	_input->free (row);
} // FilterIterator::free
