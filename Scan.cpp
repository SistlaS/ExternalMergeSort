#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include "Scan.h"
#include <filesystem>
using namespace std;

#define COLUMN_COUNT 4
#define DOMAIN_OF_VALUES 10

ScanPlan::ScanPlan (char const * const name, RowCount const count)
	: Plan (name), _count (count)
{
	TRACE (true);
} 

ScanPlan::~ScanPlan ()
{
	TRACE (true);
} 

Iterator * ScanPlan::init () const
{
	TRACE (true);
	return new ScanIterator (this);
} 

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (true);
}

ScanIterator::~ScanIterator ()
{
	TRACE (true);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} 

int generate_rand_int() {
	random_device rand;
	// Mersenne twister - seeding it with a random number
	mt19937 generator(rand());

	uniform_int_distribution<int> distribution(0,1000);
	return distribution(generator);
}

bool ScanIterator::next (Row & row)
{
	if (_count >= _plan->_count)
		return false;
	vector<string> record;
	for(int i=0;i<COLUMN_COUNT;i++){
		int s = generate_rand_int();
		record.push_back(to_string(s));
	}
	// write the record into a file(proxy for Disk)
	string fileName = "Disk.txt";
	// open it in append mode
	ofstream file;

	file.open(fileName, ios::out);

	// return if you can't open file
	if (!file) {
		cerr << "Error: Could not open file " << fileName << endl;
		return 1;
	}

	for (size_t i = 0; i < record.size(); ++i) {
		file << record[i];
		if (i != record.size() - 1) {  // Add a comma after each element except the last
			file << ", ";
		}else{
			file << "\n";
		}
	}
	row.data = record;

	file.close();

	++ _count;
	return true;
} 

void ScanIterator::free (Row & row)
{
	TRACE (true);
}
