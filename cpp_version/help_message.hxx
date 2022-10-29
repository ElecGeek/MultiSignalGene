#include <iostream>
#include <fstream>
#include <sstream>


string help()
{
  ostringstream ostrstm;

  ostrstm << "Usage: estim <options>. Returns when no input parameters channel are active anymore (or on error)" << endl;
  ostrstm << "At least one output parameters channel OR one raw file OR one audio output is compulsory" << endl;
  ostrstm << "COMMAND LINE OPTIONS" << endl;
  ostrstm << "The option order is not significant unless otherwyse" << endl;
  ostrstm << "Details the options with concurrency and with multiple invocation (MI) behavior:" << endl;
  ostrstm << endl << "-h" << endl;
  ostrstm << "\tDisplays this help. Quit if no other options (other than -v) are specified" << endl;
  ostrstm << "\tMI: The help is displayed multiple times" << endl;
  ostrstm << endl << "-v" << endl;
  ostrstm << "\tDisplays the version. Quit if no other options (other than -h) are specified" << endl;
  ostrstm << "\tMI: The version is displayed multiple times" << endl;
  ostrstm << endl << "-d" << endl;
  ostrstm << "\tSet the debug level" << endl;
  ostrstm << "\tThe default is 0: no debug messages sent to the std output. MI: Only the last usage is considered" << endl;
  ostrstm << endl << "-M" << endl;
  ostrstm << "\tSwitch the format of the input or output parameters into midi mapped notes" << endl;
  ostrstm << "\tShould be placed before a -o or a -i option" << endl;
  ostrstm << "\tIt is the defult one for the -i option. MI: Only the last usage is considered" << endl;
  ostrstm << endl << "-o filename" << endl;
  ostrstm << "\tSpecifies an output parameters channel to export text data about the values and the midi code"<< endl;
  ostrstm << "\tA filename can be given, use - for the console output" << endl;
  ostrstm << "\t0..N MI: They all run in parallel, be careful when specifying two times the same one" << endl;
  ostrstm << endl << "-i filename" << endl;
  ostrstm << "\tSpecifies an input parameters channel using midi mapped commands" << endl;
  ostrstm << "\tIf a filename is specified and the .mid is recognized, The format is the notes mapped midi" << endl;
  ostrstm << "\tIf a filename is specified and the .mid is not recognized, the format is some text formatted TODO" << endl;
  ostrstm << "\tIf - is involved, the format is some text formatted coming from the console TODO" << endl;
  ostrstm << "\t1..N MI: They all run in parallel, be careful when specifying two times the same one" << endl;
  ostrstm << endl << "-f filename" << endl;
  ostrstm << "\tSpecifies a raw signal file. Format is PCM 16 bits as many channels as defined by the -n option" << endl;  
  ostrstm << "\t0..1 MI: Only the last usage is considered" << endl;
  ostrstm << endl << "-K sine, pulses or triangle" << endl;
  ostrstm << "\tSpecifies the output signal file, raw and jackaudio (if so)." << endl;
  ostrstm << "\ts=sine, p=pulses, e=triangle, c=continuous or b=both" << endl;
  ostrstm << "\t\tb and c is/are for test purposes" << endl;
  ostrstm << "\t\tb=odd channels are sine, even are pulse. c=get the enveloppe" << endl;
  ostrstm << "\tIf not specified, the ";
#ifdef __OUTPUT_SINE_MODE__
  ostrstm << "sine";
#elif __OUTPUT_PULSES_MODE__
  ostrstm << "pulse";
#elif __OUTPUT_TRIANGLE_MODE__
  ostrstm << "triangle";
#endif
  ostrstm << " mode is used" << endl;
  ostrstm << "\tIt is a string, each character for each channel." << endl;
  ostrstm << "\t\tIn case of a single character or a string shorted than the number of channels," << endl;
  ostrstm << "\t\tthe (last) character is replicated as many time as needed." << endl;
  ostrstm << "\t0..1 MI: Only the last usage is considered" << endl;
  ostrstm << endl << "-c number" << endl;
  ostrstm << "\tSpecifies the number of audio or raw file channels." << endl;
  ostrstm << "\tThe value can be 0. In such case at least one output parameters channel should be defined" << endl;
  ostrstm << "\tThis option is mostly used to convert a parameter format into another one" << endl; 
  ostrstm << "\tIf not involved, 0 is the default MI: Only the last usage is considered" << endl;
  ostrstm << endl << "-t" << endl;
  ostrstm << "\tFollow the time beat, even if no audio output is selected" << endl;
  ostrstm << "\tIf no option to define an audio output is defined, the software computes as fast as the machine can do" << endl;
  ostrstm << "\tThis option execute the time stamps and plays as an audio output would have do" << endl;
  ostrstm << endl << "-j configuration-file" << endl;
  ostrstm << "\tUse jackaudio as audio channel" << endl;
  ostrstm << "\tThe configuration file contains:" << endl;
  ostrstm << "\t\tThe list of the peers, prefixed by 'audio<tab>', one per line" << endl;
  ostrstm << "\t\tThe list of their connections, contains 0 or more elements" << endl;
  ostrstm << "\tThe same applies for the midi, prefixed by 'midi_in<tab>' or midi_out<tab>, also one per line" << endl;
  ostrstm << "\tThe empty lines are ignored. Everything after a # is ignored" << endl;
  ostrstm << "\tThe order of all the lines is not considered" << endl;
  ostrstm << "\tPassing . as the filename uses the defult hardcoded configuration" << endl;
  ostrstm << "\t0..1 MI: Only the last usage is considered" << endl;
  ostrstm << "\tToday ONLY the defult works, TODO" << endl;
  ostrstm << endl << "-r"; ostrstm << endl;
  ostrstm << "\tSpecifies the sample rate" << endl;
  ostrstm << "\tOnly 1= 48KHz, 2= 96KHz and 4=192KHz are valid" << endl;
  ostrstm << "\tIf not involved, let the output module choose or default" << endl;
  ostrstm << "\tMI: specifies a list submited to the output module" << endl;
  ostrstm << endl << "-l"; ostrstm << endl;
  ostrstm << "\tSpecifies how much time the input is re-executed" << endl;
  ostrstm << "\tWorks only with file or other seekable input" << endl;
  ostrstm << "\t0..1 MI: Only the last usage is considered" << endl;
  ostrstm << endl << "-w"; ostrstm << endl;
  ostrstm << "\tSpecifies, in seconds, a wainting time between the initialisation" << endl;
  ostrstm << "\tand the start of the sequence. Useful to connect to the jackaudio outputs" << endl;
  ostrstm << "\t0..1 MI: Only the last usage is considered" << endl;
  return ostrstm.str();
}

