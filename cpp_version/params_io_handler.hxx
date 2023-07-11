#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

#ifndef __PARAMS_IO_HANDLER__
#define __PARAMS_IO_HANDLER__

#include "parameters.hxx"
#include "params_output_txt.hxx"
#include "params_output_mnemos.hxx"
// #include "params_input_mnemos.hxx"
#include "params_input_midi.hxx"

using namespace std;

enum params_io_format { params_io_unknown = 0,
						params_io_midi_file = 1,
						params_io_text = 2,
						params_io_mnemos = 3,
						params_io_midi_connec = 4,
						params_io_vhdl_test = 5 };
/** @brief Handles all the interfaces of all the input channels
 *
 *
 */
class params_input_handler {
  typedef map< char, string > options_list_t;
  deque< pair< short, options_list_t > > channels_list;
  ostringstream info_in_params;
  const multimap< short, string >supported_formats;
  const multimap< short, string >unsupported_formats;
  const multimap< short, string >planned_formats;  
  deque<input_params_base*> IPB_list;
public:
  params_input_handler();
  /** @brief Add a channel options
   *
   * Each time a channel has all its options defined
   * this function perform a quick sanity check 
   * and add it to the list
   */
  void AddChannelOptions( const options_list_t & );
  /** @brief Construct channels
   *
   * For each set of options, construct the channel
   * and perform the last checks
   */
  void CreateInputChannels(const unsigned short&);
  /** @brief environement needs
   *
   * Make the list of environement needs
   *   such as network alive, jackaudio etc...
   */
  void EnvironementNeeds(void);
  /** \brief Get information
   *
   * This returns both the notes and the errors
   * generated during initialisation and run.\n
   * It does NOT return any data of the channels
   */
  string GetInfos();

  bool CheckMidiFile(ifstream&)const;
  bool CheckMnemos(ifstream&)const;
  bool CheckJackMidi(ifstream&)const;

  friend class params_io_handler;
};


/** @brief Handles all the interfaces of all the input channels
 *
 *
 */
class params_output_handler {
  typedef map< char, string > options_list_t;
  deque< pair< short, options_list_t > > channels_list;
  ostringstream info_out_params;
  deque<output_params_base*> OPB_list;
  const multimap< short, string >supported_formats;
  const multimap< short, string >unsupported_formats;
  const multimap< short, string >planned_formats;
public:
  params_output_handler();
  /** @brief Add a channel options
   *
   * Each time a channel has all its options defined
   * this function perform a quick sanity check 
   * and add it to the list
   */
  void AddChannelOptions( const options_list_t & );
  /** @brief Construct channels
   *
   * For each set of options, construct the channel
   * and perform the last checks
   */
  void CreateOutputChannels(void);
  /** @brief environement needs
   *
   * Make the list of environement needs
   *   such as network alive, jackaudio etc...
   */
  void EnvironementNeeds(void);
  /** \brief Get information
   *
   * This returns both the notes and the errors
   * generated during initialisation and run.\n
   * It does NOT return any data of the channels
   */
  string GetInfos();
  bool Empty()const;

  bool CheckJackMidi()const;

  friend class params_io_handler;
};



class params_io_handler {
public:
  enum which_option { input_option, output_option, no_option };
private:
  map<char,string> options_list;
  ostringstream info_io_params;  
  which_option last_option;
  params_input_handler p_in_h;
  params_output_handler p_out_h;
public:
  params_io_handler();
  /** @brief Record option
   *
   * This places the option in a list,
   * in order to create an input parameters channel.\n
   * It does not actually do anything nor check some sanity.
   */
  void SetOption( const char&opt, const string&optarg );
  /** @brief Flush options
   *
   * This function should be called after the main got its last parameter
   */
  void FlushOptions();
  /** @brief Construct channels
   *
   * For each set of options, construct the channel
   * and perform the last checks
   */
  void CreateChannels(void);
  /** @brief environement needs
   *
   * Make the list of environement needs
   *   such as network alive, jackaudio etc...
   */
  void EnvironementNeeds(void);
  /** @brief Has input channels
   *
   * Tells if there is at least one input that passed the first sanity check.\n
   * That is not a guarantee there is at least one valid channel
   * @return true if there is at least one
   */ 
  bool stillHasInputChannels()const;
  /** @brief Has input channels
   *
   * Tells if there is at least one input that passed the last sanity check.\n
   * @return true if there is at least one
   */ 
  bool hasInputChannels()const;
  /** @brief Has output channels
   *
   * Tells if there is at least one output that passed the first sanity check.\n
   * That is not a guarantee there is at least one valid channel
   * @return true if there is atr least one
   */ 
  bool hasOutputChannels()const;

  const deque<input_params_base*>&GetInputChannels()const;
  
  const deque<output_params_base*>&GetOutputChannels()const;
  /** \brief Get information
   *
   * This returns both the notes and the errors
   * generated during initialisation and run.\n
   * It does NOT return any data of the channels
   */
  string GetInfos();
  
};

#endif
