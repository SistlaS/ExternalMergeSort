#pragma once
#include<iostream>
#include "Iterator.h"

using namespace std;

// Note: 1 Record = 1 Row in the database
// Record will contain `column_count` number of columns
class SystemConfigurations{
    public:
        static RowCount default_num_records;
        static RowCount default_max_value_in_column;

        static RowCount num_records;
        static RowSize max_value_in_column;

        /* **************************Misc Info*************************** */
        static int column_count;
        static int column_value_size;
        static int record_size;


        /* *************************Sizes(in # of records)******************************** */
        static ByteCount page_size;
        static ByteCount cache_size; 
        static ByteCount ram_size;   
        // static ByteCount disk_size;  // INF
        // static ByteCount disk_page_size;
        // static ByteCount ram_page_size;
        
        /* *************************Latency&Bandwidth******************** */

        // static double ram_latency;       // 0.01 milliseconds
        // static double disk_latency;       // 0.1  milliseconds
        // static int ram_bandwidth; // 0.2 MB/millisecond 
        // static int disk_bandwidth; // 0.1 MB/millisecond 
};

void print_configurations();

