// $Id: file_sys.cpp,v 1.6 2018-06-27 14:44:57-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

// default ctor for inode_state
inode_state::inode_state() {
   // root is shared ptr to a directory inode
   root = make_shared<inode>(file_type::DIRECTORY_TYPE );
   // cwd is init to root
   cwd = root;
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

const string& inode_state::prompt() const { return prompt_; }

void inode_state::prompt(const wordvec& words){
   string pr;
   for(size_t i = 1; i != words.size(); ++i){
      pr = pr + words.at(i) + " ";
   }
   prompt_ = pr;
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

base_file_ptr inode::get_contents(){
   return contents;
}

void inode::mkdir(const string& dirname){
   contents->mkdir(dirname);
   return;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   size_t c {1};
   for(auto str: data){
      // size of a file is char/byte count
      size = size + str.size();
      // count spaces
      if (c != data.size()) {
         ++size;
         ++c;
      }
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   for(size_t i = 2; i != words.size(); ++i){
      DEBUGF('d', "pushing " << words.at(i) << " to data" << endl);
      data.push_back(words.at(i));
   }
   DEBUGF('d', "size of file = " << size());
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

void plain_file::set_parent(inode_ptr){
   throw file_error ("is a plain file");
}

inode_ptr plain_file::find(const string&){
    throw file_error ("is a plain file");
}

directory::directory(){
   dirents.emplace(".", nullptr);
   dirents.emplace("..", nullptr);
   for(auto it: dirents){
      DEBUGF('d', "map it.first:" << it.first
       << " map it.second:" << it.second<< endl);
   }
}

directory::directory(const inode_ptr parent){
   dirents.emplace(".", nullptr);
   dirents.emplace("..", parent);
   for(auto it: dirents){
      DEBUGF('d', "map it.first: " << it.first
       << " map it.second: " << it.second<< endl);
   }
}

size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   // use default directory ctor
   inode_ptr t = make_shared<inode>(file_type::DIRECTORY_TYPE);
   // putting subdir into this dir's map
   dirents.emplace(dirname, t);
   return t;
}

void directory::set_parent(inode_ptr parent){
   DEBUGF('d', "parent = " << parent << endl);
   auto i = dirents.find("..");
   // should always be here
   if (i == dirents.end()) {DEBUGF('d', "oops" << endl);}
   else i->second = parent;
   DEBUGF('d', "i -> second = " << i->second << endl);
}

inode_ptr directory::find(const string& path){
   auto i = dirents.find(path);
   if (i == dirents.end()) {
      DEBUGF('d', "oops" << endl);
      throw file_error ("not a file");
   }
   else 
      return i->second;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode_ptr t = make_shared<inode>(file_type::PLAIN_TYPE);
   // putting subdir into this dir's map
   dirents.emplace(filename, t);
   DEBUGF('d', "address of new plain_file = " << t << endl);
   return t;
}

