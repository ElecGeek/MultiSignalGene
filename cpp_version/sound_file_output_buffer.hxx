#ifndef __SOUND_FILE_OUTPUT_BUFFER__
#define __SOUND_FILE_OUTPUT_BUFFER__

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std;

/** \brief Handles the data buffer and some specifications
 *
 * 
 *
 */


struct sound_file_output_buffer
{
  //! First and last supported channel by the output
  const pair<unsigned short,unsigned short> channels_bounds;
  //! Size of a sample 2=short, today the only one supported
  const unsigned short sample_size;
  //! True = data sent as frames of the channels, one after the other
  //! False = full data of each channel one after the other
  const bool interleave;
  //! Size of the data buffer
  size_t data_size;
  //! Data buffer pointer. To be cast according with the sample_size
  void*const   data;
  sound_file_output_buffer( const pair<unsigned short,unsigned short>&channels_bounds,
							const unsigned short&sample_size,
							const bool&interleave,
							const size_t&data_size,
							void*const&data);
  friend ostream&operator<<(ostream&,const sound_file_output_buffer&);
};

#endif
