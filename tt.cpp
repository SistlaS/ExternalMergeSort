#include "tt.h"

void Node::printNode() {
    cout << "Index: " << Index 
         << ", Data: "<<Data<<" ovc: " << ovc << endl;
}

bool Node::is_greater(Node incoming){
	if (Data > incoming.getData()){
		return true;
	}
	return false;
}

//capacity here refers to the number of records the tree the can handle
//for internal sort this is assumed to be equal to the leaf size for simplicity
//for merging no.of leaves = capacity/2





Tree::Tree(int k){
	this->capacity = k%2 == 0 ? 2*k : 2*(k + 1);
	this->heap = std::vector<Node>(this->capacity);
	this->leaf_nodes = k;
}

void Tree::print_tree() {
        cout << "Leaf nodes: " << leaf_nodes << ", Total nodes: " << capacity << endl;
        int i = 0;
        for (Node node : heap) {
            cout << "Slot: " << i << ", ";
            node.printNode();
            ++i;
        }
    }

int Tree::parent_index(int index){
	return index/2;
}

int Tree::leftChild_index(int index){
	return 2*index;
}

int Tree::rightChild_index(int index){
	return 2*index+1;
}


void Tree::construct_tree(vector<int> unsorted_input){
	//initialise the leaf nodes
	int input_size = unsorted_input.size();

	if (input_size > leaf_nodes){
		cout<<"number of records to be sorted is larger than the number of nodes"<<endl;
		return;
	}
	// Populate the leaf nodes
    int index = leaf_nodes;
    for (int i = 0; i < input_size; ++i, ++index) {
        Node temp;
        temp.setData(unsorted_input[i]);
        temp.setIndex(index);
        temp.setOvc("-1");
        heap[index] = temp;
    }
	//initialise the internal nodes recursively by computing the winner and loser

	for (int i = leaf_nodes; i < capacity; i++){
		Node current = heap[i];
		int parent_indx = parent_index(i);

		while(parent_indx >=0){
			// cout<<" i : "<<i<<" parent_indx : "<<parent_indx<<" current : "<<current.Data<< " parent_data : "<<heap[parent_indx].Data<<endl;
			if (heap[parent_indx].getData() == INT_MIN){
				heap[parent_indx] = current;
				break;
			}else if (current.is_greater(heap[parent_indx])){
				swap(heap[parent_indx], current);
			}
			if (parent_indx == 0) { break; }
			parent_indx = parent_index(parent_indx);
		}

	}
}

bool Tree::is_empty(){
	if(heap[0].getData() == INT_MAX){
		return true;
	}
	return false;
}
int Tree::pop_winner() {
    // Save the winner (root of the tree)
    Node winner = heap[0];
    int min_data = winner.getData();
    int winner_index = winner.getIndex();

    // Replace the leaf node corresponding to the winner with a new record
    // For an internal sort, insert INT_MAX; otherwise, use the next record
    Node new_rec = Node(INT_MAX); // Replace with next record logic if applicable
    heap[winner_index] = new_rec;

    // Start the propagation from the winner's index
    Node current = new_rec;
    int current_index = winner_index;

    // Propagate the new value up the tree
    int parent_indx = parent_index(current_index);
    while (parent_indx >= 0) {
        // Determine the new winner and loser at the parent
        if (heap[parent_indx].getData() == INT_MIN) {
            // Parent is uninitialized, take the current as winner
            heap[parent_indx] = current;
            break;
        }

        if (current.is_greater(heap[parent_indx])) {
            // Current becomes the new loser, propagate the winner
            swap(current, heap[parent_indx]);
        }
		if (parent_indx == 0) { break; }
		parent_indx = parent_index(parent_indx);
	}
    return min_data;
}



Tree::~Tree() {}
int main(int argc, char const *argv[])
{
	int n = 8;
    Tree tree(n);

    vector<int> test = {3,5,0,1,8,7,77,INT_MAX};
    
    tree.construct_tree(test);
    tree.print_tree();
    while(!tree.is_empty()){
    	cout<<"Popping min : "<<tree.pop_winner()<<endl;
    }
    
    // cout<<"Popping min : "<<tree.pop_winner()<<endl;
    // tree.print_tree();
    // cout<<"Popping min : "<<tree.pop_winner()<<endl;
    tree.print_tree();

	return 0;
}
