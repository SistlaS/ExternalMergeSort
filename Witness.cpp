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

	cout<<"**********************************STATS**************************************"<<endl;

	delete _input;

	cout<<"XOR : "<<XOR<<endl;

	//read disk here and xor

	traceprintf ("%s witnessed %lu rows\n",
			_plan->_name,
			(unsigned long) (_rows));
	cout<<"************************************************************************"<<endl;

} // WitnessIterator::~WitnessIterator

void WitnessIterator::computeXOR(Row & row){
	string data = row.row;
	// cout<<row.row<<endl;
	
	if(data.back() == '|') data.pop_back();
	stringstream ss(data);
	string number;
	int ct = 0;
	while (getline(ss, number, ',') && ct < Config::column_count) {
		XOR ^= stoi(number); // Convert to integer and XOR
		ct += 1;
	}
	// cout<<"computing Xor for row : "<<row.row<< " value :"<< XOR<<endl;
	// cout<<ipXOR << " -- "<< opXOR<<endl;

	// cout<<"computing XOR"<<endl;
	return;
}

bool WitnessIterator::next (Row & row)

{
	TRACE (true);

	if ( ! _input->next (row))  return false;
	computeXOR(row);
	++ _rows;
	return true;
} // WitnessIterator::next


void WitnessIterator::free (Row & row)
{
	TRACE (true);

	// cout<<"***"<<_input->next(row)<<endl;
	// if (!_input->next(row)){
	// 	cout<<"***************XOR BEFORE************ "<<XOR<<endl;
	// }
	_input->free (row);
} // WitnessIterator::free
