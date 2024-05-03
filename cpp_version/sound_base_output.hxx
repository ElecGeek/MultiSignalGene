#ifndef __SOUND_BASE_OUTPUT__
#define __SOUND_BASE_OUTPUT__

#include "main_loop.hxx"
#include "samplerate_handler.hxx"


/** \brief Base of the sound output modules
 *
 * It holds a reference on the main_loop to collect more data at a regular schedule
 * It holds a set of buffers handles
 * Some virtual bases run stuff related to the sample rate, the ready checks, the pre-run etc...
 */
class sound_file_output_base
{
protected:
  main_loop*signals;
  sample_rate_list sr_list;
  sound_data_output_buffer sfo_buffer;
  midi_data_io_buffer mdio_buffer;
public:
  sound_file_output_base()=delete;
  explicit sound_file_output_base(const sound_data_output_buffer&sfo_buffer);
  virtual~sound_file_output_base()=default;
  void set_signals(main_loop*const&);
  sample_rate_list process_sample_rate(const sample_rate_list&)const;
  virtual bool test_sound_format()const;
  virtual bool is_ready()const;
  virtual bool pre_run();
  virtual void run()=0;
};



#endif
