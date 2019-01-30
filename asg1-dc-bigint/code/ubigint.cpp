// $Id: ubigint.cpp,v 1.14 2016-06-23 17:21:26-07 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

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
   ubigint result{};
   int total;
   int carry=0;
   if(*this < that){
      // loop to that size, dont want to hit null pointer with this size
      for(unsigned int cnt = 0; cnt != that.ubig_value.size(); ++cnt){
         // this ran out of digits
         if (ubig_value.size() < cnt or ubig_value.size() == cnt){
            total = (that.ubig_value.at(cnt)-'0') + carry;
            result.ubig_value.push_back(total%10 +'0');
            carry = total/10;

         }
         // this still has digits
         else {
            total = (ubig_value.at(cnt)-'0') +
                     (that.ubig_value.at(cnt)-'0') + carry;
            result.ubig_value.push_back(total%10 +'0');
            carry = total/10;
         }
      }
      // out of loop
      if(carry != 0){
         result.ubig_value.push_back(carry+'0');
      }
      DEBUGF ('o', "carry " << carry <<" total" << total
                   << " that size "<< that.ubig_value.size() );
   }
   // mirrored this --> that
   else{
      for (unsigned int cnt = 0; cnt != ubig_value.size(); cnt++){
         if (that.ubig_value.size() < cnt 
                                 or that.ubig_value.size() == cnt){
            total = (ubig_value.at(cnt)-'0') + carry;
            result.ubig_value.push_back(total%10 +'0');
            carry = total/10;
         }else {
            total = (ubig_value.at(cnt)-'0') +
                     (that.ubig_value.at(cnt)-'0') + carry;
            result.ubig_value.push_back(total%10 +'0');
            carry = total/10;
         }
         /*if(carry!=0 and ubig_value.size() == cnt){
            result.ubig_value.push_back(carry);
         }*/
         
      }
      if(carry != 0){
         result.ubig_value.push_back(carry+'0');
      }
      DEBUGF ('o', "carry " << carry <<" total" << total
                  << " this size "<< ubig_value.size()  );
      
   }
   //result.ubig_value.push_back(static_cast<unsigned char> ('0'));
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   DEBUGF ('n', " in sub this : " << *this 
                  << " that : "<< that  );
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result{};
   int total;
   int carry=0;
   for(unsigned int cnt = 0; cnt != ubig_value.size(); ++cnt){
      if (that.ubig_value.size() <= cnt){
         total = (ubig_value.at(cnt)-'0')  + carry;
         if(total<0){
            carry = -1;
            total = total + 10;
            result.ubig_value.push_back(total +'0');
         }else{
            carry = 0;
            result.ubig_value.push_back(total +'0');
         }

      }else{
         total = (ubig_value.at(cnt)-'0') -
                     (that.ubig_value.at(cnt)-'0') + carry;
         if(total<0){
            carry = -1;
            total = total + 10;
            result.ubig_value.push_back(total +'0');
         }else{
            carry = 0;
            result.ubig_value.push_back(total +'0');
         }
      }
      DEBUGF ('h', "carry " << carry <<" total" << total
                  << " this size "<< ubig_value.size() 
                  << " this cnt " << cnt <<
                  " that size" << that.ubig_value.size()
                  << "rubig_value.at(cnt)" << result.ubig_value.at(cnt)
                  );
   }
   while (result.ubig_value.size() > 0 and
          result.ubig_value.back() == '0') result.ubig_value.pop_back();
   return result;
}

void ubigint::multiply_by_2() {
   ubigint tmp{*this};
   *this = tmp + tmp;
   return;
}

// problems
void ubigint::divide_by_2() {
   int tmp;
   // size returns |elem(vect)|
   for(unsigned int cnt = 0; cnt <= ubig_value.size(); ++cnt){
      // able to do evencheck
      if(cnt < ubig_value.size()-1){
         DEBUGF('m', "this is big_value.at(cnt+1): " <<
                     ubig_value.at(cnt+1) <<
                     " this is big_value.at(cnt): " <<
                     ubig_value.at(cnt) << " cnt: " <<
                     cnt << " this is ubig_value: " <<
                     *this);

         int even_check = ((ubig_value.at(cnt+1) - '0')%2);
         DEBUGF ('m', " div_by_2, even check:"<< 
                     (even_check)
                     << " cnt: " << cnt);

         if(even_check == 0){//next num even
            DEBUGF ('m', " even val check: "<< 
                     (ubig_value.at(cnt))
                     << " cnt " << cnt);

            tmp = ubig_value.at(cnt) -'0';
            DEBUGF ('m', " tmp - 0: "<< 
                     tmp
                     << " cnt: " << cnt);

            tmp = tmp/2;
            DEBUGF ('m', " tmp/2: "<< 
                     tmp
                     << " cnt: " << cnt);

            ubig_value.at(cnt)= tmp+'0';
            DEBUGF ('m', " even check 2: this is ubig_value "<< 
                     ubig_value.at(cnt)
                     << " cnt " << cnt);

         }else{//next num odd
            DEBUGF ('m', " odd val check "<< 
                     (ubig_value.at(cnt))
                     << " cnt " << cnt);

            tmp = ubig_value.at(cnt) - '0';
            DEBUGF ('m', " tmp - 0: "<< 
                     tmp
                     << " cnt: " << cnt);

            tmp = (tmp/2)+5;
            DEBUGF ('m', " tmp / 2 + 5: "<< 
                     tmp
                     << " cnt: " << cnt);

            ubig_value.at(cnt)= tmp+'0';
            DEBUGF ('m', " odd check 2: this is ubig_value "<< 
                     ubig_value.at(cnt)
                     << " cnt " << cnt);

         }
      }
      //ensure that we divide last number without checking null space
      else if(cnt == ubig_value.size()-1){
         tmp = ubig_value.at(cnt) -'0';
         tmp = tmp/2;
         ubig_value.at(cnt)= tmp + '0';
      }
      DEBUGF('m', "this end loop:"<< *this << " cnt: " << cnt);
   }
   // normalizer
   while (ubig_value.size() > 0 and
          ubig_value.back() == '0') ubig_value.pop_back();
   DEBUGF('m', "passed in value" << *this);
   return;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result{};
   ubigint power_of_2;
   ubigint tmp_that{};
   ubigint tmp_this{};
   tmp_that = that;
   tmp_this = *this;
   ubigint zero{};
   power_of_2.ubig_value.push_back('1');
   if (*this < that){//that > this
      while (power_of_2 < that){
         power_of_2.multiply_by_2();
         tmp_this.multiply_by_2();
      }
      power_of_2.divide_by_2();
      tmp_this.divide_by_2();
      DEBUGF ('n', " before loop: power_of_2: "<< power_of_2 <<
                   " tmp.this: "<< tmp_this << 
                   " tmp.that: "<< tmp_that);
      while(not(tmp_that == zero )){
         DEBUGF ('n', " that>this; power_of_2: "<< power_of_2 <<
                        " tmp_that; "<< tmp_that <<
                        " tmp_this; "<< tmp_this <<
                        " result; "<< result);
         if (power_of_2 < tmp_that or power_of_2 == tmp_that){
            DEBUGF ('n', " tmp_that; "<< tmp_that <<
                         " tmp_this; "<< tmp_this <<
                         " result; "<< result);
            result = result + tmp_this;
            
            tmp_that = tmp_that - power_of_2;
            power_of_2.divide_by_2();
            tmp_this.divide_by_2();
         }else{
            power_of_2.divide_by_2();
            tmp_this.divide_by_2();
         }
         
      }
      
   }else{//that < this
      while (power_of_2 < *this){
         power_of_2.multiply_by_2();
         tmp_that.multiply_by_2();
      }
      power_of_2.divide_by_2();
      tmp_that.divide_by_2();
      DEBUGF ('n', " before loop2: power_of_2: "<< power_of_2 <<
                   " tmp.this: "<< tmp_this << 
                   " tmp.that: "<< tmp_that);
      while(not(tmp_this == zero)){
         DEBUGF ('n', " that<this; power_of_2: "<< power_of_2 <<
                        " tmp_that; "<< tmp_that <<
                        " tmp_this; "<< tmp_this);
         if (power_of_2 < tmp_this or power_of_2 == tmp_this){
            result = result + tmp_that;
            DEBUGF ('n', " that<this in if; result: "<< result);
            tmp_this = tmp_this - power_of_2;
            power_of_2.divide_by_2();
            tmp_that.divide_by_2();
         }else{
            power_of_2.divide_by_2();
            tmp_that.divide_by_2();
         }

      }
   }
   


   return result;
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
   else {
      auto L_itr = ubig_value.crbegin();
      auto R_itr = that.ubig_value.crbegin();

      while (L_itr != ubig_value.crend() &&
             R_itr != that.ubig_value.crend()) {

         if (*L_itr != *R_itr) {
            return false;
         }
         ++L_itr;
         ++R_itr;
      }
      return true;
   }
}

bool ubigint::operator< (const ubigint& that) const {
   if (ubig_value.size() != that.ubig_value.size())
      return ubig_value.size() < that.ubig_value.size();
   else{
      //auto L_itr, R_itr;
      auto L_itr = ubig_value.crbegin();
      auto R_itr = that.ubig_value.crbegin();
      while (L_itr != ubig_value.crend() &&
             R_itr != that.ubig_value.crend()){

         if (*L_itr != *R_itr) {
            return *L_itr < *R_itr;
         }
         ++L_itr;
         ++R_itr;
      }
      return false;
   }
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   stringstream ss;
   if (that.ubig_value.size() != 0){
      int count=0;
      for (auto rev_itr = that.ubig_value.crbegin();
           rev_itr != that.ubig_value.crend(); ++rev_itr)
      {
          ++count;
          if(count %70==0){
             ss << '\\';
             ss << '\n';
             count=1;
         }
        ss << *rev_itr;
      }
   } else {
      ss << '0';
   }
   return out << ss.rdbuf();
}