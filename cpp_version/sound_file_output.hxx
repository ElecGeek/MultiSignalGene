#ifndef __SOUND_FILE_OUTPUT__
#define __SOUND_FILE_OUTPUT__

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include "main_loop.hxx"

using namespace std;


/** \brief Handles the timing and the output into a file or a sound channel
 *
 * 
 *
 */

class sound_file_output_base
{
protected:
  main_loop signals;
public:
  explicit sound_file_output_base(const main_loop&signals);
  virtual void run()=0;
};

class sound_file_output_jackaudio: public sound_file_output_base
{
  // No sound_file_output_buffer as it is constructe on each call
  // as one should not cache some data
public:
  explicit sound_file_output_jackaudio(const main_loop&signals);
  ~sound_file_output_jackaudio()=default;
  void run();
};

class sound_file_output_dry: public sound_file_output_base
{
  sound_file_output_buffer sfo_buffer;
  const bool follow_timebeat;
  unsigned long cumul_us_elapsed;
public:
  sound_file_output_dry(const main_loop&signals,
						const bool&follow_timebeat);
  ~sound_file_output_dry()=default;
  void run();
};

class sound_file_output_file: public sound_file_output_base
{
  ofstream outputfile_stream;
  signed short values[20000];
  sound_file_output_buffer sfo_buffer;
  const bool follow_timebeat;
public:
  sound_file_output_file(const main_loop&signals,
						 const string& filename,
						 const bool&follow_timebeat);
  ~sound_file_output_file();
  void run();
};


#endif
