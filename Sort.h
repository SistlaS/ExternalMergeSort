#pragma once
#include <vector>
#include "defs.h"
#include "Iterator.h"
#include <string>

using namespace std;

class SortPlan : public Plan {
    friend class SortIterator;

  public:
    SortPlan(char const *const name, Plan *const input);
    ~SortPlan();
    Iterator *init() const;

  private:
    Plan *const _input;
}; // class SortPlan

class SortIterator : public Iterator {
  public:
    SortIterator(SortPlan const *const plan);
    ~SortIterator();
    bool next(Row &row);
    void free(Row &row);

    // helper functions
    vector<int> computeGracefulDegradationFactors(int W, int F);

    // file operations
    bool writeDataIntoFile(string fileName, string data, int mode, string delimiter);
    bool copyFileContents(string sourceFileName, string destinationFileName);

    void firstPass(bool isRAM);
    void mergeSort(bool isRAM);

    void cacheSort();
    void ramSort();
    void diskSort();

  private:
    SortPlan const *const _plan;
    Iterator *const _input;
    RowCount _consumed, _produced;

    vector<int> gdFactors; // first pass chunk sizes
    int numRAMRecords;

    
    // ByteCount diskCapacity;
    // // PageCount diskPageSize;
    // ByteCount ramCapacity;
    // PageCount ramPageSize;

}; // class SortIterator
