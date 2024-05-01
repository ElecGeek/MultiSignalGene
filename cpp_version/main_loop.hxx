// char, short, long24 are used because this is a PoC of the VHDL version

#ifndef __MAIN_LOOP__
#define __MAIN_LOOP__

#include "amplitude_handler.hxx"
#include "frequency_handler.hxx"
#include "parameters.hxx"
#include "bundle_signals.hxx"
#include "sound_output_buffer.hxx"

#include <deque>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <list>
#include <functional>
using namespace std;


/** \brief General bundle of an entire set
 *
 * Bundles:\n
 *   All the signals\n
 *   All the parameters inputs, one instance per input control\n
 *   All the parameters outputs, one instance per output\n
 *   \n
 *   It is being implemented with some hard-coded values\n
 *     here and in the input and output modules\n
 *     The time is units of 1/10seconds, the buffers size should be around 1K etc...
 */
class main_loop {
  main_loop(void);
 public:
  /** \brief Main actions list
   *
   *  It is built by all the parameters input modules\n
   *  It is sent to all the output parameters modules\n
   *  It is sent to the bundleler for all the output channels
   */
  vector<signals_param_action>actions;
 private:
  //! Quote if the was at least one unsupported type
  bool sample_type_not_found;
  //! Sample rate in KHz
  const unsigned short&sample_rate_id;
  //! Samples counter for the parameters set update
  unsigned short samples_count;
  //! Number of samples ran between 2 parameters updates (always based on 48KHz)
  const unsigned short samples_per_param_check;
  //! Sound output schedules the output, this feature forces to follow the time in case the output is file only
  unsigned short samples_per_TS_unit;
  //! Not yet implemented
  const unsigned long shutdown_start;
  //! Not yet implemented 
  unsigned long shutdown_count;
  unsigned short div_debug;
  map<unsigned short, signal_channel*>signal_list;
  deque<input_params_base*>params_input_list;
  deque<output_params_base*>params_output_list;
  bool debug_once;
  //! Text information about the output waveform of all the channels
  string output_waveform_info;
 public:
  /** \brief Constructor
   *  \param sample_rate_id Sample rate 1=48KHz, 2=96KHz and 4 =192KHz
   *  \param mode Output wave (or debug text), see the generator
   *  \param n_channels How many output channels
   *  \param samples_per_param_check Number of samples to run for one parameter set (in or out) update
   *  \param shutdown_length Not yet implemented
  */
  main_loop( const unsigned short&sample_rate_id,
			 const string&mode,
			 const unsigned short&n_channels,
			 const unsigned short samples_per_param_check = 48,
			 const unsigned long shutdown_length = 1000);
  ~main_loop(void);
  //! Adds an input parameters interface
  main_loop&operator+=(input_params_base*const);
  main_loop&operator+=(const deque<input_params_base*>&);
  //! Adds an output parameters interface
  main_loop&operator+=(output_params_base*const);
  main_loop&operator+=(const deque<output_params_base*>&);
  /** Get the size of a sample output
	  \return The size unit is the number of signed short elements
  */
  unsigned short GetSamplesSize(void)const;


  bool test_sound_format(const size_t&, const bool&, const bool& );
  /** \brief Main run function
   *  this function is involved for each audio buffer to be sent\n
   * It populates the data of all the channels\n
   *   found in the buffer object.\n
   * It calls the input and output parameters handling on a regular basis\n
   *   It is being chosen every 48 samples, 1mS\n
   * It might be improved to take a list of buffers for multiple audio outputs\n
   *   but there are some issues such as the buffer size or the input\n
   *   parameters timing
   * \param Buffer contains the data buffer and its specifications
   * \return The time elapsed in uS
   *   It is ignored by many callers except the case\n
   *   there is no audio output and the time-beat has to be followed
   */
  //  unsigned long send_to_sound_file_output(sound_data_output_buffer&buffer);

  /* \brief Contains the glue functions to fill up buffers from the channels
   * according with the type of the buffer\n
   * Computes the first and the last channel. Today it is simple numbers,
   * tomorrow, it is going to be a real object.\n
   * For each channel, runs the operator () that compute the next sample value.
   * Repeat that until the buffer is full.\n
   * After that or in the loop, check if there are channels to fill up
   * This should be done on each buffer refill as outputs
   * such as jackaudio always sends different buffer
   * (then one should not considered as cached)
   * Compute and return data about the elapsed time
   */
  class send_to_sound_output {
	//! List of all the supported formats
	const map<tuple<size_t,bool,bool>,function< tuple< bool, unsigned long>(sound_data_output_buffer&buffer)> > sample_type_defs_list;
	unsigned long frame_size_exec;
	unsigned long frame_size_requested;
	unsigned short chan_begin;
	unsigned short chan_end;
	main_loop*const main_loop_this;
  public:
	explicit send_to_sound_output( main_loop*const main_loop_this );
	unsigned long operator()(sound_data_output_buffer&buffer);
	friend class main_loop;
  };
  send_to_sound_output send_to_sound_file_output;
  // midi
  void operator()(midi_data_io_buffer&buffer);
  /** \brief Runs the command set, if so
   * This function has to be called at each sample.\n
   * According with the sample_rate, is checks if 1 mS elapsed.
   * \return set to true only when a mS just passed
   */
  bool check_action(void);

  /** \brief Input, Output and process parameters
   *  This function is called regularly during the audio buffer refill\n
   *    It calls all the input, all the output and all the process channels\n 
   *  For real time reasons, in case an input command is (yet) incomplete,\n
   *    the channel should keep the data, return and execute the next time\n
   *    (or when it is complete)\n
   *  It is called on regular schedule.\n
   *    The number of samples in between is being hard-coded\n
   *  \param None as everything should be configured
   *  \return Flag to tell the software should quit
   *    The flag tells there is no active input channel anymore\n
   *    That means a connection has been closed\n
   *    or an end of file has been reached\n
   *    The timeout has be handled int the input channels
  */
  bool exec_actions(void);
  //  void exec_actions(vector<signals_param_action>actions);
  bool is_all_ready(void)const;
  string get_clearing(void)const;
  string get_output_waveform(void)const;
  bool HaveSampleTypeNotFound()const;
};

#endif
