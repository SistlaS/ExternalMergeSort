#include <iostream>
#pragma once
#include "defs.h"
#include <vector>
#include <string>
#include "Record.h"
typedef uint64_t RowCount;
using namespace std;
// #define COLUMN_COUNT 4
// #define DOMAIN_OF_VALUES 10

// class Row
// {
// public:
// 	vector<string> data;
// 	int offsetValueCode;
// 	Row ();
// 	virtual ~Row ();
// 	bool isFiltered() const {
//         for (string value : data) {
//             if (stoi(value) < 2) return false;  // value > 4 ==> row fails the filter
//         }
//         return true;  
//     }
// 	// ...
// private:
// 	// ...
// }; 

class Plan
{
	friend class Iterator;
public:
	Plan (char const * const name);
	virtual ~Plan ();
	virtual class Iterator * init () const = 0;
protected:
	char const * const _name;
private:
}; 

class Iterator
{
public:
	Iterator ();
	virtual ~Iterator ();
	void run ();
	virtual bool next (Row & row) = 0;
	virtual void free (Row & row) = 0;
private:
	RowCount _rows;
}; 
