#include "Tree.h"

int ROW_SIZE = 3;
int BUFFER_SIZE = 16;


string Node::getDataStr(){
	for (int num : Data) {
        if (num == INT_MAX) {
            return "";
        }
    }
    // Convert the vector to a comma-separated string
    std::ostringstream oss;
    for (size_t i = 0; i < Data.size(); ++i) {
        oss << Data[i];
        if (i + 1 < Data.size()) {
            oss << ", ";
        }
    }
    return oss.str();
}

void Node::printNode() {
        std::cout << "Node(Index: " << Index
                  << ", Data: [";
        for (size_t i = 0; i < Data.size(); ++i) {
            std::cout << Data[i] << (i + 1 < Data.size() ? ", " : "");
        }
        std::cout << "], OVC: " << ovc << ")" << std::endl;
    }

bool Node::is_greater(Node incoming){
	vector<int> incoming_data = incoming.getData();
	for (int i = 0; i<ROW_SIZE; i++){
		if (Data[i] > incoming_data[i]){
			return true;
		}else if (Data[i] < incoming_data[i]){
			return false;
		}
	}
	
	return false;
}

bool greater(int& offset, const std::vector<Node>& nodes) {
    while (++offset < ROW_SIZE) {
        if (nodes[0].getData(offset) != nodes[1].getData(offset)) {
            return nodes[0].getData(offset) > nodes[1].getData(offset);
        }
    }
    return false;
}

bool Node::greater(Node& other, bool full_, vector<Node>& heap){
	int offset;
    if (full_) {
        offset = -1;  // Start offset for full comparison
    } else if (ovc != other.ovc) {
    	cout<<"Direct comparison using ovc"<<ovc<<" -- "<<other.ovc<<endl;
        return ovc > other.ovc;  // Compare OVC directly if they differ
    } else {
        offset = ovc;  // Start from the current offset-value coding
    }

    bool const isGreater = ::greater(offset, { *this, other });

    Node& loser = (isGreater ? *this : other);
    // cout<<"Setting ovc for "<<loser.getIndex()<<","<<offset<<endl;
    if (loser.getData()[0] != INT_MAX){
	    loser.setOvc(offset);
    	heap[loser.getIndex()].setOvc(offset);
    }

    //set ovc in the leaf node too ?
    return isGreater;
}

//capacity here refers to the number of records the tree the can handle
//for internal sort this is assumed to be equal to the leaf size for simplicity
//for merging no.of leaves = capacity/2

Tree::Tree(uint k) {
    // Calculate the next power of 2 for capacity
    uint nextPowerOf2 = 1;
    while (nextPowerOf2 < 2 * k) {
        nextPowerOf2 *= 2;
    }

    this->capacity = nextPowerOf2;
    this->heap = std::vector<Node>(this->capacity);
    this->leaf_nodes = this->capacity/2;
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

uint Tree::parent_index(uint index){
	return index/2;
}

uint Tree::leftChild_index(uint index){
	return 2*index;
}

uint Tree::rightChild_index(uint index){
	return 2*index+1;
}


void Tree::construct_tree(vector<vector<int>> unsorted_input){
	//initialise the leaf nodes
	int input_size = unsorted_input.size();

	if (input_size > leaf_nodes){
		cout<<"number of records to be sorted is larger than the number of nodes"<<endl;
		return;
	}
	if (input_size < leaf_nodes){
		int diff = leaf_nodes - input_size;
		for(int i = 0;i <diff;i ++){
			unsorted_input.push_back({INT_MAX});
		}
	}
	// Populate the leaf nodes
    int index = leaf_nodes;
    for (int i = 0; i < leaf_nodes; ++i, ++index) {
        Node temp;
        temp.setData(unsorted_input[i]);
        temp.setIndex(index);
        temp.setOvc(-1);
        heap[index] = temp;
    }
	//initialise the internal nodes recursively by computing the winner and loser

	for (int i = leaf_nodes; i < capacity; i++){
		Node current = heap[i];
		uint parent_indx = parent_index(i);

		while(parent_indx >=0){
			// cout<<" i : "<<i<<" parent_indx : "<<parent_indx<<" current : "<<current.Data<< " parent_data : "<<heap[parent_indx].Data<<endl;
			if (heap[parent_indx].getData()[0] == INT_MIN){
				heap[parent_indx] = current;
				break;
			}else if (current.greater(heap[parent_indx], false, heap)){
				swap(heap[parent_indx], current);
			}
			if (parent_indx == 0) { break; }
			parent_indx = parent_index(parent_indx);
		}
	}
}

bool Tree::is_empty(){
	if(heap[0].getData()[0] == INT_MAX){
		return true;
	}
	return false;
}
Node Tree::pop_winner() {
    // Save the winner (root of the tree)
    Node winner = heap[0];
    vector<int> min_data = winner.getData();
    uint winner_index = winner.getIndex();

    // Replace the leaf node corresponding to the winner with a new record
    // For an internal sort, insert INT_MAX; otherwise, use the next record
    Node new_rec = Node({INT_MAX}); // Replace with next record logic if applicable
    heap[winner_index] = new_rec;

    // Start the propagation from the winner's index
    Node current = new_rec;
    uint current_index = winner_index;

    // Propagate the new value up the tree
    uint parent_indx = parent_index(current_index);
    while (parent_indx >= 0) {
        // Determine the new winner and loser at the parent
        if (heap[parent_indx].getData()[0] == INT_MIN) {
            // Parent is uninitialized, take the current as winner
            heap[parent_indx] = current;
            break;
        }
        cout<<"parent ind : "<<parent_indx<<" ; "<<"current ind : "<<current.getIndex()<<" ;=="<<current.is_greater(heap[parent_indx])<<" -- "<<current.greater(heap[parent_indx], false, heap)<<endl;
        // if (current.greater(heap[parent_indx], false, heap)) {
        //     // Current becomes the new loser, propagate the winner
        //     swap(current, heap[parent_indx]);
        // }
        if (current.is_greater(heap[parent_indx])) {
            // Current becomes the new loser, propagate the winner
            swap(current, heap[parent_indx]);
        }
		if (parent_indx == 0) { break; }
		parent_indx = parent_index(parent_indx);
	}
    return winner;
}

void Tree::flush_to_op(){
	//flush the buffer to op file
	ofstream outFile(opFilename);
    if (!outFile) {
        std::cerr << "Error: Could not open the file for writing!" << std::endl;
        return;
    }

    for (const string& str : opBuffer) {
    	cout<<str<<endl;
        outFile << str << std::endl;
    }
    outFile.close();

    if (outFile.good()) {
        std::cout << "File written successfully to " << opFilename << std::endl;
    } else {
        std::cerr << "Error occurred while writing to the file." << std::endl;
    }
    //clear the buffer
    opBuffer.clear();
}

void Tree::generate_runs(){
	while(!is_empty()){
    	Node temp = pop_winner();
    	opBuffer.push_back(temp.getDataStr());
    	if(opBuffer.size()==BUFFER_SIZE){
    		flush_to_op();
    	}
    }
    opBuffer.push_back(string(1, '\0')); // Adding '\0' as a string
    // Final flush to ensure all data is written
    flush_to_op();

}

Tree::~Tree() {}

int main(int argc, char const *argv[])
{
	uint n = 7;
    Tree tree(n);

    // vector<int> test = {3,5,0,1,8,7,77,INT_MAX};
    vector<vector<int>> test = {{1,3,1},{5,4,1}, {0,3,0}, {0,1,1}, {0,1,0}, {0,0,0}};
    tree.construct_tree(test);
    tree.print_tree();
    // while(!tree.is_empty()){
    // 	cout<<"Popping min : "<<endl;
    // 	Node temp = tree.pop_winner();
    // 	temp.printNode();
    // 	// tree.print_tree();
    // }
    
    tree.generate_runs();

    // cout<<"Popping min : "<<endl;
    // Node temp = tree.pop_winner();
    // temp.printNode();
    // tree.print_tree();
    // cout<<"Popping min : "<<tree.pop_winner()<<endl;
    tree.print_tree();

	return 0;
}
