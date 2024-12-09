
#include "Sort.h"
#include "Tree.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <cassert>

using namespace std;

// TT - Tournament tree
string cache = "Cache.txt"; // store the input records before it is pushed as the Cache TT input
string ram = "RAM.txt";     // store the cache-size runs in memory
string ram_buffer = "RAM2.txt"; // buffer for RAM.txt, we spill buffer into SSD for graceful degradation
string sorted_ram_output = "RAM3.txt"; // storing the output of the RAM TT in this temp file for implementation convinience; not considered for memory calculations
string disk = "Disk.txt";   // Store the final sorted output here
string temp_disk = "Disk2.txt"; // Store the spilled cache-size runs in a temp file in disk

// init Cache TT
Tree cache_tt(Config::num_cache_TT_leaf_nodes,"");
// init RAM TT
Tree ram_tt(Config::num_ram_TT_leaf_nodes,sorted_ram_output);
// init RAM TT for DiskSort
Tree disk_tt(Config::num_ram_TT_leaf_nodes,temp_disk);
RowCount SortIterator::_bufferSpills = 0;
RowCount SortIterator::_ramBufferUsed = 0;
RowCount SortIterator::_ramUsed = 0;


/* File management functions */
void print_file_contents(string filename){
    cout<< "Printing contents of file "<<filename<<endl;

    std::ifstream f(filename);

    if (f.is_open())
        std::cout << f.rdbuf()<<endl;
    
    f.close();
}

bool copyFileContents(string sourceFileName, string destinationFileName, int mode){
	// Open the source file in input mode
    ifstream inFile(sourceFileName,ios::in);
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

/* Compute optimal merge values for a given number of runs. Values are stored and referred to as necessary*/
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

/********************* SortPlan ******************************** */
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

/********************* SortIterator ******************************** */
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

    // sort into RAM-sized runs
    ramExternalSort();

    diskExternalSort();

	
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
    cout<<"Number of Cache size runs: "<<W<<endl;
    int numMergePerLevel = W;
    int mergeStep=1;
    // loop continues till all of Disk is merged into one sorted run

    while(true){

        // Open Disk.txt in input mode
        ifstream inFile(ram,ios::in);
        if (!inFile or !inFile.is_open()) {
            cout << "Error: Could not open ifstream file " << ram << endl;
            exit(1);
        }

        cout<<"!!!!!!!!!!!!!!!!!!!!!!At Merge Step: "<<mergeStep<<" !!!!!!!!!!!!!!!!!!!!!";

        vector<int> gdFactors = computeGracefulDegradationFactors(numMergePerLevel,Config::num_ram_TT_leaf_nodes);
        cout<<"GDFactors are:\n";
        for(auto x: gdFactors) {cout<<x<<" ";} cout<<endl;
        cout<<"Number of merges in this level: "<<numMergePerLevel; 
        if(gdFactors.size()==1 && gdFactors[0]==1) {
            cout<<"Breaking in Disk merge sort \n";
            break;
        }
        
        int size = gdFactors.size();
        for(int i=0;i<size;i++){
            vector<queue<string>>ram_tt_input;
            int numQ=0;
            string run;
            while(numQ< gdFactors[i] && getline(inFile,run,'\n') && !inFile.eof()){
               
                cout<<"Run is: "<<run<<endl;
                std::stringstream ss(run);
                string record;
                queue<string> q;
                int numRecords=0;
                while(getline(ss,record,'|')){
                    q.push(record);
                    numRecords++;
                }

                ram_tt_input.push_back(q);
                numQ++;
            }
            // inFile.close();
            if(!ram_tt_input.empty()){
                ram_tt.generate_runs(ram_tt_input);
                ram_tt_input.clear();
                numQ=0;
            }
            
        }

        cout<<"***************************DONE WITH MERGE LEVEL "<<mergeStep<<endl;
        inFile.close();

        clearFile(ram);

        copyFileContents(sorted_ram_output,ram,1);
        clearFile(sorted_ram_output);
        print_file_contents(ram);


        numMergePerLevel = gdFactors.size();
        mergeStep++;
    }
    copyFileContents(ram, disk, 1);
    clearFile(ram);
    clearFile(sorted_ram_output); 
}


// Sort all cache-size runs into ram-size runs. Store this data into disk
// Pulls data from RAM, RAM Buffer and from the temp disk storage
void SortIterator::ramExternalSort(){

    int maxCacheRunsInRAM = Config::ram_capacity / (Config::tt_buffer_size);

    // sort the RAM.txt currently in memory(un-spilled runs)
    cout<<"Sorting the in-memory run"<<endl;
    assert(SortIterator::_ramUsed <= Config::ram_capacity && "*******Ram capacity is greater than allocated records**********");
    if(SortIterator::_ramUsed != Config::ram_capacity){
        int frac = SortIterator::_ramUsed % Config::tt_buffer_size;
        if(frac!=0) frac = 1; // check if there any run less than 16 records (to know if we need to roundup)
        int numRuns =  (SortIterator::_ramUsed / Config::tt_buffer_size) + frac;
        ramMergeSort(numRuns);
    } else{
        ramMergeSort(maxCacheRunsInRAM);
    } 
    
    SortIterator::_ramUsed = 0;
    _numRAMRuns++; // number of RAM sized runs 
    
    // read the temp_disk, and write 64 runs into RAM. repeat till temp_disk is read fully, then clear it
    // number of cache runs in RAM currently
    int cacheRunCount = 0;

    // First process the runs in the buffer - place them into the RAM.txt
    if(SortIterator::_ramBufferUsed!=0){
        copyFileContents(ram_buffer,ram,1);
        cacheRunCount += (SortIterator::_ramBufferUsed/Config::tt_buffer_size);
        cout<<"Number of runs in the RAM buffer "<<cacheRunCount<<endl;
        print_file_contents(ram);
        clearFile(ram_buffer);
        SortIterator::_ramBufferUsed=0;
    }

    if(_bufferSpills!=0){
        cout<<"Number of buffer spills: "<<_bufferSpills<<endl;
        ifstream inFile(temp_disk,ios::in);
        if (!inFile) {
            cout << "Error: Could not open ifstream file " << temp_disk << endl;
            exit(1);
        }

        ofstream outFile(ram, ios::app);
        if (!outFile) {
            cout << "Error: Could not open ofstream file " << ram << endl;
            exit(1);
        }

        string run;
        
        cout<<"Start processing the spilled runs\n";
        while(getline(inFile,run,'\n') && !inFile.eof()){
            outFile << run<< "\n";
            outFile.flush();
            cout<<"Writing spilled cache run "<< run <<" into RAM"<<endl;
            cacheRunCount++;
            // if RAM is at capacity, perform merge sort
            if(cacheRunCount == maxCacheRunsInRAM ){
                cout<<"Starting RAM run number: "<< _numRAMRuns+1 <<"\n";
                cout<<"Number of remaining cache runs to be processed: "<<cacheRunCount<<endl;
                ramMergeSort(cacheRunCount);

                _numRAMRuns++;
                cacheRunCount=0;
            }
        }
        
        outFile.close();
        
        // process anything remaining in the RAM
        if(cacheRunCount != 0){
            cout<<"Number of remaining cache runs to be processed: "<<cacheRunCount<<endl;
            cout<<"Starting RAM run number: "<< _numRAMRuns+1 <<"\n";
            print_file_contents(ram);
            ramMergeSort(cacheRunCount);
            _numRAMRuns++;
            cacheRunCount=0;
        }
    }

    // Will be called if there are no runs in the buffer
    if(cacheRunCount != 0){
        cout<<"Number of remaining cache runs to be processed: "<<cacheRunCount<<endl;
        cout<<"Starting RAM run number: "<< _numRAMRuns+1 <<"\n";
        ramMergeSort(cacheRunCount);
        _numRAMRuns++;
        cacheRunCount=0;
    }

    // all runs processed, so clear the spilled runs from disk
    clearFile(temp_disk); 
    
}


// Generates cache size mini runs
void SortIterator::generateCacheRuns(Row row, bool lastBatch){

    // if we have enough records in Cache.txt OR if its the last batch of records, generate the cache runs
    if(_cacheUsed == Config::num_cache_TT_leaf_nodes or lastBatch){
        // pull all records into string by tokens
        string record;
        // Input for the cache TT
        vector<queue<string>> tt_input;
        queue<string> q;

        // Open Cache.txt
        ifstream inFile(cache,ios::in);
        if (!inFile) {
            cout << "Error: Could not open ifstream file " << cache << endl;
            exit(1);
        }
        
        // tokenize records and push into TT
        while(getline(inFile,record,'|') && !inFile.eof()){
            q.push(record); // queue of size 1 for the cache runs
            tt_input.push_back(q);
            q.pop(); // clear the queue and reuse
        }
        
        cache_tt.generate_runs(tt_input);
        
        // clear for next use
        tt_input.clear();
        inFile.close(); 
        clearFile(cache);
        _cacheUsed = 0;
    }
}
    

/* ----------------------------------- DISK Merge Sort -------------------------------*/

// sort the RAM-sized runs
void SortIterator::diskExternalSort(){
    cout<<"Number of RAM size runs: "<<_numRAMRuns<<endl;
    int numMergePerLevel = _numRAMRuns;
    int mergeStep=1;
    // loop continues till all of Disk is merged into one sorted run
    while(true){

        // Open Disk.txt in input mode
        ifstream inFile(disk,ios::in);
        if (!inFile or !inFile.is_open()) {
            cout << "Error: Could not open ifstream file " << disk << endl;
            exit(1);
        }

        cout<<"!!!!!!!!!!!!!!!!!!!!!!At Merge Step: "<<mergeStep<<" !!!!!!!!!!!!!!!!!!!!!";

        vector<int> gdFactors = computeGracefulDegradationFactors(numMergePerLevel,Config::num_ram_TT_leaf_nodes);
        cout<<"GDFactors are:\n";
        for(auto x: gdFactors) {cout<<x<<" ";} cout<<endl;
        cout<<"Number of merges in this level: "<<numMergePerLevel; 
        if(gdFactors.size()==1 && gdFactors[0]==1) {
            cout<<"Breaking in Disk merge sort \n";
            break;
        }

        int size = gdFactors.size();
        for(int i=0;i<size;i++){
            vector<queue<string>>ram_tt_input;
            int numQ = 0;
            string run;
            while(numQ< gdFactors[i] && getline(inFile,run,'\n') && !inFile.eof()){
               
                cout<<"Run is: "<<run<<endl;
                std::stringstream ss(run);
                string record;
                queue<string> q;
                int numRecords=0;
                while(getline(ss,record,'|')){
                    q.push(record);
                    numRecords++;
                }
                ram_tt_input.push_back(q);
                numQ++;
                cout<<"Number of records in this run: "<<numRecords<<endl;
                
            }
            // inFile.close();
            if(!ram_tt_input.empty()){
                disk_tt.generate_runs(ram_tt_input);
                ram_tt_input.clear();
                numQ=0;
            }
            
        }

        cout<<"***************************DONE WITH MERGE LEVEL "<<mergeStep<<endl;
        inFile.close();

        clearFile(disk);

        copyFileContents(temp_disk,disk,1);
        clearFile(temp_disk);
        print_file_contents(disk);

        numMergePerLevel = gdFactors.size();
        mergeStep++;
    }

}