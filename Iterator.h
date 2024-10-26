#include <iostream>
#pragma once
#include "defs.h"
#include <vector>

typedef uint64_t RowCount;

class Row
{
public:
	std::vector<int> data {7,5,2,3,5};
	int offsetValueCode;
	Row ();
	virtual ~Row ();
	bool isFiltered() const {
        for (int value : data) {
			std::cout<<value<<" val ";
            if (value < 2) return false;  // value > 4 ==> row fails the filter
        }
        return true;  
    }
	// ...
private:
	// ...
}; 

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
