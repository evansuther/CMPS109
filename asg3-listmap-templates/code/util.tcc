// $Id: util.tcc,v 1.1 2019-02-17 15:37:07-08 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

#include <sstream>
#include <typeinfo>
using namespace std;

template <typename Type>
string to_string (const Type& that) {
   ostringstream stream;
   stream << that;
   return stream.str();
}

template <typename Type>
Type from_string (const string& that) {
   stringstream stream;
   stream << that;
   Type result;
   if (not (stream >> result and stream.eof())) {
      throw domain_error (string (typeid (Type).name())
            + " from_string (" + that + ")");
   }
   return result;
}

