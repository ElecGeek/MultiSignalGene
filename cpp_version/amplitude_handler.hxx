// This class handles the final amplitude
//
// For safety reasons, there is always a volume control
// The programs sets the amplitude as a percent of the maximum
//   (means the volume value) set by the user
//
// This class hold the volume and calculate the output amplitude
//
// char, short, long24 are used because this is a PoC of the VHDL version

#ifndef __AMPLITUDE_HANDLER__
#define __AMPLITUDE_HANDLER__

class amplitude_handler
{
  unsigned char volume;
  // slew-rate uses 14 bits value plus 2 bits padding on the right
  unsigned long slewrate;
  // amplitude24 is the same range as slewrate plus one bit on the left for overflow detection
  unsigned long amplitude24;
  unsigned char requested_ampl;
  // sample_rate_id should be 1 = 48KHz 2 = 96KHz 4 = 192KHz
  const unsigned short sample_rate_id;
 public:
  amplitude_handler()=delete;
  explicit amplitude_handler(const unsigned short&sample_rate_id);
  constexpr void set_volume(const unsigned char&volumpe )
  {
	this->volume = volume;
  }
  constexpr void set_slewrate(const unsigned short&slewrate )
  {
	this->slewrate = slewrate;
	this->slewrate *= 4 * 48;
	this->slewrate /= sample_rate_id;
  }
  constexpr void set_amplitude(const unsigned char&amplitude )
  {
	requested_ampl = amplitude;
  }
  constexpr unsigned short operator()(void)
  {
	if( ((unsigned char)(amplitude24>>16)) > requested_ampl )
	  {
		amplitude24 -= slewrate;
		if ( (( amplitude24 & 0x10000000 ) == 0x10000000 ) ||
			 (((unsigned char)(amplitude24>>16)) < requested_ampl ) )
		  amplitude24 = ((unsigned long)requested_ampl) << 16;
	  }
	else if (((unsigned char)(amplitude24>>16)) < requested_ampl )
	  {
		amplitude24 += slewrate;
		if (( ( amplitude24 & 0x01000000 ) == 0x01000000 ) ||
			(((unsigned char)(amplitude24>>16)) > requested_ampl ) )
		  amplitude24 = ((unsigned long)requested_ampl) << 16;
	  }
	//    cout << hex << ((unsigned short)volume) * ((unsigned short)(amplitude24>>16)) << "  ";
	
	return ((unsigned short)volume) * ((unsigned short)(amplitude24>>16));
  }

};

#endif
