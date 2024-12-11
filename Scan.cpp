#include "Scan.h"
#include <iostream>
#include <string>
#include <fstream>
#include <random>
#include <filesystem>

using namespace std;

Config config;

/* ***********************ScanPlan****************************** */
ScanPlan::ScanPlan (char const * const name, RowCount const count, int order)
	: Plan (name), _count (count), order(order)
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
	return new ScanIterator (this,order);
} // ScanPlan::init

int ScanIterator::generate_rand_int() {
	random_device rand;
	// Mersenne twister - seeding it with a random number
	mt19937 generator(rand());

	uniform_int_distribution<int> distribution(0,Config::max_val_column);
	return distribution(generator);
}

/* ***********************ScanIterator****************************** */

ScanIterator::ScanIterator (ScanPlan const * const plan, int order) :
	_plan (plan), _count (0), sortOrder(order)
{
	TRACE (true);
    cout<<"\n*********Input generation start!!********"<<endl;
    if(order==1){
        cout<<"Test case: Sorted (Ascending) Input!!\n";
        inFileSpecial.open("sort-asc-input.txt");
    } else if(order==-1){
        cout<<"Test case: Sorted (Descending) Input!!\n"; 
        inFileSpecial.open("sort-desc-input.txt");
    }

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

    if(sortOrder!=0){
        if (inFileSpecial.is_open()){
            if (getline(inFileSpecial, _currLine, '|')){
                if(_currLine!="\n" or _currLine!=""){
                    row.setRow(_currLine+"|");
                }
            }
	    }
    }
    else{
        // generate `numRecords` number of input data with `column_count` number of columns, and store in file
        string record;
        for(int i=0;i<Config::column_count;i++){
            int s = generate_rand_int();
            record += to_string(s) + ",";
        }
        record += "|";

        row.setRow(record);    
    }

	++ _count;

	return true;
} // ScanIterator::next

void ScanIterator::free (Row & row)
{
	TRACE (true);
} // ScanIterator::free
