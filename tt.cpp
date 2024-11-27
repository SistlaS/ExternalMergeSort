#include "tt.h"

// Tree for sorting records i.e internal sort
Tree::Tree() {}

Tree::Tree(llint k, ){
    this->heap = new Node[2*k];
    this->tot_leaves = k;
    this->tot_nodes = 2*k;




}


Tree::Tree(Record *records, llint record_count){
    this->tot_nodes = record_count;
    this->tot_leaves = record_count/2;
    Record *curr_ptr = records;




}

int Tree::left(int i){
    return (2*i)
}
int Tree::right(int i){
    return (2*i+1)
}

int Tree::getParentIndex(int i)
{
    return floor(i / 2);
}

llint Tree::capacity(llint level)
{
	return (1 << level);
}