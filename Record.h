#pragma once
#define COLUMN_COUNT 4
#define DOMAIN_OF_VALUES 9
#include "defs.h"
#include <string>
using namespace std;
// class Record
// {
//     public:
//         Record();
        
//         ~Record();
// };

class Row
{
    public:
        std::vector<std::string> data;
        int offsetValueCode;
        Row ();
        virtual ~Row ();
        bool isFiltered() const {
            for (string value : data) {
                if (stoi(value) < 2) return false;
            }
            return true;  
        }
        // ...
    private:
        // ...
};

typedef struct RecordList
{
	std::vector<Row> record_ptr;
	lluint record_count = 0;
} RecordList;

// int comparator(const void *arg1, const void *arg2);
// void InternalSort(RecordList *records);