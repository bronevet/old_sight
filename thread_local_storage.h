#pragma once
#include <pthread.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include <assert.h>
//#include <fstream>
using namespace std;

// Based on http://alex.tapmania.org/2011/03/simple-thread-local-storage-with-pthreads.html
// This class encapsulates the use of thread local storage for global, function static and class static data.
// The advantage of the ThreadLocalStorage class over thread__ is that the latter does not
// work with classes since it does not call the constructor of the object declared thread__ when a 
// new thread is created (the object pointers are different for different threads but 
// construction/destruction does not occur).
// Use:
//          | Single-thread                        |ThreadLocalStorage
// ---------|--------------------------------------|-----------------------------------------------------------------
// Global   | ClassType varA;                      | ThreadLocalStorage0<ClassType> varA;
// Global   | ClassType varB(5);                   | ThreadLocalStorage1<ClassType, int> varB(15);                   
// Global   | ClassType varC(5, "hi");             | ThreadLocalStorage2<ClassType, int, std::string> varC(15, "hi");
// ---------|--------------------------------------|-----------------------------------------------------------------
// Function | void func() {                        | void func() {
// Static   |   ClassType varA;                    |   ThreadLocalStorage0<ClassType, int> varA();
//          |   ClassType varB(5);                 |   ThreadLocalStorage1<ClassType, int> varB(15);
//          |   ClassType varC(5, "hi");           |   ThreadLocalStorage2<ClassType, int, std::string> varC(15, "hi");
//          | }                                    | }
// ---------|--------------------------------------|-----------------------------------------------------------------
// Class    | class SomeClass {                    | class SomeClass {
// Static   |   ClassType varA;                    |   static ThreadLocalStorage0<ClassType, int> varA;
//          |   ClassType varB;                    |   static ThreadLocalStorage1<ClassType, int> varB;
//          |   ClassType varC;                    |   static ThreadLocalStorage2<ClassType, int, std::string> varC;
//          | };                                   | };
//          | ClassType SomeClalss::varA;          | ThreadLocalStorage0<ClassType, int> SomeClalss::varA;                     
//          | ClassType SomeClalss::varB(5);       | ThreadLocalStorage1<ClassType, int> SomeClalss::varB(15);                   
//          | ClassType SomeClalss::varC(5, "hi"); | ThreadLocalStorage2<ClassType, int, std::string> SomeClalss::varC(15, "hi");

template<typename T>
class ThreadLocalStorage
{
  private:
  pthread_key_t   key_;
  
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage(const ThreadLocalStorage& that) {}

  public:
  // Function called when a thread finishes to deallocate the thread-local copy of the object
  static void destructor(void* p) {
    //cout << "ThreadLocalStorage::destructor("<<p<<")"<<endl;
    delete (T*)p;  
  }
    
  ThreadLocalStorage()
  {
    pthread_key_create(&key_, &ThreadLocalStorage::destructor);
//    cout << "ThreadLocalStorage::ThreadLocalStorage key="<<key_<<", this="<<this<<endl;
  }

  /*ThreadLocalStorage(const T& val)
  {
    pthread_key_create(&key_, &ThreadLocalStorage::destructor);
    pthread_setspecific(key_, const_cast<ThreadLocalStorage<T>*>(this)->allocate(val));
  }*/
  ThreadLocalStorage(const T& val) {
    pthread_key_create(&key_, &ThreadLocalStorage::destructor);
//    cout << "ThreadLocalStorage::ThreadLocalStorage(val) key="<<key_<<endl;
    *get() = val;
  }
  
  /*ThreadLocalStorage(const ThreadLocalStorage<T>& that) : key_(that.key_) {
    cout << "ThreadLocalStorage copy constructor key_"<<key_<<endl;
  } */

  ~ThreadLocalStorage()
  {
//    cout << "~ThreadLocalStorage deleting key="<<key_<<endl;
    pthread_key_delete(key_);
  }
  
  /* // Initializes the thread local storage for the current thread with the given object. It will be 
  // deallocated automatically when the thread exits (the caller must not deallocate).
  ThreadLocalStorage& operator =(T* p)
  {
    // Only one object may be associated with a thread
    assert(pthread_getspecific(key_)==NULL);
    pthread_setspecific(key_, p);
    return *this;
  }*/
  /*T& operator =(T val)
  {
    cout << "ThreadLocalStorage operator=(T val)"<<endl;
    *((T*)get()) = val;
    return *get();
  }  T& operator =(T& val)
  {
    cout << "ThreadLocalStorage operator=(T& val)"<<endl;
    *((T*)get()) = val;
    return *get();
  }
  T& operator =(const T& val)
  {
    cout << "ThreadLocalStorage operator=(const T& val)"<<endl;
    *((T*)get()) = val;
    return *get();
  }*/
  

  // Allocates a new instance of an object of type T and returns it. Since
  // the arguments to the new call will vary across use-cases, this method is
  // not implemented here but rather in classes derive from this one, which 
  // implement variants for different argument options. Below are some classes of this
  // type that allow the called to provide these arguments in the constructor
  // and cache them. When user need to use more arguments than we've provided
  // classes for, they need to write their own class that extends ThreadLocalStorage and
  // implements allocate().
  virtual T* allocate()=0;

  // Ensure that the thread-specific instance of the object is initialized. This method will be 
  // called automatically before the user can access the thread-local object or may be called
  // directly by the user.
  void tlsInit() const {
    //cout << "    tlsInit("<<key_<<") before "<<pthread_getspecific(key_)<<endl;
    if(pthread_getspecific(key_)==NULL) {
      T* buf = const_cast<ThreadLocalStorage<T>*>(this)->allocate();
      pthread_setspecific(key_, buf);
      //cout << pthread_self()<<":        buf="<<buf<<", key_="<<key_<<endl;//<<", pthread_getspecific(key_)="<<pthread_getspecific(key_)<<endl;
//      pthread_setspecific(key_, const_cast<ThreadLocalStorage<T>*>(this)->allocate());
      assert(pthread_getspecific(key_) == buf);
    }
    //cout << "    tlsInit("<<key_<<") after "<<pthread_getspecific(key_)<<endl;
  }
  
  // Returns whether a value is currently mapped for this thread or not
  bool isValueMappedForThread() {
    return pthread_getspecific(key_)!=NULL;
  }

  // Operators to access the thread-local data
  T* operator ->()
  {
    tlsInit();
    return static_cast<T*>(pthread_getspecific(key_));
  }

  T& operator *()
  {
    //cout << "operator *\n";
    tlsInit();
    return *static_cast<T*>(pthread_getspecific(key_));
  }

  T* operator &()
  {
    //cout << "operator &\n";
    tlsInit();
    return static_cast<T*>(pthread_getspecific(key_));
  }

  /*operator T() const {
    tlsInit();
    return *static_cast<T*>(pthread_getspecific(key_));
  }*/

  operator T&() {
    //cout << "cast to T&\n";
    tlsInit();
    return *static_cast<T*>(pthread_getspecific(key_));
  }
 
  operator const T&() const {
    //cout << "cast to const T&\n";
    tlsInit();
    return *static_cast<T*>(pthread_getspecific(key_));
  }

  operator T*()
  {
    //cout << "cast to T*\n";
    tlsInit();
    return static_cast<T*>(pthread_getspecific(key_));
  }

  const T* get() const
  {
    //cout << "cast to const T* get\n";
    tlsInit();
    return static_cast<T*>(pthread_getspecific(key_));
  }
  
  T* get()
  {
    //cout << "cast to T* get\n";
    tlsInit();
    return static_cast<T*>(pthread_getspecific(key_));
  }

  // Assignment operator
  template<typename argT>
  T& operator =(const argT& val) {
    (*get()) = val;
    return *get();
  }
  
  // Macro that holds the standard implementation of an assignment operator
  // that should be used by derived classes. Derived classes must provide
  // their own implementation of this operator since if they provide a copy 
  // constructor the assignment operator is not inherited.
  #define TLS_ASSIGNMENT_OPERATOR                   \
    template <typename argT>                        \
    T& operator =(const argT& val)                  \
    {                                               \
      *((T*)ThreadLocalStorage<T>::get()) = (T)val; \
      return *ThreadLocalStorage<T>::get();         \
    }
 
  // Arithmetic operators
  template<typename argT>
  T operator+(const argT& right) const 
  { return (*get()) + right; }

  template<typename argT>
  T operator-(const argT& right) const 
  { return (*get()) - right; }

  template<typename argT>
  T operator*(const argT& right) const 
  { return (*get()) * right; }

  template<typename argT>
  T operator/(const argT& right) const 
  { return (*get()) / right; }

  // Pre-increment
  T operator++() {
    (*get())++;
    return *get();
  }

  // Post-increment
  T operator++(int) {
    T origVal = *get();
    (*get())++;
    return origVal;
  }

  // Pre-decrement
  T operator--() {
    (*get())--;
    return *get();
  }

  // Post-decrement
  T operator--(int) {
    T origVal = *get();
    (*get())--;
    return origVal;
  }

  // Update operators
  template<typename argT>
  T& operator+=(const argT& right) { 
    (*get()) += right; 
    return *get();
  }

  template<typename argT>
  T& operator-=(const argT& right) { 
    (*get()) -= right; 
    return *get();
  }

  template<typename argT>
  T& operator*=(const argT& right) { 
    (*get()) *= right; 
    return *get();
  }

  template<typename argT>
  T& operator/=(const argT& right) { 
    (*get()) /= right; 
    return *get();
  }

  // Relational operators
  template<typename argT>
  bool operator==(const argT& right) const 
  { return (*get()) == (const T&)right; }

  template<typename argT>
  bool operator!=(const argT& right) const 
  { return (*get()) != (const T&)right; }

  template<typename argT>
  bool operator<(const argT& right) const 
  { return (*get()) < (const T&)right; }

  template<typename argT>
  bool operator<=(const argT& right) const 
  { return (*get()) <= (const T&)right; }

  template<typename argT>
  bool operator>(const argT& right) const 
  { return (*get()) > (const T&)right; }

  template<typename argT>
  bool operator>=(const argT& right) const 
  { return (*get()) <= (const T&)right; }
}; // class ThreadLocalStorage

template<typename T>
std::ostream& operator<< (std::ostream& stream, const ThreadLocalStorage<T>& d) {
  stream << *(d.get());
  return stream;
}

// Variants of ThreadLocalStorage that are focused on constructing instances of type T
// with between 0 and 9 arguments to the constructor. These arguments are provided to the
// constructor of ThreadLocalStorage and then reused to create thread-specific instances of T.
template<typename T>
class ThreadLocalStorage0: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage0(const ThreadLocalStorage0<T>& that) {}

  public:
  ThreadLocalStorage0() {}
  ThreadLocalStorage0(const T& val): ThreadLocalStorage<T>(val) {}
  //ThreadLocalStorage0(const ThreadLocalStorage0<T>& that) : ThreadLocalStorage<T>(that) {} 
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(); }
}; // class ThreadLocalStorage0

template<typename T, typename T1>
class ThreadLocalStorage1: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage1(const ThreadLocalStorage1<T, T1>& that) {}

  T1 arg1;
  
  public:
  
  ThreadLocalStorage1(const T1& arg1): arg1(arg1) {}
  //ThreadLocalStorage1(const ThreadLocalStorage1<T, T1>& that) : ThreadLocalStorage<T>(that), arg1(that.arg1) {} 
  // For the case where T!=T1 we need another constructor to handler assinment var=val
  //ThreadLocalStorage1(const T& arg1): arg1(arg1) {}

  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1); }
}; // class ThreadLocalStorage1

template<typename T, typename T1, typename T2>
class ThreadLocalStorage2: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage2(const ThreadLocalStorage2<T, T1, T2>& that) {}
  
  T1 arg1;
  T2 arg2;
  public:
  ThreadLocalStorage2(const T1& arg1, const T2& arg2): arg1(arg1), arg2(arg2) {}
  ThreadLocalStorage2(const T& val): ThreadLocalStorage<T>(val) {}
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1, arg2); }
}; // class ThreadLocalStorage2

template<typename T, typename T1, typename T2, typename T3>
class ThreadLocalStorage3: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage3(const ThreadLocalStorage3<T, T1, T2, T3>& that) {}
  
  T1 arg1;
  T2 arg2;
  T3 arg3;
  public:
  ThreadLocalStorage3(const T1& arg1, const T2& arg2, const T3& arg3): arg1(arg1), arg2(arg2), arg3(arg3) {}
  ThreadLocalStorage3(const T& val): ThreadLocalStorage<T>(val) {}
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1, arg2, arg3); }
}; // class ThreadLocalStorage3

template<typename T, typename T1, typename T2, typename T3, typename T4>
class ThreadLocalStorage4: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage4(const ThreadLocalStorage4<T, T1, T2, T3, T4>& that) {}
  
  T1 arg1;
  T2 arg2;
  T3 arg3;
  T4 arg4;
  public:
  ThreadLocalStorage4(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4): arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4) {}
  ThreadLocalStorage4(const T& val): ThreadLocalStorage<T>(val) {}
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1, arg2, arg3, arg4); }
}; // class ThreadLocalStorage4

template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
class ThreadLocalStorage5: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage5(const ThreadLocalStorage5<T, T1, T2, T3, T4, T5>& that) {}
  
  T1 arg1;
  T2 arg2;
  T3 arg3;
  T4 arg4;
  T5 arg5;
  public:
  ThreadLocalStorage5(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5): arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4), arg5(arg5) {}
  ThreadLocalStorage5(const T& val): ThreadLocalStorage<T>(val) {}
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1, arg2, arg3, arg4, arg5); }
}; // class ThreadLocalStorage5


template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class ThreadLocalStorage6: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage6(const ThreadLocalStorage6<T, T1, T2, T3, T4, T5, T6>& that) {}
  
  T1 arg1;
  T2 arg2;
  T3 arg3;
  T4 arg4;
  T5 arg5;
  T6 arg6;
  public:
  ThreadLocalStorage6(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6): arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4), arg5(arg5), arg6(arg6) {}
  ThreadLocalStorage6(const T& val): ThreadLocalStorage<T>(val) {}
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1, arg2, arg3, arg4, arg5, arg6); }
}; // class ThreadLocalStorage6

template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class ThreadLocalStorage7: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage7(const ThreadLocalStorage7<T, T1, T2, T3, T4, T5, T6, T7>& that) {}
  
  T1 arg1;
  T2 arg2;
  T3 arg3;
  T4 arg4;
  T5 arg5;
  T6 arg6;
  T7 arg7;
  public:
  ThreadLocalStorage7(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7): arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4), arg5(arg5), arg6(arg6), arg7(arg7) {}
  ThreadLocalStorage7(const T& val): ThreadLocalStorage<T>(val) {}
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
}; // class ThreadLocalStorage7

template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
class ThreadLocalStorage8: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage8(const ThreadLocalStorage8<T, T1, T2, T3, T4, T5, T6, T7, T8>& that) {}
  
  T1 arg1;
  T2 arg2;
  T3 arg3;
  T4 arg4;
  T5 arg5;
  T6 arg6;
  T7 arg7;
  T8 arg8;
  public:
  ThreadLocalStorage8(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8): arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4), arg5(arg5), arg6(arg6), arg7(arg7), arg8(arg8) {}
  ThreadLocalStorage8(const T& val): ThreadLocalStorage<T>(val) {}
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
}; // class ThreadLocalStorage8

template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
class ThreadLocalStorage9: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorage9(const ThreadLocalStorage9<T, T1, T2, T3, T4, T5, T6, T7, T8, T9>& that) {}
  
  T1 arg1;
  T2 arg2;
  T3 arg3;
  T4 arg4;
  T5 arg5;
  T6 arg6;
  T7 arg7;
  T8 arg8;
  T9 arg9;
  public:
  ThreadLocalStorage9(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8, const T9& arg9): arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4), arg5(arg5), arg6(arg6), arg7(arg7), arg8(arg8), arg9(arg9) {}
  ThreadLocalStorage9(const T& val): ThreadLocalStorage<T>(val) {}
  
  TLS_ASSIGNMENT_OPERATOR
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); }
}; // class ThreadLocalStorage9

/*// Specialized type of ThreadLocalStorage that is focused on numeric datatypes
template<typename T>
class ThreadLocalStorageNumeric: public ThreadLocalStorage<T> {
  // Records whether the initial value of this numeric variable was specified
  bool initValKnown;

  // If was, the initial value itself
  T initVal;
  
  public:
  ThreadLocalStorageNumeric(const T& initVal): initVal(initVal) { initValKnown=true; }
  ThreadLocalStorageNumeric() { initValKnown=false; }
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T; }

  // Overload of the bracket operator for array indexing
  T& operator[](unsigned int i)
  { return ((T*)(ThreadLocalStorage<T>::get()))[i]; }

  const T& operator[](unsigned int i) const
  { return ((T*)(ThreadLocalStorage<T>::get()))[i]; }
}; // class ThreadLocalStorageNumeric
*/
// ------------------------------------------------------------------
// ----- Specialized type of ThreadLocalStorage that is focused -----
// ----- on arrays of items of type T.                          -----
// ------------------------------------------------------------------
template<typename T>
class ThreadLocalStorageArray: public ThreadLocalStorage<T> {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorageArray(const ThreadLocalStorageArray<T>& that) {}
  
  // Number of entries in each thread's instance of the array
  size_t size;
  public:
  ThreadLocalStorageArray(size_t size): size(size) {}
  
  // Allocates a new instance of an object of type T and returns it.
  T* allocate() { return new T[size]; }

  // Overload of the bracket operator for array indexing
  T& operator[](unsigned int i)
  { return ((T*)(ThreadLocalStorage<T>::get()))[i]; }

  const T& operator[](unsigned int i) const
  { return ((T*)(ThreadLocalStorage<T>::get()))[i]; }
}; // class ThreadLocalStorageArray

// Specialized type of ThreadLocalStorage that is focused on arrays of items of type std::map.
template<typename valT>
class ThreadLocalStorageSet: public ThreadLocalStorage<std::set<valT> >  {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorageSet(const ThreadLocalStorageSet<valT>& that) {}
  
  public:
  ThreadLocalStorageSet() {}
  
  // Allocates a new instance of an object of type T and returns it.
  std::set<valT>* allocate() { return new std::set<valT>(); }

  // Iterators
  typename std::set<valT>::iterator begin()
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->begin(); }
  
  typename std::set<valT>::const_iterator begin() const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->begin(); }
  
  typename std::set<valT>::iterator end()
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->end(); }
  
  typename std::set<valT>::const_iterator end() const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->end(); }

  // Capacity
  bool empty() const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->empty(); }

  size_t size() const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->size(); }

  size_t max_size() const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->max_size(); }

  // Modifiers
  typename std::pair<typename std::set<valT>::iterator, bool> insert(const valT& val)
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->insert(val); }

  typename std::set<valT>::iterator insert(typename std::set<valT>::iterator position, const valT& val)
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->insert(position, val); }

  template<typename InputIterator>
  void insert(InputIterator first, InputIterator last)
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->insert(first, last); }

  typename std::set<valT>::iterator erase(typename std::set<valT>::const_iterator position)
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->erase(position); }
  
  typename std::set<valT>::size_type erase(const valT& val)
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->erase(val); }

  typename std::set<valT>::iterator erase(typename std::set<valT>::const_iterator first, typename std::set<valT>::const_iterator last)
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->erase(first, last); }

  void swap(std::set<valT>& x)
  { return (*(std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get())).swap(x); }
  
  size_t clear() const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->clear(); }

  // Observers
  typename std::set<valT>::key_compare key_comp()
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->key_comp(); }

  typename std::set<valT>::value_compare value_comp()
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->value_comp(); }

  // Operations
  typename std::set<valT>::iterator find(const valT& val)
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->find(val); }
 
  typename std::set<valT>::size_type count(const valT& val) const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->count(val); }

  typename std::set<valT>::iterator lower_bound(const valT& val) const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->lower_bound(val); }

  typename std::set<valT>::iterator upper_bound(const valT& val) const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->upper_bound(val); }

  typename std::pair<typename std::set<valT>::iterator, typename std::set<valT>::iterator> equal_range(const valT& val) const
  { return ((std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get()))->equal_range(val); }

  // Allocator
  typename std::set<valT>::allocator_type get_allocator()
  { return (*(std::set<valT>*)(ThreadLocalStorage<std::set<valT> >::get())).get_allocator(); }
}; // class ThreadLocalStorageSet

// Specialized type of ThreadLocalStorage that is focused on arrays of items of type std::map.
template<typename keyT, typename valT>
class ThreadLocalStorageMap: public ThreadLocalStorage<std::map<keyT, valT> >  {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorageMap(const ThreadLocalStorageMap<keyT, valT>& that) {}
  
  public:
  ThreadLocalStorageMap() {}
  
  // Allocates a new instance of an object of type T and returns it.
  std::map<keyT, valT>* allocate() { 
    std::map<keyT, valT>* ret = new std::map<keyT, valT>();
    //cout << "ThreadLocalStorageMap allocated "<<ret<<endl;
    return ret;
  }

  // Overload of the bracket operator for map indexing
  valT& operator[](const keyT& key)
  { return (*(std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))[key]; }

  const valT& operator[](const keyT& key) const
  { return (*(std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))[key]; }
  
  // Iteration methods
  typename std::map<keyT, valT>::iterator find(const keyT& key)
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->find(key); }
  
  typename std::map<keyT, valT>::const_iterator find(const keyT& key) const
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->find(key); }
  
  typename std::map<keyT, valT>::iterator begin()
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->begin(); }
  
  typename std::map<keyT, valT>::const_iterator begin() const
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->begin(); }
  
  typename std::map<keyT, valT>::iterator end()
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->end(); }
  
  typename std::map<keyT, valT>::const_iterator end() const
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->end(); }

  // Content management
  typename std::map<keyT, valT>::iterator erase(typename std::map<keyT, valT>::const_iterator position)
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->erase(position); }
  
  typename std::map<keyT, valT>::size_type erase(const keyT& k)
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->erase(k); }

  typename std::map<keyT, valT>::iterator erase(typename std::map<keyT, valT>::const_iterator first, typename std::map<keyT, valT>::const_iterator last)
  { return ((std::map<keyT, valT>*)(ThreadLocalStorage<std::map<keyT, valT> >::get()))->erase(first, last); }
}; // class ThreadLocalStorageMap

// ------------------------------------------------------------------
// ----- Specialized type of ThreadLocalStorage that is focused -----
// ----- on arrays of items of type std::vector.                -----
// ------------------------------------------------------------------
template<typename valT>
class ThreadLocalStorageVector: public ThreadLocalStorage<std::vector<valT> >  {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorageVector(const ThreadLocalStorageVector<valT>& that) {}
  
  // Records whether the initial requested size of this vector was specified 
  bool initSizeKnown;
  
  // The requested initial size of the vector
  typename std::vector<valT>::size_type initSize;

  // Records whether the initial value of this numeric variable was specified 
  bool initValKnown;

  // If was, the initial value itself
  valT initVal;
  public:
  ThreadLocalStorageVector(): initSizeKnown(false), initValKnown(false) {}
  ThreadLocalStorageVector(typename std::vector<valT>::size_type initSize): initSizeKnown(true), initSize(initSize), initValKnown(false) {}
  ThreadLocalStorageVector(typename std::vector<valT>::size_type initSize, const valT& initVal): initSize(initSize), initValKnown(true), initVal(initVal) {}
  
  // Allocates a new instance of an object of type T and returns it.
  std::vector<valT>* allocate() { 
    if(initSizeKnown) {
      if(initValKnown)
        return new std::vector<valT>(initSize, initVal);
      else
        return new std::vector<valT>(initSize);
    } else
      return new std::vector<valT>();
  }


  // Iteration methods
  typename std::vector<valT>::iterator begin()
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->begin(); }
  
  typename std::vector<valT>::const_iterator begin() const
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->begin(); }
  
  typename std::vector<valT>::reverse_iterator rbegin()
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->rbegin(); }
  
  typename std::vector<valT>::const_reverse_iterator rbegin() const
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->rbegin(); }
  
  typename std::vector<valT>::iterator end()
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->end(); }
  
  typename std::vector<valT>::const_iterator end() const
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->end(); }

  typename std::vector<valT>::reverse_iterator rend()
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->rend(); }
  
  typename std::vector<valT>::const_reverse_iterator rend() const
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->rend(); }
  
  // Capacity
  typename std::vector<valT>::size_type size()
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->size(); }
  
  typename std::vector<valT>::size_type max_size()
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->max_size(); }
  
  void resize(typename std::vector<valT>::size_type size, valT initVal=valT())
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->resize(size, initVal); }
  
  bool empty()
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->empty(); }
  
  void reserve(typename std::vector<valT>::size_type size)
  { return ((std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))->reserve(size); }
  
  // Element Access
  std::vector<valT>& operator=(const std::vector<valT>& that) {
    (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).assign(that.begin(), that.end());
    return *ThreadLocalStorage<std::vector<valT> >::get();
  }

  valT& operator[](typename std::vector<valT>::size_type i)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))[i]; }

  const valT& operator[](typename std::vector<valT>::size_type i) const
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get()))[i]; }

  valT& at(typename std::vector<valT>::size_type i)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).at(i); }

  const valT& at(typename std::vector<valT>::size_type i) const
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).at(i); }

  valT& front()
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).front(); }

  const valT& front() const
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).front(); }

  valT& back()
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).back(); }

  const valT& back() const
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).back(); }

  // Modifiers
  template <class InputIterator>
  void assign(InputIterator first, InputIterator last)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).assign(first, last); }

  void assign(typename std::vector<valT>::size_type n, const valT& val)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).assign(n, val); }

  void push_back(const valT& val)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).push_back(val); }

  void pop_back()
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).pop_back(); }

  typename std::vector<valT>::iterator insert(typename std::vector<valT>::iterator position, const valT& val)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).insert(position, val); }

  typename std::vector<valT>::iterator insert(typename std::vector<valT>::iterator position, typename std::vector<valT>::size_type n, const valT& val)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).insert(position, n, val); }

  template <class InputIterator>
  void insert (typename std::vector<valT>::iterator position, InputIterator first, InputIterator last)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).insert(position, first, last); }

  typename std::vector<valT>::iterator erase(typename std::vector<valT>::iterator position)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).erase(position); }

  typename std::vector<valT>::iterator erase(typename std::vector<valT>::iterator first, typename std::vector<valT>::iterator last)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).erase(first, last); }
  
  void swap(std::vector<valT>& x)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).swap(x); }
  
  void swap(ThreadLocalStorage<std::vector<valT> >& x)
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).swap(*x.get()); }
  
  void clear()
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).clear(); }
  
  // Allocator
  typename std::vector<valT>::allocator_type get_allocator()
  { return (*(std::vector<valT>*)(ThreadLocalStorage<std::vector<valT> >::get())).get_allocator(); }
}; // class ThreadLocalStorageVector

// Comparison operators where the original type is compared to the TLS type
template<typename valT>
bool operator==(const std::vector<valT>& left, const ThreadLocalStorageVector<valT>& right)
{ return left == *right.get(); }

template<typename valT>
bool operator!=(const std::vector<valT>& left, const ThreadLocalStorageVector<valT>& right)
{ return left != *right.get(); }

template<typename valT>
bool operator<(const std::vector<valT>& left, const ThreadLocalStorageVector<valT>& right)
{ return left < *right.get(); }

template<typename valT>
bool operator<=(const std::vector<valT>& left, const ThreadLocalStorageVector<valT>& right)
{ return left <= *right.get(); }

template<typename valT>
bool operator>(const std::vector<valT>& left, const ThreadLocalStorageVector<valT>& right)
{ return left > *right.get(); }

template<typename valT>
bool operator>=(const std::vector<valT>& left, const ThreadLocalStorageVector<valT>& right)
{ return left >= *right.get(); }

// ------------------------------------------------------------------
// ----- Specialized type of ThreadLocalStorage that is focused -----
// ------ on arrays of items of type std::list.                 -----
// ------------------------------------------------------------------
template<typename valT>
class ThreadLocalStorageList: public ThreadLocalStorage<std::list<valT> >  {
  // Private copy constructor to ensure that callers cast TLS objects to T rather than make copies of them
  private:
  ThreadLocalStorageList(const ThreadLocalStorageList<valT>& that) {}
  
  // Records whether the initial requested size of this list was specified 
  bool initSizeKnown;
  
  // The requested initial size of the list
  typename std::list<valT>::size_type initSize;

  // Records whether the initial value of this numeric variable was specified 
  bool initValKnown;

  // If was, the initial value itself
  valT initVal;
  public:
  ThreadLocalStorageList(): initSizeKnown(false), initValKnown(false) {}
  ThreadLocalStorageList(typename std::list<valT>::size_type initSize): initSizeKnown(true), initSize(initSize), initValKnown(false) {}
  ThreadLocalStorageList(typename std::list<valT>::size_type initSize, const valT& initVal): initSize(initSize), initValKnown(true), initVal(initVal) {}
  
  // Allocates a new instance of an object of type T and returns it.
  std::list<valT>* allocate() { 
    if(initSizeKnown) {
      if(initValKnown)
        return new std::list<valT>(initSize, initVal);
      else
        return new std::list<valT>(initSize);
    } else
      return new std::list<valT>();
  }


  // Iteration methods
  typename std::list<valT>::iterator begin()
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->begin(); }
  
  typename std::list<valT>::const_iterator begin() const
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->begin(); }
  
  typename std::list<valT>::reverse_iterator rbegin()
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->rbegin(); }
  
  typename std::list<valT>::const_reverse_iterator rbegin() const
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->rbegin(); }
  
  typename std::list<valT>::iterator end()
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->end(); }
  
  typename std::list<valT>::const_iterator end() const
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->end(); }

  typename std::list<valT>::reverse_iterator rend()
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->rend(); }
  
  typename std::list<valT>::const_reverse_iterator rend() const
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->rend(); }
  
  // Capacity
  typename std::list<valT>::size_type size()
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->size(); }
  
  typename std::list<valT>::size_type max_size()
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->max_size(); }
  
  bool empty()
  { return ((std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get()))->empty(); }
  
  // Element Access
  valT& front()
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).front(); }

  const valT& front() const
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).front(); }

  valT& back()
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).back(); }

  const valT& back() const
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).back(); }

  // Modifiers
  template <class InputIterator>
  void assign(InputIterator first, InputIterator last)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).assign(first, last); }

  void assign(typename std::list<valT>::size_type n, const valT& val)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).assign(n, val); }

  void push_back(const valT& val)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).push_back(val); }

  void pop_back()
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).pop_back(); }

   void push_front(const valT& val)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).push_front(val); }

  void pop_front()
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).pop_front(); }

  typename std::list<valT>::iterator insert(typename std::list<valT>::iterator position, const valT& val)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).insert(position, val); }

  typename std::list<valT>::iterator insert(typename std::list<valT>::iterator position, typename std::list<valT>::size_type n, const valT& val)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).insert(position, n, val); }

  template <class InputIterator>
  void insert (typename std::list<valT>::iterator position, InputIterator first, InputIterator last)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).insert(position, first, last); }

  typename std::list<valT>::iterator erase(typename std::list<valT>::iterator position)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).erase(position); }

  typename std::list<valT>::iterator erase(typename std::list<valT>::iterator first, typename std::list<valT>::iterator last)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).erase(first, last); }
  
  void swap(std::list<valT>& x)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).swap(x); }
  
  void swap(ThreadLocalStorage<std::list<valT> >& x)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).swap(*x.get()); }
  
  void resize(typename std::list<valT>::size_type size, valT initVal)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).resize(size, initVal); }
  
   void clear()
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).clear(); }
 
  // Operations
  void splice(typename std::list<valT>::iterator position, std::list<valT>& x)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).splice(position, x); }

  void splice(typename std::list<valT>::iterator position, std::list<valT>& x, typename std::list<valT>::iterator i)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).splice(position, x, i); }

  void splice(typename std::list<valT>::iterator position, std::list<valT>& x, typename std::list<valT>::iterator i, typename std::list<valT>::iterator last)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).splice(position, x, i, last); }

  void remove(const valT& val)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).remove(val); }

  template<class Predicate>
  void remove_if(Predicate pred)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).remove_if(pred); }
  
  void unique()
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).unique(); }

  template <class BinaryPredicate>
  void unique(BinaryPredicate pred)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).unique(pred); }

  void merge(std::list<valT>& x)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).merge(x); }

  template <class Compare>
  void merge(std::list<valT>& x, Compare comp)
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).merge(x, comp); }
 
  void sort() 
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).sort(); }

  template <class Compare>
  void sort(Compare comp) 
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).sort(comp); }

  void reverse() 
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).reverse(); }

  // Observers
  typename std::list<valT>::allocator_type get_allocator()
  { return (*(std::list<valT>*)(ThreadLocalStorage<std::list<valT> >::get())).get_allocator(); }
}; // class ThreadLocalStorageList

// Comparison operators where the original type is compared to the TLS type
template<typename valT>
bool operator==(const std::list<valT>& left, const ThreadLocalStorageList<valT>& right)
{ return left == *right.get(); }

template<typename valT>
bool operator!=(const std::list<valT>& left, const ThreadLocalStorageList<valT>& right)
{ return left != *right.get(); }

template<typename valT>
bool operator<(const std::list<valT>& left, const ThreadLocalStorageList<valT>& right)
{ return left < *right.get(); }

template<typename valT>
bool operator<=(const std::list<valT>& left, const ThreadLocalStorageList<valT>& right)
{ return left <= *right.get(); }

template<typename valT>
bool operator>(const std::list<valT>& left, const ThreadLocalStorageList<valT>& right)
{ return left > *right.get(); }

template<typename valT>
bool operator>=(const std::list<valT>& left, const ThreadLocalStorageList<valT>& right)
{ return left >= *right.get(); }

// Specialized type of ThreadLocalStorage that is focused on transparently wrapping
// C++ output streams
template<typename StreamT>
class ThreadLocalStorageOStream : public ThreadLocalStorage<StreamT>
{
  //string fName;
  public:
  //ThreadLocalStorageOStream(string fName): fName(fName) {}
  ThreadLocalStorageOStream() {}
  StreamT* allocate() { return new StreamT(); }
  
  template <typename T>
  ThreadLocalStorageOStream& operator<<(T const& obj )
  {
    //cout << "this="<<this<<", thread="<<pthread_self()<<", get1()="<<ThreadLocalStorage</*StreamT*/ofstream>::get()<<endl;
    if(ThreadLocalStorage<StreamT>::get())
      (*((StreamT*)ThreadLocalStorage<StreamT>::get())) << obj;
    return *this;
  }
  
  ThreadLocalStorageOStream& operator<<(std::ostream& (*pf)(std::ostream&)) {
    //cout << "this="<<this<<", get2()="<<ThreadLocalStorage</*StreamT*/ofstream>::get()<<endl;
    if(ThreadLocalStorage<StreamT>::get())
      (*((StreamT*)ThreadLocalStorage<StreamT>::get())) << pf;
    return *this;
  }
};

// Extension of ThreadLocalStorage to opaquely wrap ostringstream
/*class ThreadLocalDbgStream: public ThreadLocalStorageOStream<ostringstream> {
  dbgStream* allocate() { return new dbgStream(); }
};*/

/*typedef std::ostream& (*ostream_manipulator)(std::ostream&);
ThreadLocalStorageOStream& operator<<(ThreadLocalStorageOStream& os, ostream_manipulator pf) {
  return operator<< <ostream_manipulator> (os, pf);
}*/

// ############################################
// ##### Tests for the TLS infrastructure #####
// ############################################
/*class B {
  int val;
  public:
  B(int val): val(val) {
    cout << "B::B() thread="<<pthread_self()<<", val="<<val<<"\n";
  }
  ~B() {
    cout << "B::~B() thread="<<pthread_self()<<", val="<<val<<"\n";
  }
  int getVal() {
    cout << "B::getVal() thread="<<pthread_self()<<"\n";
    return val;
  }
};

class C {
  std::string s;
  int i;
  public:
  static ThreadLocalStorage1<B, int> b;
  C(std::string s, int i): s(s), i(i) {
    cout << "C::C thread="<<pthread_self()<<" s="<<s<<", i="<<i<<", val="<<b->getVal()<<"\n";
  }
  ~C() {
    cout << "C::~C thread="<<pthread_self()<<" s="<<s<<", i="<<i<<", val="<<b->getVal()<<"\n";
  }
  std::string getS() { return s; }
};
ThreadLocalStorage1<B, int> C::b(15);

ThreadLocalStorage2<C, std::string, int> c("hello", 123);
void foo() {
  ThreadLocalStorage2<C, std::string, int> c2("bar", 789);
  cout << "foo() thread="<<pthread_self()<<" c2->getS()="<<c2->getS()<<endl;
}

ThreadLocalStorageOStream output("output");

void* work(void*) {
  c.tlsInit();
  cout << "work() thread="<<pthread_self()<<" c->getS()="<<c->getS()<<endl;
  cout << "output="<<(&output)<<endl;
  for(int i=0; i<5; i++) {
    output << "work("<<i<<") thread="<<pthread_self()<<" c->getS()="<<c->getS()<<endl;
  }
  foo();
  return NULL;
}

int main(int argc, char** argv) {
   #define NUM_THREADS 2
   pthread_t threads[NUM_THREADS];
   void *status;
   int rc;
   long t;
   for(t=0; t<NUM_THREADS; t++){
      printf("In main: creating thread %ld\n", t);
      rc = pthread_create(&threads[t], NULL, work, (void *)t);
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }
   }

   for(t=0; t<NUM_THREADS; t++) {
      rc = pthread_join(threads[t], &status);
      if (rc) {
         printf("ERROR; return code from pthread_join() is %d\n", rc);
         exit(-1);
         }
      printf("Main: completed join with thread %ld having a status of %ld\n",t,(long)status);
   }

   // Last thing that main() should do 
   pthread_exit(NULL);

   return 0;
}

// Instantiate all the variants of ThreadLocalStorage to make sure they're all consistent
class Test0 {
  public:
    Test0() {}
};
ThreadLocalStorage0<Test0> t0();

class Test1 {
  public:
    Test1(int a1) {}
};
ThreadLocalStorage1<Test1, int> t1(1);

class Test2 {
  public:
    Test2(int a1, int a2) {}
};
ThreadLocalStorage2<Test2, int, int> t2(1, 2);

class Test3 {
  public:
    Test3(int a1, int a2, int a3) {}
};
ThreadLocalStorage3<Test3, int, int, int> t3(1, 2, 3);

class Test4 {
  public:
    Test4(int a1, int a2, int a3,  int a4) {}
};
ThreadLocalStorage4<Test4, int, int, int, int> t4(1, 2, 3, 4);

class Test5 {
  public:
    Test5(int a1, int a2, int a3,  int a4, int a5) {}
};
ThreadLocalStorage5<Test5, int, int, int, int, int> t5(1, 2, 3, 4, 5);

class Test6 {
  public:
    Test6(int a1, int a2, int a3,  int a4, int a5, int a6) {}
};
ThreadLocalStorage6<Test6, int, int, int, int, int, int> t6(1, 2, 3, 4, 5, 6);

class Test7 {
  public:
    Test7(int a1, int a2, int a3,  int a4, int a5, int a6, int a7) {}
};
ThreadLocalStorage7<Test7, int, int, int, int, int, int, int> t7(1, 2, 3, 4, 5, 6, 7);

class Test8 {
  public:
    Test8(int a1, int a2, int a3,  int a4, int a5, int a6, int a7, int a8) {}
};
ThreadLocalStorage8<Test8, int, int, int, int, int, int, int, int> t8(1, 2, 3, 4, 5, 6, 7, 8);

class Test9 {
  public:
    Test9(int a1, int a2, int a3,  int a4, int a5, int a6, int a7, int a8, int a9) {}
};
ThreadLocalStorage9<Test9, int, int, int, int, int, int, int, int, int> t9(1, 2, 3, 4, 5, 6, 7, 8, 9);
*/
