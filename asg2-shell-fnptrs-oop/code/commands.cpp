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
      throw command_error (cmd + ": command not found");
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

//Need to get the last thing in the path
inode_ptr deal_with_path_ls(inode_ptr wd, const string& path){
   if (path == "/"){
      return wd;
   }
   else{
      wordvec parts = split(path, "/");
      inode_ptr current = wd;
      //do a more norm itor go to parts -1
      for (size_t i = 0; i != parts.size(); ++i){
      //for (auto part : parts){
         auto found = current->get_contents()->find(parts[i]);
         if(found == nullptr){
            return nullptr;
         }else{
            current = found;
         }
         
      }
      return current;
   }
}

//Need to get second to last thing in path
inode_ptr deal_with_path_mk(inode_ptr wd, const string& path){
   if (path == "/"){
      return wd;
   }
   else{
      wordvec parts = split(path, "/");
      inode_ptr current = wd;
      //do a more norm itor go to parts -1
      for (size_t i = 0; i != parts.size() -1; ++i){

         auto found = current->get_contents()->find(parts[i]);
         current = found;
      }
      return current;
   }
}
// derr's
void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() <= 1) {
      throw command_error("cat: no file specified");
   }
   for (vector<string>::const_iterator itor = words.begin();
         itor != words.end(); ++itor){
      if(itor == words.begin()){++itor;}
      inode_ptr final_path = deal_with_path_ls(state._wd_(), *itor);
      if(final_path == nullptr){
         cerr << "cat: " << *itor << 
            ": No such file or directory" << endl;
      }else{
         if(final_path->get_contents()->inode_type() ==
                file_type::DIRECTORY_TYPE){
            cerr << "cat: "<< *itor << 
               ": No such file or directory" << endl;
         }
         else{
            cout << final_path->get_contents()->readfile();
            cout << endl;
         }
         
      }
      
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() == 1){
      state.set_cwd(state._rt_());
   }
   else{
      inode_ptr final_dir = 
            deal_with_path_ls(state._wd_(), words.at(1));
      if(final_dir == nullptr){
         throw command_error("cd: "       + 
                             words.at(1) +
                             " no file specified");
      }else{
         state.set_cwd(final_dir);
      }
      
   }
   
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

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   // no argument, use wd
   ////////////////////////////////////////////////////////////////////
   inode_ptr final_path;
   if (words.size() == 1) {
      if (state._wd_() != state._rt_()){
         string dirname = 
            state._rt_()->get_contents()->build_path(state._wd_());
         cout << "/" << dirname <<  ":" << endl;
      }
      else{
         //no args and at root
         cout << "/:" << endl;
      }
      final_path = state._wd_();
   }
   else if (words.at(1)== "/"){
      cout << "/:" << endl;
      final_path = state._rt_();
   }
   else if (words.size() != 1){
      inode_ptr working_dir = state._wd_();
      final_path = deal_with_path_ls(working_dir, words.at(1));
      if(final_path == nullptr){
         throw file_error("ls: cannot access " + 
               words.at(1) + ": No such file or directory");
      }
      else{
         //do a more norm itor go to parts -1
         cout << words.at(1) <<":" << endl;
      }

   }
   final_path->print_from();
   // check if ls has operands(directories)
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr final_path;
   string dirname;
   vector<inode_ptr> subdirs;
   if (words.size() == 1) {
      if (state._wd_() != state._rt_()){
         dirname = state._rt_()->get_contents()->build_path(
            state._wd_());

      }
      else{
         //no args and at root
         dirname = "";
      }
      final_path = state._wd_();
      subdirs = 
         state._wd_()->get_contents()->get_subdirs();

   }
   //need to print from root
   else if (words.at(1)== "/"){
      dirname = "";
      final_path = state._rt_();
      subdirs = 
         state._rt_()->get_contents()->get_subdirs();
   }
   else if (words.size() != 1){
      inode_ptr working_dir = state._wd_();
      final_path = deal_with_path_ls(working_dir, words.at(1));
      if(final_path == nullptr){
         throw file_error("ls: cannot access " + 
               words.at(1) + ": No such file or directory");
      }
      else{
         subdirs = 
            final_path->get_contents()->get_subdirs();
         wordvec parts = split(words.at(1), "/");
         //make an itor to go to parts -1
         vector<string>::const_iterator itor = words.end();
         --itor;

         dirname = words.at(1);
      }

   }
   if(dirname == ""){
      cout << "/:" << endl;
   }else{
      cout << dirname <<  ":" << endl;
   }
   
   final_path->print_from();

   for(auto itor : subdirs){
      dirname = itor->get_contents()->build_path(itor);
      //cout << dirname <<  ":" << endl;
      itor->get_contents()->print_recursive();
   }
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
      new_fle = deal_with_path_mk(state._wd_(), words.at(1));
   }
   
   // pointer to empty file in directory
   
   // put the words into the file
   if(new_fle == nullptr){
      //prints file name and shouldn't*********************************
      throw file_error("make: cannot create directory '" + 
                        words.at(1) + "': No such file or directory");
   }
   else{
      new_fle->get_contents()->writefile(words);  
   }
   
}



void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // deal with paths ***********************************************
   // deal_with_path(path) will throw exn at 
   // end of path for something that doesn't exist yet
   inode_ptr working_dir = state._wd_();
   // make default dir, add to wd's map
   inode_ptr new_dir;
   //if words.at(1) doesnt have a slash
   //there is no need to deal with paths
   if (words.at(1).find("/") == string::npos){
      new_dir = working_dir->get_contents()->mkdir(words.at(1));
   }
   else{//deal with directory paths
      new_dir = deal_with_path_mk(working_dir, words.at(1));
   }
   //check if directory not found
   if(new_dir == nullptr){
      throw file_error("mkdir: cannot create directory '" + 
                        words.at(1) + "': No such file or directory");
   }
   else{// need to connect child to parent
      new_dir->get_contents()->set_parent(working_dir);
      new_dir->get_contents()->set_dot(new_dir);  
   }
   
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   state.prompt(words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << (state._rt_() == state._wd_() ? "/" :
      state._wd_()->get_contents()->build_path(state._wd_())) << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

