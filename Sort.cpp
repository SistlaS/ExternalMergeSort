#include "Sort.h"
#include "SystemConfigurations.h"
#include <fstream>
#include <iostream>
#include <queue>
#include "Tree.h"

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

bool isFileEmpty(string filename){
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    if (file.peek() == std::ifstream::traits_type::eof()) {
        std::cout << "Filename: "<< filename <<" is empty." << std::endl;
        file.close();
        return 0;
        
    } else {
        std::cout << "File is not empty." << std::endl;
        file.close();
        return 1;   
    }
    
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

bool SortIterator::copyFileContents(string sourceFileName, string destinationFileName, int mode){
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

    cout << "File contents appended from " << sourceFileName << " to " << destinationFileName << endl;

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
    uint F = isRAM ? SystemConfigurations::cache_size : SystemConfigurations::ram_size;
     
    vector<int> gd = computeGracefulDegradationFactors(W,F);
    cout<<"Graceful degradation: "<<endl;
    for(int i=0;i<gd.size();i++){cout<<gd[i]<<" ";}
    cout<<endl;

    if(isRAM){
         SortIterator::ram_gdFactors = gd;
    } else {
         SortIterator::disk_gdFactors=  gd;
    }

    std::ifstream input_file(fromFile,ios::in);
    if (!input_file or !input_file.is_open())
    {
        cout << "Could not open file: " << fromFile << "...Exit from program" << endl;
        exit(1);
    }

    // buffer contains the chunk of records
	string buffer;
	string record;
    // number of tokens encountered
	int recordCount = 0;

    // input for TT
    vector<queue<string>>v;

	for(int i=0;i<gd.size();i++){
        cout<<"Value of i: "<<i<<" and gd[i] value: "<< gd[i]<<endl;
        // Step 1: populate the buffer with gd[i] tokens
        if(isRAM){
            cout<< "Number of records in this run:" << gd[i]<<endl;
        }else cout<< "Number of sorted runs in this run:" << gd[i]<<endl;

		while(recordCount < gd[i] && !input_file.eof()){
            
            if(getline(input_file, record, '|')){
                buffer += record += "|";
                recordCount++;
            }
		}
        cout << "Buffer: " << buffer << endl;

        // Step 2: Write buffer to toFile
        bool isWriteSuccess = writeDataIntoFile(toFile, buffer, 1, "");
		if (!isWriteSuccess) {
			cout << "Error: Could not write into file: " << toFile << endl;
			exit(1);
		}
        cout<< "Wrote buffer to file " << toFile <<" successfully."<<endl;
        print_file_contents(toFile);

        // Step 3: Clear values
        buffer.clear();
        recordCount=0;

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

            record = "";
            v.clear();
            int numQ = 0;
            
            for(int j=0;j<gd[i];j++){
                queue<string> q;
                // queue of size 1
                if(getline(ip, record, '|')){
                    cout<<"Token in queue: "<< record <<"\n";
                    if(record!=""){   
                        q.push(record);
                    }
                }
                v.push_back(q);
                numQ++;
                if(ip.eof()) break;
            }

            ip.close();

            // Clear toFile
            ofstream outFile("Cache.txt", ios::trunc);
            outFile.close();

            // Output: Sorted Cache.txt
            Tree tree(numQ,v, toFile);
            
            cout<<"Cache is sorted: \n";
            print_file_contents(toFile);
        }else{
            SortIterator::numRAMRecords=gd[i];
            cout<<"Sort RAM.txt with gdFactor "<<numRAMRecords<<endl;
            ramSort();
        }

        // Step 5: Append sorted data into temp file
        bool isSortWriteSuccess = copyFileContents(toFile, tempFile,1);
		if (!isSortWriteSuccess) {
			cout << "Error: Could not copy the file " << toFile << " into " << tempFile << endl;
			exit(1);
		}

        // Step 6: Clear toFile
        ofstream outFile5(toFile, ios::trunc); // truncate file length to 0, effectively clearing it.
        outFile5.close();
   
        cout<<"Successfully cleared toFile: "<<toFile<<endl;
    }

    cout<<"File: "<<tempFile<<" is first pass sorted\n";
    // close the old file stream
	input_file.close();

	// clear fromFile, and save contents of tempFile in it
	ofstream outFile3(fromFile, ios::trunc); // truncate file length to 0, effectively clearing it.
    outFile3.close();
    cout<<"Cleared fromFile: "<<fromFile<<endl;

	// overwrite contents of tempFile into fromFile
	bool isDiskContentsCopied = copyFileContents(tempFile,fromFile,0);
	if (!isDiskContentsCopied) {
		cout << "Error: Could not copy the file " << tempFile << " into " << fromFile << endl;
        exit(1);
	}
    cout<<"Overwritten contents of fromFile: "<<fromFile<<endl;
    print_file_contents(fromFile);

	// clear tempFile
	ofstream outFile2(tempFile, ios::trunc); // truncate file length to 0, effectively clearing it.
    outFile2.close();
}

// perform "external merge sorting" - merge sorted runs till only one run remains
void SortIterator::mergeSort(bool isRAM){
    string fromFile = isRAM ? "RAM.txt" : "Disk.txt";
    string toFile = isRAM ? "Cache.txt" : "RAM.txt";
    string tempFile = isRAM ? "RAM2.txt" : "Disk2.txt";

    cout<<"In merge sort; isRAM: "<<isRAM<<"\n";

    uint F = SystemConfigurations::ram_size;    

    // sizes of runs which have already been sorted
    vector<int>runsProcessed = isRAM? SortIterator::ram_gdFactors: SortIterator::disk_gdFactors;
    cout<<"runs processed in first pass: "<<endl;
    for(int i=0;i<runsProcessed.size();i++){cout<<runsProcessed[i]<<" ";}
    cout<<endl;
    
    while(runsProcessed.size()>1){

        cout<<"In merge sort; # of sorted runs "<<runsProcessed.size()<<"\n";

        std::ifstream input_file(fromFile,ios::in);
        if (!input_file or !input_file.is_open())
        {
            cout << "Could not open file: " << fromFile << "...Exit from program" << endl;
            exit(1);
        }

        vector<int> gd2 = computeGracefulDegradationFactors(runsProcessed.size(),F);
        cout<<"Graceful degradation: "<<endl;
        for(int i=0;i<gd2.size();i++){cout<<gd2[i]<<" ";}
        cout<<endl;

        int numRuns = 0;
        string bunchOfRuns;

        for(int i=0;i<gd2.size();i++){

            vector<queue<string>>v;
            int numQ = 0;
            while(numRuns < gd2[i] && !input_file.eof()){
                queue<string> q;
                // '\n' delimits sorted runs
                // '|' delimits records
                // ',' delimits columns
                while(getline(input_file, bunchOfRuns, '\n')){
                    // we have a bunch of records (which are sorted wrt each other)
                    // put this bunch in a queue
                    cout<<"Bunch of runs: "<<bunchOfRuns<<endl;
                    std::stringstream ss(bunchOfRuns);
                    string record;
                    while (getline(ss, record, '|')) {
                        q.push(record);
                        cout<<record<<" ";
                        numRuns++;
                    }
		    v.push_back(q); 
	            numQ++; 
                    cout<<endl;  
                }
                          
            }

            // Clear toFile
            ofstream outFile4(toFile, ios::trunc);
            outFile4.close();
            
            // Output: Sorted toFile
            Tree tree(numQ,v,toFile);

            cout<<"Sorted toFile: "<<toFile<<endl;
            print_file_contents(toFile);

            // Step 5: Write sorted data into temp file
            bool isSortWriteSuccess = copyFileContents(toFile, tempFile,1);
            if (!isSortWriteSuccess) {
                cout << "Error: Could not copy the file " << toFile << " into " << tempFile << endl;
                exit(1);
            }

            // Step 6: Clear toFile
            ofstream outFile(toFile, ios::trunc); // truncate file length to 0, effectively clearing it.
            outFile.close();
            cout<<"I am here\n";
        }

        runsProcessed = gd2;

        // close the old file stream
        input_file.close();

        // clear fromFile, and save contents of tempFile in it
        ofstream outFile(fromFile, ios::trunc); // truncate file length to 0, effectively clearing it.
        outFile.close();

        // copy contents of tempFile into fromFile
        bool isDiskContentsCopied = copyFileContents(tempFile,fromFile,0);
        cout<<"REACHER DFVIDFNVDFV\n";
        if (!isDiskContentsCopied) {
            cout << "Error: Could not copy the file " << tempFile << " into " << fromFile << endl;
            exit(1);
        }

        ofstream outFile2(tempFile, ios::trunc); // truncate file length to 0, effectively clearing it.
        outFile2.close();
        cout<<"DONE!\n";
    }
}

void SortIterator::diskSort(){
    cout<<"Inside disk sort\n";
    firstPass(false);
    cout<<"MERGESORT DISK sort\n";
    mergeSort(false);

    // cleanup
    ofstream outFile2("RAM.txt", ios::trunc); // truncate file length to 0, effectively clearing it.
    outFile2.close();
    cout<<"Cleared RAM.txt!\n";

        ofstream outFile4("RAM2.txt", ios::trunc); // truncate file length to 0, effectively clearing it.
    outFile4.close();
    cout<<"Cleared RAM2.txt!\n";

        ofstream outFile3("Disk2.txt", ios::trunc); // truncate file length to 0, effectively clearing it.
    outFile3.close();
    cout<<"Cleared Disk2.txt!\n";

        ofstream outFile5("Cache.txt", ios::trunc); // truncate file length to 0, effectively clearing it.
    outFile5.close();
    cout<<"Cleared Cache.txt!\n";

}

void SortIterator::ramSort(){
    cout<<"Inside RAM sort\n";
    firstPass(true);
    cout<<"MERGESORT RAM sort\n";
    mergeSort(true);
}
