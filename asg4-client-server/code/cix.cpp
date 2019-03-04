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
   {"get" , cix_command::GET }
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
   // check FILENAME_SIZE before calling
   memset (header.filename, 0, FILENAME_SIZE);
   if (filename.size() > FILENAME_SIZE) {
      cerr << "filename size too large" << endl;
      throw cix_exit();
   }
   // couldnt directly assign header.filename with c_str()
   // then lint complained about strcpy() and suggested snprintf
   // I think I've done this in c but haven't tested here yet
   snprintf(header.filename, FILENAME_SIZE,"%s", filename.c_str());
   

   // open file, read bytes to buf, send
   FILE* fileptr = fopen (filename.c_str(), "r");
   if (fileptr == nullptr) { 
      log << filename << ": "<< strerror (errno) << endl;
      throw cix_exit();
   }
   // string ls_output;
   long file_size;
   // char * buffer;
   size_t result;

   // obtain file size:
   // these 3 lines are from cplusplus.com
   fseek (fileptr , 0 , SEEK_END);
   file_size = ftell (fileptr);
   rewind (fileptr);
   // file_size = fileptr->tellg(); requires ifstream

   header.nbytes = static_cast<size_t> (file_size);
   // using 
   auto buffer = make_unique<char[]> (file_size);
   // fread with size, size mthd from cplusplus.com
   // asg suggests read from istream
   result = fread ( buffer.get(), 1, file_size, fileptr);
   //result = fread (buffer)
   if (result != static_cast<size_t> (file_size)) {
      // CHANGE ****************************
      fputs ("Reading error",stderr); exit (3);
   }
   /*if (size_or_eof < 0) log << filename << ": " << strerror (errno) 
                           << endl;
              else log << filename << "read was fucked" << endl; */

   int status = fclose (fileptr);
   if (status < 0) log << filename << ": " << strerror (errno) << endl;
              else log << filename << ": exit " << (status >> 8)
                       << " signal " << (status & 0x7F)
                       << " core " << (status >> 7 & 1) << endl;
   
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   log << "sending payload " << endl;
   send_packet (server, buffer.get(), file_size);
   log << "sent " << file_size << " bytes" << endl;

   // get response from server child
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command == cix_command::NAK ) {
      log << "sent PUT, server returned NAK" << endl;
      log << "server returned header:" << header << endl;
   }else {
      // still code from ls, just wait for ack/nak
      //auto buffer2 = make_unique<char[]> (header.nbytes + 1);
      //recv_packet (server, buffer2.get(), header.nbytes);
      log << "received header with nbytes = " 
             << header.nbytes << " bytes" << endl;
      //buffer[header.nbytes] = '\0';
      //cout << buffer2.get();
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
   // couldnt directly assign header.filename with c_str()
   // then lint complained about strcpy() and suggested snprintf
   // I think I've done this in c but haven't tested here yet
   snprintf(header.filename, FILENAME_SIZE,"%s", filename.c_str());
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   // get response from server child
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command == cix_command::NAK ) {
      log << "sent PUT, server returned NAK" << endl;
      log << "server returned header:" << header << endl;
   }else {
      // still code from ls, just wait for ack/nak
      //auto buffer2 = make_unique<char[]> (header.nbytes + 1);
      //recv_packet (server, buffer2.get(), header.nbytes);
      log << "received header with nbytes = " 
             << header.nbytes << " bytes" << endl;
      //buffer[header.nbytes] = '\0';
      //cout << buffer2.get();
      // open file, read bytes to buf, send
      FILE* fileptr = fopen (filename.c_str(), "w");
      if (fileptr == nullptr) { 
         log << filename << ": "<< strerror (errno) << endl;
         throw cix_exit();
      }
      auto buffer = make_unique<char[]> (header.nbytes);
      recv_packet(server, buffer.get(), header.nbytes);
      fwrite(buffer.get(), 1, header.nbytes, fileptr);
      int status = fclose (fileptr);
      if (status < 0) {
         log << header.filename << ": " << strerror (errno) << endl;
         header.command = cix_command::NAK;
         header.nbytes = errno;
         send_packet (server, &header, sizeof header);
         return; 
      }
      else{
         log << header.filename << ": exit " << (status >> 8)
                   << " signal " << (status & 0x7F)
                   << " core " << (status >> 7 & 1) << endl;
      }
      
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
         log << "wordvec.at(0) = " << wordvec.at(0) << endl;
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
               cix_put (server, wordvec.at(1));
               break;
            case cix_command::GET:
               cix_get (server, wordvec.at(1));
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

