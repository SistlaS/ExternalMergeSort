#include "Tree.h"
#include <cassert>
#include "Sort.h"

int ROW_SIZE = Config::column_count;
int BUFFER_SIZE = Config::tt_buffer_size;
bool DEBUG_ = Config::DEBUG_;

// ************UTIL METHODS*********************
vector<int> convertToInt(const string& str) {
    vector<int> result;
    stringstream ss(str);
    string temp;
   
    while (getline(ss, temp, ',')) {
        result.push_back(stoi(temp)); 
    }
    return result;
}

bool greater_row_comparisons(int& offset, const std::vector<Node>& nodes) {
    while (offset < ROW_SIZE) {
        if (nodes[0].getData(offset) != nodes[1].getData(offset)) {
            return nodes[0].getData(offset) > nodes[1].getData(offset);
        }
        offset++;
    }
    if (offset == ROW_SIZE) return true;
    return false;
}
// ***********************************************
// ************NODE METHODS*********************
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
	
	return true;
}

bool Node::greater(Node& other, bool full_, vector<Node>& heap){
	int offset;

	bool isGreater;
	if ((ovc == -1)|| (other.ovc == -1)){
		offset = 0;
		isGreater = ::greater_row_comparisons(offset, { *this, other });
	}else{
		if (ovc != other.ovc){
			if (DEBUG_){
				cout<<"Direct comparison using ovc"<<ovc<<" -- "<<other.ovc<<" ;; res = "<< (ovc <other.ovc)<<endl;	
			}
        	return ovc < other.ovc;  // Compare OVC directly if they differ
		}else{
			//do row comparison
			offset = ovc;
			isGreater = ::greater_row_comparisons(offset, { *this, other });
		}
	}

    Node& loser = (isGreater ? *this : other);
    if (DEBUG_){
    	cout<<"Loser node in ovc comparison -- ";
    	loser.printNode();
    }
    if (loser.Data[0] == INT_MAX){
        offset = -1;
        return isGreater;
    }

    loser.setOvc(offset);
	heap[loser.getIndex()].setOvc(offset);
    return isGreater;
}
// ***********************************************

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

Tree::~Tree() {
    heap.clear();
    opBuffer.clear();
    input.clear();
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

void Tree::setOpFilename(string file){
    opFilename = file;
}

void Tree::construct_tree(){
	//initialise the leaf nodes
	uint input_size = input.size();

	if (input_size > leaf_nodes){
		cout<<"number of records to be sorted is larger than the number of nodes "<<input_size<<"---"<<leaf_nodes<<endl;
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
    for (uint i = 0; i < leaf_nodes; ++i, ++index) {
        if(input[i].empty()){
            temp = Node({INT_MAX}, index);
        }else{
            vector<int> rec = convertToInt(input[i].front());
            temp = Node(rec, index);
            input[i].pop();
        }
        heap[index] = temp;
    }
    //populate the internal nodes
    for (uint i = leaf_nodes; i < capacity; i++){

		Node current = heap[i];
		uint parent_indx = parent_index(i);

		while(parent_indx >=0){
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

void checkQueueSizes(std::vector<std::queue<std::string>>& input) {
    std::cout << "Queue sizes: " << std::endl;
    for (size_t i = 0; i < input.size(); ++i) {
        std::cout << "Queue " << i << " size: " << input[i].size() << std::endl;
    }
}
// returns the next input from the queue where the previous winner came from
Node Tree::get_next_record(uint winner_index){
    uint ip_queue_no = winner_index-leaf_nodes;
    Node new_rec;
    if (input[ip_queue_no].empty()) {
        new_rec = Node({INT_MAX}, winner_index);
    } else {
        string next_data = input[ip_queue_no].front();
        vector<int> rec = convertToInt(next_data);
        new_rec = Node(rec, winner_index);
        input[ip_queue_no].pop();
    }
    return new_rec;
}

// pops the least record and updates the tree with a leaf to root pass using OVC for comparisons
Node Tree::pop_winner() {
    // Save the winner (root of the tree)
    heap[0].setOvc(-1);
    Node winner = heap[0];
    uint winner_index = winner.getIndex();
    // Replace the leaf node corresponding to the winner with a new record
    // For an internal sort, insert INT_MAX; otherwise, use the next record
    Node next_rec = get_next_record(winner_index);
    heap[winner_index] = next_rec;

    // Start the propagation from the winner's index
    Node current = next_rec;
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
        if (DEBUG_){
            bool act_comp = current.is_greater(heap[parent_indx]);
            if(ovc_comp != act_comp) cout<<ovc_comp<<"<- ovc---------------------------------------------------act ->"<<act_comp<<endl;
            // assert(ovc_comp == act_comp && "OVC comparison is different from row wise comparison");
        }
        if (ovc_comp) {
            // Current becomes the new loser, propagate the winner
            swap(current, heap[parent_indx]);
        }
		if (parent_indx == 0) { break; }
		parent_indx = parent_index(parent_indx);
	}
    return winner;
}

void Tree::flush_to_op(bool eof){
	//flush the buffer to op file
    
    if (isRam){
        //write the buffer contents to the file
        ofstream outFile(opFilename, ios::app);
        if (!outFile) {
        std::cerr << "Error: Could not open the file for writing!" << std::endl;
        return;
        }
        for (size_t i = 0; i < opBuffer.size(); ++i) {
            outFile << opBuffer[i] << "|";
        }
        if(eof){
            outFile << "\n";
        }
        outFile.close();

        if (!outFile.good()) {
            std::cerr << "Error occurred while writing to the file." << std::endl;
        }
        
    }else{
        string opString;
        for (size_t i = 0; i < opBuffer.size(); ++i) {
            opString += opBuffer[i] + "|";
        }
        if(eof){
            opString += '\n';
        }
        insertCacheRunsInRAM(opString);
    }
    //clear the buffer
    opBuffer.clear();
}

void Tree::clear_heap(){
    this->heap = std::vector<Node>(this->capacity);
}

void Tree::generate_runs(vector<queue<string>> input){
    this->input = input;
    if (DEBUG_)cout<<"**********************Input size : "<<input.size()<<"******************"<<endl;
   int tot_recs = 0;

    construct_tree();
    if (DEBUG_)cout<<"_____________STARTING RUNS____________"<<endl;
	while(!is_empty()){
    	Node temp = pop_winner();
    	if (DEBUG_)cout<<"*******popping : "<< temp.getDataStr()<<endl;
        tot_recs += 1;
    	opBuffer.push_back(temp.getDataStr());
        int size = opBuffer.size();
    	if(size==BUFFER_SIZE){
            if (isRam ){     
                flush_to_op(false); 
            }
            else{
                flush_to_op(true);
            }
            
    	}
    }
    flush_to_op(true);
    
    clear_heap();
    if (DEBUG_)cout<<"*******************EOR********************** flushed # recs : "<<tot_recs<<endl;
}

// int main(int argc, char const *argv[])
// {
//     vector<queue<string>> input;
//     queue<string> q1;
//     string s1 = "0, 5, 8, 2|0, 7, 1, 4|0, 8, 0, 8|1, 2, 8, 6|2, 1, 5, 0|2, 8, 9, 1|3, 1, 5, 0|3, 8, 9, 4|4, 0, 3, 2|4, 2, 6, 8|5, 1, 1, 1|5, 7, 9, 1|6, 5, 2, 7|7, 6, 9, 0|8, 2, 1, 8|9, 1, 2, 9|";
//     std::stringstream ss(s1); // Create a stringstream from s1
//     std::string token;

//     // Use getline with delimiter '|'
//     while (std::getline(ss, token, '|')) {
//         if (!token.empty()) { // Avoid adding empty strings if any
//             q1.push(token);
//         }
//     }
//     // q1.push("6, 10, 1, 7");
//     input.push_back(q1);

//     queue<string> q2;
//     string s2 = "0, 2, 3, 4|0, 8, 3, 5|2, 2, 1, 1|3, 2, 6, 2|3, 9, 0, 6|4, 0, 6, 5|4, 2, 8, 9|4, 6, 1, 9|6, 7, 9, 1|7, 6, 1, 5|7, 6, 6, 4|7, 9, 3, 8|9, 0, 6, 7|9, 2, 6, 6|9, 7, 7, 3|9, 8, 4, 5|";
//     std::stringstream ss2(s2); // Create a stringstream from s1

//     // Use getline with delimiter '|'
//     while (std::getline(ss2, token, '|')) {
//         if (!token.empty()) { // Avoid adding empty strings if any
//             q2.push(token);
//         }
//     }
//     // q2.push("6, 4, 9, 6");
//     input.push_back(q2);

//     queue<string> q3;
//     string s3 = "0, 0, 9, 1|2, 3, 1, 2|3, 3, 2, 8|3, 4, 9, 3|3, 7, 1, 1|4, 6, 5, 6|4, 7, 5, 6|5, 1, 2, 2|5, 9, 7, 7|5, 9, 9, 2|6, 1, 7, 8|6, 3, 0, 9|6, 5, 2, 7|6, 8, 2, 6|7, 2, 4, 7|9, 6, 3, 7|";
//     std::stringstream ss3(s3); // Create a stringstream from s1

//     // Use getline with delimiter '|'
//     while (std::getline(ss3, token, '|')) {
//         if (!token.empty()) { // Avoid adding empty strings if any
//             q3.push(token);
//         }
//     }
//     // q2.push("6, 4, 9, 6");
//     input.push_back(q3);


//     queue<string> q4;
//     string s4 = "1, 4, 2, 3|1, 4, 4, 8|1, 5, 4, 9|1, 5, 5, 4|1, 8, 8, 2|3, 7, 5, 1|4, 7, 1, 0|5, 6, 1, 0|5, 8, 3, 7|6, 0, 4, 0|8, 6, 7, 3|9, 0, 1, 6|9, 1, 4, 9|9, 1, 8, 3|9, 8, 1, 0|9, 8, 5, 0|";
//     std::stringstream ss4(s4); // Create a stringstream from s1

//     // Use getline with delimiter '|'
//     while (std::getline(ss4, token, '|')) {
//         if (!token.empty()) { // Avoid adding empty strings if any
//             q4.push(token);
//         }
//     }
//     // q2.push("6, 4, 9, 6");
//     input.push_back(q4);
//     uint n = 4;
//     string outputFilename = "output.txt";

//     Tree tree(n, outputFilename);
//     tree.generate_runs(input);
//     // vector<queue<string>> input2;
//     // queue<string> q3;

//     // q1.push("86, 10, 1, 22");
//     // input2.push_back(q3);

//     // queue<string> q4;
//     // q2.push("16, 4, 9, 6");
//     // input2.push_back(q4);
//     // tree.generate_runs(input);
//     // Construct the tree
//     // tree.construct_tree();
    
//     tree.print_tree();
//     return 0;
// }

// // int main(int argc, char const *argv[])
// // {
// //     vector<queue<string>> input;
// //     queue<string> q1;
    
// //     q1.push("2, 4, 3, 0");
// //     q1.push("3, 0, 1, 3");
// //     // q1.push("5, 5, 3, 4");
// //     input.push_back(q1);

// //     queue<string> q2;
// //     q2.push("2, 4, 3, 0");
// //     q2.push("2, 4, 4, 5");
// //     // q2.push("4, 4, 8, 9");
// //     input.push_back(q2);

// //     // queue<string> q3;
// //     // q3.push("2, 4, 4, 6");
// //     // q3.push("4, 5, 0, 6");
// //     // q3.push("9, 8, 8, 6");
// //     // input.push_back(q3);

// //     // queue<string> q4;
// //     // q4.push("5,0,0,0,");
// //     // q4.push("5,0,3,0,");
// //     // input.push_back(q4);

// //     // queue<string> q5;
// //     // q5.push("6,1,10,3,");
// //     // q5.push("6,1,10,8,");
// //     // input.push_back(q5);
// //     uint n = 2;
// //     string outputFilename = "output.txt";

// //     Tree tree(n, outputFilename);
// //     tree.generate_runs(input);
// //     // vector<queue<string>> input2;
// //     // queue<string> q3;

// //     // q1.push("86, 10, 1, 22");
// //     // input2.push_back(q3);

// //     // queue<string> q4;
// //     // q2.push("16, 4, 9, 6");
// //     // input2.push_back(q4);
// //     // tree.generate_runs(input);
// //     // Construct the tree
// //     // tree.construct_tree();
    
// //     tree.print_tree();
// //     return 0;
// // }

