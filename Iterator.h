#pragma once

#include "defs.h"
#include <string>
#include <iostream>


typedef uint64_t RowCount;

using namespace std;

class Row
{
public:
	Row ();
	virtual ~Row ();

    string row;

    string getRow() const {
        return row;
    }

    void setRow(const string& newRow)  {
        row = newRow;
    }
    
    void clear(){
        row.clear();
    }
private:
	// string row;
}; // class Row

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
}; // class Plan

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

}; // class Iterator
