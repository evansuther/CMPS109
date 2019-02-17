// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <regex>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);
   regex comment_regex {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};
   str_str_map test;
   //will not enter loop if no files are given
   for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
      //test.prompt(argp);
      std::fstream fs;
      string curr = *argp;
      string line;
      fs.open (curr, std::fstream::ios_base::in);
      int counter = 1;
      str_str_pair pair{};
      if(fs.is_open()){
         while ( getline (fs, line) ){
            //read each file line by line
            if (fs.eof()) break;
            cout << argp << ": "<< counter << ": " << line << endl;
            ++counter;
            smatch result;
            //if found comment line
            if (regex_search (line, result, comment_regex)) {
               cout << "Comment or empty line." << endl;
               continue;
            }
            //if found '=' in line
            if (regex_search (line, result, key_value_regex)) {
               cout << "key  : \"" << result[1] << "\"" << endl;
               cout << "value: \"" << result[2] << "\"" << endl;
               if(result[1] == "" and result[2] == ""){
                  //print listmap
                  DEBUGF('s', "this is key: " << result[1] <<
                              " this is value: " << result[2]);
               }
               else if(result[1] == ""){
                  //find by value and print value
                  DEBUGF('s', "this is key: " << result[1] <<
                              " this is value: " << result[2]);
               }
               else if(result[2] == ""){
                  //see if key is in listmap
                  //if in key map delete node
                  //else error no key to be deleted??
                  DEBUGF('s', "this is key: " << result[1] <<
                              " this is value: " << result[2]);
               }
               else{
                  //first see if key is in listmap
                  // if in there replace value
                  // else insert key value
                  DEBUGF('s', "this is key: " << result[1] <<
                              " this is value: " << result[2]);
                  pair = str_str_pair(result[1], result[2]);
                  test.insert(pair);
               }
            }
            //need to check if this is a key
            else if (regex_search (line, result, trimmed_regex)) {
               cout << "query: \"" << result[1] << "\"" << endl;
               //iterator temp = test.find(result[1]);
               //if(temp == end()){
                  //thing not found
                  //cerr << result[1] << ": key not found";
               //}
               //else{
               //   temp.print_value();
               //}
            }
         }
      }else{// file did not print
         //cerr << complain();
      }
      //str_str_pair pair (*argp, to_string<int> (argp - argv));
      //cout << "Before insert: " << pair << endl;
      //test.insert (pair);
      fs.close();
   }

   //must have things in listmap first to enter???????????????????????
   for (str_str_map::iterator itor = test.begin();
        itor != test.end(); ++itor) {
      cout << "During iteration: " << *itor << endl;
   }

   str_str_map::iterator itor = test.begin();
   test.erase (itor);

   cout << "EXIT_SUCCESS" << endl;
   return EXIT_SUCCESS;
}

