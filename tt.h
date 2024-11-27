#pragma once

#include "defs.h"
#include "Record.h"
#include <vector>

class Record;

struct Node{
    Record *curr_rec;
    llint level;
    bool is_empty;
    bool is_leaf;
    int run_id;

    Node(){
        // rec = NULL;
        level = 0;
        is_empty = true;
        is_leaf = false;
    }
};

class Tree{
    public:
        struct Node *heap;
        std::vector<Record *> generated_run;
        llint tot_leaves, tot_nodes, height;
        lluint tot_record_count;

        Tree();
        Tree(llint k);
        int left(int i);
		int right(int i);
        int getParentIndex(int i);
		// Deconstructor
		~Tree();

};
