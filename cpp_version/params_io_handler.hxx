#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <optional>
#include <filesystem>
// #include <execution>

#ifndef __PARAMS_IO_HANDLER__
#define __PARAMS_IO_HANDLER__

#include "parameters.hxx"
#include "sound_output_buffer.hxx"
#include "params_output_txt.hxx"
#include "params_output_mnemos.hxx"
#include "params_input_mnemos.hxx"
#include "params_input_midi.hxx"

using namespace std;

enum params_file_format { params_file_8 = 0,
						  params_file_16BE = 1,
						  params_file_32BE = 2};
enum params_io_format { params_io_unknown = 0,
						params_io_midi_file = 1,
						params_io_text = 2,
						params_io_mnemos = 3,
						params_io_midi_connec = 4,
						params_io_vhdl_test = 5,
						params_io_test = 6};

/** @brief Base of the handler of the files, input and output parameters
 *
 * Built by inherited classes, it holds the common sanity checks
 *   such as the supported, planned and known-unsupported formats
 * The specific parameters are handled in the code of the inherited classes.
 */
class params_fio_handler_base
{
protected:
  unsigned char related_param;
  typedef map< char, string > options_list_t;
  deque< pair< short, options_list_t > > fio_channels_with_opts;
  ostringstream info_fio_params;
  const multimap< short, string >supported_formats;
  const multimap< short, string >unsupported_formats;
  const multimap< short, string >planned_formats;
  /** different ways to say TRUE and FALSE */
  const multimap< bool, string > boolean_parameter;
  string err_msg_prefix;
  bool is_bad;

  params_fio_handler_base(unsigned char, string err_msg_prefix,
						  multimap< short, string > supported_formats,
						  multimap< short, string > unsupported_formats,
						  multimap< short, string > planned_formats);
  params_fio_handler_base()=delete;

  virtual void AddChannelOptions( const options_list_t & ) = 0;
  virtual void EnvironementNeeds(void) = 0;
  
  /** @brief Parse a boolean parameter
   *
   * @return the result or the default
   */
  bool GetBoolParam(const options_list_t&the_params_list,
					const unsigned char&param_code, const bool default_data = false );
  /** @brief Check the files type
   *
   * Sanity check on the file type before opening it.
   * It is used by the input parameters, output parameters wave sound output,
   *   trigger output (to be written).
   * The type regular file is always allowed.
   * Some types, such as directory are always rejected.
   * Other types are accepted or rejected according with the calling parameters.
   * @return nothing if it is accepted, an error string if the type is rejected.
   */
  optional<string> FileTypeChecker( const filesystem::path& the_path,
						const string&prefix,
						const bool&check_exists, const bool&check_not_fifo, const bool&check_not_dev_char);
/** \brief Get information
   *
   * This returns both the notes and the errors
   * generated during initialisation and run.\n
   * It does NOT return any data of the channels
   */
  string GetInfos();

  friend class params_io_handler;
};
/** @brief Handles all the interfaces of all the input channels
 *
 *
 */
class params_input_handler : public params_fio_handler_base {

  deque<input_params_base*> IPB_list;

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
  /** @brief environment needs
   *
   * Make the list of environment needs
   *   such as network alive, jackaudio etc...
   */
  void EnvironementNeeds(void);
  
  bool CheckMidiFile(ifstream&)const;
  bool CheckMnemos(ifstream&)const;
  bool CheckJackMidi(ifstream&)const;

  friend class params_io_handler;
};


/** @brief Handles all the interfaces of all the output channels
 *
 *
 */
class params_output_handler  : public params_fio_handler_base {
  deque<output_params_base*> OPB_list;

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
  /** @brief environment needs
   *
   * Make the list of environment needs
   *   such as network alive, jackaudio etc...
   */
  void EnvironementNeeds(void);
  bool Empty()const;

  bool CheckJackMidi()const;

  friend class params_io_handler;
};

/** @brief Handles all the interfaces of all the file channels
 *
 *
 */
class params_file_handler  : public params_fio_handler_base {

  ofstream output_file_stream;
  //! Create the buffer with samples data,
  //! the buffer is going to be extended of the buffers data by the output class 
  sound_data_output_buffer sfo_buffer;

  params_file_handler();
  ~params_file_handler();
  /** @brief Add a channel options
   *
   * Each time a channel has all its options defined
   * this function perform a quick sanity check 
   * and add it to the list
   */
  void AddChannelOptions( const options_list_t & );
  /** @brief environment needs
   *
   * Make the list of environment needs
   *   such as network alive, jackaudio etc...
   */
  void EnvironementNeeds(void);

  void CreateOutputFile();

  friend class params_io_handler;
};


class params_io_handler {
private:
  const deque<params_fio_handler_base*>option_list_by_type;
  decltype( option_list_by_type )::const_iterator option_type_current;
  map<char,string> options_list;
  ostringstream info_io_params;  
  params_input_handler p_in_h;
  params_output_handler p_out_h;
  params_file_handler p_file_h;
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
  void CreateChannels(const unsigned short&n_loops);
  /** @brief environment needs
   *
   * Make the list of environment needs
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
   * @return true if there is at least one
   */ 
  bool hasOutputChannels()const;
  /** @brief Has file defined
   *
   * Tells if there is a file-name (or pipe) that passed the first sanity check.\n
   * That is not a guarantee there is at least one valid channel
   * @return true if there is at least one
   */ 
  bool hasFileOutput()const;
  /** @brief Has really file output
   *
   * Tells if there is at least one input that passed the first sanity check.\n
   * That is not a guarantee there is at least one valid channel
   * @return true if there is at least one
   */ 
  bool stillHasFileOutput()const;

  const deque<input_params_base*>&GetInputChannels()const;
  
  const deque<output_params_base*>&GetOutputChannels()const;

  pair< sound_data_output_buffer, ostream* > GetFileOutput();
/** \brief Get information
   *
   * This returns both the notes and the errors
   * generated during initialisation and run.\n
   * It does NOT return any data of the channels
   */
  string GetInfos();
  
};

#endif
