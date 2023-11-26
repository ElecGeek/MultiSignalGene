#ifndef __SOUND_FILE_OUTPUT__
#define __SOUND_FILE_OUTPUT__

#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <limits>
#include "main_loop.hxx"
#include "samplerate_handler.hxx"

using namespace std;



/** \brief Handles the timing and the output into a file or a sound channel
 *
 * 
 *
 */
class sound_file_output_base
{
  sound_file_output_base()=delete;
protected:
  main_loop*signals;
  sample_rate_list sr_list;
  sound_file_output_buffer sfo_buffer;
public:
  explicit sound_file_output_base(const sound_file_output_buffer&sfo_buffer);
  virtual~sound_file_output_base()=default;
  void set_signals(main_loop*const&);
  sample_rate_list process_sample_rate(const sample_rate_list&)const;
  virtual bool test_sound_format()const;
  virtual bool is_ready()const;
  virtual bool pre_run();
  virtual void run()=0;
};


class sound_file_output_dry: public sound_file_output_base
{
  const bool follow_timebeat;
  unsigned long cumul_us_elapsed;
public:
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
  sound_file_output_file(const pair< sound_file_output_buffer, ostream* >&data,
						 const bool&follow_timebeat);
  ~sound_file_output_file()=default;
  // bool is_open()const;
  bool test_sound_format()const;
  void run();
};


#endif
