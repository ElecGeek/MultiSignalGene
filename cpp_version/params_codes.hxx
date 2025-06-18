#ifndef __PARAMS_CODES__
#define __PARAMS_CODES__


struct midi_event {
  unsigned char code;
  unsigned char key;
  unsigned char value;
  midi_event();
  enum status_t{ warming_up, running, end_track } status;
};

// Keep left and right in order to allow comma or point as a decimal separator
// and even a mix inside the same file
// The restriction is 3 digits blocs are not allowed e.g. 123,456.5
struct mnemo_event {
  string TS_left;
  string TS_right;
  string TS_unit;
  string channel;
  string mnemo;
  string value_left;
  string value_right;
  string value_unit;
  mnemo_event();
  mnemo_event( const string&, const string&, const string&,
			   const string&channel,
			   const string&mnemo,
			   const string&, const string&, const string& );
  enum status_t{ warming_up, running, end_track } status;
};



#endif
