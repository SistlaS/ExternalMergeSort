#include "Scan.h"
#include <iostream>
#include <string>
#include <fstream>
#include <random>
#include <filesystem>

using namespace std;

Config config;

/* ***********************ScanPlan****************************** */
ScanPlan::ScanPlan (char const * const name, RowCount const count)
	: Plan (name), _count (count)
{
	TRACE (true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
	TRACE (true);
} // ScanPlan::~ScanPlan

Iterator * ScanPlan::init () const
{
	TRACE (true);
	return new ScanIterator (this);
} // ScanPlan::init

int ScanIterator::generate_rand_int() {
	random_device rand;
	// Mersenne twister - seeding it with a random number
	mt19937 generator(rand());

	uniform_int_distribution<int> distribution(0,Config::max_val_column);
	return distribution(generator);
}

/* ***********************ScanIterator****************************** */

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (true);
    cout<<"\n*********Input generation start!!********"<<endl;
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	TRACE (true);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
    cout<<"\n*********Input generation complete!!********"<<endl;
} // ScanIterator::~ScanIterator

bool ScanIterator::next (Row & row)
{
	TRACE (true);
    if (_count >= _plan->_count){
        cout<<"\n\nAll records scanned\n\n";
		return false;
    }

    // generate `numRecords` number of input data with `column_count` number of columns, and store in file
    string record;
    for(int i=0;i<Config::column_count;i++){
        int s = generate_rand_int();
        record += to_string(s) + ",";
    }
    record += "|";

    row.setRow(record);
    cout<<"Printing row: " << row.getRow() <<" \n";
    
	++ _count;

	return true;
} // ScanIterator::next

void ScanIterator::free (Row & row)
{
	TRACE (true);
} // ScanIterator::free
