// $Id: listmap.tcc,v 1.1 2019-02-17 15:37:07-08 - - $
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
   DEBUGF ('l',"&pair="<< &pair << "->" << pair << endl);

   node* temp;
   if (empty()) {
      temp = new node(end().where, end().where, pair);
      end().where->next = temp;
      end().where->prev = temp;

      DEBUGF('i', "empty case ==> begin() = " << *begin() << endl);

      return begin();
   }

   // probably a node* ?
   DEBUGF('i',"begin() == end : " << (begin() == end()) << endl);
   auto curr = begin();
   while (curr != end() and 
            less ( curr->first, pair.first)) {
      ++curr;
      DEBUGF('i', "curr =" << &curr);
   }
   if(curr == end()){
      //need to insert at end of list, so must update anchor.prev
      temp = new node(end().where, (--end()).where, pair);
      (--curr).where->next = temp;
      end().where->prev = temp;
      //curr.where-> prev = temp;
      DEBUGF('i', "curr =" << &curr);
   }
   //if(less (curr->first, pair.first) )
   else  {
      //inserting in the middle of list
      temp = new node(curr.where, curr.where->prev, pair);
      temp->prev->next = temp;
      curr.where->prev = temp;
      DEBUGF('i', "curr =" << &curr);
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
      return end();
   }
   else{
      for (; itor != end(); ++itor) {
         if(itor->first == that){
            return itor;
         }
      }
   }
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
      return end();
   }
   else{
      //make pointer to position.where->next and directly reference
      node* next_prev = position.where->next;
      node* prev_next = position.where->prev;
      next_prev->prev = prev_next;
      prev_next->next = next_prev;
      delete position.where;
      return iterator(next_prev);
      
   }
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

