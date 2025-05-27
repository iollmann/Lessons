//
//  LinkedList.h
//  LinkedLists
//
//  Created by Ian Ollmann on 5/19/25.
//
//  Assignment
//  ==========
//  1) Implement the below classes.  They are template classes.
//      • You will meed to learn how to write template class method implementations, incl. where
//        they go in the headers / source files.
//      • The classes are missing declarations for private member data. You'll need to add
//        that in order to implement them
//
//  2) Methods that return a ClassType pointer should be marked ALWAYS_USE_RESULT
//      You'll need to figure out how to label the functions so that the compiler warns
//      when the function returns a result but the calling code doesn't use it.
//      Change the #define to the right #define to make this happen. Currently it warns all the time,
//      which is undesired. Understand clang's __attribute__s and how to use them.
//      The goal here is to make sure that if someone calls SwapNext() for example
//      the old next pointer is not just dropped on the floor, since that would be a leak.
//      It is intended you learn to use SwapNext instead of SetNext as part of your
//      daily practice so as to cut down on the number of bugs. This is a good idea
//      in general whenever the setter function is replacing a pointer to something on the heap.
//
//  3) Write a test for all of this. It is time to become deeply suspicious of your own code!
//      It can be a simple main.cpp file, or you can use the Xcode test infrastructure. Apple will want you
//      to do the latter. If you do not want to use XCTest, then we can cover how to do this next week.
//
//  4) make sure pointers are properly set to NULL when they no longer reference anything valid on the heap
//
//  You may consider the internet, including stackoverflow, to be your textbook for these activities.
//  Please avoid AI. You won't learn the intended lessons which will cause trouble in interviews.
//
//              https://en.cppreference.com/w/cpp/language/class_template
//              https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
//              https://www.headspin.io/blog/xctest-a-complete-guide
//              https://developer.apple.com/documentation/xctest?language=objc
//              https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/testing_with_xcode/chapters/05-running_tests.html
//
//  On completion, I have my own test, we will run it and see how you do! Anything we find will
//  actually be two bugs:  the bug and the failure in the test code that didn't catch it.
//  You'll be expected to fix them before we can go on. Fix the test to catch the bug first, then
//  fix the bug to pass the test.
//
//  Definitions:
//  ------------
//  Linked list -- a data structure consisting of a set of nodes, in which each node has a pointer
//                 to the next (or maybe previous) item in the list.
//
//  Atomic -- An indivisible operation in computer programming. At any given time, including from
//            other threads, the operation either has completed fully or cant be observed to have done
//            anything at all.
//
//              https://en.wikipedia.org/wiki/Linearizability
//              https://en.cppreference.com/w/cpp/atomic/atomic
//
//              Consider:   x++
//
//                  On most machines, this is actually three operations:
//                      read X from memory
//                      x = x + 1
//                      write x back to memory
//
//                  If two threads do this at the same time, we might observe:
//
//                      Thread 1                        Thread 2
//                      ============================================
//                      Read X from memory
//                                                      Read X from memory
//                      x = x + 1                       x = x + 1       (note: this is a different x from thread 1)
//                                                      Write X back to memory
//                      Write X back to memory
//
//                  Is X now X+2 or X+1?  It is now X+1. Both threads wrote X+1 to memory, because their
//                  local copy of x were distinct.  If we want this to work correctly, the operation x=x+1
//                  needs to be atomic, such that neither thread can read the
//
//  Stale -- Describes data which has the wrong value or not provably the right value in multithreaded situations.
//           The current status of something is almost always stale unless atomic memory operations are used
//           because by the time you read the information, some other thread might have changed it. Example:
//           the current retain count on reference counted objects.
//
//  Scope -- the region of code from which a variable can be viewed. If it is a stack variable, its
//           scope is the stack frame (or curly brace) that contains it, as long as you don't hand a
//           pointer to it off to some other scope. If it is a global variable, then it has global scope
//           and any thread can see it.  Limiting scope limits who can change a variable or be screwed up
//           by a variable changing. General rule: if you can't see or be affected by a variable then you
//           are safe when it changes. More limited scopes are to be preferred over broader scopes to make
//           code thread safe. Rule of thumb: Avoid all use of globals, except when absolutely, absolutely
//           necessary.
//
//  Thread safe -- code that can safely be called from multiple threads. Sometimes this can be nuanced.
//              Many Apple data structures can be safely read by many threads at once, but only as long
//              as you don't change them. Certain things like CGImageRef are only safe because the API guarantees
//              that once they are made, they never change. This is a design pattern you can use in your code too!
//
//  Reentrant -- a function that is called from multiple threads at the same time. Reentrant access on code
//              not designed for reentrant access is a common cause of very difficult to track down bugs!
//
//  Reentrant safe -- a function that can be safely called by multiple threads concurrently
//                 What if other programs call your function? Protected memory prevents this from being a
//                 problem in almost every case. The code itself is marked constant and can not change. Any
//                 variables in use by this code have separate copies per process.  If you are using something
//                 like IPC that explicitly shares memory between multiple processes, then you'll need to
//                 worry about that.
//
//  Last in; First out (LIFO) -- a list order in which the last thing put into the list is the first thing that
//                  comes out, e.g. a stack.
//
//  First in; First out (FIFO) -- a list order in which the first thing put into the list is the first thing that
//                  comes out, e.g. the line at the grocery store. https://www.spiceworks.com/tech/devops/articles/fifo-vs-lifo/
//
//  Queue -- an ordered list of things.
//

/*! @abstract  TODO:  Fix this to issue warning only when the result of the function is not used instead of all the time*/
#ifndef ALWAYS_USE_RESULT
#   define ALWAYS_USE_RESULT       __attribute__((warning("The result of this function is not used!")))
#endif

/*! @abstract The base class for things that are in the linked lists */
template <typename ClassType>
class LinkedListNode
{
private:
    ClassType * __nullable next;
    
public:
    LinkedListNode();
    LinkedListNode(const LinkedListNode & node);
    virtual ~LinkedListNode();
    
    /*! @abstract  Return the next item in the list */
    inline ClassType * ALWAYS_USE_RESULT __nullable GetNext() const;

    /*! @abstract  Swap the next item in the list for a new value. Return the old value. */
    inline ClassType * ALWAYS_USE_RESULT __nullable SwapNext( ClassType * __nullable newValue);
};

/*! @abstract Singly linked list that operates in a Last-in, First-out order. The list nodes will be subclasses of LinkedListNode<SubClass> */
template <typename ClassType>
class LIFOLinkedList
{
private:
    /*! @abstract  TODO: What private data members are needed here? */
    ClassType * __nullable list;
    
    LIFOLinkedList(const LIFOLinkedList & list ) = delete;              // Declared private so we don't accidentally called it. Do not implement.
    LIFOLinkedList & operator=(const LIFOLinkedList & list) = delete;   // Declared private so we don't accidentally called it. Do not implement.
    
public:
    LIFOLinkedList();
    LIFOLinkedList(ClassType * __nullable nodes);
    
    ~LIFOLinkedList();
    
    /*! @abstract Returns true of the node is in the list */
    inline bool Contains(ClassType * __nullable node) const;

    /*! @abstract Returns the number of nodes in the list */
    inline unsigned long GetCount() const;

    /*! @abstract Look at the first item on the list. Do not remove from the list */
    inline const ClassType * __nullable ALWAYS_USE_RESULT GetHead() const;

    /*! @abstract Look at the last item on the list. Do not remove from the list */
    inline const ClassType * __nullable ALWAYS_USE_RESULT GetTail() const;

    /*! @abstract Remove the most recently added node from the list */
    inline ClassType * __nullable ALWAYS_USE_RESULT Pop();

    /*! @abstract Add nodes to the list such that the last node in newNodes will be the first one off */
    inline void Push(ClassType * __nullable newNodes );
    inline void Push(LIFOLinkedList<ClassType> & list); // empties list

    /*! @abstract Reverse the order of the list */
    inline void Reverse();

    /*! @abstract Steal the list nodes. List becomes empty and a naked linked list of the old nodes is returned out the left hand side */
    inline ClassType * __nullable ALWAYS_USE_RESULT StealList();

    static constexpr bool kIterateContinue = false;
    static constexpr bool kIterateStop = true;
    /*! @abstract Iterate over the list of nodes, applying a block to each
     *  @param  block   The block to call for each node */
    inline void Iterate( bool(^ __nonnull block)(const ClassType * __nonnull node))  const;
};

/*! @abstract Singly linked list that operates in a First-in, First-out order */
template <typename ClassType>
class FIFOLinkedList
{
private:
    /*! @abstract  TODO: What private data members are needed here? */
    ClassType * __nullable head;
    ClassType * __nullable tail;

    FIFOLinkedList(const FIFOLinkedList & list ) = delete;                // Declared private so we don't accidentally called it. Do not implement.
    FIFOLinkedList & operator=(const FIFOLinkedList & list)  = delete;    // Declared private so we don't accidentally called it. Do not implement.

public:
    FIFOLinkedList();
    FIFOLinkedList(ClassType * __nullable nodes);
    ~FIFOLinkedList();

    /*! @abstract Returns true of the node is in the list */
    inline bool Contains(ClassType * __nullable node) const;

    /*! @abstract Returns the number of nodes in the list */
    inline unsigned long GetCount() const;

    /*! @abstract Look at the first item on the list. Do not remove from the list */
    inline const ClassType * __nullable ALWAYS_USE_RESULT GetHead() const;

    /*! @abstract Look at the last item on the list. Do not remove from the list */
    inline const ClassType * __nullable ALWAYS_USE_RESULT GetTail() const;

    /*! @abstract Remove the most recently added node from the list */
    inline ClassType * __nullable ALWAYS_USE_RESULT Dequeue();

    /*! @abstract Add nodes to the end of the list  */
    inline void Enqueue(ClassType * __nullable newNodes );
    inline void Enqueue(FIFOLinkedList<ClassType> & list ); // empties list

    /*! @abstract Reverse the order of the list */
    inline void Reverse();

    /*! @abstract Steal the list nodes. List becomes empty and a naked linked list of the old nodes is returned out the left hand side */
    inline ClassType * __nullable ALWAYS_USE_RESULT StealList();
    
    static constexpr bool kIterateContinue = false;
    static constexpr bool kIterateStop = true;
    /*! @abstract Iterate over the list of nodes, applying a block to each
     *  @param  block   The block to call for each node */
    inline void Iterate( bool(^ __nonnull block)(const ClassType * __nonnull node))  const;
};

#include <atomic>

template <typename ClassType>
class LinkedListNodeAtomic
{
private:
    std::atomic<ClassType * __nullable>     next;

public:
    LinkedListNodeAtomic();
    LinkedListNodeAtomic(const LinkedListNodeAtomic<ClassType> & node);
    virtual ~LinkedListNodeAtomic();
    
    /*! @abstract  Return the next item in the list */
    inline ClassType * ALWAYS_USE_RESULT __nullable GetNext() const;

    /*! @abstract  Swap the next item in the list for a new value. Return the old value. */
    inline ClassType * ALWAYS_USE_RESULT __nullable SwapNext( ClassType * __nullable newValue);
};


/*! @abstract Singly linked list that operates in a Last-in, First-out order -- atomic */
template <typename ClassType>
class LIFOLinkedListAtomic
{
private:
    /*! @abstract  TODO: What private data members are needed here? */
    std::atomic<ClassType * __nullable>         list;
    
    LIFOLinkedListAtomic(const LIFOLinkedListAtomic & list ) = delete;              // Declared private so we don't accidentally called it. Do not implement.
    LIFOLinkedListAtomic & operator=(const LIFOLinkedListAtomic & list) = delete;   // Declared private so we don't accidentally called it. Do not implement.
    
    static const ClassType * __nonnull kReservedNode;
    
public:
    LIFOLinkedListAtomic();
    ~LIFOLinkedListAtomic();
    
    /*! @abstract Take a raw linked list of nodes and reverse the order
     *  @discussion  Why is this a static function and not a class method like in the other classes? */
    static ClassType * __nullable ALWAYS_USE_RESULT ReverseList( ClassType * __nullable nodes );

    /*! @abstract Remove the most recently added node from the list atomically*/
    inline ClassType * __nullable ALWAYS_USE_RESULT Pop();

    /*! @abstract Add nodes atomically to the list such that the last node in newNodes will be the first one off */
    inline void Push(ClassType * __nullable newNodes );
    
    inline ClassType * __nullable GetHead() { ClassType * oldHead; do{ oldHead = atomic_load_explicit( &list, std::memory_order_acquire); }while( oldHead == kReservedNode); return oldHead; }

    /*! @abstract Steal the list nodes. List becomes empty and a naked linked list of the old nodes is returned out the left hand side */
    inline ClassType * __nullable ALWAYS_USE_RESULT StealList();
};

