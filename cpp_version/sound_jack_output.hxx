#ifndef __SOUND_JACK_OUTPUT__
#define __SOUND_JACK_OUTPUT__

#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <limits>
#include "main_loop.hxx"
#include "sound_file_output.hxx"

using namespace std;

#include <jack/jack.h>
#include <jack/midiport.h>



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
  bool test_sound_format()const;
  bool is_ready()const;
  bool pre_run();
  void run();
};



#endif
