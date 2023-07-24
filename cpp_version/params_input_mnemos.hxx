#include <string>
#include <deque>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

#ifndef __PARAMS_INPUT_MNEMOS__
#define __PARAMS_INPUT_MNEMOS__

#include "parameters.hxx"
#include "params_codes.hxx"


class mnemos_bytes_stream : public mnemo_event {
  // just copied and pasted, have to be enchired by the line parsing
  enum state_t{ state_ts, state_code, state_key, state_val, state_string, state_end } state;
  ostream&info_out_str;
  input_params_base::clearing_t&mbs_clearing;
protected:
  unsigned long timestamp_construct;
  unsigned long track_line;
  const bool with_time_stamp;
public:
  mnemos_bytes_stream(void)=delete;
  mnemos_bytes_stream(ostream&,const bool&with_time_stamp, input_params_base::clearing_t&clearing );
  bool get_event(istream&);
  bool eot(void) const;
  // For debug purposes, sends the content of a mnemo message
  friend ostream&operator<<( ostream&, const mnemos_bytes_stream& );
};
class mnemos_bytes_datagram_test : public mnemo_event {
  ostream&info_out_str;
  input_params_base::clearing_t&mbs_clearing;
protected:
  unsigned long timestamp_construct;
public:
  mnemos_bytes_datagram_test(void)=delete;
  mnemos_bytes_datagram_test(ostream&,const bool&with_time_stamp, input_params_base::clearing_t&clearing );
  bool get_event(deque<mnemo_event>::const_iterator&curr,
				 deque<mnemo_event>::const_iterator&end);
  bool eot(void) const;
};
class input_params_mnemos_2_action
{
  ostream&info_out_str;
  enum params_input_mnemos {
	P_base_freq = 0,
	P_main_SR = 1,
	P_main_ampl = 2,
	P_abort = 3,
	P_ampl_mod_depth = 4,
	P_pulse_depth = 5,
	P_mode_pulse = 6,
	P_mod_ampl_mod = 7,
	P_phase_shift_base = 8,
	P_phase_shift_pulse = 9,
	P_phase_shift_ampl_mod = 10,
	P_phase_overwrite_base = 11,
	P_phase_overwrite_pulse = 12,
	P_phase_overwrite_ampl_mod = 13,
	P_nop = 14,
	P_ampl_mod_freq = 15,
	P_pulse_freq = 16,
	P_pulse_hold_high = 17,
	P_pluse_hold_low = 18
  };
  /* \brief Converts channel data into mode
   *
   * The result is the mode (today from 0 to 1)
   * \param value Reference to return the value
   * \return error string or empty string if no error
   */
  string Mode_strings_2_val( unsigned char&value) const;   
  /* \brief Converts channel data into channel code
   *
   * The result is the channel id or 0 for all of them
   * \param value Reference to return the value
   * \return error string or empty string if no error
   */
  string Channel_string_2_val( unsigned short&value) const;   
  /* \brief Converts angles to numeric value for the phase shift and reset functions
   *
   * The result is a format from 0 to 15 per 22.5 degres\n
   * Checks there is no unit or / or degre, otherwise return an error without any future processing\n
   * Checks if the before the decimal separator the value is 0\n
   *   If so, the scale 0 for a 0 angle to (excluded) 1.0 for a 2.PI angle is used\n
   *   If not a real angle is used\n
   * Strings and unit string are taken from the base class
   * \param value Reference to return the value
   * \return error string or empty string if no error
   */
string Angle_strings_2_val(unsigned long&value) const;
  /* \brief Converts depth to numeric value for the modulator functions
   *
   * The result is a format from 0 to 255
   * Checks there is no unit or %, otherwise return an error without any future processing\n
   * Checks if the before the decimal separator the value is greater then 1\n
   *   If so, the scale 0 to 100 is used\n
   *   If not the scale from 0 to 1 is used\n
   * TODO TODO the scal 0 to (excl) 1
   * Strings and unit string are taken from the base class
   * \param value Reference to return the value
   * \return error string or empty string if no error
   */
string Depth_strings_2_val(unsigned long&) const;
  // Get the value mantissa + exponent compiled into an unsigned long used as 24 bits
  // Format is: velocity is always, full 7 bits the mantissa,
  // exponent_size of the right bits of the key (note code) is the exponent
  // a constant is the additional exponent
  unsigned long get_value( const unsigned char&exponent_size, const unsigned char&exponent_const )const;
  const mnemo_event&the_event;
  input_params_base::clearing_t&ipm2a_clearing;
  mnemo_event::status_t&ipm2a_status;
  const multimap< short, string >mnemos_list;
  constexpr unsigned long long str2ll(const char*str, int h = 0)
  {
	if ( str[h] == 0 )
	  return 0;
	else
	  return str2ll( str, h + 1 ) * 256 | (unsigned long long) str[ h ];
  }
public:
  input_params_mnemos_2_action(void)=delete;
  input_params_mnemos_2_action(ostream&info_out_str,
							   const mnemo_event&,
							  input_params_base::clearing_t&clearing,
							  mnemo_event::status_t&status);
  void mnemos_2_action_run(vector<signals_param_action>&actions_list);
};


/** \brief Bundle class byte stream
 *
 * Bundles the layers of:\n
 * The interface input_params_base to deliver the actions\n
 * The bytes collection from the stream and preprocessing to sort out
 *   the TS, its mneno, its value, its unit (opt), its comment (voided)\n
 * The convertion into an action type\n
 *  
 * This should go into a template, but let's wait more interfaces to write a generic template,
 *   based on the 7 network OSI layer style
 */
class input_params_mnemos_byte_stream : public input_params_base, public mnemos_bytes_stream, public input_params_mnemos_2_action {
  istream& i_stm;
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;  
 public:
  input_params_mnemos_byte_stream(void)=delete;
  input_params_mnemos_byte_stream(ostream&,istream&,const bool&);
  void exec_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
  bool is_ready(void);
  friend ostream&operator<<(ostream&,const input_params_mnemos_byte_stream&);
};
/** \brief Bundle class byte file
 *
 * Bundles the layers of:\n
 * The interface input_params_base to deliver the actions\n
 * The bytes collection from the file and preprocessing to sort out
 *   the TS, its mneno, its value, its unit (opt), its comment (voided)\n
 * The convertion into an action type\n
 *  
 * This should go into a template, but let's wait more interfaces to write a generic template,
 *   based on the 7 network OSI layer style
 */
class input_params_mnemos_file : public input_params_base, public mnemos_bytes_stream, public input_params_mnemos_2_action{
  ifstream if_stm;
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;  
  unsigned short loops_counter;
 public:
  input_params_mnemos_file(void)=delete;
  input_params_mnemos_file(ostream&,ifstream&input_stream,const unsigned short&loops_counter);
  ~input_params_mnemos_file(void);
  void exec_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
  bool is_ready(void);
  bool exec_loops();
};
/** \brief Bundle class byte hardcoded
 *
 * This is for test purposes\n
 * Bundles the layers of:\n
 * The interface input_params_base to deliver the actions\n
 * The bytes collection from an hardcoded list and preprocessing to sort out
 *   the TS, its mneno, its value, its unit (opt), its comment (voided)\n
 * The convertion into an action type\n
 *  
 * This should go into a template, but let's wait more interfaces to write a generic template,
 *   based on the 7 network OSI layer style
 */
class input_params_mnemos_hardcoded : public input_params_base, public mnemos_bytes_datagram_test, public input_params_mnemos_2_action{
  const deque<mnemo_event>the_list;
  deque<mnemo_event>::const_iterator the_list_iter;
  deque<mnemo_event>::const_iterator the_list_end;
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;  
public:
  input_params_mnemos_hardcoded(void)=delete;
  input_params_mnemos_hardcoded(ostream&);
  void exec_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
  bool is_ready(void);
  bool exec_loops();
};

#endif
