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
#include "sound_jack_links.hxx"

using namespace std;

#ifndef __NO_JACK__
#include <jack/jack.h>
#include <jack/midiport.h>
#endif


class sound_file_output_jackaudio: public sound_file_output_base
{
#ifndef __NO_JACK__
  static int call_back_audio( jack_nframes_t nframes, void * arg );
  static int call_back_jack_shutdown( void * arg );
  static int call_back_samplerate_change( jack_nframes_t nframes, void * arg );
  vector<jack_port_t*>output_sound_ports;
  vector<jack_port_t*>output_midi_ports;
  jack_client_t *client;
#endif
  bool is_open_b;
  bool is_started;
  // For the call back function
  bool sound_started;
  bool shutdown_requested;
  // No sound_file_output_buffer as it is constructed on each call
  // as one should not cache its pointer
  sound_jack_connections_lists connections_list;
  /** Peer prefix in the jack connection system
   */
  string jack_peer;
public:
  sound_file_output_jackaudio(const unsigned char&,const deque<string>& jack_connections);
  ~sound_file_output_jackaudio();
  bool test_sound_format()const;
  bool is_ready()const;
#ifndef __NO_JACK__
  bool pre_run();
#endif
  void run();
};



#endif
