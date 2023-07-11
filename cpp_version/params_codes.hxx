#ifndef __PARAMS_CODES__
#define __PARAMS_CODES__

/* Designed for the documentation tools */
/** \brief Base of the midi input and output method
 *
 * \n
 * Midi note binary, midi velocity, midi note music (range), mnemonic, description, limits\n
 * Values format: e = exponent, m = mantissa, c = constant to exponent\n
 * 0000 0eee  0mmmmmmm c=1   C00 to G00  OF  Base frequency             max = 2976.56Hz step = 23.44Hz min = 0.183Hz, 5.46s step = 0.366Hz\n
 * 0000 1eee  0mmmmmmm c=0  G#00 to D#0  OS  Main amplitude slew-rate   max = ??? min step = ? max step = ?\n
 * 0001 000e  0mmmmmmm c=1    E0 to  F0  OA  Main amplitude             max = 254 min step = 1 max step = 2\n
 * 0001 0010  0xxxxxxx na.          F#0  NQ  Abort this parameters input channel, use this for testing\n
 * 0001 0011           na.           G0   /  Reserved\n
 * 0001 0100  0mmmmmmm c=1          G#0  AA  Amplitude modulation depth max = 127 step = 2\n
 * 0001 0101  0mmmmmmm c=1           A0  BA  Pulse depth                max = 127 step = 2\n
 *                                           Modulates: using a sine from -1 to +1 trnaslated into 0 to +1 if mm = 00\n
 *                                             using the absolute value of a sine from 0 to +1 if mm = 01, be careful freq is double!\n
 * 0001 0110  000000mm c=0          A#0  BM  ... for the pulse signal\n
 * 0001 0110  000001mm c=0          A#0  AM  ... for the amplitude modulation\n 
 * 0001 0110  0001mmmm c=0          A#0  OP  Phase shift of the base signal, use with caution\n
 * 0001 0110  0010mmmm c=0          A#0  BP  Phase shift of the pulse signal, use with caution\n
 * 0001 0110  0011mmmm c=0          A#0  AP  Phase shift of the amplitude modulation signal, use with caution\n
 * 0001 0110  0101mmmm c=0          A#0  OO  Phase (re)set (overwrite) of the base signal, use with caution\n  
 * 0001 0110  0110mmmm c=0          A#0  BO  Phase (re)set (overwrite) overwrite of the pulse signal, use with caution\n  
 * 0001 0110  0111mmmm c=0          A#0  AO  Phase (re)set (overwrite) overwrite of the amplitude modulation signal, use with caution\n  
 * 0001 0111  0xxxxxxx               B0  NN  NOP\n
 * 0001 1eee  0mmmmmmm c=0    C1 to  G1  AF  Amplitude modulation freq  max = 186.035Hz step = 1.465Hz min = 0.01144Hz, 87.38s step 43.69s\n
 * 0010 00ee  0mmmmmmm c=0   G#1 to  B1  BF  Pulse frequency max = 106.306Hz step = 1.465Hz min = 0.01144Hz, 87.38s step 43.69s\n
 * 0010 01ee  0mmmmmmm c=6    C2 to D#2  BH  Pulse high hold time\n
 * 0010 10ee  0mmmmmmm c=6    E2 to  B2  BI  Pulse low hold time\n
 * 0010 11ee  0mmmmmmm     Reserved\n
 * \n
 * 1xxx xxxx xxxxxxxx     Can not be used as it is not a midi note code
 */ 


struct midi_event {
  unsigned char code;
  unsigned char key;
  unsigned char value;
  midi_event();
 public:
  enum status_t{ warming_up, running, end_track } status;
};
struct mnemo_event {
  string code;
  string key;
  string value;
  string value_unit;
  //  mnemo_event();
 public:
  enum status_t{ warming_up, running, end_track } status;
};



#endif
