// $Id: main.cpp,v 1.1 2019-02-17 15:37:07-08 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

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
//using str_key = str_str_map::key_type;

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

void deal_with_lines(str_str_map& my_map, istream& instream, 
                                             string filename){
   regex comment_regex {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};
   string line;
   int counter;
   counter = 1;
   while( getline(instream, line) ){
      if (instream.eof()) break;
      cout << filename <<": "<< counter << ": " << line << endl;
      ++counter;
      smatch result;
      //if found comment line
      if (regex_search (line, result, comment_regex)) {
         cout << "Comment or empty line." << endl;
         continue;
      }
      //if found '=' in line
      if (regex_search (line, result, key_value_regex)) {
         //cout << "key  : \"" << result[1] << "\"" << endl;
         //cout << "value: \"" << result[2] << "\"" << endl;
         if(result[1] == "" and result[2] == ""){
            //print listmap
            DEBUGF('s', "this is key: " << result[1] <<
                        " this is value: " << result[2]);
         }
         else if(result[1] == ""){
            //find by value and all pairs corresp to value
            DEBUGF('s', "this is key: " << result[1] <<
                        " this is value: " << result[2]);
         }
         else if(result[2] == ""){
            // erase case
            //see if key is in listmap
            //if in key map delete node
            //else error no key to be deleted??
            DEBUGF('s', "this is key: " << result[1] <<
                        " this is value: " << result[2]);
            str_str_map::iterator itor = my_map.begin();
            str_str_map::key_type temp = result[1];

            itor = my_map.find(temp);
            DEBUGF('s', "itor: " << *itor);
            if ( itor == my_map.end() ){
               cout << result[1] << ": key not found" << endl;
               DEBUGF('s', "this is key: " << result[1] <<
                        " this is value: " << result[2]);
            }
            else{
               my_map.erase(itor);
            }
         }
         else{
            //first see if key is in listmap
            // if in there replace value
            // else insert key value
            DEBUGF('s', "this is key: " << result[1] <<
                        " this is value: " << result[2]);
            str_str_map::iterator itor = my_map.begin();
            str_str_map::key_type temp = result[1];
            itor = my_map.find(temp);
            if(itor == my_map.end()){
               str_str_pair pair(result[1], result[2]);
               my_map.insert(pair);
            }
            else{//if key found in map then replace
               my_map.erase(itor);
               str_str_pair pair(result[1], result[2]);
               my_map.insert(pair);
            }
            cout << result[1] << " = " << result[2] << endl;
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
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);

   str_str_map my_map;
   char** argp = &argv[optind];

   if(argp ==  &argv[argc]){
      deal_with_lines(my_map, cin, "-");
   }
   else{
      //will not enter loop if no files are given
      for (; argp != &argv[argc]; ++argp) {
         //test.prompt(argp);
         string curr = *argp;
         ifstream fs(curr, std::fstream::ios_base::in);
         
         //counter = 1;
         //str_str_pair pair;
         if(fs.is_open()){
            deal_with_lines(my_map, fs, curr);
         }
         else{// file did not print
            //cerr << complain();
         } 
         //str_str_pair pair (*argp, to_string<int> (argp - argv));
         //cout << "Before insert: " << pair << endl;
         //test.insert (pair);
         fs.close();
      }
   }

   //must have things in listmap first to enter???????????????????????
   for (str_str_map::iterator itor = my_map.begin();
        itor != my_map.end(); ++itor) {
      cout << "During iteration: " << *itor << endl;
   }

   while (not (my_map.empty())){
      my_map.erase (my_map.begin());
   }

   cout << "EXIT_SUCCESS" << endl;
   return EXIT_SUCCESS;
}

