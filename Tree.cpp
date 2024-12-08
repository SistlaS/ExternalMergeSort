#include "Tree.h"
#include <cassert>

int ROW_SIZE = Config::column_count;
int BUFFER_SIZE = Config::cache_tt_buffer_size;
bool DEBUG_ = Config::DEBUG_;


vector<int> convertToInt(const string& str) {
    vector<int> result;
    stringstream ss(str);
    string temp;
   
    while (getline(ss, temp, ',')) {
        result.push_back(stoi(temp)); 
    }
    return result;
}

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
        // cout<<Data[i]<< " --- "<< incoming_data[i]<<endl;
		if (Data[i] > incoming_data[i]){
			return true;
		}else if (Data[i] < incoming_data[i]){
			return false;
		}
	}
	
	return false;
}

bool greater(int& offset, const std::vector<Node>& nodes) {
    while (offset < ROW_SIZE) {
        if (nodes[0].getData(offset) != nodes[1].getData(offset)) {
            return nodes[0].getData(offset) > nodes[1].getData(offset);
        }
        offset++;
    }
    return false;
}

bool Node::greater(Node& other, bool full_, vector<Node>& heap){
	int offset;

	bool isGreater;
	if ((ovc == -1)|| (other.ovc == -1)){
		offset = 0;
		isGreater = ::greater(offset, { *this, other });
	}else{
		if (ovc != other.ovc){
			if (DEBUG_){
				cout<<"Direct comparison using ovc"<<ovc<<" -- "<<other.ovc<<" ;; res = "<< (ovc <other.ovc)<<endl;	
			}
        	return ovc < other.ovc;  // Compare OVC directly if they differ
		}else{
			//do row comparison
			if (DEBUG_){
				cout<<"OVC is same"<<endl;
			}
			offset = ovc;
			isGreater = ::greater(offset, { *this, other });
		}
	}

    // if (full_) {
    //     offset = -1;  // Start offset for full comparison
    // } else if (ovc != other.ovc && (ovc != -1) && (other.ovc != -1)) {
    // 	cout<<"Direct comparison using ovc"<<ovc<<" -- "<<other.ovc<<endl;
    //     return ovc < other.ovc;  // Compare OVC directly if they differ
    // } else {
    // 	cout<<"OVC is same comparison"<<endl;
    //     offset = ovc;  // Start from the current offset-value coding
    // }

    // bool const isGreater = ::greater(offset, { *this, other });

    Node& loser = (isGreater ? *this : other);
    if (DEBUG_){
    	cout<<"Loser node in ovc comparison -- ";
    	loser.printNode();
		cout<<"Setting ovc for "<<loser.getIndex()<<","<<offset<<endl;
    }
    
    loser.setOvc(offset);
	heap[loser.getIndex()].setOvc(offset);
    
    return isGreater;
}

//capacity here refers to the number of records the tree the can handle
//for internal sort this is assumed to be equal to the leaf size for simplicity
//for merging no.of leaves = capacity/2

Tree::Tree(uint k, string opFilename){
    //the next power of 2 for capacity
    uint nextPowerOf2 = 1;
    while (nextPowerOf2 < 2 * k) {
        nextPowerOf2 *= 2;
    }

    this->capacity = nextPowerOf2;
    this->heap = std::vector<Node>(this->capacity);
    this->leaf_nodes = this->capacity/2;
    this->opFilename = opFilename;

    if (opFilename.empty()){
        this->isCache = true;
    }else{
        this->isRam = true;
    }
}

Tree::~Tree() {}

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


void Tree::construct_tree(){
	//initialise the leaf nodes
	int input_size = input.size();

	if (input_size > leaf_nodes){
		cout<<"number of records to be sorted is larger than the number of nodes"<<endl;
		return;
	}
    //push empty queues if there are empty leaf nodes
	if (input_size < leaf_nodes){
		int diff = leaf_nodes - input_size;
		for(int i = 0;i <diff;i ++){
			input.push_back(queue<string>()); //empty queue
		}
	}
	// Populate the leaf nodes
    uint index = leaf_nodes;
    Node temp;
    for (int i = 0; i < leaf_nodes; ++i, ++index) {
        if(input[i].empty()){
            temp = Node({INT_MAX}, index);
        }else{
            vector<int> rec = convertToInt(input[i].front());
            temp = Node(rec, index);
            input[i].pop();
        }
        // temp.printNode();
        heap[index] = temp;
    }

    //populate the internal nodes
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
	if (DEBUG_) print_tree();
}

bool Tree::is_empty(){
	if(heap[0].getData()[0] == INT_MAX){
		return true;
	}
	return false;
}

void checkQueueSizes(const std::vector<std::queue<std::string>>& input) {
    std::cout << "Queue sizes: " << std::endl;
    for (size_t i = 0; i < input.size(); ++i) {
        std::cout << "Queue " << i << " size: " << input[i].size() << std::endl;
    }
}

Node Tree::pop_winner() {
    // Save the winner (root of the tree)
    heap[0].setOvc(-1);
    Node winner = heap[0];
    uint winner_index = winner.getIndex();
    // Replace the leaf node corresponding to the winner with a new record
    // For an internal sort, insert INT_MAX; otherwise, use the next record
    Node new_rec;
    int ip_queue_no = winner_index-leaf_nodes;
    // cout<<ip_queue_no<<" -- "<<input[ip_queue_no].front()<<endl;
    if (input[ip_queue_no].empty()) {
        new_rec = Node({INT_MAX}, winner_index);
    } else {
        string next_data = input[ip_queue_no].front();
        vector<int> rec = convertToInt(next_data);
        new_rec = Node(rec, winner_index);
        input[ip_queue_no].pop();
    }
    // new_rec.printNode();
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
        if (DEBUG_){
        	cout<<"current ind : ";
	        current.printNode();
	        cout<<"parent ind : ";
	        heap[parent_indx].printNode();
        }
        
        bool ovc_comp = current.greater(heap[parent_indx], false, heap);
        bool act_comp = current.is_greater(heap[parent_indx]);
        if (DEBUG_) cout<<ovc_comp<<"<- ovc-----act ->"<<act_comp<<endl;
        assert(ovc_comp == act_comp && "OVC comparison is different from row wise comparison");
        if (ovc_comp) {
            // Current becomes the new loser, propagate the winner
            swap(current, heap[parent_indx]);
        }
        // if (current.is_greater(heap[parent_indx])) {
        //     // Current becomes the new loser, propagate the winner
        //     swap(current, heap[parent_indx]);
        // }
		if (parent_indx == 0) { break; }
		parent_indx = parent_index(parent_indx);
	}
    return winner;
}

void Tree::flush_to_op(bool eof){
	//flush the buffer to op file
    cout<<"In flush"<<endl;
    if (isRam){
        ofstream outFile(opFilename, ios::app);
        if (!outFile) {
        std::cerr << "Error: Could not open the file for writing!" << std::endl;
        return;
        }
        for (size_t i = 0; i < opBuffer.size(); ++i) {
            cout<<opBuffer[i]<<endl;
            outFile << opBuffer[i] << "|";
        }
        if(eof){
            outFile << "\n";
        }
        outFile.close();

        if (outFile.good()) {
            std::cout << "File written successfully to " << opFilename << std::endl;
        } else {
            std::cerr << "Error occurred while writing to the file." << std::endl;
        }
        //clear the buffer
        
    }else{
        string opString;
        for (size_t i = 0; i < opBuffer.size(); ++i) {
            opString += opBuffer[i] + "|";
        }
        opString += '\n';
        SortIterator::insertCacheRunsinRAM(opString);
    }
    opBuffer.clear();
}

void Tree::generate_runs(vector<queue<string>> input){
    this->input = input;
    construct_tree();
	while(!is_empty()){
    	Node temp = pop_winner();
    	cout<<"popping :";
    	temp.printNode();
    	opBuffer.push_back(temp.getDataStr());
    	if(opBuffer.size()==BUFFER_SIZE){ 
    		flush_to_op(false);
    	}
    }
    // opBuffer.push_back(string(1, '\n'));
    // Final flush to ensure all data is written
    flush_to_op(true);

}

int main(int argc, char const *argv[])
{
    vector<queue<string>> input;
    queue<string> q1;

    q1.push("6, 10, 1, 7");
    input.push_back(q1);

    queue<string> q2;
    q2.push("6, 4, 9, 6");
    input.push_back(q2);

    // q1.push("2, 4, 3, 0");
    // q1.push("3, 0, 1, 3");
    // q1.push("5, 5, 3, 4");
    // input.push_back(q1);

    // queue<string> q2;
    // q2.push("2, 2, 0, 1");
    // q2.push("2, 4, 4, 5");
    // q2.push("4, 4, 8, 9");
    // input.push_back(q2);

    // queue<string> q3;
    // q3.push("4, 5, 0, 6");
    // q3.push("9, 8, 8, 6");
    // input.push_back(q3);

    // queue<string> q4;
    // q4.push("5,0,0,0,");
    // q4.push("5,0,3,0,");
    // input.push_back(q4);

    // queue<string> q5;
    // q5.push("6,1,10,3,");
    // q5.push("6,1,10,8,");
    // input.push_back(q5);
    uint n = 2;
    string outputFilename = "output.txt";

    Tree tree(n, outputFilename);
    tree.generate_runs(input);
    // Construct the tree
    // tree.construct_tree();
    
    tree.print_tree();
    return 0;
}