#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>
#include <string>
#include "Misc.h"
using namespace std;
const String WHITESPACE = " \n\r\t\f\v";
 
String ltrim(const String &s) {
  /* function to trim the whitespace, tabs, and newline
   * characters on left side of string. 
   */
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}
 
String rtrim(const String &s) {
  /* function to remove whitespace, tabs, and newline 
   * characters on right side of string. 
   */
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
String trim(const String &s) {
  /* function to trim whitespace and newline/tab characters on
   * both left and right sides of string.
   */
  return rtrim(ltrim(s));
}

void print_datetime() {
  /* **********************************************
   * function print_datetime():
   * Function to print the current date/time to the
   * Linux or UNIX-like console.
   * **********************************************
   */
  time_t start   = time(0);
  char* start_dt = ctime(&start); // pass in memory address of (pointer) start.
  tm *gmtm       = gmtime(&start);
  start_dt       = asctime(gmtm);
  cout << "  " << start_dt << endl;
}

void print_error_msg_and_exit( const char* Msg ) {
/* **************************************************
 * function print_error_msg_and_exit( const char * ):
 *   Simple function to print an error message and
 *   exit this program. 
 */
  cout << "\n";
  cout << "   top-of-atmosphere conversion program encountered error:\n";
  cout << Msg << "\n";
  cout << "   exiting ... \n";
  cout << endl;
  exit(1);
}

/* ****************************************
 * C++ inline function to check to see if a
 * file exists.
 * ****************************************
 */
bool file_exists( const std::string& filename ) { // note: C++ reference parameter.

  // create pointer to FILE object. Set this
  // memory address to have the value of the output of fopen().
  FILE *file = nullptr;
  file = fopen(filename.c_str(),"r" );

  // check to see if file exists. Return True if yes, False if not.
  if( file ) {
    fclose(file); // close file object. release memory.
    return true;
  } else {
    return false;
  }
}
