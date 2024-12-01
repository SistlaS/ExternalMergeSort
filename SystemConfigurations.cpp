#include<iostream>
#include "SystemConfigurations.h"

using namespace std;

RowCount SystemConfigurations::default_num_records = 100;
RowCount SystemConfigurations::default_max_value_in_column = 10000;

RowCount SystemConfigurations::num_records;
RowSize SystemConfigurations::max_value_in_column;

/* **************************Misc Info*************************** */
int SystemConfigurations::column_count = 4;
int SystemConfigurations::column_value_size = sizeof(int); // 4 bytes
int SystemConfigurations::record_size = column_count * column_value_size; // 4*4=16 bytes


/* *************************Sizes(in # of records)******************************** */
ByteCount SystemConfigurations::page_size = 4;
ByteCount SystemConfigurations::cache_size = 3;      // 50 KB Cache
ByteCount SystemConfigurations::ram_size = 5;     // 5 MB RAM
// ByteCount SystemConfigurations::disk_size = 100*1024*1024;   // Disk size is INF

/* *************************Latency&Bandwidth (in bytes & ms)******************** */

// double SystemConfigurations::ram_latency = 0.01;       // 0.01 milliseconds
// double SystemConfigurations::disk_latency = 0.1;       // 0.1  milliseconds
// int SystemConfigurations::ram_bandwidth = 0.2 * 1024 * 1024; // 0.2 MB/millisecond 
// int SystemConfigurations::disk_bandwidth = 0.1 * 1024 * 1024; // 0.1 MB/millisecond 

void print_configurations(){
            cout<< "\n\n**Here are the system configurations for the external merge sort algorithm:**\n\n";

            cout<<"A. Memory Hierarchy Sizes(in # of records) -:\n";
            cout<<"\t- Cache Size: " << SystemConfigurations::cache_size << endl;
            cout<<"\t- RAM Size: " << SystemConfigurations::ram_size << endl;
            cout<<"\t- Disk Size: INF" << endl;

            // cout<<"B. Latency(in ms) & Bandwidth (in B/ms) Data -:\n";
            // cout<<"\t- Cache Latency: 0" << endl;
            // cout<<"\t- Cache Bandwidth: INF" << endl;
            // cout<<"\t- RAM Latency: " << SystemConfigurations::ram_latency << endl;
            // cout<<"\t- RAM Bandwidth: " << SystemConfigurations::ram_bandwidth << endl;
            // cout<<"\t- Disk Latency: " << SystemConfigurations::disk_latency << endl;
            // cout<<"\t- Disk Bandwidth: " << SystemConfigurations::disk_bandwidth << endl;

            cout<<"B. Database data -:\n";
            cout<<"\t- Number of records(ie, number of rows): " << SystemConfigurations::num_records << endl;
            cout<<"\t- Minimum value in a column: 0" << endl;
            cout<<"\t- Maximum value in a column: " << SystemConfigurations::max_value_in_column << endl;
            cout<<"\t- Number of columns in a record: " << SystemConfigurations::column_count << endl;
            cout<<"\t- Size of a record (in bytes): " << SystemConfigurations::record_size << endl;
            cout<<"\t- Number of records in a page (in bytes): " << SystemConfigurations::record_size << endl;
};

