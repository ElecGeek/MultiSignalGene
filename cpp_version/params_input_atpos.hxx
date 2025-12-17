#include <string>
#include <deque>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <functional>
#include <optional>
using namespace std;

#ifndef __PARAMS_INPUT_ATPOSS__
#define __PARAMS_INPUT_ATPOSS__

#include "parameters.hxx"

class atpos_event_action {
  unsigned long rel_timestamp;
  unsigned char level;
};
struct atpos_event_metadata {
  string md_key;
  string md_val;
};
struct atpos_event {
  atpos_event(void);
  atpos_event(const unsigned long&,const unsigned char);
  atpos_event(const string&,const string&);
  optional<atpos_event_action> action;
  optional<atpos_event_metadata> metadata;
};

class atposs_bytes_stream : public atpos_event, public input_event {
  // just copied and pasted, have to be enriched by the line parsing
  enum state_t{ state_ts, state_code, state_key, state_val, state_string, state_end } state;
  enum { blk_start=0, blk_in_channel=1, blk_in_act_list=2, blk_in_action=4} block_state;
  
public:
  ostream&info_out_str;
private:
  input_params_base::clearing_t&mbs_clearing;
protected:
  unsigned long track_line;
  // The software accepts both CR, CR-LF or LF as end of lines
  //   as this syntax does not consider the end of lines.
  // However, it is an issue for the line counting (used in the error messages)
  // After EACH line, the first used is the one used for counting,
  //   in case of CR or LF sequences
  unsigned char crlf_first_used;
  unsigned short current_channel;
  const bool with_time_stamp;
public:
  atposs_bytes_stream(void)=delete;
  atposs_bytes_stream(ostream&,const bool&with_time_stamp, input_params_base::clearing_t&clearing );
  bool get_event(istream&);
  bool eot(void) const;
  // For debug purposes, sends the content of a atpos message
  friend ostream&operator<<( ostream&, const atposs_bytes_stream& );
};
class atposs_bytes_datagram_test : public atpos_event, public input_event {
  ostream&info_out_str;
  input_params_base::clearing_t&mbs_clearing;
public:
  atposs_bytes_datagram_test(void)=delete;
  atposs_bytes_datagram_test(ostream&,const bool&with_time_stamp, input_params_base::clearing_t&clearing );
  bool get_event(deque<atpos_event>::const_iterator&curr,
				 deque<atpos_event>::const_iterator&end);
  bool eot(void) const;
};
class input_params_atposs_2_action
{
  ostream&info_out_str;
  enum params_input_atposs_extended { P_abort = 100
  };
protected:
  /* \brief Converts channel data into channel code
   *
   * The result is the channel id or 0 for all of them
   * \param value Reference to return the value
   * \return error string or empty string if no error
   */
  // Get the value mantissa + exponent compiled into an unsigned long used as 24 bits
  // Format is: velocity is always, full 7 bits the mantissa,
  // exponent_size of the right bits of the key (note code) is the exponent
  // a constant is the additional exponent
  optional<unsigned long> timestamp_value;
private:
  unsigned long get_value( const unsigned char&exponent_size, const unsigned char&exponent_const )const;
  const atpos_event&the_event;
  input_params_base::clearing_t&ipm2a_clearing;
  input_event::status_t&ipm2a_status;
  const multimap< short, string >atposs_list;
  // Not that readable, but it avoid excessive indentation in the constructor
  const map< string, function< string(unsigned long&,signals_param_action::action_list&)> >mrf;
  //constexpr unsigned long long str2ll(const char*str, int h = 0)
  //{
  //	if ( str[h] == 0 )
  //	  return 0;
  //	else
  //	  return str2ll( str, h + 1 ) * 256 | (unsigned long long) str[ h ];
  //}
public:
  input_params_atposs_2_action(void)=delete;
  input_params_atposs_2_action(ostream&info_out_str,
							   const atpos_event&,
							  input_params_base::clearing_t&clearing,
							  input_event::status_t&status);
  void atposs_2_action_run(vector<signals_param_action>&actions_list);
};


/** \brief Bundle class byte stream
 *
 * Bundles the layers of:\n
 * The interface input_params_base to deliver the actions\n
 * The bytes collection from the stream and pre-processing to sort out
 *   the TS, its mneno, its value, its unit (opt), its comment (voided)\n
 * The conversion into an action type\n
 *  
 * This should go into a template, but let's wait more interfaces to write a generic template,
 *   based on the 7 network OSI layer style
 */
class input_params_atposs_byte_stream : public input_params_base, public atposs_bytes_stream, public input_params_atposs_2_action {
  istream& i_stm;
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;  
 public:
  input_params_atposs_byte_stream(void)=delete;
  input_params_atposs_byte_stream(ostream&,istream&,const bool&);
  void import_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
  bool is_ready(void);
  friend ostream&operator<<(ostream&,const input_params_atposs_byte_stream&);
};
/** \brief Bundle class byte file
 *
 * Bundles the layers of:\n
 * The interface input_params_base to deliver the actions\n
 * The bytes collection from the file and pre-processing to sort out
 *   the TS, its mneno, its value, its unit (opt), its comment (voided)\n
 * The conversion into an action type\n
 *  
 * This should go into a template, but let's wait more interfaces to write a generic template,
 *   based on the 7 network OSI layer style
 */
class input_params_atposs_file : public input_params_base, public atposs_bytes_stream, public input_params_atposs_2_action{
  ifstream if_stm;
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;  
  unsigned short loops_counter;
 public:
  input_params_atposs_file(void)=delete;
  input_params_atposs_file(ostream&,ifstream&input_stream,const unsigned short&loops_counter);
  ~input_params_atposs_file(void);
  void import_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
  bool is_ready(void);
  bool exec_loops();
};
/** \brief Bundle class byte hard-coded
 *
 * This is for test purposes\n
 * Bundles the layers of:\n
 * The interface input_params_base to deliver the actions\n
 * The bytes collection from an hard-coded list and pre-processing to sort out
 *   the TS, its mneno, its value, its unit (opt), its comment (voided)\n
 * The conversion into an action type\n
 *  
 * This should go into a template, but let's wait more interfaces to write a generic template,
 *   based on the 7 network OSI layer style
 */
class input_params_atposs_hardcoded : public input_params_base, public atposs_bytes_datagram_test, public input_params_atposs_2_action{
  const deque<atpos_event>the_list;
  deque<atpos_event>::const_iterator the_list_iter;
  deque<atpos_event>::const_iterator the_list_end;
  // string for the ostringstream, in order to reserve size
  string ioss;
  ostringstream info_out_str_stream;  
public:
  input_params_atposs_hardcoded(void)=delete;
  input_params_atposs_hardcoded(ostream&);
  void import_next_event(vector<signals_param_action>&actions);
  unsigned long check_next_time_stamp(void);
  bool eot(void) const;
  bool is_ready(void);
  bool exec_loops();
};

#endif
