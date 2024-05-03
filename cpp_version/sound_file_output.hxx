#ifndef __SOUND_FILE_OUTPUT__
#define __SOUND_FILE_OUTPUT__

#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <limits>

using namespace std;

#include "sound_base_output.hxx"



class sound_file_output_dry: public sound_file_output_base
{
  const bool follow_timebeat;
  unsigned long cumul_us_elapsed;
public:
  sound_file_output_dry()=delete;
  sound_file_output_dry(const bool&follow_timebeat);
  ~sound_file_output_dry()=default;
  void run();
};

class sound_file_output_file: public sound_file_output_base
{
  ostream*const output_stream;
  signed short values[20000];
  const bool follow_timebeat;
  unsigned long cumul_us_elapsed;
  // bool is_open_b;
public:
  sound_file_output_file()=delete;
  sound_file_output_file(const pair< sound_data_output_buffer, ostream* >&data,
						 const bool&follow_timebeat);
  ~sound_file_output_file()=default;
  // bool is_open()const;
  bool test_sound_format()const;
  void run();
};


#endif
