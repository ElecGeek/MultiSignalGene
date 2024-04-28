#ifndef __SOUND_OUTPUT_BUFFER__
#define __SOUND_OUTPUT_BUFFER__

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <vector>
//#include <typeinfo>

using namespace std;


struct sound_data_output_buffer
{
  //! Unified variable for type, float flag and interleave flag
  size_t type_float_interleave;
  //! First and last supported channel by the output
  pair<unsigned short,unsigned short> channels_bounds;
  //! Size of a sample 2=short, today the only one supported
  //  const unsigned short sample_size;
  //! True = data sent as frames of the channels, one after the other
  //! False = full data of each channel one after the other
  //const bool interleave;
  //! Size of the data buffer
  size_t data_size;
  //! Data buffer pointer. To be cast according with the sample_size
  vector<void*>  data;
  sound_data_output_buffer();
  sound_data_output_buffer( const size_t&type_hash,
							const bool&is_float,
							const bool&has_interleave,
							const size_t&data_size,
							const vector<void*>&data);
  /** \brief Compute the parameters into one unique hash
   * Since the key of the map type is not able (in C++14)
   * to be and to be search on a tuple,
   * this function compute a "super hash"
   * including the type and some meta data.
   * \param the_type is the hash returned by the typeid class
   * \param is_float is a little bit redundant but useful to ensure the correctness
   * \param has_interleave is there one buffer with interleave or N buffers
   * \return the super hash to be used as the key of the map
   */
static size_t get_super_hash( const size_t&the_type, const bool&is_float, const bool&has_interleave );
/** \brief Handles the data buffer and some specifications
 *
 * 
 *
 */
  friend ostream&operator<<(ostream&,const sound_data_output_buffer&);
};
/** \brief unified strucutre for midi input and output
 *
 */
struct midi_data_io_buffer
{
  deque<tuple<unsigned char, unsigned char, unsigned char> >midi_list;
};



#endif
