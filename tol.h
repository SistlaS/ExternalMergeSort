typedef unsigned int Key;
typedef unsigned int Index;
typedef int Level;
typedef unsigned int Offset;

#define badIndex -1;

struct Node{
    Index index;
    Key key;
    Offset offset;

    void init(Index idx, Key k){
        index = idx;
        key = k;
    }

    void swap(Node &candidate){
        std::swap(index, candidate.index);
        std::swap(key, candidate.key);
    }

    bool less (Node candidate){
        if(key <= candidate.key){
            return true;
        }
        return false;
    }
};

class PQ{
public:
    Level height;
    Node* heap;

    PQ(Level const h);
    ~PQ();

    Index capacity() const;
    Index root() const;

    void leaf(Index const index, Index &slot) const;
    void leaf(Index const index, Index &slot, Level &level) const;
    
    void parent(Index &slotl) const;
    void parent(Index &slot, Level &level) const;

    Key early_fence (Index const index) const;
    Key late_fence (Index const index) const;

    bool empty();
    Index poptop(bool const invalidate);
    Index top();
    Index pop();
    void push(Index const index, Key const key);
    void insert(Index const index, Key const key);
    void update(Index const index, Key const key);
    void delete_(Index const index);

private:
    void pass(Index const index, Key const key);
};