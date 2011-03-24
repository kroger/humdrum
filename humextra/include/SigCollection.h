//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Feb  5 19:42:53 PST 1997
// Last Modified: Tue Apr 22 20:28:16 GMT-0800 1997
// Last Modified: Fri Sep 14 15:50:52 PDT 2001 (added last() function)
// Last Modified: Wed Sep  8 17:18:15 PDT 2010 (added getGrowth())
// Filename:      ...sig/maint/code/base/SigCollection/SigCollection.h
// Web Address:   http://sig.sapp.org/include/sigBase/SigCollection.h
// Documentation: http://sig.sapp.org/doc/classes/SigCollection
// Syntax:        C++ 
//
// Description:   A dynamic array which can grow as necessary.
//                This class can hold any type of item, but the
//                derived Array class is specifically for collections
//                of numbers.
//

#ifndef _SIGCOLLECTION_H_INCLUDED
#define _SIGCOLLECTION_H_INCLUDED

// Name change to avoid namespace collision with an Apple typedef
//#define SigCollection Collection

template<class type>
class SigCollection {
   public:
                SigCollection     (void);
                SigCollection     (int arraySize);
                SigCollection     (int arraySize, type *aCollection);
                SigCollection     (SigCollection<type>& aCollection);
               ~SigCollection     ();

      void      allowGrowth       (int status = 1);
      void      append            (type& element);
      void      appendcopy        (type element);
      void      append            (type* element);
      type     *getBase           (void) const;
      long      getAllocSize      (void) const;
      long      getSize           (void) const;
      type     *pointer           (void);
      void      setAllocSize      (long aSize);
      void      setGrowth         (long growth);
      long      getGrowth         (void) { return growthAmount; }
      void      setSize           (long newSize);
      type&     operator[]        (int arrayIndex);
      type      operator[]        (int arrayIndex) const;
      void      grow              (long growamt = -1);
      type&     last              (void);
      int       increase          (int addcount = 1);


   protected:
      long      size;             // actual array size
      long      allocSize;        // maximum allowable array size
      type     *array;            // where the array data is stored
      char      allowGrowthQ;     // allow/disallow growth
      long      growthAmount;     // number of elements to grow by if index
				  //    element one beyond max size is accessed
      long maxSize;               // the largest size the array is allowed 
                                  //    to grow to, if 0, then ignore max
  
      void      shrinkTo          (long aSize);
};


#include "SigCollection.cpp"



#endif  /* _SIGCOLLECTION_H_INCLUDED */



// md5sum: 01bec04835c0bd117f40c2bfe51c4abd SigCollection.h [20030102]
