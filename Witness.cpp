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
	_rows (0), XOR(0), _inversions(0), _hasPrevious(false)
{
	TRACE (true);
	// cout<<"When is this printing"<<endl;
} // WitnessIterator::WitnessIterator


WitnessIterator::~WitnessIterator ()
{
	TRACE (true);

	cout<<"************************************************************************"<<endl;
	printf("Stats for %s \n",_plan->_name);
	printf("XOR : %llu \n",XOR);
	printf("Number of inversions : %lu \n", (unsigned long) (_inversions));
	printf("Witnessed rows : %lu \n",(unsigned long) (_rows));
	// traceprintf ("%s witnessed %lu rows\n",
	// 		_plan->_name,
	// 		(unsigned long) (_rows));
	cout<<"************************************************************************"<<endl;
	delete _input;

} // WitnessIterator::~WitnessIterator

void WitnessIterator::computeXOR(vector<int> data){
	// string data = row.row;
	// // cout<<row.row<<endl;
	
	// if(data.back() == '|') data.pop_back();
	// stringstream ss(data);
	// string number;
	// int ct = 0;
	// while (getline(ss, number, ',') && ct < Config::column_count) {
	// 	XOR ^= stoi(number); // Convert to integer and XOR
	// 	ct += 1;
	// }

	for (int i = 0; i < Config::column_count; i++){
		XOR ^= data[i];
	}
	return;
}

vector<int> convertData(Row row){
	string strData = row.row;
	vector<int> data;
	if(strData.back() == '|') strData.pop_back();

	stringstream ss(strData);
	string number;
	int ct = 0;
	while (getline(ss, number, ',') && ct < Config::column_count) {
		data.push_back(stoi(number)); // Convert to integer and XOR
		ct += 1;
	}
	return data;

}

bool _greater(vector<int> a, vector<int> b){
	for(int i = 0; i <Config::column_count; i++){
		if (a[i] > b[i]) {
            return true;
        } else if (a[i] < b[i]) {
            return false;
        }
	}
	return false;
}

void WitnessIterator::countInversions(vector<int> data){
	if (_hasPrevious)
    {
        if (_greater(_previous, data)) // Modify comparison logic as needed
        {
            ++_inversions;
        }
    }
    else
    {
        _hasPrevious = true;
    }
    // Update the previous row
    _previous = data;

	return;
}

bool WitnessIterator::next (Row & row)

{
	TRACE (true);

	if ( ! _input->next (row)) { 
		return false;
	}
	vector<int> curr_data = convertData(row);
	computeXOR(curr_data);
	countInversions(curr_data);
	++ _rows;
	return true;
} // WitnessIterator::next


void WitnessIterator::free (Row & row)
{
	TRACE (true);
	_input->free (row);
} // WitnessIterator::free
