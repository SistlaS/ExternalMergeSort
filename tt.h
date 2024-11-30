#include <vector>
#include <iostream>
#include <limits.h>
#include <string>

using namespace std;
#define DEBUG false;


class Node{
	private:
		int Index;
		int Data;
		// vector<int> Data;
		string ovc;
	public:
	    Node() : Data(INT_MIN), Index(-1), ovc("-1") {}

	    Node(int x) : Data(x), Index(-1), ovc("-1") {}

	    // Node(int data, int index = -1, int ovc = -1) 
	    //     : Data(data), Index(index), ovc(ovc) {}

	    bool is_greater(Node incoming);

	    int getIndex() const { return Index; }
	    void setIndex(int index) { Index = index; }

	    int getData() const { return Data; }
	    void setData(const int data) { Data = data; }

	    string getOvc() const { return ovc; }
	    void setOvc(string ovc_value) { ovc = ovc_value; }

	    void printNode();
};

class Tree{
	public:
		int capacity;
		int leaf_nodes;
		vector<Node> heap;

		Tree(int capacity);

		void construct_tree(vector<int> unsorted_input);
		int pop_winner();

		//utility funcs
		bool is_empty();
		int parent_index(int);
		int leftChild_index(int);
		int rightChild_index(int);
		void print_tree();

		~Tree();

};
