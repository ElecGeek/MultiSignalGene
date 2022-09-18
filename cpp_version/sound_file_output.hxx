#ifndef __SOUND_FILE_OUTPUT__
#define __SOUND_FILE_OUTPUT__

#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <limits>
#include "main_loop.hxx"

using namespace std;

#include <jack/jack.h>
#include <jack/midiport.h>


/** \brief Sample rate list
 *
 * Functions to find the sample rate accroding with the restrictions \n
 * of the output modules and the requested sample rate
 */
class sample_rate_list
{
  bool don_t_care;
  set<unsigned char>list_sr;
public:
  sample_rate_list();
  void add_value(const unsigned char&);
  /* \brief Find common values
   *
   * In case of don't care, copy the right list
   * In case not, merges the lists
   */
  sample_rate_list intersect_sr_list( const sample_rate_list&)const;
  /* \brief Get the sample rate to use
   *
   * Accroding with some default choices, return the sample rate to use
   */
  unsigned char get_sample_rate()const;
  friend ostream&operator<<(ostream&,const sample_rate_list&);
};



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
  virtual bool is_open()const;
  virtual void run()=0;
};



class sound_file_output_jackaudio: public sound_file_output_base
{
  static int call_back_audio( jack_nframes_t nframes, void * arg );
  vector<jack_port_t*>output_ports;
  jack_client_t *client;
  bool is_open_b;
  // No sound_file_output_buffer as it is constructed on each call
  // as one should not cache some data
public:
  sound_file_output_jackaudio(const unsigned char&,const string& jack_peer);
  ~sound_file_output_jackaudio()=default;
  bool is_open()const;
  void run();
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
  ofstream outputfile_stream;
  signed short values[20000];
  const bool follow_timebeat;
  // bool is_open_b;
public:
  sound_file_output_file(const string& filename,
						 const bool&follow_timebeat);
  ~sound_file_output_file();
  // bool is_open()const;
  void run();
};


#endif
