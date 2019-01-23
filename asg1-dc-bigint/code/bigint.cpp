// $Id: bigint.cpp,v 1.76 2016-06-14 16:34:24-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that):
                ubig_value(static_cast<unsigned char>(that)),
                is_negative (that < 0) {
   //int x = static_cast<int>(that);
  // unsigned char y = static_cast<unsigned char>(x);
  // ubig_value();
  // DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue, bool is_negative):
                ubig_value(uvalue), is_negative(is_negative) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   ubig_value = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   bigint result{ubigint{ubig_value}, this->is_negative};
   return result;
}

bigint bigint::operator- () const {
   bigint result{this->ubig_value, not is_negative};
   return result;
}

bigint bigint::operator+ (const bigint& that) const {
   bigint result;
   if(is_negative == that.is_negative){//same sign 
      result.is_negative = is_negative;
      ubigint sum = ubig_value + that.ubig_value;
      result.ubig_value = sum;
   }//else if is new************************
   else if(is_negative){//this -, that +
      if(that.ubig_value < ubig_value){//this value > that value
         result.ubig_value = ubig_value - that.ubig_value;
         result.is_negative = is_negative;
      }else{//this value < that value
         result.ubig_value = that.ubig_value - ubig_value;
         result.is_negative = that.is_negative;
      }
   }else{//that -, this +
      if(that.ubig_value < ubig_value){//this value > that value
         result.ubig_value = ubig_value - that.ubig_value;
         result.is_negative = is_negative;
      }else{//this value < that value
         result.ubig_value = that.ubig_value - ubig_value;
         result.is_negative = that.is_negative;
      }
   }
   return result;
}

bigint bigint::operator- (const bigint& that) const {
   bigint result{};
   if(is_negative == that.is_negative){//same sign
      if(is_negative){//NEG
         if(ubig_value < that.ubig_value){//that bigger
            result.ubig_value = that.ubig_value - ubig_value;
            result.is_negative = false;
         }else{//this bigger
            result.ubig_value = ubig_value - that. ubig_value;
            result.is_negative = true;
         }
      }else{//POS
         DEBUGF ('h', "1is_negative " << is_negative 
                  << " that.is_negative "
                  << that.is_negative);
         if(ubig_value < that.ubig_value){//that bigger
            result.ubig_value = that.ubig_value - ubig_value;
            result.is_negative = true;
         }else{//this bigger
            DEBUGF ('h', "2is_negative " << is_negative 
                  << " that.is_negative "
                  << that.is_negative);
            result.ubig_value = ubig_value - that. ubig_value;
            result.is_negative = false;
         }
      }
   }else if(is_negative){//this - , that +
      if(ubig_value < that.ubig_value){//that > this
         result.ubig_value = that.ubig_value + ubig_value;
         result.is_negative = false;
      }else{//this>that
         result.ubig_value = that.ubig_value + ubig_value;
         result.is_negative = true;
      }
   }else{//this + , that -
      result.ubig_value = that.ubig_value + ubig_value;
      result.is_negative = is_negative;
   }

  return result;
}

bigint bigint::operator* (const bigint& that) const {
   bigint result{};
   if(is_negative == that.is_negative){
      result.is_negative = false;
   }else{
      result.is_negative = true;
   }
   result.ubig_value = ubig_value * that.ubig_value;
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result = that;
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result = that;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return (is_negative == that.is_negative and 
         ubig_value == that.ubig_value);
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? ubig_value > that.ubig_value
                      : ubig_value < that.ubig_value;
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << (that.is_negative ? "-" : "")
              << that.ubig_value;
}


