//
//  Subclass.hpp
//  LinkedLists
//
//  Created by Ian Ollmann on 5/19/25.
//

#ifndef SUBCLASS_HPP
#define SUBCLASS_HPP    1


#ifdef DEBUG
#else
#   define NDEBUG 1
#endif
#include <assert.h>

#include "LinkedList.hpp"
#include <limits.h>

class SubClass : public LinkedListNode<SubClass>
{
protected:
    unsigned long   value;
    bool            isValid;

public:
    /*! @abstract return an integer less than, equal to, or greater than zero if the first argument is considered to be respectively less than, equal to, or greater than the second. */
    static int Compare( const SubClass & a, const SubClass & b){ return  a.value < b.value ? -1 : a.value > b.value; }
    
    SubClass( unsigned long v) : value(v), isValid(true){}
    SubClass( SubClass & s) : LinkedListNode<SubClass>(s), value(s.value){ assert( s.IsValid()); }
    ~SubClass(){value = INT_MIN; isValid = false;}
    
    inline unsigned long GetValue() const { return value; }
    inline bool IsValid() const { return isValid; }
};
typedef LIFOLinkedList<SubClass>    SubClassLIFO;
typedef FIFOLinkedList<SubClass>    SubClassFIFO;


#endif  /* SUBCLASS_HPP */

