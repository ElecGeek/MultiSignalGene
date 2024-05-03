#ifndef __SOUND_LINKS__
#define __SOUND_LINKS__

#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <limits>
#include <deque>
#include <list>
#include <map>
#include <numeric>
#include <algorithm>
// #include "main_loop.hxx"
// #include "sound_file_output.hxx"

using namespace std;



/* \brief Peers and connections
 *
 * Handles the list of peers with their connections
 */
class sound_jack_connections_data
{
  multimap<string,string>connections_list;
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
					  const string_view& sce_peer,
					  const string_view&dest_app, const string_view&dest_peer,
					  const bool& stereo );
  deque<string>::size_type peers_size()const;
  const multimap<string,string>links_list()const;
  const deque<string>peers_list()const;
};

class sound_jack_connections_lists
{
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
  sound_jack_connections_lists()=delete;
  sound_jack_connections_lists( const unsigned char&nbre_channels,const deque<string>& );
  deque<string>::size_type audio_peers_size()const;
  bool has_atleast_one_audio()const;
  const sound_jack_connections_data&audio()const;  
  const sound_jack_connections_data&midi_in()const;  
  const sound_jack_connections_data&midi_out()const;  
};


#endif
