
#include "Sort.h"
#include "Tree.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>

using namespace std;

string ram = "RAM.txt";
string cache = "Cache.txt";
string buf_ram = "RAM2.txt";
string disk = "Disk.txt";

// TODO: Placeholder inits
// init Cache TT
// Tree cache_tt(Config::num_cache_TT_leaf_nodes,{},ram);
// init RAM TT
// Tree ram_tt(Config::num_ram_TT_leaf_nodes,{},disk);

SortPlan::SortPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (true);
} // SortPlan::SortPlan

SortPlan::~SortPlan ()
{
	TRACE (true);
	delete _input;
} // SortPlan::~SortPlan

Iterator * SortPlan::init () const
{
	TRACE (true);
	return new SortIterator (this);
} // SortPlan::init

SortIterator::SortIterator (SortPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	TRACE (true);

	for (Row row;  _input->next (row);  _input->free (row)){
        generateCacheRuns(row,false);
    }

    // process any rows not processed till now
    if(_cacheUsed!=0){
        SortIterator::isLastBatch = true;
        Row row; // passing dummy value
        generateCacheRuns(row,true);
    }
	
	traceprintf ("%s consumed %lu rows\n",
			_plan->_name,
			(unsigned long) (_consumed));
} // SortIterator::SortIterator

SortIterator::~SortIterator ()
{
	TRACE (true);

	traceprintf ("%s produced %lu of %lu rows\n",
			_plan->_name,
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
} // SortIterator::~SortIterator

bool SortIterator::next (Row & row)
{
	TRACE (true); 
	if (_produced >= _consumed)  return false;
	++ _produced;
	return true;
} // SortIterator::next

void SortIterator::free (Row & row)
{
	TRACE (true);
} // SortIterator::free

void SortIterator::generateCacheRuns(Row row, bool lastBatch){

    // Open Cache.txt
    fstream cache_file(cache, ios::in | ios::out);
    if(!cache_file.is_open()){
        cerr<<"Issue with opening Cache.txt, exit\n";
        exit(1);
    }

    // if we have enough records in Cache.txt OR if its the last batch of records, generate the cache runs
    if(_cacheUsed == Config::num_cache_TT_leaf_nodes or lastBatch){
        // pull all records into string by tokens
        string record;
        // Input for the cache TT
        vector<queue<string>> tt_input;
        queue<string> q;

        while(getline(cache_file,record,'|')){
            record += "|"; // re-adding the delimiter
            q.push(record); // queue of size 1 for the cache runs
            tt_input.push_back(q);

            q.pop(); // clear the queue and reuse
        }

        // call the cache TT
        // cache_tt.generate_runs(tt_input);

        // clear the cache file
        cache_file.close(); // Close the file first
        cache_file.open(cache, ios::out | ios::trunc); // Truncate the file
        if (!cache_file.is_open()) {
            cerr << "Failed to clear the file!" << endl;
            exit(1);
        }
    }

    if(lastBatch) return;
    cache_file << row.getRow();
    ++ _consumed;
    ++ _cacheUsed;
}

void SortIterator::insertCacheRunsInRAM(string cacheRun){
    // Open RAM.txt
    fstream ram_file(cache, ios::in | ios::out);
    if(!ram_file.is_open()){
        cerr<<"Issue with opening RAM.txt, exit\n";
        exit(1);
    }

    // if we have enough records in RAM.txt OR if its the last batch of records
    if(_ramUsed == Config::num_ram_TT_leaf_nodes){
        
    }

    ram_file << cacheRun;
    ++ _ramUsed;
}