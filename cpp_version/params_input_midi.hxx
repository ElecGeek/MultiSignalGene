// char, short, long24 are used because this is a PoC of the VHDL version

#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

#ifndef __PARAMS_INPUT_MIDI__
#define __PARAMS_INPUT_MIDI__

#include "parameters.hxx"
#include "params_codes.hxx"

/** \brief Handles midio bytes datagram
 *
 *
 * Reads a datagram such as a jackaudio event 
 * and populates the midi event data ( timestamp (if so ), code, key, value )\n
 * This event is empty because jackaudio has not yet been written
 */
class midi_bytes_datagram : public midi_event {

};
/** \brief Handles midi byte stream
 *
 * Reads bytes comming from a file, serial port etc...
 * and populates the midi event data ( timestamp (if so ), code, key, value )
 */
class midi_bytes_stream : public midi_event {
  enum state_t{ state_ts, state_code, state_key, state_val, state_string, state_end } state;
  ostream&info_out_str;
  input_params_base::clearing_t&mbs_clearing;
  midi_bytes_stream(void);
protected:
  unsigned short str_length;
  string user_str;
  unsigned long track_tellg;
  unsigned long timestamp_construct;
  const bool with_time_stamp;
  unsigned char header;
 public:
  midi_bytes_stream(ostream&,const bool&with_time_stamp, input_params_base::clearing_t&clearing );
  bool get_event(istream&);
  bool eot(void) const;
  // For debug purposes, sends the content of a midi message
  friend ostream&operator<<( ostream&, const midi_bytes_stream& );
  //  friend class parameters_midi;
  //friend class parameters_midi_file;
  //friend class parameters_midi_connec;
};
class input_params_midi_2_action
{
  const midi_event&the_event;
  // Get the value mantissa + exponent compiled into an unsigned long used as 24 bits
  // Format is: velocity is always, full 7 bits the mantissa,
  // exponent_size of the right bits of the key (note code) is the exponent
  // a constant is the additional exponent
  unsigned long get_value( const unsigned char&exponent_size, const unsigned char&exponent_const )const;
  input_params_base::clearing_t&ipm2a_clearing;
  midi_event::status_t&ipm2a_status;
public:
  input_params_midi_2_action( const midi_event&,
							  input_params_base::clearing_t&clearing,
							  midi_event::status_t&status);
  void midi_2_action_run(vector<signals_param_action>&actions_list);
};
/** \brief Base of the midi method to set the parameters
 *
 * Decodes the midi messages and sets the parameters.
 * It is involved when something has to be done,
 * regardless on the fly or on schedule
 * In case of a multi track smf or midd file, instances as many as tracks
 * should be created
 * 
 */
class input_params_midi_byte_stream : public input_params_base, public midi_bytes_stream, public input_params_midi_2_action
 {
  istream& i_stm;
 public:
  input_params_midi_byte_stream(istream&,const bool&);
  void exec_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  friend ostream&operator<<(ostream&,const input_params_midi_byte_stream&);
};
/** \brief Midi file control
 *
 * Handles the midi files containing events and timestamps
 */
class input_params_midi_file : public input_params_base, public midi_bytes_stream, public input_params_midi_2_action {
  ifstream if_stm;
  unsigned short loops_counter;
 public:
  input_params_midi_file(ifstream&input_stream,const unsigned short&loops_counter);
  ~input_params_midi_file(void);
  void exec_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
  bool is_ready(void);
  bool exec_loops();
};
/** \brief Midi connection control
 *
 * Handles the midi event coming on the fly from a connection
 * (electronic or network)
 */
class input_params_midi_connec : public input_params_base, public midi_bytes_datagram{
  stringstream i_str;
 public:
  // relevant connection parameter here
  input_params_midi_connec(void);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
};



#endif
