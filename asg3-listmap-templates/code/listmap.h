// $Id: listmap.h,v 1.1 2019-02-17 15:37:07-08 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

#ifndef __LISTMAP_H__
#define __LISTMAP_H__

#include "xless.h"
#include "xpair.h"

template <typename Key, typename Value, class Less=xless<Key>>
class listmap {
   public:
      using key_type = Key;
      using mapped_type = Value;
      using value_type = xpair<const key_type, mapped_type>;
   private:
      Less less;
      struct node;
      struct link {
         node* next{};
         node* prev{};
         link (node* next, node* prev): next(next), prev(prev){}
      };
      struct node: link {
         value_type value{};
         node (node* next, node* prev, const value_type&);
      };
      node* anchor() { return static_cast<node*> (&anchor_); }
      link anchor_ {anchor(), anchor()};
   public:
      class iterator;
      listmap(){};
      listmap (const listmap&);
      listmap& operator= (const listmap&);
      ~listmap();
      iterator insert (const value_type&);
      iterator find (const key_type&);
      iterator find_value(const mapped_type&, iterator);
      iterator erase (iterator position);
      iterator begin() { return anchor()->next; }
      iterator end() { return anchor(); }
      bool empty() { return begin() == end(); }
};


template <typename Key, typename Value, class Less>
class listmap<Key,Value,Less>::iterator {
   private:
      friend class listmap<Key,Value,Less>;
      listmap<Key,Value,Less>::node* where {nullptr};
      iterator (node* where): where(where){};
   public:
      iterator(){}
      value_type& operator*();
      value_type* operator->();
      iterator& operator++(); //++itor
      iterator& operator--(); //--itor
      void erase();
      bool operator== (const iterator&) const;
      bool operator!= (const iterator&) const;
};

#include "listmap.tcc"
#endif

