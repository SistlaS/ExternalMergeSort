
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

// init Cache TT
Tree cache_tt(Config::num_cache_TT_leaf_nodes,"");
// init RAM TT
Tree ram_tt(Config::num_ram_TT_leaf_nodes,sorted_ram_output);
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


// copy file contents from src to dest
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

    // sort into RAM-sized runs
    ramExternalSort();
    // diskExternalSort();
	
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

// perform "external merge sorting" - merge sorted runs till only one run remains
// void SortIterator::mergeSort(bool isRAM){
//     string fromFile = isRAM ? "RAM.txt" : "Disk.txt";
//     string toFile = isRAM ? "Cache.txt" : "RAM.txt";
//     string tempFile = isRAM ? "RAM3.txt" : "Disk2.txt";

//     cout<<"In merge sort; isRAM: "<<isRAM<<"\n";

//     uint F = SystemConfigurations::ram_size;    

//     // sizes of runs which have already been sorted
//     vector<int>runsProcessed = isRAM? SortIterator::ram_gdFactors: SortIterator::disk_gdFactors;
    
//     while(runsProcessed.size()>1){

//         std::ifstream input_file(fromFile,ios::in);
//         if (!input_file or !input_file.is_open())
//         {
//             cout << "Could not open file: " << fromFile << "...Exit from program" << endl;
//             exit(1);
//         }

//         vector<int> gd2 = computeGracefulDegradationFactors(runsProcessed.size(),F);

//         int numRuns = 0;
//         string bunchOfRuns;

//         for(int i=0;i<gd2.size();i++){

//             vector<queue<string>>v;
//             int numQ = 0;
//             while(numRuns < gd2[i] && !input_file.eof()){
//                 queue<string> q;
//                 // '\n' delimits sorted runs
//                 // '|' delimits records
//                 // ',' delimits columns
//                 while(getline(input_file, bunchOfRuns, '\n')){
//                     // we have a bunch of records (which are sorted wrt each other)
//                     // put this bunch in a queue
//                     std::stringstream ss(bunchOfRuns);
//                     string record;
//                     while (getline(ss, record, '|')) {
//                         q.push(record);
//                         numRuns++;
//                     }
//                     v.push_back(q);
//                     queue<string> emptyQueue;
//                     swap(q, emptyQueue);
//                     numQ++; 
//                 }
                
//             }

//             // Clear toFile
//             ofstream outFile4(toFile, ios::trunc);
//             outFile4.close();

//             for (size_t i = 0; i < v.size(); ++i) {
//                 cout << "Queue " << i + 1 << ":" << endl;

//                 // Access the queue as a constant reference
//                 const queue<string>& currentQueue = v[i];

//                 // Use a copy for printing without modifying the original queue
//                 queue<string> tempQueue = currentQueue;

//                 while (!tempQueue.empty()) {
//                     cout << tempQueue.front() << " ";
//                     tempQueue.pop(); // Modify the copy, not the original
//                 }
//                 cout << endl;
//             }

            
//             // Output: Sorted toFile
//             Tree tree(numQ,v,toFile);

//             // Step 5: Write sorted data into temp file
//             bool isSortWriteSuccess = copyFileContents(toFile, tempFile,1);
//             if (!isSortWriteSuccess) {
//                 cout << "Error: Could not copy the file " << toFile << " into " << tempFile << endl;
//                 exit(1);
//             }

//             // Step 6: Clear toFile
//             ofstream outFile(toFile, ios::trunc); // truncate file length to 0, effectively clearing it.
//             outFile.close();
//         }

//         runsProcessed = gd2;

//         // close the old file stream
//         input_file.close();

//         // clear fromFile, and save contents of tempFile in it
//         ofstream outFile(fromFile, ios::trunc); // truncate file length to 0, effectively clearing it.
//         outFile.close();

//         // copy contents of tempFile into fromFile
//         bool isDiskContentsCopied = copyFileContents(tempFile,fromFile,0);
//         if (!isDiskContentsCopied) {
//             cout << "Error: Could not copy the file " << tempFile << " into " << fromFile << endl;
//             exit(1);
//         }

//         ofstream outFile2(tempFile, ios::trunc); // truncate file length to 0, effectively clearing it.
//         outFile2.close();
//     }
// }

// performs external merge sort on cache size runs present in the RAM. Generates RAM-sized runs and writes them to disk
void ramMergeSort(int W){

    // if the number of runs in the RAM will be less than its capacity (64 cache runs), pass the number of runs in function argument
    int numCacheRuns = W;
    cout<<"Number of cache size runs: "<<W<<endl;

    vector<int> gdFactors = computeGracefulDegradationFactors(numCacheRuns,Config::num_ram_TT_leaf_nodes); 
    int numRuns = gdFactors.size();

    // loop continues till all of RAM is merged into one sorted run
    while(numRuns > 1){

        // Open RAM.txt in input mode
        ifstream inFile(ram,ios::in);
        if (!inFile or !inFile.is_open()) {
            cout << "Error: Could not open ifstream file " << ram << endl;
            exit(1);
        }
        numRuns = gdFactors.size();
        cout<<"Number of intermediate runs: "<<numRuns<<endl;
        if(numRuns==1) break;

        for(int i=0;i<numRuns;i++){
            vector<queue<string>>ram_tt_input;
            while(ram_tt_input.size()< gdFactors[i] && !inFile.eof()){
                string run;
                // run is delimited by newline character
                while(getline(inFile,run,'\n') && !inFile.eof()){
                    // break the run into records and push the run into a queue
                    cout<<"Run is: "<<run<<endl;
                    if(run.size()==0) break;
                    std::stringstream ss(run);
                    string record;
                    queue<string> q;
                    int numRecords=0;
                    while(getline(ss,record,'|')){
                        q.push(record);
                        numRecords++;
                    }
                    ram_tt_input.push_back(q);
                    cout<<"Number of records in this run: "<<numRecords<<endl;
                }
            }
            inFile.close();

            cout<<"RAM TT Input is:"<<endl;
            for(int i=0;i<ram_tt_input.size();i++){
                queue<string> tmp_q = ram_tt_input[i]; //copy the original queue to the temporary queue
                cout<<"Size of queue "<<i<<" is: "<<tmp_q.size() <<" Contents of the queue is: ";
                while (!tmp_q.empty())
                {
                    string q_element = tmp_q.front();
                    std::cout << q_element <<"\n";
                    tmp_q.pop();
                } 
            }

            if(!ram_tt_input.empty()){
                // clear RAM3.txt
                clearFile(sorted_ram_output);
                cout<<"Calling RAM TT"<<endl;
                // TT will flush its output into RAM3.txt
                ram_tt.generate_runs(ram_tt_input);
                cout<<"Checking if RAM3.txt has data in it\n";
                print_file_contents(sorted_ram_output);
                // clear RAM.txt
                clearFile(ram);
                // copy file contents from RAM3.txt to RAM.txt
                copyFileContents(sorted_ram_output,ram,1);
                cout<<"Checking if RAM.txt has data in it\n";
                print_file_contents(ram);
            }
        }
        gdFactors = computeGracefulDegradationFactors(numRuns,Config::num_ram_TT_leaf_nodes);
        if(gdFactors.size()<=1) break;
    }

    // in the last run,
    // processing is complete, move sorted data fom RAM.txt to Disk.txt
    copyFileContents(ram,disk,1);
    clearFile(ram);
    
}

// Sort all cache-size runs into ram-size runs. Store this data into disk
// Pulls data from RAM, RAM Buffer and from the temp disk storage
void SortIterator::ramExternalSort(){

    int maxCacheRunsInRAM = Config::ram_capacity / (Config::cache_tt_buffer_size);

    // sort the RAM.txt currently in memory(un-spilled runs)
    cout<<"Sorting in-memory run"<<endl;
    if(SortIterator::_ramUsed != Config::ram_capacity){
        int frac = SortIterator::_ramUsed % Config::cache_tt_buffer_size;
        if(frac!=0) frac = 1; // is there any run less than 16 records (to know if we need to roundup)
        int numRuns =  SortIterator::_ramUsed / Config::cache_tt_buffer_size + frac;
        ramMergeSort(numRuns);
    } else{
        ramMergeSort(maxCacheRunsInRAM);
    } 
    
    _numRAMRuns++; 
    
    // read the temp_disk, and write 64 runs into RAM. repeat till temp_disk is read fully, then clear it
    // number of cache runs in RAM currently
    int cacheRunCount = 0;

    // First process the runs in the buffer - place them into the RAM.txt
    if(SortIterator::_ramBufferUsed!=0){
        copyFileContents(ram_buffer,ram,1);
        cacheRunCount += (SortIterator::_ramBufferUsed/Config::cache_tt_buffer_size);
        cout<<"Number of runs in the RAM buffer "<<cacheRunCount<<endl;
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
            outFile << run << "\n";
            cacheRunCount++;
            //
            if(cacheRunCount == maxCacheRunsInRAM){
                outFile.close(); // since we are opening a ifstream to RAM.txt in ramMergeSort(), closing this ofstream
                cout<<"Starting RAM run number: "<< _numRAMRuns+1 <<"\n";
                ramMergeSort(maxCacheRunsInRAM);
                _numRAMRuns++;
                

                cacheRunCount=0;
                ofstream outFile(ram,ios::app); // back in business
                if (!outFile) {
                    cout << "Error: Could not open ofstream file " << ram << endl;
                    exit(1);
                }
            }
            
        }

        if(cacheRunCount != 0){
            // cout<<"Number of remaining cache runs to be processed: "<<cacheRunCount<<endl;
            outFile.close();
            ramMergeSort(cacheRunCount);
            _numRAMRuns++;
        }

        // all runs processed, so clear the spilled runs from disk
        clearFile(temp_disk); 
    }
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
        
        // generate runs
        // cout<<"Cache TT Input is:"<<endl;
        // for(int i=0;i<tt_input.size();i++){
        //     queue<string> tmp_q = tt_input[i]; //copy the original queue to the temporary queue
        //     cout<<"Size of queue "<<i<<" is: "<<tmp_q.size() <<endl;


        //     while (!tmp_q.empty())
        //     {
        //         string q_element = tmp_q.front();
        //         std::cout << q_element <<" ";
        //         tmp_q.pop();
        //     } 
        // }
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

void insertCacheRunsInRAM(string cacheRun){
    
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

        // close the file
        ram_buffer_file.close();
        
        SortIterator::_ramBufferUsed+=Config::cache_tt_buffer_size;
        return;
    }

    // Open RAM.txt
    ofstream ram_file(ram, ios::app);
    if(!ram_file.is_open()){
        cerr<<"Issue with opening RAM.txt, exit\n";
        exit(1);
    }

    // add run to RAM.txt - we do NOT put this in disk
    cout<<"Cache run value: "<<cacheRun<<endl;
    ram_file << cacheRun;

    // close the file
    ram_file.close();
    SortIterator::_ramUsed += Config::cache_tt_buffer_size;
    cout<<"Number of records in RAM: "<<SortIterator::_ramUsed <<endl;
}

// sort the RAM-sized runs
void SortIterator::diskExternalSort(){
    RowCount W = _numRAMRuns; // number of RAM runs in disk
    int F = Config::num_ram_TT_leaf_nodes;

    vector<int> gdFactors = computeGracefulDegradationFactors(W,F); 
    vector<queue<string>>ram_tt_input; 

    int numRuns = gdFactors.size();
    // loop continues till all of Disk is merged into one sorted run
    while(numRuns > 1){

        // Open Disk.txt in input mode
        ifstream inFile(disk,ios::in);
        if (!inFile or !inFile.is_open()) {
            cout << "Error: Could not open ifstream file " << disk << endl;
            exit(1);
        }
        numRuns = gdFactors.size();

        for(int i=0;i<numRuns;i++){
            while(ram_tt_input.size()< gdFactors[i] && !inFile.eof()){
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
                ram_tt.generate_runs(ram_tt_input);
                ram_tt_input.clear();
            }
            inFile.close();
            // TT will flush its output into RAM3.txt
                
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