#include "Witness.h"
#include <sstream>
#include <string>

WitnessPlan::WitnessPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (true);
} // WitnessPlan::WitnessPlan

WitnessPlan::~WitnessPlan ()
{
	TRACE (true);
	delete _input;
} // WitnessPlan::~WitnessPlan

Iterator * WitnessPlan::init () const
{
	TRACE (true);
	return new WitnessIterator (this);
} // WitnessPlan::init

WitnessIterator::WitnessIterator (WitnessPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_rows (0)
{
	TRACE (true);
	// cout<<"When is this printing"<<endl;
} // WitnessIterator::WitnessIterator


WitnessIterator::~WitnessIterator ()
{
	TRACE (true);

	cout<<"**********XOR AFTER SORT************"<< opXOR<<endl;

	delete _input;


	//read disk here and xor

	traceprintf ("%s witnessed %lu rows\n",
			_plan->_name,
			(unsigned long) (_rows));
} // WitnessIterator::~WitnessIterator

void WitnessIterator::computeXOR(Row & row, int &XOR){
	string data = row.row;
	// cout<<row.row<<endl;
	
	if(data.back() == '|') data.pop_back();
	stringstream ss(data);
	string number;
	while (getline(ss, number, ',')) {
		XOR ^= stoi(number); // Convert to integer and XOR
	}
	cout<<"computing Xor for row : "<<row.row<< " value :"<< XOR<<endl;
	// cout<<ipXOR << " -- "<< opXOR<<endl;

	// cout<<"computing XOR"<<endl;
	return;
}

bool WitnessIterator::next (Row & row)
{
	TRACE (true);

	if ( ! _input->next (row))  return false;
	if(_plan->_name == "input") {
		computeXOR(row, ipXOR);
	}else{
		computeXOR(row, opXOR);
	}

	
	// cout<<row.row<<" -----in witness next "<<endl;
	++ _rows;
	return true;
} // WitnessIterator::next

void WitnessIterator::free (Row & row)
{
	TRACE (true);

	// cout<<"***"<<_input->next(row)<<endl;
	if (!_input->next(row)){
		cout<<"***************XOR BEFORE************ "<<ipXOR<<endl;
	}
	_input->free (row);
} // WitnessIterator::free
