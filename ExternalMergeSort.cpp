#include "Filter.h"
#include "Iterator.h"
#include "SystemConfigurations.h"
#include "Scan.h"
#include "Sort.h"
#include "defs.h"
#include <math.h>
#include <cstring>
#include <iostream>
#include <string>
#include <exception>

using namespace std;

/*

   -> User may provide two arguments: number of records(to be sorted) [with -n option] and maximum
   record value [with -m option]

   -> Records generated may or may not be duplicated

   -> Run instructions: ./ExternalMergeSort -n 100 -m 100000

   -> If user does not provide any input, we assume default values for both arguments

*/
void read_user_inputs(int argc, char *argv[]) {

    int num_records = SystemConfigurations::default_num_records;
    int max_column_value = SystemConfigurations::default_max_value_in_column;

    cout << "Program usage:\nUser may provide two arguments: number of records(to be sorted) [with -n option] and maximum column value[with - max option]\n";
    cout << "Program Behaviours:\n1.If you have not specified either or any of the required arguments, we assume default values of:\n \tNumber of records: " << num_records << " and\n\tMaximum column value: " << max_column_value << endl;
    cout << "2.Any errenous or unexpected argument is ignored and default value is considered.\n";
    cout << "3.The maximum and minimum values must be within the range of an int variable.\n";

    if(argc < 2){
        cout<< "You have not specified any arguments - hence we take default values of:\n \tNumber of records: " << num_records << " and \n\tMaximum record value: " << max_column_value << endl;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 < argc) {
                try {
                    num_records = stoi(argv[++i]);
                    if(num_records <= 0){
                        num_records = SystemConfigurations::default_num_records;
                    }
                } catch (const exception& e) {
                    cout << "Error parsing '-n' value. Using default: " << num_records << endl;
                }
                SystemConfigurations::num_records = num_records;
            }
        } else if (strcmp(argv[i], "-max") == 0) {
            if (i + 1 < argc) {
                try {
                    max_column_value = stoi(argv[++i]);
                } catch (const exception& e) {
                    cout << "Error parsing '-max' value. Using default: " << max_column_value << endl;
                }                
                SystemConfigurations::max_value_in_column = max_column_value;
            }
        }
    }
}

void run_program(){
    // generate data
    Plan *scanPlan = new ScanPlan("Disk.txt", SystemConfigurations::num_records);

    // SortIterator::diskSort(false);

    // Do the sort
    Plan *const plan = new SortPlan("Disk.txt",scanPlan);
    Iterator *const itr = plan->init();
    // itr->run();

    // cleanup
    cout<<"\n***Cleanup start****"<<endl;
    // delete itr;
    // delete plan;
    cout<<"***Cleanup complete****"<<endl;
}

int main(int argc, char *argv[]) {

    cout << "\n\n********WELCOME TO THE EXTERNAL MERGE SORT PROGRAM***********\n\n";

    // Read command line arguments + Input validation
    read_user_inputs(argc, argv);

    // initializing system to default configs 
    print_configurations();

    run_program();

    //TODO: Witness, etc.
}
