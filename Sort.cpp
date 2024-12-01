#include "Sort.h"
#include "SystemConfigurations.h"
#include <fstream>
#include <iostream>
#include <queue>

using namespace std;


/* **************** SortPlan ********************** */ 

SortPlan::SortPlan (char const * const filename, Plan * const input)
	: Plan (filename), _input (input)
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




/* **************** SortIterator ********************** */ 


SortIterator::SortIterator (SortPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	TRACE (true);
    
    diskSort();

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

void print_file_contents(string filename){
    cout<< "Printing contents of file "<<filename<<endl;

    std::ifstream f(filename);

    if (f.is_open())
        std::cout << f.rdbuf()<<endl;
    
    f.close();
}

vector<int> SortIterator::computeGracefulDegradationFactors(int W, int F){
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

// mode 0 = overwrite
// mode 1 = append
bool SortIterator::writeDataIntoFile(string fileName, string data, int mode, string delimiter){

	ofstream file;
	if(mode==0){
		file.open(fileName, ios::out); // overwrite mode
	}
	else{
		file.open(fileName, ios::app); // append mode
	}

	if (!file) {
		cout << "Error: Could not open file " << fileName << endl;
		return false;
	}

	// write data into file
	file << data;

	if(delimiter != ""){
		file<<delimiter;
	}
	file.close();
	return true;
}

bool SortIterator::copyFileContents(string sourceFileName, string destinationFileName){
	// Open the source file in input mode
    ifstream inFile(sourceFileName, ios::in);
    if (!inFile) {
        cout << "Error: Could not open source file " << sourceFileName << endl;
        return false;
    }

    // Open the destination file in output mode (will append if exists)
    ofstream outFile(destinationFileName, ios::app);
    if (!outFile) {
        cout << "Error: Could not open destination file " << destinationFileName << endl;
        return false;
    }

    // Copy content from source file to destination file
    outFile << inFile.rdbuf();

    // Close both files
    inFile.close();
    outFile.close();

    cout << "File contents copied from " << sourceFileName << " to " << destinationFileName << endl;

    return true;
}


// perform "internal sorting" - sort records into sorted chunks
void SortIterator::firstPass(bool isRAM){

    string fromFile = isRAM ? "RAM.txt" : "Disk.txt";
    string toFile = isRAM ? "Cache.txt" : "RAM.txt";
    string tempFile = isRAM ? "RAM2.txt" : "Disk2.txt";

    cout<<"isRAM = " << isRAM << endl;
    // TODO: write to-sort chunks to RAM in terms of RAM page size
    // TODO: write the sorted chunks to Disk in terms of Disk page size
    int W = isRAM ? SortIterator::numRAMRecords : SystemConfigurations::num_records;
    int F = isRAM ? SystemConfigurations::cache_size : SystemConfigurations::ram_size;

    // if(isRAM) tt.init(F);
     
    vector<int> gd = computeGracefulDegradationFactors(W,F);
    cout<<"Graceful degradation: "<<endl;
    for(int i=0;i<gd.size();i++){cout<<gd[i]<<" ";}
    cout<<endl;
    SortIterator::gdFactors = gd;

    std::ifstream input_file(fromFile,ios::in);
    if (!input_file or !input_file.is_open())
    {
        cout << "Could not open file: " << fromFile << "...Exit from program" << endl;
        exit(1);
    }

    // buffer contains the chunk of records
	string buffer;
    // token contains each record
	string token;
    // number of tokens encountered
	int tokenCount = 0;
    int totalTokenCount=0;

    // input for TT
    vector<queue<string>>v;

	for(int i=0;i<gd.size();i++){
        cout<<"Vlaue of i: "<<i<<" and gd[i] value: "<< gd[i]<<endl;
        // Step 1: populate the buffer with gd[i] tokens
        if(isRAM){
            cout<< "Number of records in this run:" << gd[i]<<endl;
        }else cout<< "Number of sorted runs in this run:" << gd[i]<<endl;

		while(tokenCount < gd[i] && !input_file.eof()){
            
            if(getline(input_file, token, '\n')){
                buffer += token += "\n";
                tokenCount++;
                totalTokenCount++;
            }
		}
        cout << "Buffer :" << buffer << endl;
        if(buffer == ""){
            cout<<"file is empty, break\n";
        }

        // Step 2: Write buffer to toFile
        bool isWriteSuccess = writeDataIntoFile(toFile, buffer, 1, "");
		if (!isWriteSuccess) {
			cout << "Error: Could not write into file: " << toFile << endl;
			exit(1);
		}
        cout<< "Wrote buffer to file " << toFile <<" successfully."<<endl;

        // Step 3: Clear values
        buffer.clear();
        tokenCount=0;

        // Step 4: Perform sort in smaller memory level
        if(isRAM){
            // Input: Unsorted Cache.txt
            cout<<"Sort cache.txt"<<endl;
            std::ifstream ip(toFile,ios::in);
            if (!ip or !ip.is_open())
            {
                cout << "Could not open file: " << toFile << "...Exit from program" << endl;
                exit(1);
            }

            for(int i=0;i<gd[i];i++){
                queue<string> q;
                // queue of size 1
                if(getline(ip, token, '\n')){
                    if(token!="")q.push(token);
                }
                if(token!=""){
                    cout<<"Token in queue: "<< token <<"\n";
                    v.push_back(q);
                }

                if(ip.eof()) break;
            }

            ip.close();

            // TT(F,v,toFile)
            // Output: Sorted Cache.txt
            // TODO: Sort in cache with TT and write back in cache.txt
            cout<<"Cache is sorted: \n";
            print_file_contents(toFile);
        }else{
            cout<<"Sort ram.txt"<<endl;
            SortIterator::numRAMRecords=gd[i];
            ramSort();
        }

        // Step 5: Write sorted data into temp file
        bool isSortWriteSuccess = copyFileContents(toFile, tempFile);
		if (!isSortWriteSuccess) {
			cout << "Error: Could not copy the file " << toFile << " into " << tempFile << endl;
			exit(1);
		}
        cout<<"Write sorted data into temp file: "<<tempFile<<endl;
        print_file_contents(tempFile);

        // Step 6: Clear toFile
        // ofstream outFile(toFile, ios::trunc); // truncate file length to 0, effectively clearing it.
        // outFile.close();
        cout<<"Clear toFile: "<<toFile<<endl;
        print_file_contents(toFile);

    }
    cout<<"File: "<<tempFile<<" is first pass sorted\n";
    // close the old file stream
	input_file.close();

	// clear fromFile, and save contents of tempFile in it
	// ofstream outFile(fromFile, ios::trunc); // truncate file length to 0, effectively clearing it.
	// outFile.close();
    // cout<<"Cleared fromFile: "<<fromFile<<endl;
    // print_file_contents(fromFile);

	// copy contents of tempFile into fromFile
	bool isDiskContentsCopied = copyFileContents(tempFile,fromFile);
	if (!isDiskContentsCopied) {
		cout << "Error: Could not copy the file " << tempFile << " into " << fromFile << endl;
        exit(1);
	}
    cout<<"copy contents of tempFile into fromFile: "<<fromFile<<endl;
    print_file_contents(fromFile);

	// clear tempFile
	// ofstream outFile2(tempFile, ios::trunc); // truncate file length to 0, effectively clearing it.
	// outFile2.close();

}

// perform "external merge sorting" - merge sorted runs till only one run remains
void SortIterator::mergeSort(bool isRAM){
    // TODO: Merge sort happens only in RAM
    string fromFile = isRAM ? "RAM.txt" : "Disk.txt";
    string toFile = isRAM ? "Cache.txt" : "RAM.txt";
    string tempFile = isRAM ? "RAM2.txt" : "Disk2.txt";

    int F = isRAM ? SystemConfigurations::cache_size : SystemConfigurations::ram_size;

    // TODO: TT.init(F);

    // sizes of runs which have already been sorted
    vector<int>runsProcessed = SortIterator::gdFactors; 

    std::ifstream input_file(fromFile,ios::in);
    if (!input_file or !input_file.is_open())
    {
        cout << "Could not open file: " << fromFile << "...Exit from program" << endl;
        exit(1);
    }

    while(runsProcessed.size()>1){
        vector<int> gd2 = computeGracefulDegradationFactors(runsProcessed.size(),F);

        // number of tokens processed in each new run (of gd2)
        vector<int> tokProc;
        // input for TT
        vector<queue<string>>v;
        // ptr indicates at what pos we are in the runsProcessed array
        int prevRunPtr = 0;

        // # of tokens processed
        int tokenCount=0;
        string token;

        for(int i=0;i<gd2.size();i++){
            // number of runs to be processed
            int numRunsProc = gd2[i]; 
            for(int j=prevRunPtr;j<prevRunPtr+numRunsProc; j++){
                queue<string> q;
                while(tokenCount < runsProcessed[j] && !input_file.eof()){
                    if(getline(input_file, token, '\n')){
                        q.push(token);
                        tokenCount++;
                    }
                }
                v.push_back(q);

                // TODO: Sort using v in TT and append O/P to toFile

                // Step 5: Write sorted data into temp file
                bool isSortWriteSuccess = copyFileContents(toFile, tempFile);
                if (!isSortWriteSuccess) {
                    cout << "Error: Could not copy the file " << toFile << " into " << tempFile << endl;
                    exit(1);
                }

                // Step 6: Clear toFile
                ofstream outFile(toFile, ios::trunc); // truncate file length to 0, effectively clearing it.
                outFile.close(); 
                    
            }
            prevRunPtr+=numRunsProc;

            // number of tokens processed in this run
            tokProc.push_back(tokenCount);
            // reset to 0
            tokenCount=0;
        }

        // tokProc is now processed, hence it becomes the new runsProcessed
        runsProcessed = tokProc;
        tokProc.clear();

    }

    // close the old file stream
	input_file.close();

	// clear fromFile, and save contents of tempFile in it
	ofstream outFile(fromFile, ios::trunc); // truncate file length to 0, effectively clearing it.
	outFile.close();

	// copy contents of tempFile into fromFile
	bool isDiskContentsCopied = copyFileContents(tempFile,fromFile);
	if (!isDiskContentsCopied) {
		cout << "Error: Could not copy the file " << tempFile << " into " << fromFile << endl;
        exit(1);
	}

	// clear tempFile
	ofstream outFile2(tempFile, ios::trunc); // truncate file length to 0, effectively clearing it.
	outFile2.close();
}

void SortIterator::diskSort(){
    cout<<"Inside disk sort\n";
    firstPass(false);
    cout<<"MERGEsORT disk sort\n";
    mergeSort(false);
}

void SortIterator::ramSort(){
    cout<<"Inside RAM sort\n";
    firstPass(true);
    cout<<"MERGEsORT disk sort\n";
    mergeSort(true);
}
