#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Witness.h"
#include <cstring>

int main (int argc, char * argv [])
{
	int num_records = 10000;
    int sortOrder = 0;

    cout << "Program usage:\nUser may provide two arguments: number of records(to be sorted) [with -n option] and input sort order [with -i option]\n";
    cout << "Program Behaviours:\n1.If you have not specified either or any of the required arguments, we assume default values of:\tNumber of records: " << num_records << " and input sort order: unsorted input"<<endl;
    cout << "2.The number of records must be greater than or equal to 0.\n";
    cout << "3.Sort order must be -1, 0, or 1 for descending sorted input, unsorted input, and ascending sorted input\n";

    if(argc < 2){
        cout<< "You have not specified any arguments - hence we take default values of: \tNumber of records: " << num_records << " and input sort order: unsorted input"<<endl;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 < argc) {
                try {
                    num_records = stoi(argv[++i]);
                    if(num_records < 0){
                        cout<<"We cannot have negative number of records in the database ----- Exiting the program\n";
                        exit(1);
                    } else if(num_records == 0){
                        cout<<"Number of records entered = 0.\n There are no records in the database ----- Exiting the program\n";
                        exit(0);
                    } else{
                        cout<<"\n*********Number of records: "<<num_records<<"***********"<<endl;
                    }
                } catch (const exception& e) {
                    num_records = 10000;
                    cout << "Error parsing '-n' value. Using default: " << num_records << endl;
                }
            } 
        } else if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 < argc) {
                try {
                    int order = stoi(argv[++i]);
                    if(order == -1){
                        cout<<"Input will be sorted in descending sorted order\n";
                        sortOrder = order;
                    } else if(order == 0){
                        cout<<"Input will be unsorted\n";
                        sortOrder = order;
                    } else if(order==1){
                        cout<<"Input will be sorted in ascending sorted order\n";
                        sortOrder = order;
                    }
                } catch (const exception& e) {
                    num_records = 10000;
                    cout << "Error parsing '-i' value. Using default: " << endl;
                }
            }  
        }
    }

	Plan * const plan =
			new WitnessPlan ("output",
				new SortPlan ("*** The main thing! ***",
					new WitnessPlan ("input",
						new FilterPlan ("half",
							new ScanPlan ("source", num_records,sortOrder)
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
