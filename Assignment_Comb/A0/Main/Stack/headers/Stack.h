
#ifndef STACK_H
#define STACK_H

// this is the node class used to build up the LIFO stack
template <class Data>
class Node {

private:

	Data holdMe;
	Node *next;
	
public:

	/*****************************************/
	/** WHATEVER CODE YOU NEED TO ADD HERE!! */
	/*****************************************/
    Node() {
        next = NULL;
    }
    Node(Data item, Node *nextNode = NULL) {
        holdMe = item;
        next = nextNode;
    }

    Node* getNext() {
        return next;
    }

    Data getData() {
        return holdMe;
    }
};

// a simple LIFO stack
template <class Data> 
class Stack {

	Node <Data> *head;

public:

	// destroys the stack
	~Stack () {
	    while(!isEmpty()) {
	        pop();
	    }
	}

	// creates an empty stack
	Stack () {head = NULL;}

	// adds pushMe to the top of the stack
	void push (const Data &pushMe) {
	    Node<Data> *newHead = new Node<Data>(pushMe, head);
	    head = newHead;
	}

	// return true if there are not any items in the stack
	bool isEmpty () {
	    return (head == NULL);
	}

	// pops the item on the top of the stack off, returning it...
	// if the stack is empty, the behavior is undefined
	Data pop () {
        if(isEmpty()) {
            return Data();
        }
        Node<Data> *oldHead = head;
        head = head->getNext();
        Data del = oldHead->getData();
        delete oldHead;
        return del;
	}
};

#endif
