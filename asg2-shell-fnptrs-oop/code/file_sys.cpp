// $Id: file_sys.cpp,v 1.6 2018-06-27 14:44:57-07 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

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
   // set . and .. for root
   root->get_contents()->set_parent(root);
   root->get_contents()->set_dot(root);
   // cwd is init to root
   cwd = root;
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

const string& inode_state::prompt() const { return prompt_; }

// implements prompt fn
void inode_state::prompt(const wordvec& words){
   string pr;
   for(size_t i = 1; i != words.size(); ++i){
      pr = pr + words.at(i) + " ";
   }
   prompt_ = pr;
}

// used to change cwd in cd
void inode_state::set_cwd(inode_ptr final_dir){
   cwd = final_dir;
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

void inode::disown(){
   contents->disown();
}

void inode::rm(string rm_me){
   // called on parent directory to deal with from there
   contents->rm(rm_me);
}

size_t inode::size() const{
   return contents->size();
}

// num_digits is used to figure the amount of spaces needed in ls
size_t num_digits (size_t);
size_t num_digits (size_t size){
   size_t num {0};
   if(size != 0){
      while(size) {
         ++num;
         size /= 10;
      }
   }
   else{
      num = {1};
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

// returns the spaces for ls formatting
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

inode_ptr inode::find(string find_me){
   return contents->find(find_me);
}

// call on parent directory
string inode::find_name(inode_ptr find_me){
   return contents->find_name(find_me);
}

file_type inode::inode_type(){
   return contents->inode_type();
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

// calculates the size of a file for ls
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

// used in make to store file contents in wordvec
void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   if (data.size() != 0)
   {
      data = {};
   }
   for(size_t i = 2; i != words.size(); ++i){
      DEBUGF('d', "pushing " << words.at(i) << " to data" << endl);
      data.push_back(words.at(i));
   }
   DEBUGF('d', "size of file = " << size());
}

// for OOP 
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
    return nullptr;
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

void plain_file::disown(){
   return;
}

string plain_file::find_name(inode_ptr) {
   throw file_error ("is a plain file");
}

vector<inode_ptr> plain_file::get_subdirs(){
   throw file_error ("is a plain file");
}

inode_ptr plain_file::get_parent(){
   throw file_error ("is a plain file");
}
string plain_file::build_path(inode_ptr){
   throw file_error ("is a plain file");
}

void plain_file::print_recursive(){
   throw file_error ("is a plain file");
}

void plain_file::rm(string){
   throw file_error ("RM $$$is a plain file");
}

// default ctor
directory::directory(){
   dirents.emplace(".", nullptr);
   dirents.emplace("..", nullptr);
   for(auto it: dirents){
      DEBUGF('d', "map it.first:" << it.first
       << " map it.second:" << it.second<< endl);
   }
   size_ = 2;
}

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
   // increment size, dir size is number of entries
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
      return nullptr;
   }
   else 
      return i->second;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode_ptr t = make_shared<inode>(file_type::PLAIN_TYPE);
   auto map_itr = dirents.find(filename);
   // directory already exists
   if (map_itr != dirents.end()){
      return map_itr->second;
   }
   // putting subdir into this dir's map
   dirents.emplace(filename, t);
   DEBUGF('d', "address of new plain_file = " << t << endl);
   ++size_;
   return t;
}

void directory::print(){
   string spaces_nr, spaces_size, name;
   size_t nr_digits;
   for (auto itor: dirents){
      nr_digits = num_digits(itor.second->get_inode_nr());

      spaces_nr = _spaces(nr_digits);
      size_t tmp_size = itor.second->get_contents()->size();
      size_t size_digits = num_digits(tmp_size);
      spaces_size = _spaces(size_digits);
      // lol
      name = itor.first + 
         ((itor.second->inode_type() ==file_type::DIRECTORY_TYPE) and
          (itor.first != "." and itor.first != "..")              
          ? "/" : "");
     
      cout << spaces_nr << itor.second->get_inode_nr() << "  "
         << spaces_size << tmp_size << "  " << name << endl;
   }
}

void directory::print_recursive(){
   string path = build_path(dirents.find(".")->second);
   cout << path << ":" << endl;
   print();
   vector<inode_ptr> subdirs = get_subdirs();
   for (auto inode_itor: subdirs){
      inode_itor->get_contents()->print_recursive();
   }
}

file_type directory::inode_type(){
   return file_type::DIRECTORY_TYPE;
}

// used to free allocated memory for directory
// gets rid of memory leak 
void directory::disown(){
   for (auto itor: dirents){
      if (itor.first != ".." and itor.first != ".")
         itor.second->get_contents()->disown();
   }
   dirents.erase(".");
   dirents.erase("..");
}

// used to remove an entry in a directory
void directory::rm(string rm_me){
   auto maybe_me = dirents.find(rm_me);
   if (maybe_me == dirents.end()){
      throw file_error("No such file or directory");
   }
   else{
      maybe_me->second->disown();
      dirents.erase(rm_me);
   }
   --size_;
}

// returns the name of the inode we want to find
string directory::find_name(inode_ptr find_me) {
   base_file_ptr dir_maybe_me;
   vector<base_file_ptr> subdirs {};
   // iterate the hash map
   for (auto inode_itor : dirents){
      // we do not want find these 3 things ever
      if(inode_itor.first != "."  && 
         inode_itor.first != ".." &&
         inode_itor.second != nullptr)
      {
         dir_maybe_me = inode_itor.second->get_contents();
         // found the inode corresp to the ptr arg find_me
         if (inode_itor.second == find_me){
            DEBUGF('f', "found_name: " << inode_itor.first << endl);
            return inode_itor.first;
         }
         // still not found push subdirs to check later
         else if (dir_maybe_me->inode_type() == 
                           file_type::DIRECTORY_TYPE)
         {
            subdirs.push_back(dir_maybe_me);
         }
      }
   }
   string name;
   // iterate sub directories to see if there is a path to find name
   for (auto subdirs_itor : subdirs){
      name = subdirs_itor->find_name(find_me);
      if (name != ""){
         return name;
      }
   }
   return "";
}

//gets all sub directories to be able to check all paths
vector<inode_ptr> directory::get_subdirs(){
   base_file_ptr maybe_dir;
   vector<inode_ptr> subdirs{};
   for (auto inode_itor : dirents){
      if(inode_itor.first != "."  && 
         inode_itor.first != ".." &&
         inode_itor.second != nullptr)
      {
         maybe_dir = inode_itor.second->get_contents();
         if (maybe_dir->inode_type() == 
                           file_type::DIRECTORY_TYPE)
         {
            subdirs.push_back(inode_itor.second);
         }
      }
   }
   return subdirs;
}

//find the parent of the current directory
inode_ptr directory::get_parent(){
   auto parent = dirents.find("..");
   return parent->second;
}

//used for printing complete path names
string directory::build_path(inode_ptr find_me){
   string path{};
   //parent is parent of current directory
   inode_ptr parent{find_me->get_contents()->get_parent()};
   inode_ptr current{parent};
   //intialize path to final path we want
   path = '/' + parent->get_contents()->find_name(find_me);
   DEBUGF('q', "this is find me: " << find_me 
      << "this is parent: " << parent << "this is parent's parent"
      << parent->get_contents()->get_parent()<< endl);
   //parent is equal to itself we are at the root of our tree
   while(parent != parent->get_contents()->get_parent()){
      //iterate parent
      parent = parent->get_contents()->get_parent();
      // add parent name to path
      path = '/' + parent->get_contents()->find_name(current) + path;
      //current makes sure we can still access parent
      //when parent is now the grand parent
      current = parent;

      DEBUGF('q', "this is new parent: " << parent << endl);
   }
   return path;
}
