// Project identifier: 43DE0E0C4C76BFAA6D8C2F5AEAE0518A9C42CF4E

#ifndef PAIRINGPQ_H
#define PAIRINGPQ_H

#include "Eecs281PQ.h"
#include <deque>
#include <utility>

using namespace std;

// A specialized version of the priority queue ADT implemented as a pairing
// heap.
template<typename TYPE, typename COMP_FUNCTOR = std::less<TYPE>>
class PairingPQ : public Eecs281PQ<TYPE, COMP_FUNCTOR> {
    // This is a way to refer to the base class object.
    using BaseClass = Eecs281PQ<TYPE, COMP_FUNCTOR>;

public:
    // Each node within the pairing heap
    class Node {
        public:
            // TODO: After you add add one extra pointer (see below), be sure
            //       to initialize it here.
            explicit Node(const TYPE &val)
                : elt{ val }, child{ nullptr }, sibling{ nullptr }, previous{ nullptr }
            {}

            // Description: Allows access to the element at that Node's
            //              position. There are two versions, getElt() and a
            //              dereference operator, use whichever one seems
            //              more natural to you.
            // Runtime: O(1) - this has been provided for you.
            const TYPE &getElt() const { return elt; }
            const TYPE &operator*() const { return elt; }

            // The following line allows you to access any private data
            // members of this Node class from within the PairingPQ class.
            // (ie: myNode.elt is a legal statement in PairingPQ's add_node()
            // function).
            friend PairingPQ;

        private:
            TYPE elt;
            Node *child;
            Node *sibling;
            Node *previous;
            // TODO: Add one extra pointer (parent or previous) as desired.
    }; // Node


    // Description: Construct an empty pairing heap with an optional
    //              comparison functor.
    // Runtime: O(1)
    explicit PairingPQ(COMP_FUNCTOR comp = COMP_FUNCTOR()) :
        BaseClass{ comp }, root(nullptr), numNodes(0) {
        // TODO: Implement this function.
    } // PairingPQ()


    // Description: Construct a pairing heap out of an iterator range with an
    //              optional comparison functor.
    // Runtime: O(n) where n is number of elements in range.
    template<typename InputIterator>
    PairingPQ(InputIterator start, InputIterator end, COMP_FUNCTOR comp = COMP_FUNCTOR()) :
        BaseClass{ comp }, root(nullptr), numNodes(0) {
        // TODO: Implement this function.
        for(InputIterator iter = start; iter != end; ++iter){
            push(*iter);
        }
    } // PairingPQ()


    // Description: Copy constructor.
    // Runtime: O(n)
    PairingPQ(const PairingPQ &other) :
        BaseClass{ other.compare }, root(nullptr), numNodes(0) {
        // TODO: Implement this function.
        deque<Node*> copy;
        copy.push_back(other.root);
        while(!copy.empty()){
            Node* next = copy.front();
            copy.pop_front();
            if(next->child != nullptr){
                copy.push_back(next->child);
            }
            if(next->sibling != nullptr){
                copy.push_back(next->sibling);
            }
            push(next->getElt());
        }
    } // PairingPQ()


    // Description: Copy assignment operator.
    // Runtime: O(n)
    PairingPQ &operator=(const PairingPQ &rhs) {
        // TODO: Implement this function.
        PairingPQ temp(rhs);
        swap(numNodes, temp.numNodes);
        swap(root, temp.root);
        return *this;
    } // operator=()


    // Description: Destructor
    // Runtime: O(n)
    ~PairingPQ() {
        // TODO: Implement this function.
        if(root != nullptr){
            deque<Node*> copy;
            copy.push_back(root);
            while(!copy.empty()){
                Node* next = copy.front();
                copy.pop_front();
                if(next->child != nullptr){
                    copy.push_back(next->child);
                }
                
                if(next->sibling != nullptr)
                    copy.push_back(next->sibling);
                delete next;
            }
        }
        
    } // ~PairingPQ()


    // Description: Assumes that all elements inside the pairing heap are out
    //              of order and 'rebuilds' the pairing heap by fixing the
    //              pairing heap invariant. You CANNOT delete 'old' nodes
    //              and create new ones!
    // Runtime: O(n)
    virtual void updatePriorities() {
        // TODO: Implement this function.
        deque<Node*> copy;
        copy.push_back(root);
        root = nullptr;
        while(!copy.empty()){
            Node* next = copy.front();
            copy.pop_front();
            if(next->child != nullptr)
                copy.push_back(next->child);
            if(next->sibling != nullptr)
                copy.push_back(next->sibling);
            
            //cut all connections
            next->previous = nullptr;
            next->sibling = nullptr;
            next->child = nullptr;
            if(!root){
                root = next;
            }
            else{
                root = meld(root, next);
            }
        }
    } // updatePriorities()


    // Description: Add a new element to the pairing heap. This is already
    //              done. You should implement push functionality entirely in
    //              the addNode() function, and this function calls
    //              addNode().
    // Runtime: O(1)
    virtual void push(const TYPE &val) {
        addNode(val);
    } // push()


    // Description: Remove the most extreme (defined by 'compare') element
    //              from the pairing heap.
    // Note: We will not run tests on your code that would require it to pop
    //       an element when the pairing heap is empty. Though you are
    //       welcome to if you are familiar with them, you do not need to use
    //       exceptions in this project.
    // Runtime: Amortized O(log(n))
    virtual void pop() {
        Node *p = root->child;      
        delete root;
        if(!p){             //root doesn't have a child
            root = nullptr;
        }
        else{
            deque<Node*> children;
            for(Node* ptr = p; ptr != nullptr; ptr = ptr->sibling){
                children.push_back(ptr);
            }
            while(children.size() != 1){
                //pop two nodes from the front, break connections and meld them
                Node* p1 = children.front();
                children.pop_front();
                p1->sibling = nullptr;
                p1->previous = nullptr;
                Node* p2 = children.front();
                children.pop_front();
                p2->sibling = nullptr;
                p2->previous = nullptr;

                //put the result at the back
                children.push_back(meld(p1, p2));
            }
            root = children.front();
            root->previous = nullptr;
            root->sibling = nullptr;
        }
        --numNodes;
    } // pop()


    // Description: Return the most extreme (defined by 'compare') element of
    //              the pairing heap. This should be a reference for speed.
    //              It MUST be const because we cannot allow it to be
    //              modified, as that might make it no longer be the most
    //              extreme element.
    // Runtime: O(1)
    virtual const TYPE &top() const {
        // TODO: Implement this function
        return root->getElt();
    } // top()


    // Description: Get the number of elements in the pairing heap.
    // Runtime: O(1)
    virtual std::size_t size() const {
        return (size_t)numNodes; // TODO: Delete or change this line
    } // size()

    // Description: Return true if the pairing heap is empty.
    // Runtime: O(1)
    virtual bool empty() const {
        // TODO: Implement this function
        return numNodes == 0; // TODO: Delete or change this line
    } // empty()


    // Description: Updates the priority of an element already in the pairing
    //              heap by replacing the element refered to by the Node with
    //              new_value. Must maintain pairing heap invariants.
    //
    // PRECONDITION: The new priority, given by 'new_value' must be more
    //               extreme (as defined by comp) than the old priority.
    //
    // Runtime: As discussed in reading material.
    void updateElt(Node* node, const TYPE &new_value) {
        // TODO: Implement this function
        node->elt = new_value;
        //Node *prev = node->previous;
        if(node == root)    return;
        else if(node->previous->child == node){     //left-most child
            node->previous->child = node->sibling;
        }
        else{                           //keep searching for the sibling until find left of node
            node->previous->sibling = node->sibling;
            node->sibling->previous = node->previous;
        }
        node->sibling = nullptr;
        node->previous = nullptr;
        root = meld(node, root);

    } // updateElt()


    // Description: Add a new element to the pairing heap. Returns a Node*
    //              corresponding to the newly added element.
    // NOTE: Whenever you create a node, and thus return a Node *, you must
    //       be sure to never move or copy/delete that node in the future,
    //       until it is eliminated by the user calling pop(). Remember this
    //       when you implement updateElt() and updatePriorities().
    // Runtime: O(1)
    Node* addNode(const TYPE &val) {
        // TODO: Implement this function
        Node * newNode = new Node(val);
        if(numNodes == 0){      //empty PQ
            root = newNode;
        }
        else{
            root = meld(root, newNode);
        }
        ++numNodes;
        return newNode; // TODO: Delete or change this line
    } // addNode()


private:
    // TODO: Add any additional member variables or member functions you
    //       require here.
    Node * root;
    int numNodes;
    // TODO: We recommend creating a 'meld' function (see the Pairing Heap
    //       papers).
    Node * meld(Node * pa, Node * pb){      //CHANGE THIS
        //pa/pb are roots of two pairing PQs that have no parent/sibling
        //(pa->previous || pa->sibling || pb->previous || pb->sibling)
        if(pa->previous || pa->sibling || pb->previous || pb->sibling){
            //cout << "Invalid Melding!\n";
            return nullptr;
        }
        if(this->compare(pa->elt, pb->elt)){    //pb has a high priority than pa
            if(pb->child){
                pa->sibling = pb->child;
                pb->child->previous = pa;
            }
            pb->child = pa;
            pa->previous = pb;
            return pb;
        }
        else{
            if(pa->child){
                pb->sibling = pa->child;
                pa->child->previous = pb;
            }
            pa->child = pb;
            pb->previous = pa;
            return pa;
        }
    }
    // NOTE: For member variables, you are only allowed to add a "root
    //       pointer" and a "count" of the number of nodes. Anything else
    //       (such as a deque) should be declared inside of member functions
    //       as needed.
};


#endif // PAIRINGPQ_H
