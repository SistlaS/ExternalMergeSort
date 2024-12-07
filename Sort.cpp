
#include "Sort.h"
#include "Tree.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>

using namespace std;

// TT - Tournament tree
string cache = "Cache.txt"; // store the input records before it is pushed as the Cache TT input
string ram = "RAM.txt";     // store the cache-size runs in memory
string ram_buffer = "RAM2.txt"; // buffer for RAM.txt, we spill buffer into SSD for graceful degradation
string sorted_ram_output = "RAM3.txt"; // storing the output of the RAM TT in this temp file for implementation convinience; not considered for memory calculations
string disk = "Disk.txt";   // Store the final sorted output here
string temp_disk = "Disk2.txt"; // Store the spilled cache-size runs in a temp file in disk

// TODO: Placeholder inits
// init Cache TT
// Tree cache_tt(Config::num_cache_TT_leaf_nodes,{},ram);
// init RAM TT
// Tree ram_tt(Config::num_ram_TT_leaf_nodes,{},disk);


/* File management functions */

// copy file contents from src to dest
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

// clear file contents
void clearFile(string filename){
    ofstream clear_file(filename, ios::trunc);
    if (!clear_file.is_open()) {
        cerr << "Failed to clear the file " << filename << " !" << endl;
        exit(1);
    }
    clear_file.close();
}

vector<int> computeGracefulDegradationFactors(int W, int F){
    vector<int> gd;

    int f1 = ((W-2)%(F-1))+2;
    W -= f1;
    gd.push_back(f1);

    while(W>=F){
        gd.push_back(F);
        W -= F;
    }

    if(W!=0) gd.push_back(W);
    return gd;
}

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


void ramMergeSort(int W){
    // read K= num_ram_TT_leaf_nodes runs from RAM, and perform merge sort on that
    // continue till all runs in RAM.txt is processed
    // write output into RAM3.txt // not considering this to be part of memory, as this is an implementation choice for convienience
    // once RAM3 has 1024 records, write it into Disk2.txt
    // clear the file after processing completes

    // if the number of runs in the RAM will be less than its capacity (64 bytes), pass the number of runs in function argument
    int numRAMRuns = W==0? Config::ram_capacity/Config::cache_tt_buffer_size : W;
    vector<int> gdFactors = computeGracefulDegradationFactors(numRAMRuns,Config::num_ram_TT_leaf_nodes); 
    vector<queue<string>>ram_tt_input; 

    int numRuns = gdFactors.size();
    // loop continues till all of RAM is merged into one sorted run
    while(numRuns > 1){
        numRuns = gdFactors.size();
        for(int i=0;i<numRuns;i++){
            while(ram_tt_input.size()< gdFactors[i]){

                // Open RAM.txt in input mode
                ifstream inFile(ram);
                if (!inFile) {
                    cout << "Error: Could not open file " << ram << endl;
                    exit(1);
                }

                string run;
                // run is delimited by newline character
                while(getline(inFile,run,'\n')){
                    // break the run into records and push the run into a queue
                    std::stringstream ss(run);
                    string record;
                    queue<string> q;
                    while(getline(ss,record,'|')){
                        q.push(record);
                    }
                    ram_tt_input.push_back(q);
                }

                inFile.close();
            }
            // TODO: call the RAM TT
            // TT will flush its output into RAM3.txt
            ram_tt_input.clear();

            // clear RAM.txt
            clearFile(ram);
            // copy file contents from RAM3.txt to RAM.txt
            copyFileContents(sorted_ram_output,ram,1);
            // clear RAM3.txt
            clearFile(sorted_ram_output);
        }
        
        gdFactors = computeGracefulDegradationFactors(numRuns,Config::num_ram_TT_leaf_nodes);
    }

    // in the last run,
    // processing is complete, move sorted data fom RAM.txt to Disk.txt
    copyFileContents(ram,disk,1);
    clearFile(ram);
    
}

void SortIterator::ramExternalSort(){
    // sort the RAM.txt currently in memory
    ramMergeSort(0); 
    
    // read the temp_disk, and write 64 runs into RAM. repeat till temp_disk is read fully, then clear it
    int maxCacheRunsInRAM = Config::ram_capacity / (Config::cache_tt_buffer_size);
    int runCount = 0;

    // First process the runs in the buffer - place them into the RAM.txt
    copyFileContents(ram_buffer,ram,1);
    runCount += (Config::ram_buffer_capacity / Config::cache_tt_buffer_size);
    clearFile(ram_buffer);

    ifstream inFile(temp_disk);
    if (!inFile) {
        cout << "Error: Could not open file " << temp_disk << endl;
        exit(1);
    }

    ofstream outFile(ram, ios::out);
    if (!outFile) {
    cout << "Error: Could not open file " << ram << endl;
    exit(1);
    }

    string run;
    while(getline(inFile,run,'\n')){
        outFile << run << "\n";
        runCount++;
        if(runCount == maxCacheRunsInRAM){
            outFile.close(); // since we are opening a ifstream to RAM.txt in ramMergeSort(), closing this ofstream
            ramMergeSort(0);

            runCount=0;
            ofstream outFile(ram,ios::out); // back in business
        }
    }


    if(runCount != 0){
        outFile.close();
        ramMergeSort(runCount);
    }
    


}

void SortIterator::generateCacheRuns(Row row, bool lastBatch){

    // if we have enough records in Cache.txt OR if its the last batch of records, generate the cache runs
    if(_cacheUsed == Config::num_cache_TT_leaf_nodes or lastBatch){
        // pull all records into string by tokens
        string record;
        // Input for the cache TT
        vector<queue<string>> tt_input;
        queue<string> q;

        // Open Cache.txt
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
        clearFile(cache);
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
    copyFileContents(ram_buffer, temp_disk,1);
}

void SortIterator::insertCacheRunsInRAM(string cacheRun){

    // if we have enough records in RAM.txt OR if its the last batch of records
    if(_ramUsed == Config::ram_capacity){
        if(_ramBufferUsed == Config::ram_buffer_capacity){
            spillBufferToDisk();
            clearFile(ram_buffer);
            _ramBufferUsed=0;
        } 
        ofstream ram_buffer_file(ram_buffer, ios::out);
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

    // Open RAM.txt
    ofstream ram_file(cache, ios::out);
    if(!ram_file.is_open()){
        cerr<<"Issue with opening RAM.txt, exit\n";
        exit(1);
    }

    // add run to RAM.txt - we do NOT put this in disk
    ram_file << cacheRun << "\n";

    // close the file
    ram_file.close();
    ++ _ramUsed;
}