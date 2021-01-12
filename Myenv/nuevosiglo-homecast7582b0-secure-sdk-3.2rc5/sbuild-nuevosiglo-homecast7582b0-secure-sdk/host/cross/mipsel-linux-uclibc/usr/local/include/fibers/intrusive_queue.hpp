/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)
 * For conditions of distribution and use, see copyright notice in fiber.h
 */

#ifndef INTRUSIVE_QUEUE_HPP
#define INTRUSIVE_QUEUE_HPP
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
  #pragma once
#endif

#include "external/system_defs.hpp"

#include <vector>

////////////////////////////////////////////////
namespace intrusive {
////////////////////////////////////////////////

  //-------------------------------------------------------------------------
  ///////////////////////////////////////////////////////////////////////////
  //  binary heap, with updating nodes.
  //  T - is a value kept in the heap.
  //  Toper - is a struct with static operations:
  //    bool  less(const T& lhs, const T& rhs);
  //    uint& heap_index(T& node);
  //    uint  heap_index(const T& node);
  //    void  swap(T& lhs, T& rhs);
  //  Baligned - determines if a special container should be used.
  //
  ///////////////////////////////////////////////////////////////////////////
  //-------------------------------------------------------------------------

  template< typename T >
  struct bin_heap_oper_base
  {
    struct fake_intref
    {
      INL const fake_intref& operator=(unsigned int) const
      { return *this; };
    };

    INL static void swap(T& lhs, T& rhs)
    { std::swap(lhs, rhs); };
    // derived class should return T::some_member&, or it won't be able to use update_node.
    INL static fake_intref heap_index(const T&)
    { return fake_intref(); };
  };
  template< typename T >
  struct bin_heap_oper_simple
    : bin_heap_oper_base<T>
  {
    INL static bool less(const T& lhs, const T& rhs)
    { return lhs < rhs; };
  };


  template< typename T >
  struct bin_heap_node_base
    : bin_heap_oper_simple<T>
  {
  private:
    mutable  unsigned int _heap_index;
  private:
    typedef struct bin_heap_node_base<T> this_type;
  public:
    bin_heap_node_base(): _heap_index(0) {};
    static unsigned int & heap_index(const this_type* node) {
      return node->_heap_index;
    };
  };


  template< typename T, typename Toper=T>
  class bin_heap
  {
    ///////////////////-------------------
    // Types.
    ///////////////////-------------------
  public:
    typedef T  node_type;
    typedef T  value_type;
    typedef Toper  oper_type;

  public:
    typedef std::vector<node_type>  cont_type;
    typedef typename cont_type::size_type  size_type;
    typedef unsigned int  index_type;

  private:
    template < typename A >
    struct insert_arg_deduct { typedef const A& type; };
    template < typename A >
    struct insert_arg_deduct<A*> { typedef A* type; };

    typedef typename insert_arg_deduct<T>::type  insert_arg_type;

    ///////////////////-------------------
    // initialization.
    ///////////////////-------------------
  public:
    bin_heap(void)
    {
      // we don't use index 0 for simplified calculations.
      m_Cont.resize(1);
    };


    template<typename InputIter>
    bin_heap(InputIter input, const size_type cnt)
      : m_Cont(1+cnt)
    {
      const index_type elems = (index_type)cnt;
      for (index_type i=1; i<=elems; ++i) {
        m_Cont[i] = *input;
        oper_type::heap_index(m_Cont[i]) = i;
        ++input;
      };
      Heapify();
    };

    ///////////////////-------------------
    // Data.
    ///////////////////-------------------
  private:
    cont_type  m_Cont;

  public:
    INL void  clear(void)
    {
      m_Cont.resize(1);
    };
    INL size_type  size(void) const
    {
      return m_Cont.size() - 1;
    };
    INL bool  empty(void) const
    {
      return (m_Cont.size() <= 1);
    };

    INL value_type  _elem(int n) const
    {
      return m_Cont[n+1];
    };

    ///////////////////-------------------
    // Maintaining.
    ///////////////////-------------------
  private:
    int  RepairUp(index_type fromIndex)
    {
      int moved = 0;
      index_type currInd = fromIndex;
      for (;;)
      {
        // if we're already at the top of the heap.
        if (currInd == 1)
          break;
        const index_type upperInd = currInd / 2;
        // check if fields exchange is required.
        if (!oper_type::less(m_Cont[currInd], m_Cont[upperInd]))
          break;

        _TotalCheck(true);

        oper_type::swap(m_Cont[currInd], m_Cont[upperInd]);
        oper_type::heap_index(m_Cont[currInd]) = currInd;
        oper_type::heap_index(m_Cont[upperInd]) = upperInd;
        currInd = upperInd;
        moved = 1;

        _TotalCheck(true);
      };
      return moved;
    };

    int  RepairDown(index_type fromIndex)
    {
      int moved = 0;
      const size_type elemCnt = size();
      index_type upperInd = fromIndex;
      index_type lowerInd = fromIndex;
      for (;;)
      {
        index_type leftInd = upperInd * 2;
        if ( leftInd <= elemCnt &&  oper_type::less(m_Cont[leftInd], m_Cont[lowerInd]) )
          lowerInd = leftInd;
        index_type rightInd = upperInd * 2 + 1;
        if ( rightInd <= elemCnt && oper_type::less(m_Cont[rightInd], m_Cont[lowerInd]) )
          lowerInd = rightInd;

        if (lowerInd == upperInd)
          break;

        _TotalCheck(true);

        oper_type::swap(m_Cont[upperInd], m_Cont[lowerInd]);
        oper_type::heap_index(m_Cont[lowerInd]) = lowerInd;
        oper_type::heap_index(m_Cont[upperInd]) = upperInd;
        upperInd = lowerInd;
        moved = 1;

        _TotalCheck(true);
      };
      return moved;
    };

    void  Heapify(void)
    {
      const index_type elemCnt = (index_type)size();
      for (index_type upperInd=elemCnt/2; upperInd>0; upperInd--)
      {
        RepairDown(upperInd);
      };
      _TotalCheck();
    };

    void _TotalCheck(bool indicesOnly=false) const
    {
      (void)indicesOnly;
#if 0
      //Logger << "_TotalCheck(" << indicesOnly << ")\n";

      const size_type elemCnt = size();
      //for (index_type upperInd=1; upperInd<=elemCnt; ++upperInd)
      //{
      //  Logger << "[" << m_Cont[upperInd]->_map_index << ",";
      //  Logger << (m_Cont[upperInd]->m_Pfind.valCurr) << "] ";
      //};
      //Logger << "\n";

      for (index_type upperInd=1; upperInd<=elemCnt; ++upperInd)
      {
      //  if (oper_type::heap_index(m_Cont[upperInd]) != upperInd) {
      //  //  Logger << "Index currupted: " << upperInd << "\n\n\n";
      //    throw upperInd;
      //  };
        if (indicesOnly)
          continue;

      //  index_type leftInd = upperInd * 2;
      //  if ( leftInd <= elemCnt && oper_type::less(m_Cont[leftInd], m_Cont[upperInd]) ) {
      //  //  Logger << "Nodes not-ordered: " << leftInd << " " << upperInd << "\n\n\n";
      //    throw leftInd;
      //  };
      //  index_type rightInd = upperInd * 2 + 1;
      //  if ( rightInd <= elemCnt && oper_type::less(m_Cont[rightInd], m_Cont[upperInd]) ) {
      //  //  Logger << "Nodes not-ordered: " << rightInd << " " << upperInd << "\n\n\n";
      //    throw rightInd;
      //  };
      };
#endif
    };

    ///////////////////-------------------
    // Commands.
    ///////////////////-------------------
  public:
    void  insert(insert_arg_type node)
    {
      m_Cont.push_back(node); // FIXME: throwing!!!

      const index_type index = (index_type)size();
      oper_type::heap_index(m_Cont[index]) = index;

      RepairUp( index );
      _TotalCheck();
    };

    void  remove(insert_arg_type node)
    {
      const index_type index = oper_type::heap_index(node);
      // assert(index >= 1 && index <= size() && node == m_Cont[index]);
      if (index < size()) {
        m_Cont[index] = m_Cont[ size() ];
        oper_type::heap_index(m_Cont[index]) = index;
        m_Cont.pop_back();
        update_node(index);
      } else {
        m_Cont.pop_back();
      };
      _TotalCheck();
    };

    void  update_node(const value_type & node)
    {
      const index_type index = oper_type::heap_index(node);
      // assert(index >= 1 && index <= size() && node == m_Cont[index]);
      update_node(index);
      _TotalCheck();
    };

    value_type  get_min(void) const
    {
      return m_Cont[1];
    };

    value_type  extract_min(void)
    {
      const value_type node = m_Cont[1];

      if (size() > 1) {
        m_Cont[1] = m_Cont[ size() ];
        oper_type::heap_index(m_Cont[1]) = 1;
        m_Cont.pop_back();
        RepairDown(1);
      } else {
        m_Cont.pop_back();
      };
      _TotalCheck();

      return node;
    };

  private:
    void  update_node(index_type index)
    {
      if (!RepairUp(index))
        RepairDown(index);
    };
  };


////////////////////////////////////////////////
}; // namespace intrusive
////////////////////////////////////////////////

#endif // #ifndef INTRUSIVE_QUEUE_HPP
