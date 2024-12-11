#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Witness.h"
#include <cstring>

int main (int argc, char * argv [])
{
	int num_records = 10000;

    cout << "Program usage:\nUser may provide one arguments: number of records(to be sorted) [with -n option]\n";
    cout << "Program Behaviours:\n1.If you have not specified either or any of the required arguments, we assume default values of:\tNumber of records: " << num_records << endl;
    cout << "2.The number of records must be between 1 and 10^8.\n";

    if(argc < 2){
        cout<< "You have not specified any arguments - hence we take default values of:\n \tNumber of records: " << num_records << endl;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 < argc) {
                try {
                    num_records = stoi(argv[++i]);
                    if(num_records < 0){
                        cout<<"We cannot have negative number of records in the database ----- Exiting the program\n";
                    } else if(num_records == 0){
                        cout<<"Number of records entered = 0.\n There are no records in the database ----- Exiting the program\n";
                    } else{
                        cout<<"*********Number of records: "<<num_records<<endl;
                    }
                } catch (const exception& e) {
                    num_records = 10000;
                    cout << "Error parsing '-n' value. Using default: " << num_records << endl;
                }
            }
        }
    }

	Plan * const plan =
			new WitnessPlan ("output",
				new SortPlan ("*** The main thing! ***",
					new WitnessPlan ("input",
						new FilterPlan ("half",
							new ScanPlan ("source", num_records)
						)
					)
				)
			); 

	Iterator * const it = plan->init ();
	it->run ();
	delete it;

	delete plan;

	return 0;
} // main
