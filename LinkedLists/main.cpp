//
//  main.cpp
//  LinkedLists
//
//  Created by Ian Ollmann on 5/19/25.
//




#include <iostream>
#include <dispatch/dispatch.h>
#include <stdlib.h>
#if DEBUG
#else
#   define NDEBUG 1
#endif
#include <assert.h>


#define USE_DADDYS_IMPLEMENTATIONS  1

#pragma mark - Implementations

#if USE_DADDYS_IMPLEMENTATIONS
#   define USE_SINGLE_PASS_ATOMICS    0
#   include "Daddy.hpp"
#else
#   include "SubClass.hpp"
#endif  /* USE_DADDYS_IMPLEMENTATIONS */

#pragma mark -
#define TEST( _test )       ({ __typeof__ (_test) _result = _test; if( (__typeof__ _result) 0 == _result){ __assert(#_test, __FILE__, __LINE__); return -1;} })

int TestLIFO(const unsigned long listSize )
{
    SubClassLIFO list;          // Test default ctor
    
    // add listSize Nodes
    for( unsigned long i = 0; i < listSize; i++)
        list.Push( new SubClass(i) );
    
    // Check the head
    const SubClass * head = list.GetHead();
    if( 0 == listSize)
        TEST(NULL == head);
    else
    {
        TEST(NULL != head);
        TEST(head->IsValid());
        TEST(head->GetValue() == listSize-1);
    }
    
    // Check the tail
    const SubClass * tail = list.GetTail();
    if( 0 == listSize)
        TEST(NULL == tail);
    else
    {
        TEST(NULL != tail );
        TEST(tail->IsValid());
        TEST( tail->GetValue() == 0);
    }
    
    // Check head == tail for list size = 1
    if( 1 == listSize)
        TEST( list.GetHead() == list.GetTail());
    
    // Check the count
    TEST( listSize == list.GetCount());
    
    // Check the indices are in the right reverse order
    __block unsigned long index = listSize;
    list.Iterate(^bool(const SubClass * _Nonnull node) {
        --index;
        TEST(node->GetValue() == index);
        TEST(node->IsValid());
        return LIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST(index == 0);
    
    // Verify Iterate will terminate early
    __block const SubClass * p = NULL;
    list.Iterate(^bool(const SubClass * _Nonnull node) {
        p = node;
        TEST(node->IsValid());
        return LIFOLinkedList<SubClass>::kIterateStop;
    });
    TEST( p == list.GetHead());
    
    // Test Push and pop
    SubClassLIFO list2;
    for( index = listSize; index;)
    {
        index--;
        SubClass * node = list.Pop();
        TEST(node->IsValid());
        TEST( node->GetValue() == index );    // make sure we have the right node
        list2.Push(node);
        TEST( list2.GetHead() == node);       // make sure we had the right node at head in the new list
    }
    TEST( listSize == list2.GetCount());
    
    // Check to make sure the list is now in the right order
    index = 0;
    __block unsigned long count = 0;
    list2.Iterate(^bool(const SubClass * _Nonnull node) {
        TEST(node->IsValid());
        TEST( node->GetValue() == index++);
        count++;
        return LIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST( count == list2.GetCount());
    
    // Reverse the order
    list2.Reverse();
    // Check to make sure the list is now in the reverse order
    index = listSize;
    list2.Iterate(^bool(const SubClass * _Nonnull node) {
        TEST(node->IsValid());
        --index;
        TEST(node->GetValue() == index);
        return LIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST(index == 0);
    
    // Try constructor with an node list
    SubClassLIFO list3(list2.StealList());

    // Check to make sure the list is now in the reverse order
    index = listSize;
    list3.Iterate(^bool(const SubClass * _Nonnull node) {
        TEST(node->IsValid());
        --index;
        TEST(node->GetValue() == index);
        return LIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST(index == 0);
    
    SubClassLIFO list4;
    list4.Push(list3.StealList());
    index = 0;
    list4.Iterate(^bool(const SubClass * _Nonnull node) {
        TEST(node->IsValid());
        TEST(node->GetValue() == index);
        index++;
        return LIFOLinkedList<SubClass>::kIterateContinue;
    });

    // Check to make sure list2 is empty
    TEST(list2.GetHead() == NULL);
    TEST(list2.GetTail() == NULL);
    TEST(list2.GetCount() == 0);
    TEST(list.GetHead() == NULL);
    TEST(list.GetTail() == NULL);
    TEST(list.GetCount() == 0);

    return 0;
}

int TestFIFO(const unsigned long listSize )
{
    SubClassFIFO list;          // Test default ctor
    
    // add listSize Nodes
    for( unsigned long i = 0; i < listSize; i++)
        list.Enqueue( new SubClass(i) );
    
    // Check the head
    const SubClass * head = list.GetHead();
    if( 0 == listSize)
        TEST(NULL == head);
    else
    {
        TEST(NULL != head);
        TEST(head->IsValid());
        TEST(head->GetValue() == 0);
    }

    // Check the tail
    const SubClass * tail = list.GetTail();
    if( 0 == listSize)
        TEST(NULL == tail);
    else
    {
        TEST(NULL != tail);
        TEST(tail->IsValid());
        TEST(tail->GetValue() == listSize-1);
    }

    // Check head == tail for list size = 1
    if( 1 == listSize)
        TEST( list.GetHead() == list.GetTail());
    
    // Check the count
    TEST( listSize == list.GetCount());
    
    // Check the indices are in the right forward order
    __block unsigned long index = 0;
    list.Iterate(^bool(const SubClass * _Nonnull node) {
        TEST(node->IsValid());
        TEST(node->GetValue() == index);
        index++;
        return FIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST( index == listSize);
    
    // Verify Iterate will terminate early
    __block const SubClass * p = NULL;
    list.Iterate(^bool(const SubClass * _Nonnull node) {
        TEST(node->IsValid());
        p = node;
        return FIFOLinkedList<SubClass>::kIterateStop;
    });
    TEST( p == list.GetHead());
    
    // Test Enqueue and Dequeue
    SubClassFIFO list2;
    for( index = 0; index < listSize; index++)
    {
        SubClass * node = list.Dequeue();
        TEST(node->IsValid());
        TEST( node->GetValue() == index );    // make sure we have the right node
        list2.Enqueue(node);
        TEST( list2.GetTail() == node);       // make sure we had the right node at head in the new list
    }
    TEST( listSize == list2.GetCount());
    
    // Check to make sure the list is now in the right order
    index = 0;
    __block unsigned long count = 0;
    list2.Iterate(^bool(const SubClass * _Nonnull node) {
        TEST(node->IsValid());
        TEST( node->GetValue() == index++);
        count++;
        return FIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST( count == list2.GetCount());
    
    // Reverse the order
    list2.Reverse();
    
    // Check to make sure the list is now in the reverse order
    index = listSize;
    list2.Iterate(^bool(const SubClass * _Nonnull node) {
        --index;
        TEST(node->IsValid());
        TEST(node->GetValue() == index);
        return FIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST(index == 0);
    
    // Try constructor with an node list
    SubClassFIFO list3(list2.StealList());

    // Check to make sure the list is now in the reverse order
    index = listSize;
    list3.Iterate(^bool(const SubClass * _Nonnull node) {
        --index;
        TEST(node->IsValid());
        TEST(node->GetValue() == index);
        return FIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST(index == 0);

    SubClassFIFO list4;
    list4.Enqueue(list3.StealList());
    index = listSize;
    list4.Iterate(^bool(const SubClass * _Nonnull node) {
        --index;
        TEST(node->IsValid());
        TEST(node->GetValue() == index);
        return FIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST(index == 0);

    SubClassFIFO list5;
    list4.Enqueue(list4);
    index = listSize;
    list4.Iterate(^bool(const SubClass * _Nonnull node) {
        --index;
        TEST(node->IsValid());
        TEST(node->GetValue() == index);
        return FIFOLinkedList<SubClass>::kIterateContinue;
    });
    TEST(index == 0);

    // Check to make sure lists are empty
    TEST(list3.GetHead() == NULL);
    TEST(list3.GetTail() == NULL);
    TEST(list3.GetCount() == 0);
    TEST(list2.GetHead() == NULL);
    TEST(list2.GetTail() == NULL);
    TEST(list2.GetCount() == 0);
    TEST(list.GetHead() == NULL);
    TEST(list.GetTail() == NULL);
    TEST(list.GetCount() == 0);

    return 0;
}

int TestAtomic( int numThreads)
{
    SubClassAtomicLIFO list;
    
    constexpr unsigned long runLength = 1024;
    __block int result = 0;
    SubClassAtomicLIFO * listP = &list;
    dispatch_apply(numThreads, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^(size_t iteration) {
        printf( "[");   fflush(stdout);
        for( unsigned long i = 0; i < runLength; i++)
            listP->Push( new SubClassAtomic(iteration * runLength + i) );       // using list directly would be const here
        printf( "]");   fflush(stdout);
    });
    
    unsigned long count = numThreads * runLength;
    unsigned long i = 0;
    SubClassAtomic * contents = list.StealList();
    TEST(0 == numThreads || NULL != contents);
    for( SubClassAtomic * item = contents; item; item = item->GetNext())
        i++;
    TEST( i == count);
    
    // Put the contents back on the list
    list.Push(contents);
    TEST(NULL == contents || contents->GetNext() == NULL);      // contents should now be the last item on the list
    
    SubClassAtomic * * array = (SubClassAtomic**) calloc( count, sizeof(array[0]));
    dispatch_apply(numThreads, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^(size_t iteration) {
        for( unsigned long i = 0; i < runLength; i++ )
        {
            SubClassAtomic * item = listP->Pop();   // using list directly would be const here
            assert(item);                           // make sure we did not prematurely run out of items
            assert(NULL == item->GetNext());        // make sure we correctly set the next pointer to NULL in pop
            unsigned long value = item->GetValue();
            assert(array[value] == NULL);           // check for no duplicate items
            array[value] = item;
        }
    });
    TEST( list.StealList() == NULL);

    // Verify all nodes are present
    for( unsigned long i = 0; i < count; i++)
    {
        TEST(array[i] != NULL);
        TEST(array[i]->GetNext() == NULL);
        delete array[i];
        array[i] = NULL;
    }
    free(array);

    // Make sure we chain delete nodes
    for( unsigned long i = 0; i < 3; i++)
        list.Push( new SubClassAtomic(i) );       // Will be reported as a leak if it doesn't get deleted automatically as list leaves scope

    // call automatic list destructor
    return result;
}

#pragma mark -

static void DetectLeaks()
{
    char cmd[256];  snprintf(cmd, sizeof(cmd),  "/usr/bin/leaks %d\n", getpid());
    int hasLeaks = system(cmd);
    
    assert( 0 == hasLeaks);     // If we stopped here, the code is leaking memory. See debug console for leaks report.
    // Try turning on Malloc Stack logging in the Xcode Scheme for better backtrace info in the leaks report
}


int main(int argc, const char * argv[])
{
    atexit( DetectLeaks );
    
    int error = 0;
    for( int i = 0; i <= 100; i++)
        if( (error = TestLIFO(i)) )
            return error;
    
    for( int i = 0; i <= 100; i++)
        if( (error = TestFIFO(i)) )
            return error;

    for( int i = 0; i <= 100; i++)
        if( (error = TestAtomic(i)) )
            return error;

    return 0;
}
