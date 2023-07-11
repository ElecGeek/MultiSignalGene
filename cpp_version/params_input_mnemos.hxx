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
  mnemos_bytes_stream(void);
protected:
  unsigned long timestamp_construct;
  unsigned long track_line;
  const bool with_time_stamp;
public:
  mnemos_bytes_stream(ostream&,const bool&with_time_stamp, input_params_base::clearing_t&clearing );
  bool get_event(istream&);
  bool eot(void) const;
  // For debug purposes, sends the content of a mnemo message
  friend ostream&operator<<( ostream&, const mnemos_bytes_stream& );
  // Get the value mantissa + exponent compiled into an unsigned long used as 24 bits
  // Format is: velocity is always, full 7 bits the mantissa,
  // exponent_size of the right bits of the key (note code) is the exponent
  // a constant is the additional exponent
  unsigned long get_value( const unsigned char&exponent_size, const unsigned char&exponent_const )const;

};
class input_params_mnemos_2_action
{
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
  const mnemo_event&the_event;
  // Get the value mantissa + exponent compiled into an unsigned long used as 24 bits
  // Format is: velocity is always, full 7 bits the mantissa,
  // exponent_size of the right bits of the key (note code) is the exponent
  // a constant is the additional exponent
  unsigned long get_value( const unsigned char&exponent_size, const unsigned char&exponent_const )const;
  input_params_base::clearing_t&ipm2a_clearing;
  mnemo_event::status_t&ipm2a_status;
  const multimap< short, string >mnemos_list;
public:
  input_params_mnemos_2_action( const mnemo_event&,
							  input_params_base::clearing_t&clearing,
							  mnemo_event::status_t&status);
  void mnemos_2_action_run(vector<signals_param_action>&actions_list);
};


/** \brief Base of the mnemos method to set the parameters
 *
 * It is involved when something has to be done,
 * regardless on the fly or on schedule
 * 
 */
class input_params_mnemos_byte_stream : public input_params_base, public mnemos_bytes_stream, public input_params_mnemos_2_action {
  istream& i_stm;
 public:
	input_params_mnemos_byte_stream(istream&,const bool&);
  void exec_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  friend ostream&operator<<(ostream&,const input_params_mnemos_byte_stream&);
};
/** \brief Mnemos file control
 *
 * Handles the mnemos files containing events and timestamps
 */
class input_params_mnemos_file : public input_params_base, public mnemos_bytes_stream, public input_params_mnemos_2_action{
  ifstream if_stm;
  unsigned short loops_counter;
 public:
  input_params_mnemos_file(ifstream&input_stream,const unsigned short&loops_counter);
  ~input_params_mnemos_file(void);
  void exec_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
  bool is_ready(void);
  bool exec_loops();
};



#endif
