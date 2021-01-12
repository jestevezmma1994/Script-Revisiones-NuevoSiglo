/* Copyright (C) 2008  Michal Wojcikowski (deffer44@gmail.com)
 * For conditions of distribution and use, see copyright notice in fiber.h
 */

#ifndef INTRUSIVE_LIST_HPP
#define INTRUSIVE_LIST_HPP
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#  pragma once
#endif

#include "external/system_defs.hpp"

#include <cstddef>
#include <iterator>

/////////////////////////////////
namespace intrusive
/////////////////////////////////
{

  template< typename Tag >
  class list_node
  {
    typedef list_node<Tag>  this_type;
  private:
    this_type* prev;
    this_type* next;
  public:
    list_node(): prev(NULL), next(NULL) {};
    this_type*  Prev() const { return prev; };
    this_type*  Next() const { return next; };
    this_type*& Prev() { return prev; };
    this_type*& Next() { return next; };
  };

  template< typename T, typename Tag >
  class list;


  /////////////////////////////////
  namespace _list_detail
  /////////////////////////////////
  {
    /*
      list_traits:
       1. assert( _T is a pointer ).
       2. add 'const' to node_type iff original type is const (as we're static_cast-ing alot).
    */

    template< typename _T, typename _Tag >
    struct list_traits;

    template< typename T, typename Tag >
    struct list_traits_base
    {
      typedef Tag  tag_type;
      typedef list_node<Tag>  mutable_node_type;
      typedef const list_node<Tag>  const_node_type;
      typedef list<T, Tag>  list_type;
    };

    template< typename _T, typename Tag >
    struct list_traits< _T*, Tag > : public list_traits_base<_T*, Tag>
    {
      typedef list_traits_base<_T*, Tag> base_traits;
      typedef _T  orig_type;
      typedef typename base_traits::mutable_node_type  mutable_node_type;
      typedef typename base_traits::mutable_node_type  node_type;

      static node_type & Node(_T* t)
      {  return static_cast<node_type&>(*t); };
      static const node_type & Node(const _T* t)
      {  return static_cast<const node_type&>(*t); };

      static mutable_node_type & MutableNode(const _T* t)
      {  return const_cast<mutable_node_type&>(Node(t)); };
    };
    template< typename _T, typename Tag >
    struct list_traits< const _T*, Tag > : public list_traits_base<const _T*, Tag>
    {
      typedef list_traits_base<const _T*, Tag> base_traits;
      typedef const _T  orig_type;
      typedef typename base_traits::mutable_node_type  mutable_node_type;
      typedef typename base_traits::const_node_type    node_type;

      static node_type & Node(const _T* t)
      {  return static_cast<node_type&>(*t); };

      static mutable_node_type & MutableNode(const _T* t)
      {  return const_cast<mutable_node_type&>(Node(t)); };
    };

    template< typename _T, typename _Tag >
    class iter;
    template< typename _T, typename _Tag >
    class const_iter;

    template< typename _T, typename _Tag, typename _Derived, typename _Node >
    class iter_base
    {
      typedef iter_base<_T, _Tag, _Derived, _Node>  this_type;
      typedef _Derived  derived_type;
    public:
      typedef list_traits<_T, _Tag>  traits_type;
      typedef typename traits_type::orig_type  orig_type;
      typedef typename traits_type::tag_type  tag_type;
      typedef _Node  node_type;
      typedef typename traits_type::list_type  list_type;
      FRIEND(traits_type::list_type, list_type);
    public:
      typedef std::bidirectional_iterator_tag  iterator_category;
      typedef orig_type*  value_type;
      typedef ptrdiff_t  difference_type;
      // no pointers allowed, only values.
      typedef orig_type*  const_pointer;
      typedef const_pointer  pointer;
      // no references allowed, only values.
      typedef orig_type*  const_reference;
      typedef const_reference  reference;
    public:
      template<typename _T2, typename _Tag2, typename _Derived2, typename _Node2>
      friend class iter_base;
      template< typename _T2, typename _Tag2 >
      friend class iter;
      template< typename _T2, typename _Tag2 >
      friend class const_iter;

    protected:
      explicit iter_base(node_type* p = NULL) : curr(p) {};
      iter_base(const iter_base & it) : curr(it.curr) {};
    public:
      pointer  operator*(void) const
      {  return static_cast<pointer>(curr); };
      pointer  operator->(void) const
      {  return static_cast<pointer>(curr); };

    public:
      derived_type& operator++()
      {
        curr = curr->Next();
        return static_cast<derived_type&>(*this);
      };
      derived_type operator++(int)
      {
        derived_type tmp = static_cast<derived_type&>(*this);
        ++(*this);
        return tmp;
      };

      this_type& operator--()
      {
        curr = curr->Prev();
        return static_cast<derived_type&>(*this);
      };
      this_type operator--(int)
      {
        derived_type tmp = static_cast<derived_type&>(*this);
        --(*this);
        return tmp;
      };

      template< typename _Derived2, typename _Node2 >
      bool operator==(const iter_base<_T, _Tag, _Derived2, _Node2> & it) const
      {  return curr == it.curr; };
      template< typename _Derived2, typename _Node2 >
      bool operator!=(const iter_base<_T, _Tag, _Derived2, _Node2> & it) const
      {  return !(*this == it); };

    protected:
      node_type* curr;
    };

    template< typename _T, typename _Tag >
    class iter
      : public iter_base< _T, _Tag, iter<_T, _Tag>, typename list_traits<_T, _Tag>::mutable_node_type >
    {
      typedef iter_base< _T, _Tag, iter<_T, _Tag>, typename list_traits<_T, _Tag>::mutable_node_type > base_type;
      typedef typename list_traits<_T, _Tag>::mutable_node_type  node_type;
      typedef typename base_type::list_type  list_type;
      FRIEND(base_type::list_type, list_type);
    public:
      iter() {};
    private:
      explicit iter(node_type* p) : base_type(p) {};
    };

    template< typename _T, typename _Tag >
    class const_iter
      : public iter_base< _T, _Tag, const_iter<_T, _Tag>, typename list_traits<_T, _Tag>::node_type >
    {
      typedef iter_base< _T, _Tag, const_iter<_T, _Tag>, typename list_traits<_T, _Tag>::node_type > base_type;
      typedef typename list_traits<_T, _Tag>::node_type  node_type;
      typedef typename base_type::list_type  list_type;
      FRIEND(base_type::list_type, list_type);
    public:
      const_iter() {};
      explicit const_iter(const iter<_T, _Tag> & ci) : base_type(ci.curr) {};
      const_iter& operator=(const iter<_T, _Tag> & ci) { this->curr = ci.curr; return *this; };
    private:
      explicit const_iter(node_type* p) : base_type(p) {};
    };

  /////////////////////////////////
  }; // (namespace _list_detail)
  /////////////////////////////////


  template< typename T, typename Tag >
  class list //: public boost::noncopyable
  {
    typedef list<T, Tag>  this_type;
    typedef _list_detail::list_traits<T, Tag>  traits_type;
    typedef typename traits_type::orig_type  orig_type;
    typedef typename traits_type::tag_type  tag_type;
    typedef typename traits_type::node_type  node_type;
    typedef typename traits_type::mutable_node_type  mutable_node_type;
    typedef typename traits_type::list_type  list_type;
  public:
    typedef size_t  size_type;
    typedef ptrdiff_t  difference_type;
  public:
    typedef orig_type*  value_type;
    // no pointers allowed, only values.
    typedef orig_type*  const_pointer;
    typedef orig_type*  pointer;
    // no references allowed, only values.
    typedef orig_type*  const_reference;
    typedef orig_type*  reference;
  public:
    typedef _list_detail::const_iter<T,Tag>  const_iterator;
    typedef _list_detail::iter<T,Tag>  iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  public:
    list() : m_cnt(0) {
      m_EndNode.Prev() = &m_EndNode;
      m_EndNode.Next() = &m_EndNode;
    };
    template< typename InputIter >
    list(InputIter b, InputIter e) : m_cnt(0) {
      m_EndNode.Prev() = &m_EndNode;
      m_EndNode.Next() = &m_EndNode;
      insert(end(), b, e);
    };
    ~list() {
      //clear();
    };

  public:
    template< typename InputIter >
    void assign(InputIter b, InputIter e) {
      clear();
      insert(end(), b, e);
    };

  private:
    list(const this_type & o);
    this_type & operator=(const this_type & o);

  private:
    mutable mutable_node_type  m_EndNode;
    size_t  m_cnt;

  public:
    iterator  begin() { return iterator(m_EndNode.Next()); };
    const_iterator  begin() const { return const_iterator(m_EndNode.Next()); };
    iterator  end() { return iterator(&m_EndNode); };
    const_iterator  end() const { return const_iterator(&m_EndNode); };

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    reverse_iterator rend()  { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const  { return const_reverse_iterator(begin()); }

  public:
    bool  empty() const { return m_cnt == 0; };
    size_type  size() const { return m_cnt; };

  public:
    reference  front() { return *begin(); };
    const_reference  front() const { return *begin(); };
    reference  back() { return *--end(); };
    const_reference  back() const { return *--end(); };

  public:
    void push_front(pointer t) {
      mutable_node_type & curr = traits_type::MutableNode(t);
      curr.Prev() = &m_EndNode;
      curr.Next() = m_EndNode.Next();
      m_EndNode.Next()->Prev() = &curr;
      m_EndNode.Next() = &curr;
      m_cnt++;
    };
    void pop_front() {
      mutable_node_type & curr = *m_EndNode.Next();
      m_EndNode.Next() = curr.Next();
      m_EndNode.Next()->Prev() = &m_EndNode;
      m_cnt--;
    };
    void push_back(pointer t) {
      mutable_node_type & curr = traits_type::MutableNode(t);
      curr.Next() = &m_EndNode;
      curr.Prev() = m_EndNode.Prev();
      m_EndNode.Prev()->Next() = &curr;
      m_EndNode.Prev() = &curr;
      m_cnt++;
    };
    void pop_back() {
      mutable_node_type & curr = *m_EndNode.Prev();
      m_EndNode.Prev() = curr.Prev();
      m_EndNode.Prev()->Next() = &m_EndNode;
      m_cnt--;
    };

  public:
    void insert(iterator before, pointer t) {
      mutable_node_type & curr = traits_type::_Node(t);
      mutable_node_type & bef = *before.curr;
      curr.Next() = &bef;
      curr.Prev() = bef.Prev();
      bef.Prev() = &curr;
      curr.Prev()->Next() = &curr;
      m_cnt++;
    };
    template< typename InputIterator >
    void insert(iterator before, InputIterator b, InputIterator e) {
      for (; b != e; ++b)
        insert(before, *b);
    };

    iterator  erase(iterator it) {
      mutable_node_type & curr = *it.curr;
      mutable_node_type & aft = *curr.Next();
      curr.Next()->Prev() = curr.Prev();
      curr.Prev()->Next() = curr.Next();
      m_cnt--;
      return iterator(&aft);
    };
    iterator  erase(iterator b, iterator e) {
      if (b == begin() && e == end()) {
        clear();
        return end();
      } else {
        while (b != e)
          b = erase(b);
        return b;
      };
    };

    void  clear() {
      m_EndNode.Prev() = &m_EndNode;
      m_EndNode.Next() = &m_EndNode;
      m_cnt = 0;
    };
    void  swap(this_type & other) {
      std::swap(m_EndNode.Prev(), other.m_EndNode.Prev());
      std::swap(m_EndNode.Next(), other.m_EndNode.Next());
      std::swap(m_cnt, other.m_cnt);
    };

  private:
    void _splice(iterator before, iterator b, iterator e) {
      // make the actual transfer:
      --e;
      mutable_node_type & bef = *before.curr;
      mutable_node_type & node_b = *b.curr;
      mutable_node_type & node_e = *e.curr;
      // 1. tie the other list.
      node_b.Prev()->Next() = node_e.Next();
      node_e.Next()->Prev() = node_b.Prev();
      // 2. append to this list.
      node_e.Next() = &bef;
      bef.Prev() = &node_e;
      node_b.Prev() = &bef.Prev();
      bef.Prev()->Next() = &node_b;
    };
    void _splice(iterator before, list<T,Tag> & x, iterator b, iterator e, size_type cnt) {
      // all dangerous cases ruled out.
      _splice(before, b, e);
      m_cnt += cnt;
      x.m_cnt -= cnt;
    };
  public:
    void splice(iterator before, list<T,Tag> & x, iterator b, iterator e) {
      if (b == e || (this == &x && before == e))
        return;
      if (b == x.begin() && e == x.end()) {
        _splice(before, x, b, e, x.size());
      } else {
        // the only case when complexity is O(n)
        _splice(before, x, b, e, std::distance(b, e));
      };
    };
    void splice(iterator before, list<T,Tag> & x) {
      if (this != &x && !x.empty())
        _splice(before, x, x.begin(), x.end(), x.size());
    };
    void splice(iterator before, list<T,Tag> & x, iterator i) {
      if (i != x.end()) {
        iterator e = i; ++e;
        if(!(this == &x && (before == i || before == e)))
          _splice(before, x, i, e, 1);
      };
    };


  public:
    // Extras!
    void  remove(pointer t) {
      erase(iterator(&traits_type::Node(t)));
    };

    template<class _Pr1>
    void remove_if(_Pr1 _Pred)
    {  // erase each element satisfying _Pr1
      iterator _Last = end();
      for (iterator _First = begin(); _First != _Last; )
        if (_Pred(*_First))
          _First = erase(_First);
        else
          ++_First;
    };
  };


  template<class T, typename Tag > inline
  void swap(list<T, Tag>& _Left, list<T, Tag>& _Right)
  {  // swap _Left and _Right lists
    _Left.swap(_Right);
  }
  template<class T, typename Tag > inline
  bool operator==(const list<T, Tag>& _Left, const list<T, Tag>& _Right)
  {  // no element could be on two different list at once.
    return (&_Left) == (&_Right);
  }
  template<class T, typename Tag > inline
  bool operator!=(const list<T, Tag>& _Left, const list<T, Tag>& _Right)
  {
    return (!(_Left == _Right));
  }

/////////////////////////////////
}; // (namespace intrusive)
/////////////////////////////////

#endif // #ifndef INTRUSIVE_LIST_HPP
