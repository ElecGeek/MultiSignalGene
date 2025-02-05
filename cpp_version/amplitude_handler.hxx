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
  // slew-rate uses 16 bits value plus 2 bits padding on the right
  unsigned long slewrate;
  // amplitude25 is the same range as slew-rate plus one bit on the left for overflow detection
  long amplitude25;
  unsigned char requested_ampl;
  // sample_rate_K should be 48 = 48KHz 96 = 96KHz 192 = 192KHz
  const unsigned short sample_rate_K;
 public:
  amplitude_handler()=delete;
  explicit amplitude_handler(const unsigned short&sample_rate_K);
  constexpr void set_volume(const unsigned char&volume )
  {
	this->volume = volume;
  }
  constexpr void set_slewrate(const unsigned short&slewrate )
  {
	this->slewrate = slewrate;
	// 384KHz sampling rate may have problems here
	this->slewrate *= 4 * 48;
	this->slewrate /= sample_rate_K;
  }
  constexpr void set_amplitude(const unsigned char&amplitude )
  {
	requested_ampl = amplitude;
  }
  constexpr unsigned short operator()(void)
  {
	if( ((unsigned char)(amplitude25>>17)) > requested_ampl )
	  {
		amplitude25 -= slewrate;
		// negative (high bit 1) or under the requested value
		if ( (( amplitude25 & 0x80000000 ) == 0x80000000 ) ||
			 (((unsigned char)(amplitude25>>17)) < requested_ampl ) )
		  amplitude25 = ((unsigned long)requested_ampl) << 17;
	  }
	else if (((unsigned char)(amplitude25>>17)) < requested_ampl )
	  {
		amplitude25 += slewrate;
		// Greater than 134 217 728 or over the requested value
		if (( ( amplitude25 & 0x02000000 ) == 0x02000000 ) ||
			(((unsigned char)(amplitude25>>17)) > requested_ampl ) )
		  amplitude25 = ((unsigned long)requested_ampl) << 17;
	  }
	//    cout << hex << ((unsigned short)volume) * ((unsigned short)(amplitude25>>17)) << "  ";
	
	return ((unsigned short)volume) * ((unsigned short)(amplitude25>>17));
  }

};

#endif
