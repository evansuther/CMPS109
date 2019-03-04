// $Id: cix.cpp,v 1.7 2019-02-07 15:14:37-08 - - $
/*
 * Partner: Evan Suther (esuther@ucsc.edu)
 * Partner: Derrick DeBose (ddebose@ucsc.edu)
 */

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"put" , cix_command::PUT },
   {"get" , cix_command::GET },
   {"rm", cix_command::RM}
};

static const string help = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      log << "sent LS, server did not return LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      auto buffer = make_unique<char[]> (header.nbytes + 1);
      recv_packet (server, buffer.get(), header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer.get();
   }
}

void cix_put (client_socket& server, string filename) {
   cix_header header;
   header.command = cix_command::PUT;
   // check FILENAME_SIZE before calling?
   memset (header.filename, 0, FILENAME_SIZE);
   if (filename.size() > FILENAME_SIZE) {
      cerr << "filename size too large" << endl;
      throw cix_exit();
   }
   // set (C->S) header filename
   strncpy(header.filename, filename.c_str(), FILENAME_SIZE);

   FILE* fileptr = fopen (filename.c_str(), "r");
   if (fileptr == nullptr) { 
      // file couldn't be accessed 
      cerr << filename << ": "<< strerror (errno) << endl;
      throw cix_exit();
   }
   long file_size;
   size_t result;
   unique_ptr<char[]> buffer;
   // obtain file size:
   // these 3 lines are from cplusplus.com
   fseek (fileptr , 0 , SEEK_END);
   file_size = ftell (fileptr);
   rewind (fileptr);

   header.nbytes = static_cast<size_t> (file_size);

   if (file_size != 0) {
      buffer = make_unique<char[]> (file_size);
      // fread with size, size mthd from cplusplus.com
      // asg suggests read from istream
      result = fread ( buffer.get(), 1, file_size, fileptr);
      if (result != static_cast<size_t> (file_size)) {
         // file couldn't be read at some point 
         cerr << "put: " << filename << ": Reading error";
         throw cix_exit();
      }
   }

   int status = fclose (fileptr);
   if (status < 0) { 
      // file couldn't be closed, print why 
      cerr << filename << ": "<< strerror (errno) << endl;
      throw cix_exit();
   }

   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);

   // don't send payload if no size
   if (file_size != 0) { 
      log << "sending payload " << endl;
      send_packet (server, buffer.get(), file_size);
      log << "sent " << file_size << " bytes" << endl;

   }

   // get response from server child
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command == cix_command::NAK ) {
      // some weird error happened on the server side
      log << "put: "<< header.filename << ": " 
            << strerror(header.nbytes) << endl;
   }
   // check for not ack?***************
   else if (header.command != cix_command::ACK){
      log << "sent PUT, server did not return ACK OR NAK" << endl;
      log << "server returned " << header << endl;
   }
   else {
      // still code from ls, just wait for ack/nak
      log << "received header with nbytes = " 
             << header.nbytes << " bytes" << endl;
   }
}

void cix_get (client_socket& server, string filename) {
   cix_header header;
   header.command = cix_command::GET;
   // check FILENAME_SIZE before calling
   memset (header.filename, 0, FILENAME_SIZE);
   if (filename.size() > FILENAME_SIZE) {
      cerr << "filename size too large" << endl;
      throw cix_exit();
   }
   strncpy(header.filename, filename.c_str(), FILENAME_SIZE);
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   // get response from server child
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command == cix_command::NAK ) {
         // some weird error happened on the server side
         cerr << "get: "<< header.filename << ": " 
               << strerror(header.nbytes) << endl;   
         throw cix_exit();
   }
   // check for not FILEOUT?***************
   else if (header.command != cix_command::FILEOUT){
      log << "sent GET, server did not return ACK OR NAK" << endl;
      log << "server returned " << header << endl;
   }
   else {
      log << "received header with nbytes = " 
             << header.nbytes << " bytes" << endl;
      // try to open file and check if it worked
      FILE* fileptr = fopen (filename.c_str(), "w");
      if (fileptr == nullptr) { 
         cerr << filename << ": "<< strerror (errno) << endl;
         throw cix_exit();
      }
      // buffer to hold incoming payload
      auto buffer = make_unique<char[]> (header.nbytes);
      if (header.nbytes != 0)
         recv_packet(server, buffer.get(), header.nbytes);
         
      
      // write whole buffer byte by byte, check status?????????
      fwrite(buffer.get(), 1, header.nbytes, fileptr);
      // attempt to close; if error, print it
      int status = fclose (fileptr);
      if (status < 0) {
         cerr << header.filename << ": " << strerror (errno) << endl;
         header.command = cix_command::NAK;
         header.nbytes = errno;
         send_packet (server, &header, sizeof header);
         return; 
      }
      // will be deleted
      else{
         log << header.filename << ": exit " << (status >> 8)
                   << " signal " << (status & 0x7F)
                   << " core " << (status >> 7 & 1) << endl;
      }
      
   }

}

void cix_rm (client_socket& server, string filename) {
   cix_header header;
   header.command = cix_command::RM;
   // check FILENAME_SIZE before calling
   memset (header.filename, 0, FILENAME_SIZE);
   if (filename.size() > FILENAME_SIZE) {
      cerr << "filename size too large" << endl;
      throw cix_exit();
   }
   snprintf(header.filename, FILENAME_SIZE,"%s", filename.c_str());
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command == cix_command::NAK ) {
      log << header.filename << ": remove failed: " 
          << strerror (header.nbytes) << endl;
   }
}



void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         log << "command line:" << line << endl;

         // split line on whitespace
         vector<string> wordvec = split(line, " \t");
         // look up cmd not whole line
         const auto& itor = command_map.find (wordvec.at(0));
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::PUT:
               if (wordvec.size() != 2){
                  cerr << "put: insufficient arguments" << endl;
                  throw cix_exit();
               }
               cix_put (server, wordvec.at(1));
               break;
            case cix_command::GET:
               if (wordvec.size() != 2){
                  cerr << "get: insufficient arguments" << endl;
                  throw cix_exit();
               }
               cix_get (server, wordvec.at(1));
               break;
            case cix_command::RM:
               if (wordvec.size() != 2){
                  cerr << "rm: insufficient arguments" << endl;
                  throw cix_exit();
               }
               cix_rm (server, wordvec.at(1));
               break;
            default:
               log << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

