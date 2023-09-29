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
  // The input iterator should come here
  // but after this class is going to be constructed rather than inherited by the input_params_midi_xxx classes
  // Indeed some of them reveives a input file stream which is move-copied.
  // Since the inherited classes are constructed before the local data, the input stream passed is
  // invalidated by the move-copy of the input file stream
  //istream&i_byte_stm;
  input_params_base::clearing_t&mbs_clearing;
protected:
  unsigned short str_length;
  string user_str;
  unsigned long track_tellg;
  unsigned long timestamp_construct;
  const bool with_time_stamp;
  unsigned char header;
 public:
  midi_bytes_stream(void)=delete;
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
  ostream&info_out_str;
  /* \brief Converts a midi mapped key and velocity into the value
   *
   * 
   * The velocity is always a full 7 bits mantissa. 
   * \param exponent_size This is the number of bits in the key (right justified)
   * \param exponent_const This is constant added to the exponent
   */
  unsigned long get_value( const unsigned char&exponent_size, const unsigned char&exponent_const )const;
  const midi_event&the_event;
  input_params_base::clearing_t&ipm2a_clearing;
  midi_event::status_t&ipm2a_status;
public:
  input_params_midi_2_action(void)=delete;
  input_params_midi_2_action( ostream&info_out_str,
							  const midi_event&,
							  input_params_base::clearing_t&clearing,
							  midi_event::status_t&status);
  void midi_2_action_run(vector<signals_param_action>&actions_list);
};
/** \brief Bundle class byte stream
 *
 * Bundles the layers of:\n
 * The interface input_params_base to deliver the actions\n
 * The bytes collection from the stream and preprocessing to sort out
 *   the midi command code, the midi note and the midi velocity
 * The convertion into an action type\n
 *  
 * This should go into a template, but let's wait more interfaces to write a generic template,
 *   based on the 7 network OSI layer style\n
 * This should move from 3 inheritence into 3 pointers as well
 */
class input_params_midi_byte_stream : public input_params_base, public midi_bytes_stream, public input_params_midi_2_action
 {
  istream& i_stm;
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;
 public:
  input_params_midi_byte_stream(void)=delete;
   input_params_midi_byte_stream(ostream&,istream&,const bool&);
  void import_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  friend ostream&operator<<(ostream&,const input_params_midi_byte_stream&);
};
/** \brief Bundle class byte stream
 *
 * Bundles the layers of:\n
 * The interface input_params_base to deliver the actions\n
 * The bytes collection from the file and preprocessing to sort out
 *   the midi command code, the midi note and the midi velocity
 * The convertion into an action type\n
 *  
 * This should go into a template, but let's wait more interfaces to write a generic template,
 *   based on the 7 network OSI layer style\n
 * This should move from 3 inheritence into 3 pointers as well
 */
class input_params_midi_file : public input_params_base, public midi_bytes_stream, public input_params_midi_2_action {
  ifstream if_stm;
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;
  unsigned short loops_counter;
 public:
  input_params_midi_file(void)=delete;
  input_params_midi_file(ostream&,ifstream&input_stream,const unsigned short&loops_counter);
  ~input_params_midi_file(void);
  void import_next_event(vector<signals_param_action>&actions);
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
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;  
 public:
  // relevant connection parameter here
  input_params_midi_connec(void);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
};



#endif
