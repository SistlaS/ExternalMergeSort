#include "Witness.h"
#include <iostream>
#include <string>
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
	// Retrieve the next row from the input iterator
	if (!_input->next(row)) return false;

	// Perform witness checks
	
	// checkOrder(row)
	++_rows; // Increment row count
	witness(row);
	return true; // Return true as we've successfully retrieved a row

}

// bool WitnessIterator::checkOrder(Row &row){

// }

void WitnessIterator::witness(Row& row){
	for (auto x : row.data){
		std::cout<<x<<" ";
	}
	int currRowCounts = row.data.size();
	std::cout << "currRowCounts: " << currRowCounts << std::endl;
	int xorResult = calculateXOR(row);
	std::cout << "XOR of row values: " << xorResult << std::endl;
	// int inversions = countInversions(row);
	traceprintf ("%s witnessed %lu rows\n",
			_plan->_name,
			(unsigned long) (_rows));
}

// int countInversions(Row& row) {
// 	int inversions = 0;
// 	for (size_t i = 0; i < row.data.size(); ++i) {
// 		for (size_t j = i + 1; j < row.data.size(); ++j) {
// 			if (row.data[i] > row.data[j]) {
// 				++inversions;
// 			}
// 		}
// 	}
// 	return inversions;
// }

int WitnessIterator::calculateXOR(Row& row) {  // Definition with const
    int xorResult = 0;
    for (std::string value : row.data) {
        xorResult ^= std::stoi(value);
    }
    return xorResult;
}

void WitnessIterator::free (Row & row)
{
	TRACE (true);
	_input->free (row);
}
