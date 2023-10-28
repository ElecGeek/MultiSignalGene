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
  set<unsigned short>list_sr;
public:
  sample_rate_list();
  void add_value(const unsigned short&);
  /* \brief Find common values
   *
   * In case of don't care, copy the right list
   * In case not, merges the lists
   */
  sample_rate_list intersect_sr_list( const sample_rate_list&)const;
  /* \brief Get the sample rate to use
   *
   * Accroding with some default choices, return the sample rate to use
   * And tell if the value is low
   */
  pair<unsigned short,bool> get_sample_rate()const;
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
  virtual bool is_ready()const;
  virtual bool pre_run();
  virtual void run()=0;
};

/* \brief Peers and connections
 *
 * Handles the list of peers with their connections
 */
class sound_jack_connections_data
{
  // C++17 is going to simplify the writing
  deque< pair< string, deque< string > > >connections_list;
public:
  /* \brief Add a new connection or peer
   *
   * If the peer exists, the destination is added.\n
   * If the peer does not exist, it is append at the end;
   * and its connection is added
   */
  void insert_append( const string&peer, const string&dest );
  /* \brief Additional defaults connections
   *
   * If N is lower or equal to the size of the list,
   * takes the elements and add <dest_app>:<dest-peer><ind> for ind 1 to N\n
   * If N is greater, execute the actions above
   * and create as many as needed additional peers named gene_<ind>
   */
  void insert_append( const unsigned char&N,
					  const string& sce_peer,
					  const string&dest_app, const string&dest_peer,
					  const bool& stereo );
  const deque< pair< string, deque< string> > >&operator()()const;
};

class sound_file_output_jackaudio: public sound_file_output_base
{
  static int call_back_audio( jack_nframes_t nframes, void * arg );
  static int call_back_jack_shutdown( void * arg );
  static int call_back_samplerate_change( jack_nframes_t nframes, void * arg );
  vector<jack_port_t*>output_ports;
  jack_client_t *client;
  bool is_open_b;
  bool is_started;
  // For the call back function
  bool sound_started;
  bool shutdown_requested;
  // No sound_file_output_buffer as it is constructed on each call
  // as one should not cache its pointer
  /** Peer prefix in the jack connection system
   */
  string jack_peer;
  /** \brief list of peers and connections
   *
   */  
  sound_jack_connections_data connections_list_audio;
  /** \brief list of peers and connections
   *
   */  
  sound_jack_connections_data connections_list_midi_in;
  /** \brief list of peers and connections
   *
   */  
  sound_jack_connections_data connections_list_midi_out;
public:
  sound_file_output_jackaudio(const unsigned char&,const deque<string>& jack_connections);
  ~sound_file_output_jackaudio();
  bool is_ready()const;
  bool pre_run();
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
  unsigned long cumul_us_elapsed;
  // bool is_open_b;
public:
  sound_file_output_file(const string& filename,
						 const bool&follow_timebeat);
  ~sound_file_output_file();
  // bool is_open()const;
  void run();
};


#endif
