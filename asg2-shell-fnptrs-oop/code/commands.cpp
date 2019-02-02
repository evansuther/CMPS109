// $Id: commands.cpp,v 1.17 2018-01-25 14:02:55-08 - - $

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

inode_ptr deal_with_path(inode_ptr wd, const string& path){
   if (path == "/"){
      return wd;
   }
   else{
      wordvec parts = split(path, "/");
      inode_ptr current = wd;
      for (auto part : parts){
         auto found = current->get_contents()->find(part);
         current = found;
      }
      return current;
   }
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() <= 1) {
      throw command_error("cat: no file specified");
   }
   inode_ptr f = deal_with_path(state._wd_(), words.at(1));
   cout << f->get_contents()->readfile() << endl;
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   int ex_st = 0;
   if (words.size() != 1){
      ex_st = atoi(words.at(1).c_str());
      // non-numeric argument
      if (ex_st == 0 && words.at(1).at(0) != '0')
         ex_st = 127;
   }
   exit_status::set(ex_st);
   throw ysh_exit();
}

// delete if still not useful +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*size_t num_digits (size_t);
size_t num_digits (size_t size){
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
   for (size_t i = 1; i != size - 6; ++i){
      spces += " ";
   }
   return spces;
}*/

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   // no argument, use wd
   if (words.size() == 1) {
      cout << "/:" << endl;
      state._wd_()->print_from();
   }
   // check if ls has operands(directories)
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // deal with paths ***********************************************
   // deal_with_path(path) will throw exn at 
   // end of path for something that doesn't exist yet
   inode_ptr new_fle;
   if (words.at(1).find("/") == string::npos){
      new_fle = state._wd_()->get_contents()->mkfile(words.at(1));
   }
   else{
      new_fle = deal_with_path(state._wd_(), words.at(1));
   }
   
   // pointer to empty file in directory
   
   // put the words into the file
   new_fle->get_contents()->writefile(words);
}



void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // deal with paths ***********************************************
   // deal_with_path(path) will throw exn at 
   // end of path for something that doesn't exist yet
   inode_ptr working_dir = state._wd_();
   // make default dir, add to wd's map
   inode_ptr new_dir = working_dir->get_contents()->mkdir(words.at(1));
   // need to connect child to parent
   new_dir->get_contents()->set_parent(working_dir);
   new_dir->get_contents()->set_dot(new_dir);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   state.prompt(words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << (state._rt_() == state._wd_() ? "/" : "??") << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

