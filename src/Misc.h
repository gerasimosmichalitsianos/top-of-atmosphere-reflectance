#ifndef MISC_H_
#define MISC_H_
#include <iostream>
typedef std::string String;
String ltrim(const String &s);
String rtrim(const String &s);
String trim(const String &s);
void print_datetime();
void print_error_msg_and_exit( const char* );
bool file_exists( const std::string& );
#endif
