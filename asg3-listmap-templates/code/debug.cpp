// $Id: debug.cpp,v 1.1 2019-02-17 15:37:07-08 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

#include <climits>
#include <iostream>

using namespace std;

#include "debug.h"
#include "util.h"

debugflags::flagset debugflags::flags {};

void debugflags::setflags (const string& initflags) {
   for (const unsigned char flag: initflags) {
      if (flag == '@') flags.set();
                  else flags.set (flag, true);
   }
}

// getflag -
//    Check to see if a certain flag is on.

bool debugflags::getflag (char flag) {
   // WARNING: Don't TRACE this function or the stack will blow up.
   return flags.test (static_cast<unsigned char> (flag));
}

void debugflags::where (char flag, const char* file, int line,
                        const char* pretty_function) {
   cout << sys_info::execname() << ": DEBUG(" << flag << ") "
        << file << "[" << line << "] " << endl
        << "   " << pretty_function << endl;
}

