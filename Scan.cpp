#include "Scan.h"
#include "SystemConfigurations.h"
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <filesystem>
using namespace std;


/* ***********************ScanPlan****************************** */
ScanPlan::ScanPlan (char const * const filename, RowCount const count)
	: Plan (filename), _count (count)
{
	TRACE (true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
	TRACE (true);
} // ScanPlan::~ScanPlan


/* ***********************ScanIterator****************************** */

Iterator * ScanPlan::init () const
{
	TRACE (true);
	return new ScanIterator (this);
} // ScanPlan::init

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (true);

    cout<<"\n*********Input generation start!!********"<<endl;

    bool isInputSuccess = generateData(_plan->_name, _plan->_count);

    if(!isInputSuccess){
        cout<<"Error during input generation\n";
        exit(1);
    }

    cout<<"\n*********Input generation complete!!********"<<endl;


} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	TRACE (true);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next (Row & row)
{
	TRACE (true);

	if (_count >= _plan->_count)
		return false;

	++ _count;
	return true;
} // ScanIterator::next

void ScanIterator::free (Row & row)
{
	TRACE (true);
} // ScanIterator::free

int ScanIterator::generate_rand_int(int idx) {
	random_device rand;
	// Mersenne twister - seeding it with a random number
	mt19937 generator(rand());

    // number cannot have leading 0
    int min_val = idx==0? 1:0;
	uniform_int_distribution<int> distribution(min_val,SystemConfigurations::max_value_in_column);
	return distribution(generator);
}

bool ScanIterator::generateData (char const * const fileName,int numRecords)
{

    ofstream outFile2(fileName, ios::trunc); // truncate file length to 0, effectively clearing it.
    outFile2.close();

	// write the record into Input.txt, which exists in Disk
	ofstream file(fileName, ios::out); // overwrite mode

	if (!file) {
		cerr << "Error: Could not open file " << fileName << endl;
		return false;
	}

	// generate `numRecords` number of input data with `column_count` number of columns, and store in file
	for(int x=0;x<numRecords;x++){
        // for(int j=0;j<SystemConfigurations::page_size;j++){
            string record;
            for(int i=0;i<SystemConfigurations::column_count;i++){
                int s = generate_rand_int(i);
                record += to_string(s) + ",";
            }
            file << record << "|";
        // }
		
	}

	file.close();
	return true;
} 
