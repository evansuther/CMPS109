// $Id: ubigint.cpp,v 1.14 2016-06-23 17:21:26-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <sstream>
using namespace std;

#include "ubigint.h"
#include "debug.h"


// Possibly leave for easy creation of constants? + easy default init
ubigint::ubigint (unsigned char that) {
   ubig_value.push_back (that);
   //DEBUGF ('~', this << " -> " << ubig_value)
}


 ubigint::ubigint (const string& that) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (auto rev_itr = that.crbegin();
        rev_itr != that.crend(); ++rev_itr){

      if (not isdigit (*rev_itr)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }

      ubig_value.push_back(static_cast<unsigned char>(*rev_itr));
      DEBUGF ('~', "pushed:" << *rev_itr);
   }
  // DEBUGF ('~', ubig_value);
}

ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint result{that};
   result.ubig_value.push_back(static_cast<unsigned char> ('0'));
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   return *this;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result{that};
   result.ubig_value.push_back(static_cast<unsigned char> ('0'));
   return *this;
}

void ubigint::multiply_by_2() {
   return;
}

void ubigint::divide_by_2() {
   return;
}



struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, ubigint divisor) {
   // Note: divisor is modified so pass by value (copy).
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   if (ubig_value.size() != that.ubig_value.size())
      return false;
   else return true;
}

bool ubigint::operator< (const ubigint& that) const {
   if (ubig_value.size() != that.ubig_value.size())
      return false;
   else return true;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   stringstream ss;
   
   for (auto rev_itr = that.ubig_value.crbegin();
        rev_itr != that.ubig_value.crend(); ++rev_itr){
      ss << *rev_itr;
   }
   return out << "ubigint(" << ss.rdbuf() << ")";
}

