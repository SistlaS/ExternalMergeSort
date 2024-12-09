#pragma once

#include <vector>
#include <iostream>
#include <limits.h>
#include <string>
#include <sstream>
#include <fstream>
#include <queue>
#include "defs.h"


typedef unsigned int uint;
using namespace std;
#define DEBUG false;


class Node{
	private:
		uint Index;
		vector<int> Data;
		int ovc;
	public:
	    Node() : Data({INT_MIN}), Index(-1), ovc(-1) {}

	    Node(vector <int> x) : Data(x), Index(-1), ovc(-1) {}
		Node(vector <int> x, uint index) : Data(x), Index(index), ovc(-1) {}

	    // Node(int data, int index = -1, int ovc = -1) 
	    //     : Data(data), Index(index), ovc(ovc) {}


	    bool greater(Node& other, bool const full, vector<Node>& heap);

	    bool is_greater(Node incoming);

	    uint getIndex() const { return Index; }
	    void setIndex(uint index) { Index = index; }

	    vector <int> getData() const { return Data; }
	    int getData(int i) const { return Data[i]; }
	    void setData(const vector<int> data) { Data = data; }
	    string getDataStr();

	    int getOvc() const { return ovc; }
	    void setOvc(int ovc_value) { ovc = ovc_value; }

	    void printNode();
};

class Tree{
	public:
		uint capacity;
		uint leaf_nodes;
		vector<Node> heap;
		//change this to pointer
		vector<queue<string>> input;
		vector<string> opBuffer;
		string opFilename;
		bool isCache;
		bool isRam;

		Tree(uint capacity, string opFilename);

		void construct_tree();
		Node pop_winner();
		void generate_runs(vector<queue<string>> input, bool isDisk);
		void flush_to_op(bool eof, bool isDisk);

		//utility funcs
		bool is_empty();
		uint parent_index(uint);
		uint leftChild_index(uint);
		uint rightChild_index(uint);
		void print_tree();
		void clear_heap();


		~Tree();

};