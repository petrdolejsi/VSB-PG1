#ifndef TUTORIALS_H_
#define TUTORIALS_H_

void error_handler( void * user_ptr, const RTCError code, const char * str = nullptr );

int tutorial_1( const char * config = "threads=0,verbose=3" );
int tutorial_2();
int allied_avenger( const std::string file_name, const char * config = "threads=0,verbose=0" );
int allied_avenger_bigger(const std::string file_name, const char * config = "threads=0,verbose=0");
int sphere(const std::string file_name, const char * config = "threads=0,verbose=0");
int box(const std::string file_name, const char * config = "threads=0,verbose=0");

#endif
