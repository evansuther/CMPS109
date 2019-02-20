// $Id: listmap.tcc,v 1.1 2019-02-17 15:42:58-08 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (node* next, node* prev,
                                     const value_type& value):
            link (next, prev), value (value) {
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
   DEBUGF ('l', reinterpret_cast<const void*> (this));
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {
   DEBUGF ('l',"&pair="<< &pair << "->" << pair);
   DEBUGF('i',"begin() == end : " << (begin() == end()) << endl);
   auto curr = begin();
   node* temp;
   if(empty() == true){
      // insert node into empty list
      temp = new node( end().where, end().where, pair);
      end().where->next = temp;
      end().where->prev = temp;
   }
   else{
      // list has at least one node
      while (curr != end() and 
             less ( curr->first, pair.first)) {
         // want to place node in asending lexico order
         ++curr;
         DEBUGF('i', "curr =" << &curr);
      }
      if(curr == end()){
         // need to insert at end of list, so must update anchor.prev
         temp = new node(end().where, (--end()).where, pair);
         (--curr).where->next = temp;
         end().where->prev = temp;
         DEBUGF('i', "curr =" << &curr);
      }
      else {
         // inserting in the middle of list
         temp = new node(curr.where, curr.where->prev, pair);
         temp->prev->next = temp;
         curr.where->prev = temp;
         DEBUGF('i', "curr =" << &curr);
      }
   }
   return curr;
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that) {
   DEBUGF ('l', that);
   auto itor = begin();
   if(empty()){
      // can not find anything in an empty list
      return end();
   }
   else{
      for (; itor != end(); ++itor) {
         if(itor->first == that){
            // if key found return its iterator
            return itor;
         }
      }
   }
   // if nothing is found return end
   return end();
}

//
// listmap::find_value(const mapped_type&, iterator)
//
// mapped_type arg is searched for, starting from iterator arg
// if found, returns itorator to list elem; else returns end() 
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find_value(const mapped_type& that,
                                       iterator itor){
   DEBUGF ('l', that);
   if(empty()){
      // can not find anything in an empty list
      return end();
   }
   else{
      while (itor != end()) {
         if(itor->second == that){
            // if value found return its iterator
            return itor;
         }
         ++itor;
      }
   }
   // if nothing is found return end
   return end();
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
   DEBUGF ('l', &*position);
   if(empty()){
      // can not erase an empty list
      return end();
   }
   else{
      // deal with pointers surrounding position
      node* next_ = position.where->next;
      node* prev_ = position.where->prev;
      next_->prev = prev_;
      prev_->next = next_;
      delete position.where;
      DEBUGF('d', "erase returning next_ = " << next_ << endl);
      return iterator(next_);
      
   }
   // will never get ran
   return end();
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
   DEBUGF ('l', where);
   return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->() {
   DEBUGF ('l', where);
   return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
   DEBUGF ('l', where);
   where = where->next;
   return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
   DEBUGF ('l', where);
   where = where->prev;
   return *this;
}


//
// bool listmap::iterator::operator== (const iterator&)
//
// changed from const to use in insert
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
            (const iterator& that) const {
   return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
            (const iterator& that) const {
   return this->where != that.where;
}

