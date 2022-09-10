#include "sound_file_output.hxx"
#include "sound_file_output_buffer.hxx"

#include <chrono>
#include <thread>


sound_file_output_buffer::sound_file_output_buffer( const pair<unsigned short,unsigned short>&channels_bounds,
													const unsigned short&sample_size,
													const bool&interleave,
													const size_t&data_size,
													void*const& data):
	channels_bounds(channels_bounds),
	sample_size(sample_size),
	interleave(interleave),
	data_size(data_size),
	data(data)
{}
ostream&operator<<(ostream&the_out,const sound_file_output_buffer&a)
{
  the_out<<"Channels: ["<<a.channels_bounds.first<<":"<<a.channels_bounds.second<<"]"<<endl;
  switch( a.sample_size )
	{
	case 2: the_out << "Signed short, ";break;
	case 3: the_out << "Signed long, ";break;
	default: the_out << "Unsupported format, ";break;
	}
  if( a.interleave )
	the_out << "frames one after the other containing all the channels" << endl;
  else
	the_out << "one sub buffer per channel" << endl;

  return the_out;
}



sound_file_output_base::sound_file_output_base(const main_loop&signals):
  signals( signals )
{}

sound_file_output_jackaudio::sound_file_output_jackaudio(const main_loop&signals):
  sound_file_output_base(signals)
  // no sfo ({0,1},4,true)
{}
//sound_file_output_jackaudio::~sound_file_output_jackaudio()
//{}
// the code inside should go into the callback function
void sound_file_output_jackaudio::run()
{
  sound_file_output_buffer sfo_buffer({0,2},4,true,1000,NULL);

  unsigned short limit=0;
  while( signals.send_to_sound_file_output( sfo_buffer ) > 0)
	{
	  //	  limit++;
	  // if( limit > 2000 )
	  //	break;
	}
}


sound_file_output_dry::sound_file_output_dry(const main_loop&signals,
											 const bool&follow_timebeat):
  sound_file_output_base(signals),
  sfo_buffer( pair<unsigned short,unsigned short>(numeric_limits<unsigned short>::min(),
												  numeric_limits<unsigned short>::min()),
			  2,true,0,NULL),
  follow_timebeat(follow_timebeat),
  cumul_us_elapsed( 0 )
{}
void sound_file_output_dry::run()
{
  // for debug purpose, limit the run
  //  unsigned short limit=0;
  unsigned long microseconds_elapsed;
  while( true )
	{
	  microseconds_elapsed = signals.send_to_sound_file_output( sfo_buffer );
	  if ( microseconds_elapsed == 0 )
		break;
	  if ( follow_timebeat )
		{
		  cumul_us_elapsed += microseconds_elapsed;
		  if( ( cumul_us_elapsed / 1000 ) != 0 )
			{ 
			  this_thread::sleep_for(chrono::milliseconds(cumul_us_elapsed / 1000));
			  cumul_us_elapsed -= 1000 * ( cumul_us_elapsed / 1000);
			  }
		}
	  // limit++;
	  // if( limit > 200000 )
	  //   break;
	}
}



sound_file_output_file::sound_file_output_file(const main_loop&signals,
											   const string& filename,
											   const bool&follow_timebeat):
  sound_file_output_base(signals),
  sfo_buffer( pair<unsigned short,unsigned short>(numeric_limits<unsigned short>::min(),
												  numeric_limits<unsigned short>::max()),
			  2,true,19200,values),
			  follow_timebeat(follow_timebeat),
  outputfile_stream( filename, ostream::binary | ostream::trunc )
{}
sound_file_output_file::~sound_file_output_file()
{
  outputfile_stream.flush();
  outputfile_stream.close();
}
void sound_file_output_file::run()
{
  unsigned short limit=0;
  while( signals.send_to_sound_file_output( sfo_buffer ) > 0)
	{
	  outputfile_stream.write( (const char*)sfo_buffer.data, sfo_buffer.data_size );
	  outputfile_stream.flush();
	  limit++;
	  //	  if( limit > 100000 )
	  //	break;
	}
}

