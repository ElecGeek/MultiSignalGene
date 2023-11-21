// char, short, long24 are used because this is a PoC of the VHDL version

#include <deque>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

#ifndef __PARAMETERS__
#define __PARAMETERS__

#include "sample_step.hxx"

/** \brief Data of a parameter change
 *
 * Contains the channel, action and the value of an elementary change
 * It is intended to link the input parameters modules, the output parameters modules
 * and the signal generator.
 * It contains the what (the action), the how much (the value) and the to who (the channel_id)
 */
struct signals_param_action
{
  enum action_list { base_freq, base_phase_shift, base_phase_set,
					 main_ampl_val, main_ampl_slewrate,
					 pulse_freq, pulse_depth, pulse_high_hold, pulse_low_hold,
					 pulse_phase_shift, pulse_phase_set, pulse_modul_mode,
					 ampl_modul_freq, ampl_modul_depth,
					 ampl_modul_phase_shift, ampl_modul_phase_set, ampl_modul_modul_mode,
					 user_volume, track_name, nop };
  unsigned short channel_id;
  action_list action;
  unsigned short value;
  signals_param_action(void);
  signals_param_action( const unsigned short&,const action_list&,const unsigned short&);
  friend ostream&operator<<(ostream&,const signals_param_action&);
};
class params_base
{
public:
  const unsigned long samples_per_second_base = 48000;
};
/** \brief Base of the parameters control for all the possible methods
 *
 * The bundle class holds a list of this class, one per input channel/method\n
 * Many classes can be registered to allow more than one to take the control, be careful!\n
 * The goal is also run more than one specialized control input, one per type\n
 * Another goal is to read some multi track files in case each track appends the previous one
 *   (in such case the file is opened multiple times
 */
class input_params_base : public params_base {
  bool wait_for_next_TS;
  unsigned long requested_samples;
  virtual unsigned long check_next_time_stamp(void) = 0;
  virtual void import_next_event(vector<signals_param_action>&actions) = 0;
 public:
  unsigned long cumul_time_stamp;
  unsigned long current_samples;
 protected:
  unsigned short samples_per_TS_unity;
  ostringstream info_out_stream;
  /** \brief set some defaults values
   *
   * Sets samples_per_TS_unity to a small non 0 value\n
   * The caller should set to its real value as soon as it is known\n
   * (after opening inputs such as files)
   * If the channel is an "ASAP" one, let the default
   */
  input_params_base();
 public:
  virtual ~input_params_base(void)=default;
  bool check_next_event( const unsigned short&elapsed_samples, vector<signals_param_action>&action );
  virtual bool eot(void) const = 0;
  virtual bool is_ready(void) = 0;
  virtual bool exec_loops();
  enum clearing_t { c_unknown, c_end_of_data, c_abort, c_file_err, c_net_err, c_data_err } clearing;
};
/** \brief Base of the parameters control for all the possible methods
 *
 * The bundle class holds a list of this class, one per output channel/method\n
 * Many classes can be registered to allow more than one output\n
 * By this way the programmer of the UI can receive a feedback and display all the events\n
 * The goal is also run more than one specialized control input, one per type
 */
class output_params_base : public params_base {
  virtual void export_next_event(const unsigned long&absolute_TS,
								 const unsigned long&diff_TS,
								 const signals_param_action&) = 0;
  virtual void export_keep_alive(const unsigned long&absolute_TS);
 public:
  unsigned long cumul_time_stamp;
  unsigned long current_samples;
 private:
  unsigned short keep_alive_count;
  const decltype( keep_alive_count ) keep_alive_max;
 protected:
  unsigned short samples_per_TS_unity;
  ostringstream info_out_stream;
  /** \brief set some defaults values
   *
   * Sets samples_per_TS_unity to a small non 0 value\n
   * The caller should set to its real value as soon as it is known\n
   * (after opening inputs such as files)
   * If the channel is an "ASAP" one, let the default
   */
  output_params_base();
 public:
  virtual ~output_params_base(void)=default;
  bool check_next_event( const unsigned short&elapsed_samples, const vector<signals_param_action>&action );
};



#endif
