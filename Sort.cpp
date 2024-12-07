
#include "Sort.h"
#include "Tree.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>

using namespace std;

string ram = "RAM.txt";
string cache = "Cache.txt";
string ram_buffer = "RAM2.txt";
string disk = "Disk.txt";
string temp_disk = "Disk2.txt";

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


bool copyFileContents(string sourceFileName, string destinationFileName, int mode){
	// Open the source file in input mode
    ifstream inFile(sourceFileName);
    if (!inFile) {
        cout << "Error: Could not open source file " << sourceFileName << endl;
        return false;
    }

    // Open the destination file in output mode (will append if exists)
    ofstream outFile;
	if(mode==0){
		outFile.open(destinationFileName, ios::out); // overwrite mode
	}
	else{
		outFile.open(destinationFileName, ios::app); // append mode
	}

    if (!outFile) {
        cout << "Error: Could not open destination file " << destinationFileName << endl;
        return false;
    }

    // Copy content from source file to destination file
    outFile << inFile.rdbuf();

    // Close both files
    inFile.close();
    outFile.close();

    return true;
}

void SortIterator::generateCacheRuns(Row row, bool lastBatch){

    // if we have enough records in Cache.txt OR if its the last batch of records, generate the cache runs
    if(_cacheUsed == Config::num_cache_TT_leaf_nodes or lastBatch){
        // pull all records into string by tokens
        string record;
        // Input for the cache TT
        vector<queue<string>> tt_input;
        queue<string> q;

        // Open the source file in input mode
        ifstream inFile(cache);
        if (!inFile) {
            cout << "Error: Could not open file " << cache << endl;
            exit(1);
        }

        while(getline(inFile,record,'|')){
            record += "|"; // re-adding the delimiter
            q.push(record); // queue of size 1 for the cache runs
            tt_input.push_back(q);

            q.pop(); // clear the queue and reuse
        }

        // call the cache TT
        // cache_tt.generate_runs(tt_input);
        tt_input.clear();

        // clear the cache file
        inFile.close(); // Close ifstream first

        // Clear file
        ofstream outFile(cache, ios::trunc);
        if (!outFile.is_open()) {
            cerr << "Failed to clear the file!" << endl;
            exit(1);
        }
        outFile.close();
    }

    if(lastBatch) return;

    // write record into cache
    ofstream outfile(cache,ios::out);
    if(!outfile.is_open()){
        cerr<<"Issue with opening Cache.txt, exit\n";
        exit(1);
    }
    outfile << row.getRow();
    outfile.close();

    ++ _consumed;
    ++ _cacheUsed;
}

// move RAM buffer contents into a temp file in Disk
void SortIterator::spillBufferToDisk(){
    bool isSortWriteSuccess = copyFileContents(ram_buffer, temp_disk,1);
    if (!isSortWriteSuccess) {
        cout << "Error: Could not copy the file " << ram_buffer << " into " << temp_disk << endl;
        exit(1);
    }
}

void SortIterator::insertCacheRunsInRAM(string cacheRun){
    // Open RAM.txt
    fstream ram_file(cache, ios::in | ios::out);
    if(!ram_file.is_open()){
        cerr<<"Issue with opening RAM.txt, exit\n";
        exit(1);
    }

    // if we have enough records in RAM.txt OR if its the last batch of records
    if(_ramUsed == Config::ram_capacity){
        if(_ramBufferUsed == Config::ram_buffer_capacity){
            spillBufferToDisk();

            // clear the buffer
            fstream ram_buffer_file(ram_buffer, ios::trunc);
            ram_buffer_file.close();
            _ramBufferUsed=0;
        } 
        fstream ram_buffer_file(ram_buffer, ios::in | ios::out);
        if(!ram_buffer_file.is_open()){
            cerr<<"Issue with opening RAM_buffer.txt, exit\n";
            exit(1);
        }
        // add record in buffer
        ram_buffer_file << cacheRun << "\n";

        // close the file
        ram_buffer_file.close();
        _ramBufferUsed++;
    }

    // add run to RAM.txt
    ram_file << cacheRun << "\n";

    // close the file
    ram_file.close();
    ++ _ramUsed;
}