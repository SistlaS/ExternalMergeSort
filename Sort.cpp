#include "Sort.h"
#include <algorithm>
#include <iostream>
#include <fstream>

#include <sstream>
#include <vector>
#include <string>


int recordsize = 0;
SortPlan::SortPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (true);
} 

SortPlan::~SortPlan ()
{	

	TRACE (true);
	delete _input;
}

Iterator * SortPlan::init () const
{
	TRACE (true);
	return new SortIterator (this);
}


void write_ramfile(const std::vector<std::vector<std::string>> data, const std::string& fileName) {
    // Open the output file
    std::ofstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i != row.size() - 1) {
                file << ",";
            }
        }
        
        file << "\n";
    }
	file.close();
	
}
void SortIterator::quickSort(std::vector<std::vector<std::string>> data)
{
	TRACE(true);


	// std::vector<std::vector<std::string>> data = read_ramfile();
	traceprintf("DOING QUICKSORT \n");
	std::sort(data.begin(), data.end());
	write_ramfile(data, "output.txt");
	data.clear();
}



SortIterator::SortIterator (SortPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	TRACE (true);
	std::vector<std::vector<std::string>> rec;
	for (Row row;  _input->next (row);  _input->free (row)){
		rec.push_back(row.data);
		++ _consumed;
	}
	quickSort(rec);
	delete _input;
	

	// ifstream inputFile(fileName, ios::binary | ios::ate);
	// if (!inputFile)
	// {
	// 	// cout << "cannot open the hard disk" << endl;
	// 	exit(1);
	// }
	// streampos curr = inputFile.tellg();
	// _recsize = curr/_consumed;
	// inputFile.close();
	// recordsize = _recsize;
	
	// runs = 0;

	traceprintf ("%s consumed %lu rows\n",
			_plan->_name,
			(unsigned long) (_consumed));
}

SortIterator::~SortIterator ()
{
	TRACE (true);
	
	traceprintf ("%s produced %lu of %lu rows\n",
			_plan->_name,
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
}
std::vector<std::vector<std::string>> read_ramfile()
{
	std::string fileName = "Disk.txt";
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
    }
    std::vector<std::vector<std::string>> data;

    std::string line;

    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string item;

        while (std::getline(ss, item, ',')) {
            row.push_back(item);
        }

        data.push_back(row);
    }
    file.close();
    
    return data;
}


bool SortIterator::next (Row & row)
{
	TRACE (true);
	for (auto x : row.data){
		std::cout<<x<<" ======================";
	}
	if (_produced >= _consumed)  return false;
	// for (auto x : row.data){
	// 	std::cout<<x<<" ======================";
	// }
	++ _produced;
	return true;
}

void SortIterator::free (Row & row)
{
	TRACE (true);
}
