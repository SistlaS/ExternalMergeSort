
#include "Sort.h"
#include "Tree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <chrono>

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

RowCount SortIterator::_bufferSpills = 0;
RowCount SortIterator::_ramBufferUsed = 0;
RowCount SortIterator::_ramUsed = 0;


/* --------------------------------- File management helper functions --------------------------------------- */

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

/* ------------------------ SortPlan --------------------------------*/

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

/* ------------------------- SortIterator -----------------------------*/
std::string removeLastColumn(const std::string& input) {
    if (input.empty()) {
        return input; // If input is empty, return as is
    }
    
    size_t lastComma = input.find_last_of(','); // Find the last comma
    if (lastComma == std::string::npos) {
        return ""; // If no comma is found, return an empty string
    }
    
    return input.substr(0, lastComma); // Return the substring before the last comma
}
void cleanOVCFromDisk(){
    ifstream inFile(disk, ios::in);
    if (!inFile) {
        cout << "Error: Could not open ifstream file " << cache << endl;
        exit(1);
    }
    ofstream outFile;
    outFile.open("Output.txt", ios::out); // overwrite mode
    if (!outFile) {
        cout << "Error: Could not open destination file Output.tx" << endl;
        exit(1);
    }
    string token;
    while(getline(inFile, token, '|')){
        string data = removeLastColumn(token);
        outFile<<data<<"|";
    }
    return;
}

SortIterator::SortIterator (SortPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0), _currentLine("0")
{
	TRACE (true);

    // to enable multiple runs of executable w/o manually deleting old results
    clearFile(disk);

    // High-Level Overview of the Sort Algorithm:

    // Step 1: as each row enters into memory, we add it into the Cache.
    // Step 2: once Cache reaches capacity, we generate miniruns and send it to RAM
    // Step 3: if RAM reaches capacity, we spill the cache-size miniruns - first into a buffer located within RAM, and finally to a location in disk.
    // Step 4: After all records are sorted into miniruns, and are flushed to RAM, we perform RAM merge sort - to sort the miniruns into RAM-sized runs. 
    // RAM-sized runs are stored into disk.
    // Step 5: Once all records are sorted ino RAM-sized runs, we perform external merge sort algorithm on them and finally process into a single sorted run, which is returned to the user
	
    // Steps 1 & 2

    auto start = chrono::high_resolution_clock::now();

    for (Row row;  _input->next (row);  _input->free (row)){
        generateCacheRuns(row,false);
    }

    // process all unprocessed not processed till now
    if(_cacheUsed!=0){
        SortIterator::isLastBatch = true;
        Row row; // passing dummy value
        generateCacheRuns(row,true);
    }

    
    //Step 4: sort into RAM-sized runs
    ramExternalSort();

    //Step 5: sort the RAM sized runs into final sorted run
    mergeSort(true,_numRAMRuns);

    //clean Disk.txt to remove last column
    cleanOVCFromDisk();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsed = end - start;
	cout << "***************Execution time for exeternal merge sort: " << elapsed.count() << " ms ***************"<<endl;
	traceprintf ("%s consumed %lu rows\n",
			_plan->_name,
			(unsigned long) (_consumed));
    outFile.open("Disk.txt");
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
	
	if (outFile.is_open())
	{
		if (getline(outFile, _currentLine, '|'))
		{
            cout<<"bbbb  "<<_currentLine<<endl;
			row.setRow(_currentLine);
		}
	}
    ++ _produced;
	return true;
} // SortIterator::next

void SortIterator::free (Row & row)
{
	TRACE (true);
} // SortIterator::free

/* --------------------------------- Sort helper function --------------------------------------- */

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

/*  ----------------------------------CACHE & CACHE->RAM ---------------------------------------*/

// Step 1 & 2: Generates cache size mini runs
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
        
        // generates the runs. The queue flushes output to the RAM
        cache_tt.generate_runs(tt_input);
        
        // clear for next use
        tt_input.clear();
        inFile.close(); 
        clearFile(cache);
        _cacheUsed = 0;
    }
    
    // don't write to cache.txt if last batch
    if(lastBatch) return;

    // write record into cache
    ofstream outfile(cache,ios::app);
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
void spillBufferToDisk(){
    copyFileContents(ram_buffer, temp_disk,1);
    clearFile(ram_buffer);
    SortIterator::_bufferSpills++;
}


// Step 3: Spill cache-size runs to disk as necessary
void insertCacheRunsInRAM(string cacheRun){
    if(cacheRun=="\n") return;

    // if we have enough records in RAM.txt OR if its the last batch of records
    if(SortIterator::_ramUsed == Config::ram_capacity){
        // if buffer is full, spill to disk
        if(SortIterator::_ramBufferUsed == Config::ram_buffer_capacity){
            spillBufferToDisk();
            SortIterator::_ramBufferUsed=0;
        }
        // else, write into RAM buffer 
        ofstream ram_buffer_file(ram_buffer, ios::app);
        if(!ram_buffer_file.is_open()){
            cerr<<"Issue with opening RAM_buffer.txt, exit\n";
            exit(1);
        }

        // add record in buffer
        ram_buffer_file << cacheRun;
        ram_buffer_file.close();
        
        SortIterator::_ramBufferUsed+=Config::tt_buffer_size;
        return;
    }

    // Open RAM.txt
    ofstream ram_file(ram, ios::app);
    if(!ram_file.is_open()){
        cerr<<"Issue with opening RAM.txt, exit\n";
        exit(1);
    }

    // add run to RAM.txt - we do NOT put this in disk
    ram_file << cacheRun;

    // close the file
    ram_file.close();
    SortIterator::_ramUsed += Config::tt_buffer_size;
}


/*  -------------------- RAM & RAM->Disk ----------------------------*/

// Step 4: Sort all cache-size runs into ram-size runs. Store this data into disk
// Pulls data from RAM, RAM Buffer and from the temp disk storage
void SortIterator::ramExternalSort(){

    int maxCacheRunsInRAM = Config::ram_capacity / (Config::tt_buffer_size);

    // sort the RAM.txt currently in memory(un-spilled runs)
    cout<<"Sorting the in-memory run present in RAM:"<<endl;

    // since RAM used counts the number of records, we are calculating the number of runs present in the RAM
    if(SortIterator::_ramUsed != Config::ram_capacity){
        int frac = SortIterator::_ramUsed % Config::tt_buffer_size;
        if(frac!=0) frac = 1; // is there any run less than 16 records (to know if we need to roundup)
        int numCacheRuns =  (SortIterator::_ramUsed / Config::tt_buffer_size) + frac;
        mergeSort(false,numCacheRuns);
    } else{
        mergeSort(false,maxCacheRunsInRAM);
    } 
    
    SortIterator::_ramUsed = 0; // reset values
    _numRAMRuns++; // number of RAM sized runs 
    int cacheRunCount = 0; // number of cache runs in RAM currently

    
    // First process the runs in the buffer if any - place them into the RAM.txt
    if(SortIterator::_ramBufferUsed!=0){
        copyFileContents(ram_buffer,ram,1);
        cacheRunCount += (SortIterator::_ramBufferUsed/Config::tt_buffer_size);
        cout<<"Number of runs in the RAM buffer "<<cacheRunCount<<endl;
        print_file_contents(ram);
        clearFile(ram_buffer);
        SortIterator::_ramBufferUsed=0;
    }

    // Next process the runs in the disk if any - place them into the RAM.txt
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
            // cout<<"Writing spilled cache run "<< run <<" into RAM"<<endl;
            cacheRunCount++;
            // process the buffer runs in RAM-size batches
            if(cacheRunCount == maxCacheRunsInRAM ){
                cout<<"Starting RAM run number: "<< _numRAMRuns+1 <<"\n";
                cout<<"Number of remaining cache runs to be processed: "<<cacheRunCount<<endl;
                mergeSort(false,cacheRunCount);

                _numRAMRuns++;
                cacheRunCount=0;
            }
        }
        
        outFile.close();
        
        // if there are any unprocessed runs remaining, or if total # of spilled runs < maxCacheRunsInRAM, control flows here
        if(cacheRunCount != 0){
            cout<<"Number of remaining cache runs to be processed: "<<cacheRunCount<<endl;
            cout<<"Starting RAM run number: "<< _numRAMRuns+1 <<"\n";
            mergeSort(false,cacheRunCount);
            _numRAMRuns++;
            cacheRunCount=0;
        }
    }

    // if buffer was not empty && there are no spilled runs, control flows here
    if(cacheRunCount != 0){
        cout<<"Number of remaining cache runs to be processed: "<<cacheRunCount<<endl;
        cout<<"Starting RAM run number: "<< _numRAMRuns+1 <<"\n";
        mergeSort(false,cacheRunCount);
        _numRAMRuns++;
        cacheRunCount=0;
    }

    // all runs processed, so clear the spilled runs from disk
    clearFile(temp_disk); 
    
}

/*  ----------------------------------- Merge Sort for RAM and Disk ------------------------------------------------*/

// sort the Cache/RAM-sized runs
void SortIterator::mergeSort(bool isDiskSort, int numRuns){
    if(isDiskSort){
        ram_tt.setOpFilename(temp_disk);
        cout<<"Number of RAM size runs: "<<numRuns<<endl;
    } else{
        cout<<"Number of Cache size runs: "<<numRuns<<endl;
    }

    
    int mergeLevel=1;
    // loop continues till all of Disk is merged into one sorted run
    while(true){

        // Open file in input mode
        string filename = isDiskSort?disk:ram;
        string tt_output_filename = isDiskSort?temp_disk:sorted_ram_output;


        ifstream inFile(filename,ios::in);
        if (!inFile or !inFile.is_open()) {
            cout << "Error: Could not open ifstream file " << filename << endl;
            exit(1);
        }

        cout<<"!!!!!!!!!!!!!!!!!!!!!!    At Merge Step: "<< mergeLevel <<" !!!!!!!!!!!!!!!!!!!!!\n";

        vector<int> gdFactors = computeGracefulDegradationFactors(numRuns,Config::num_ram_TT_leaf_nodes);
        // cout<<"The GDFactors are:\n";
        for(auto x: gdFactors) {cout<<x<<" ";} cout<<endl;
        // cout<<"Number of merges in this level: "<<numRuns<<endl; 
        int size=gdFactors.size();
        if(size==1 && gdFactors[0]==1) {
            string breakMsg = isDiskSort? "Merge completed for DiskSort!!\n" : "Merge completed for RAMSort!!\n"; 
            cout<<breakMsg;
            break;
        }
        
        for(int i=0;i<size;i++){
            vector<queue<string>>ram_tt_input;
            int numQ=0;
            string run;
            // get the number of runs to be processed in this step and push into the TT
            while(numQ < gdFactors[i] && getline(inFile,run,'\n') && !inFile.eof()){
               
                std::stringstream ss(run);
                string record;
                queue<string> q;
                while(getline(ss,record,'|')){
                    q.push(record);
                }
                ram_tt_input.push_back(q);
                numQ++;
            }

            // generate the runs - TT pushes the output into respective file
            ram_tt.generate_runs(ram_tt_input);

            // reset values
            ram_tt_input.clear();
            numQ=0;            
        }

        cout<<"***************************  DONE WITH MERGE LEVEL "<<mergeLevel<<" ******************************"<<endl;
        inFile.close();

        // prepare the file to receive the sorted rows
        clearFile(filename);

        copyFileContents(tt_output_filename,filename,1);

        // prepare the output file for the next batch
        clearFile(tt_output_filename);

        // number of runs changes with every merge level
        numRuns = gdFactors.size();
        mergeLevel++;
    }

    // if it is RAM sort, we move the final RAM-sized sorted output to Disk
    if(!isDiskSort){
        copyFileContents(ram, disk, 1);
        clearFile(ram);
        // Not considering the output files for memory consideration in RAM - it is possible to store the data in the same (input) file, but chosen this method for convinience
        clearFile(sorted_ram_output);
    }
}