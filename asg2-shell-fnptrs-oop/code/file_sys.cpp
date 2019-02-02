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

/*inode_ptr deal_with_path(inode_ptr wd, const string& path){
   if (path == "/"){

   }
}*/

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
   root->get_contents()->set_parent(root);
   root->get_contents()->set_dot(root);
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


size_t num_digits (size_t);
size_t num_digits (size_t size){
   size_t num {0};
   while(size) {
      ++num;
      size /= 10;
   }
   return num;
}

size_t num_digits (int);
size_t num_digits (int size){
   DEBUGF('p', "in num_digits(int)" << endl);
   size_t num {0};
   while(size) {
      ++num;
      size /= 10;
   }
   return num;
}

string _spaces(size_t);
string _spaces(size_t size){
   string spces = "";
   for (size_t i = 1; i != 6 - size; ++i){
      spces += " ";
   }
   return spces;
}

void inode::print_from(){
   contents->print();
}

file_type inode::inode_type(){
   return contents->inode_type();
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

void plain_file::print(){
    throw file_error ("is a plain file");
}

void plain_file::set_dot(inode_ptr){
   throw file_error ("is a plain file");
}

file_type plain_file::inode_type(){
   return file_type::PLAIN_TYPE;
}



directory::directory(){
   dirents.emplace(".", nullptr);
   dirents.emplace("..", nullptr);
   for(auto it: dirents){
      DEBUGF('d', "map it.first:" << it.first
       << " map it.second:" << it.second<< endl);
   }
   size_ = 2;
}

/*directory::directory(const inode_ptr parent){
   dirents.emplace(".", nullptr);
   dirents.emplace("..", parent);
   for(auto it: dirents){
      DEBUGF('d', "map it.first: " << it.first
       << " map it.second: " << it.second<< endl);
   }

}*/

size_t directory::size() const {
   DEBUGF ('i', "size = " << size_);
   return size_;
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
   auto map_itr = dirents.find(dirname);
   // directory already exists
   if (map_itr != dirents.end()){
      throw file_error("mkdir: cannot create directory '" + 
                        dirname + "': File exists");
   }
   dirents.emplace(dirname, t);
   ++size_;
   DEBUGF('d', "size of working directory = " << size_ << endl);
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

void directory::set_dot(inode_ptr self){
   DEBUGF('d', "self = " << self << endl);
   auto i = dirents.find(".");
   // should always be here
   if (i == dirents.end()) {DEBUGF('d', "oops" << endl);}
   else i->second = self;
   DEBUGF('d', "i -> second = " << i->second << endl);
}

// helper fn to get pointer to an inode given a string
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
   ++size_;
   return t;
}

void directory::print(){
   string spaces_nr, spaces_size, name;
   size_t nr_digits;
   DEBUGF('p', " made it to dir::print()" << endl);
   for (auto itor: dirents){
      DEBUGF('p', "made it into loop");
      nr_digits = num_digits(itor.second->get_inode_nr());

      DEBUGF('p', "made it past num_digits");
      spaces_nr = _spaces(nr_digits);
      DEBUGF('p', "made it past _spaces");
      size_t tmp_size = itor.second->get_contents()->size();
      size_t size_digits = num_digits(tmp_size);
      spaces_size = _spaces(size_digits);
      name = itor.first + 
         ((itor.second->inode_type() ==file_type::DIRECTORY_TYPE) and
          (itor.first != "." and itor.first != "..") ?
            "/" : "");
     
      cout << spaces_nr << itor.second->get_inode_nr() << "  "
         << spaces_size << tmp_size << "  " << name << endl;
   }
}

file_type directory::inode_type(){
   return file_type::DIRECTORY_TYPE;
}
