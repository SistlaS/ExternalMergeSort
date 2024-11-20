#include <iostream>
#include "tol.h"

bool even(Index const index, Level const shift)
{
    return ((index >> shift) & Index(1)) == Index(0);
}

void PQ::pass(Index const index, Key const key)
{

    Node candidate;
    candidate.init(index, key);
    Index slot;
    // Level level;

    for (leaf(index, slot); parent(slot), slot != root();)
    {
        if (heap[slot].less(candidate))
            heap[slot].swap(candidate);
    }
    heap[root()] = candidate;
}
Index PQ::capacity() const { return Index(1 << height); }
Index PQ::root() const { return Index(0); }

void PQ::leaf(Index const index, Index &slot) const
{
    slot = capacity() + index;
}
void PQ::parent(Index &slot) const { slot /= 2; }
void PQ::leaf(Index const index, Index &slot, Level &level) const
{
    level = 0;
    leaf(index, slot);
}
void PQ::parent(Index &slot, Level &level) const
{
    ++level;
    parent(slot);
}

Key PQ::early_fence(Index const index) const { return Key(index); }
Key PQ::late_fence(Index const index) const { return ~Key(index); }

PQ::~PQ() { delete[] heap; }

PQ::PQ(Level const h)
    : height(h), heap(new Node[1 << h])
{
    for (Index index = capacity(); --index > Index(0);)
    {
        Level level;
        Index slot;

        leaf(index, slot, level);
        do
            parent(slot, level);
        while (even(index, level - 1));

        heap[slot].init(index, early_fence(index));
    }
    heap[root()].init(Index(0), early_fence(Index(0)));
}
bool PQ::empty()
{
    Node const &hr = heap[root()];
    while (hr.key == early_fence(hr.index))
        pass(hr.index, late_fence(hr.index));
    return hr.key == late_fence(hr.index);
}
Index PQ::poptop(bool const invalidate)
{
    if (empty())
        return badIndex;
    if (invalidate)
    {
        heap[root()].key = early_fence(heap[root()].index);
        return heap[root()].index;
    }
    return heap[root()].index;
}
Index PQ::top() { 
    return poptop(false); 
}
Index PQ::pop() { 
    return poptop(true); 
}
void PQ::push(Index const index, Key const key)
{
    pass(index, early_fence(capacity()) + key);
}
void PQ::insert(Index const index, Key const key)
{
    push(index, key);
}
void PQ::update(Index const index, Key const key)
{
    push(index, key);
}
void PQ::delete_(Index const index)
{
    pass(index, late_fence(index));
}
bool PQ::Node::less (Node & other, bool const full)
{
    Offset offset;
    if (full) offset = Offset (−1);
    else if (key != other.key) return (key < other.key);
    else offset = offsetFromKey (key);
    bool const isLess = less (index, other.index, offset);
    Node & loser = (isLess ? other : * this);
    loser.key = keyFromOffset (loser.index, offset);
    return isLess;
}

bool less (Index const left, Index const right, Offset & offset)
{
    while ( ++ offset < size)
    if (data [left] [offset] != data [right] [offset])
    return data [left] [offset] < data [right] [offset];
    return false;
}

int main()
{
    PQ pq(3);

    std::cout << "Heap capacity: " << pq.capacity() << std::endl;

    // node with index 0 and key 100
    pq.push(0, 200);
    pq.push(1, 100);

    std::cout << "Top element index: " << pq.top() << std::endl;
    pq.pop();
    std::cout << "After pop, top element index: " << pq.top() << std::endl;

    pq.insert(3, 300);
    std::cout << "After insert, top element index: " << pq.top() << std::endl;

    pq.update(2, 10);
    std::cout << "After update, top element index: " << pq.top() << std::endl;

    // pq.delete_(2);
    // std::cout << "After delete, top element index: " << pq.top() << std::endl;

    return 0;
}