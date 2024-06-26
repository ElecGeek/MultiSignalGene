#include "sound_file_output.hxx"
#include "sound_output_buffer.hxx"

#include <chrono>
#include <thread>



sound_data_output_buffer::sound_data_output_buffer( const tuple<size_t,bool,bool>&hash_float_interleave,
													const size_t&data_size,
													const vector<void*>& data):
  channels_bounds(pair<unsigned short,unsigned short>
													(numeric_limits<unsigned short>::min(),
													 numeric_limits<unsigned short>::max())),
  hash_float_interleave( hash_float_interleave ),
  data_size(data_size),
  data(data)
{}
sound_data_output_buffer::sound_data_output_buffer():
  channels_bounds(pair<unsigned short,unsigned short>
													(numeric_limits<unsigned short>::min(),
													 numeric_limits<unsigned short>::max())),
  type_float_interleave(0),
  data_size(0)
  //data is a list/vector/deque, it uses the default constructor
{}
ostream&operator<<(ostream&the_out,const sound_data_output_buffer&a)
{
  the_out<<"Channels: ["<<a.channels_bounds.first<<":"<<a.channels_bounds.second<<"] "<<endl;
  the_out<<"type_hash: "<<get<0>(a.hash_float_interleave);
  the_out<<" float:"<<get<1>(a.hash_float_interleave);
  the_out<<" float:"<<get<2>(a.hash_float_interleave);
  the_out <<", data size:"<< a.data_size<<endl;

  return the_out;
}


sound_file_output_base::sound_file_output_base(const sound_data_output_buffer&sfo_buffer):
  sfo_buffer(sfo_buffer)
{}

void sound_file_output_base::set_signals(main_loop*const&signals)
{
  this->signals = signals;
}
bool sound_file_output_base::test_sound_format()const
{
  return signals->test_sound_format( typeid(void).hash_code(),false,false);
}
bool sound_file_output_base::is_ready()const
{
  return true;
}
bool sound_file_output_base::pre_run()
{
  return true;
}
sample_rate_list sound_file_output_base::process_sample_rate(const sample_rate_list&a)const
{
  return sr_list.intersect_sr_list( a );
}




sound_file_output_dry::sound_file_output_dry(const bool&follow_timebeat):
  sound_file_output_base( sound_data_output_buffer( make_tuple( typeid(void).hash_code(), false, false),0,vector<void*>())),
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
	  microseconds_elapsed = signals->send_to_sound_file_output( sfo_buffer );
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



sound_file_output_file::sound_file_output_file(const pair< sound_data_output_buffer, ostream* >&data,
											   const bool&follow_timebeat):
  sound_file_output_base( data.first),
  follow_timebeat(follow_timebeat),
  output_stream(data.second ),
  //  outputfile_stream( filename, ostream::binary | ostream::trunc ),
  cumul_us_elapsed( 0 )
{
  sfo_buffer.data_size = sizeof( values );
  sfo_buffer.data.push_back( (void*)values );
}
bool sound_file_output_file::test_sound_format()const
{
  return signals->test_sound_format( typeid(short).hash_code(),false,true);
}
void sound_file_output_file::run()
{
  //unsigned short limit=0;
  unsigned long microseconds_elapsed;
  while( true )
	{
	  microseconds_elapsed = signals->send_to_sound_file_output( sfo_buffer );
	  output_stream->write( (const char*)sfo_buffer.data[0], sfo_buffer.data_size );
	  output_stream->flush();
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

