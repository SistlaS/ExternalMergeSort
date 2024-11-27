#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Witness.h"
#include "Record.h"

int main (int argc, char * argv [])
{
	TRACE (true);

	Plan * const plan =
			new WitnessPlan ("output",
				new SortPlan ("*** The main thing! ***",
					new WitnessPlan ("input",
						new FilterPlan ("half",
							new ScanPlan ("source", 10)
						)
					)
				)
			);

	// Plan * const plan =
	// 		new SortPlan ("*** The main thing! ***",
	// 				new WitnessPlan ("input",
	// 					new FilterPlan ("half",
	// 						new ScanPlan ("source", 5)
	// 					)
	// 				)
	// 			);

	// Plan * const plan =
	// 		new WitnessPlan ("input",
	// 			new FilterPlan ("half",
	// 				new ScanPlan ("source", 5)
	// 			)
	// 		);

	Iterator * const it = plan->init ();
	it->run ();
	delete it;

	delete plan;

	return 0;
} // main
