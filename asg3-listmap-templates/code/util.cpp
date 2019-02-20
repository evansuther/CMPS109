// $Id: util.cpp,v 1.1 2019-02-17 15:37:07-08 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>
using namespace std;

#include "debug.h"
#include "util.h"

int sys_info::exit_status_ = EXIT_SUCCESS;
string sys_info::execname_; // Must be initialized from main().

void sys_info_error (const string& condition) {
   throw logic_error ("main() has " + condition
               + " called sys_info::execname()");
}

void sys_info::execname (const string& argv0) {
   if (execname_ != "") sys_info_error ("already");
   int slashpos = argv0.find_last_of ('/') + 1;
   execname_ = argv0.substr (slashpos);
   cout << boolalpha;
   cerr << boolalpha;
   DEBUGF ('u', "execname_ = " << execname_);
}

const string& sys_info::execname () {
   if (execname_ == "") sys_info_error ("not yet");
   return execname_;
}

void sys_info::exit_status (int status) {
   if (execname_ == "") sys_info_error ("not yet");
   exit_status_ = status;
}

int sys_info::exit_status () {
   if (execname_ == "") sys_info_error ("not yet");
   return exit_status_;
}

const string datestring () {
   time_t clock = time (nullptr);
   struct tm *tm_ptr = localtime (&clock);
   char timebuf[256];
   strftime (timebuf, sizeof timebuf,
             "%a %b %e %H:%M:%S %Z %Y", tm_ptr);
   return timebuf;
}



ostream& complain() {
   sys_info::exit_status (EXIT_FAILURE);
   cerr << sys_info::execname () << ": ";
   return cerr;
}

void syscall_error (const string& object) {
   complain() << object << ": " << strerror (errno) << endl;
}

