//
//  LinkedList.cpp
//  LinkedLists
//
//  Created by Ian Ollmann on 5/19/25.
//


#define UNUSED  __attribute__ ((unused))
#define ALWAYS_USE_RESULT   __attribute__((warn_unused_result))

#include "Subclass.hpp"


class SubClassAtomic : public LinkedListNodeAtomic<SubClassAtomic>
{
protected:
    unsigned long   value;
    bool            isValid;

public:
    /*! @abstract return an integer less than, equal to, or greater than zero if the first argument is considered to be respectively less than, equal to, or greater than the second. */
    static int Compare( const void * __nonnull _a,
                        const void * __nonnull _b)
    {
        const SubClassAtomic * a = static_cast<const SubClassAtomic *>(_a);
        const SubClassAtomic * b = static_cast<const SubClassAtomic *>(_b);
        return  a->GetValue() < b->GetValue() ? -1 : a->GetValue() > b->GetValue();
    }
    
    SubClassAtomic( unsigned long v) : value(v), isValid(true){}
    SubClassAtomic( const SubClassAtomic & s) : LinkedListNodeAtomic<SubClassAtomic>(), value(s.value){ assert( s.IsValid()); }
    ~SubClassAtomic(){value = INT_MIN; isValid = false;}
    
    inline unsigned long GetValue() const { return value; }
    inline bool IsValid() const { return isValid; }
};
typedef LIFOLinkedListAtomic<SubClassAtomic>    SubClassAtomicLIFO;

#warning  Using Daddy's implementations!
template <typename ClassType>
LinkedListNode<ClassType>::LinkedListNode() : next(NULL){}

template <typename ClassType>
LinkedListNode<ClassType>::LinkedListNode(const LinkedListNode & node) : LinkedListNode(){}

template <typename ClassType>
LinkedListNode<ClassType>::~LinkedListNode(){ delete next; next = NULL; }

template <typename ClassType>
ClassType * ALWAYS_USE_RESULT __nullable LinkedListNode<ClassType>::GetNext() const{ return next;}

template <typename ClassType>
ClassType * ALWAYS_USE_RESULT __nullable LinkedListNode<ClassType>::SwapNext( ClassType * __nullable newValue)
{
    ClassType * oldValue = next;
    next = newValue;
    return oldValue;
}

#pragma mark - LIFO

template <typename ClassType>
LIFOLinkedList<ClassType>::LIFOLinkedList() : list(NULL){}

template <typename ClassType>
LIFOLinkedList<ClassType>::LIFOLinkedList(ClassType * __nullable nodes) : list(nodes){}

template <typename ClassType>
LIFOLinkedList<ClassType>::~LIFOLinkedList(){ delete list; list = NULL; }

template <typename ClassType>
inline bool LIFOLinkedList<ClassType>::Contains(ClassType * __nullable node) const
{
    for( const ClassType * n = list; n; n = n->GetNext())
        if( node == n)
            return true;
    return false;
}

template <typename ClassType>
inline unsigned long LIFOLinkedList<ClassType>::GetCount() const
{
    unsigned long count = 0;
    for( const ClassType * n = list; n; n = n->GetNext())
        count++;
    return count;
}

template <typename ClassType>
inline const ClassType * __nullable ALWAYS_USE_RESULT LIFOLinkedList<ClassType>::GetHead() const { return list;}

template <typename ClassType>
inline const ClassType * __nullable ALWAYS_USE_RESULT LIFOLinkedList<ClassType>::GetTail() const
{
    ClassType * __nullable result = NULL;
    for( ClassType * n = list; n; n = n->GetNext())
        result = n;
    return result;
}

template <typename ClassType>
inline ClassType * __nullable ALWAYS_USE_RESULT LIFOLinkedList<ClassType>::Pop()
{
    ClassType * result = list;
    if(result)
        list = result->SwapNext(NULL);
    return result;
}

template <typename ClassType>
inline void LIFOLinkedList<ClassType>::Push(ClassType * __nullable newNodes )
{
    if(NULL == newNodes)
        return;
    
    ClassType * temp = NULL;
    ClassType * head = list;
    ClassType * n = newNodes;
    while( n )
    {
        temp = n;
        n = n->SwapNext(head);
        head = temp;
    }
    list = head;
}

template <typename ClassType>
inline void LIFOLinkedList<ClassType>::Push(LIFOLinkedList<ClassType> & list2 )
{
    ClassType * n = list2.StealList();
    while(n)
    {
        ClassType * current = n;
        n = current->SwapNext(list);
        list = current;
    }
}

/*! @abstract Reverse the order of the list */
template <typename ClassType>
inline void LIFOLinkedList<ClassType>::Reverse()
{
    ClassType * newList = NULL;
    ClassType * oldList = list;
    while(oldList)
    {
        ClassType * node = oldList;
        oldList = oldList->SwapNext(newList);
        newList = node;
    }
    list = newList;
}

/*! @abstract Steal the list nodes. List becomes empty and a naked linked list of the old nodes is returned out the left hand side */
template <typename ClassType>
inline ClassType * __nullable ALWAYS_USE_RESULT LIFOLinkedList<ClassType>::StealList()
{
    ClassType * result = list;
    list = NULL;
    return result;
}

template <typename ClassType>
inline void LIFOLinkedList<ClassType>::Iterate( bool(^ __nonnull block)(const ClassType * __nonnull node)) const
{
    for(ClassType * n = list; n; n = n->GetNext() )
        if(block(n))
            break;
}


#pragma mark - FIFO


template <typename T>
FIFOLinkedList<T>::FIFOLinkedList() : head(NULL), tail(NULL){}

template <typename T>
FIFOLinkedList<T>::FIFOLinkedList(T * __nullable nodes) : head(nodes), tail(NULL)
{ for( T * p = head; p; p = p->GetNext()) tail = p; }

template <typename T>
FIFOLinkedList<T>::~FIFOLinkedList(){ delete head; head = tail = NULL;}

template <typename T>
unsigned long FIFOLinkedList<T>::GetCount() const
{
    unsigned long result = 0;
    for( T * p = head; p; p = p->GetNext())
        result++;
    return result;
}

template <typename T>
inline const T * __nullable ALWAYS_USE_RESULT FIFOLinkedList<T>::GetHead() const { return head; }

template <typename T>
inline const T * __nullable ALWAYS_USE_RESULT FIFOLinkedList<T>::GetTail() const { return tail; }

template <typename T>
inline T * __nullable ALWAYS_USE_RESULT FIFOLinkedList<T>::Dequeue()
{
    T * result = head;
    if( NULL == result)
        return result;
    
    head = result->SwapNext(NULL);
    if(NULL == head )
        tail = NULL;
    return result;
}

template <typename T>
inline void FIFOLinkedList<T>::Enqueue(T * __nullable newNodes )
{
    
    T * newTail = NULL;
    for( T * p = newNodes; p; p = p->GetNext())
        newTail = p;
    
    if( NULL == newTail)
        return;

    T * nullPtr = NULL;
    if( NULL == tail)
    {
        head = newNodes;
        tail = newTail;
        return;
    }

    nullPtr = tail->SwapNext(newNodes);
    assert(NULL == nullPtr);
    tail = newTail;
}

template <typename T>
inline void FIFOLinkedList<T>::Enqueue(FIFOLinkedList<T> & list ){ return Enqueue(list.StealList()); }
 

template <typename T>
inline void FIFOLinkedList<T>::Iterate( bool(^ __nonnull block)(const T * __nonnull node)) const
{
    for( const T * p = head; p; p = p->GetNext())
        if( FIFOLinkedList<T>::kIterateStop == block(p) )
            return;
}

template <typename T>
inline void FIFOLinkedList<T>::Reverse()
{
    T * newHead = NULL;
    T * current = head;
    T * newTail = head;
    while( current)
    {
        head = current->SwapNext(newHead);
        newHead = current;
        current = head;
    }
    
    tail = newTail;
    head = newHead;
}

template <typename T>
inline T * __nullable FIFOLinkedList<T>::StealList()
{
    T * result = head;
    head = tail = NULL;
    return result;
}

#pragma mark - Atomic
template <typename T>
LinkedListNodeAtomic<T>::LinkedListNodeAtomic(){ atomic_store_explicit( &next, NULL, std::memory_order_release); }

template <typename T>
LinkedListNodeAtomic<T>::LinkedListNodeAtomic(const LinkedListNodeAtomic<T> & node){ atomic_store_explicit( &next, &node, std::memory_order_release); }

template <typename T>
LinkedListNodeAtomic<T>::~LinkedListNodeAtomic(){ delete SwapNext(NULL); }

/*! @abstract  Return the next item in the list */
template <typename T>
inline T * ALWAYS_USE_RESULT __nullable LinkedListNodeAtomic<T>::GetNext() const{ return std::atomic_load_explicit( &next, std::memory_order_acquire); }

/*! @abstract  Swap the next item in the list for a new value. Return the old value. */
template <typename T>
inline T * ALWAYS_USE_RESULT __nullable LinkedListNodeAtomic<T>::SwapNext( T * __nullable newValue){ return atomic_exchange_explicit( &next, newValue, std::memory_order_acq_rel ); }



template <typename T>
LIFOLinkedListAtomic<T>::LIFOLinkedListAtomic(){ atomic_store_explicit( &list, NULL, std::memory_order_release); }

template <typename T>
LIFOLinkedListAtomic<T>::~LIFOLinkedListAtomic(){ delete StealList();}

template <typename T>
T * __nullable ALWAYS_USE_RESULT LIFOLinkedListAtomic<T>::ReverseList( T * __nullable nodes )
{
    T * newList = NULL;
    while( nodes )
    {
        T * node = nodes;
        nodes = nodes->SwapNext(newList);
        newList = node;
    }
    
    return newList;
}

template <typename T>
inline T * __nullable ALWAYS_USE_RESULT LIFOLinkedListAtomic<T>::Pop()
{
    T * oldHead;
    T * newHead;
    
#if USE_SINGLE_PASS_ATOMICS
    do
    {
        oldHead = atomic_load_explicit( &list, std::memory_order_acquire);
        if(NULL == oldHead)
            return NULL;
        
        newHead = oldHead->GetNext();   // BUG: if old head is already deleted this can crash;
                                        //      also ABA problem https://en.wikipedia.org/wiki/ABA_problem
    } while (! atomic_compare_exchange_weak_explicit(&list, &oldHead, newHead, std::memory_order_acq_rel, std::memory_order_relaxed));
    
    T * UNUSED unused = oldHead->SwapNext(NULL);
    return oldHead;
#else
    do
    { // reserve the pointer to prevent ABA
        oldHead = GetHead();
        if(NULL == oldHead)
            return NULL;
    }while (! atomic_compare_exchange_weak_explicit(&list, &oldHead, const_cast<T*>(kReservedNode), std::memory_order_acq_rel, std::memory_order_relaxed));
    
    assert(oldHead != kReservedNode && oldHead != NULL);
    newHead = oldHead->SwapNext(NULL);
    bool success = atomic_compare_exchange_strong_explicit( &list, const_cast<T**>(&kReservedNode), newHead, std::memory_order_acq_rel, std::memory_order_relaxed);
    assert(success);
    return oldHead;
#endif
}

template <typename T>
const T * __nonnull  LIFOLinkedListAtomic<T>::kReservedNode = (const T *) 1L;

template <typename T>
inline void LIFOLinkedListAtomic<T>::Push(T * __nullable newNodes )
{
    if(NULL == newNodes)
        return;
    
    // reverse the list
    T * newTail = newNodes;
    T * newHead = NULL;
    while( newNodes )
    {
        T * currentNode = newNodes;
        newNodes = newNodes->SwapNext(newHead);
        newHead = currentNode;
    }

    T * oldHead;
#if USE_SINGLE_PASS_ATOMICS
    do
    {
        oldHead = atomic_load_explicit( &list, std::memory_order_acquire);
        T * UNUSED ignored = newTail->SwapNext(oldHead);   // BUG: two threads could be doing this at the same time. If the wrong one succeeds on the next line we may have list corruption.
    }while( ! atomic_compare_exchange_weak_explicit(&list, &oldHead, newHead, std::memory_order_acq_rel, std::memory_order_relaxed));
#else
    // reserve the atomic list
    do
    { // reserve the pointer to prevent ABA
        oldHead = GetHead();
    }while (! atomic_compare_exchange_weak_explicit(&list, &oldHead, const_cast<T*>(kReservedNode), std::memory_order_acq_rel, std::memory_order_relaxed));
    
    T * unused = newTail->SwapNext( oldHead );
    assert( NULL == unused);
    
    bool success = atomic_compare_exchange_strong_explicit( &list, const_cast<T**>(&kReservedNode), newHead, std::memory_order_acq_rel, std::memory_order_relaxed);
    assert(success);
#endif
    
}

template <typename T>
inline T * __nullable ALWAYS_USE_RESULT LIFOLinkedListAtomic<T>::StealList()
{
    T * oldHead;
#if USE_SINGLE_PASS_ATOMICS
    oldHead = atomic_exchange_explicit( &list, NULL, std::memory_order_acq_rel );
#else
    do
    { // reserve the pointer to prevent ABA
        if( NULL == (oldHead = GetHead()))
            return NULL;
    }while (! atomic_compare_exchange_weak_explicit(&list, &oldHead, const_cast<T*>(kReservedNode), std::memory_order_acq_rel, std::memory_order_relaxed));
    
    bool success = atomic_compare_exchange_strong_explicit( &list, const_cast<T**>(&kReservedNode), NULL, std::memory_order_acq_rel, std::memory_order_relaxed);
    assert(success);
#endif
    return oldHead;
}

SubClassAtomic * __nullable SortList( SubClassAtomic * __nullable list )
{
    if( NULL == list )
        return NULL;
    
    unsigned long count = 0;
    for( SubClassAtomic * node = list; node; node = node->GetNext())
        count++;
    
    SubClassAtomic * * array = (SubClassAtomic **) calloc( count, sizeof(array[0]));
    if( NULL == array )
        return list;        // sort failed
    
    for( unsigned long i = 0; i < count; i++ )
    {
        assert(list);
        array[i] = list;
        list = list->SwapNext(NULL);
    }
    assert(NULL == list);
    
    qsort(array, count, sizeof(array[0]), SubClassAtomic::Compare );
    
    while(count)
    {
        --count;
        assert( NULL != array[count]);
        SubClassAtomic * nullValue = array[count]->SwapNext(list);
        assert(NULL == nullValue);
        list = array[count];
        array[count] = NULL;
    }
    
    free(array);
    return list;
}
