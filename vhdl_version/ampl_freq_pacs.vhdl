library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

--! More information is in the files located in the cpp_version folder
package ampl_freq_pacs is
  component amplitude_handler is
    generic (
      sample_rate_id_pwr2 : integer range 0 to 3;
      --! Since the amplitude handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      write_prefix : in std_logic_vector);
    port (
      --! master clock
      CLK     :  in std_logic;
      RST       :  in std_logic;
      --! Low bit:\n
      --! Steps forward when is '1'. This is to control the amplitude
      --! The result of the previous run should be latched no later
      --! than one clock cycle after the EN\n
      --! Other bits:\n
      --! Since it is a state machine that acts "sometimes"
      --!   it can handle more than 1 machine.
      --! This tells which machine.
      --! The number of machines should be a power of 2.
      --! The size of the vector should be the same as parameter machine
      EN_ADDR :  in std_logic_vector;
      --! Similar as the EN_ADDR but tells the channel is available
      --! There is no ready for next signal as this module is not able
      --! to handle more than one channel at a time
      cycle_completed : out std_logic_vector;
      master_volume :  in std_logic_vector( 7 downto 0 );
      amplitude : in std_logic_vector( 7 downto 0 );
      parameter_data :  in std_logic_vector( 15 downto 0 );
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      parameter_write_prefix : in std_logic_vector;
      --! Low bit:\n
      --! Write enable of a pramameter\n
      --! Other bits:\n
      --! Tells which channel
      parmeter_channel :  in std_logic_vector;
      --! 0000= set the slewrate
      which_parameter : in std_logic_vector( 3 downto 0 );
      amplitude_out   : out std_logic_vector);
    end component amplitude_handler;


  component frequency_handler is
    generic (
      --! See in the cpp version
      sample_rate_id_pwr2 : integer range 0 to 3;
      --! See in the cpp version
      division_rate_pwr2 : integer range 0 to 5;
      --! This prevents the high or low to be set
      --! By this way, the optimiser eliminate the according logic
      --! Otherwyse, it can not know the parameter is never set
      hi_low_hold_always_0 : boolean := false;
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      write_prefix : in std_logic_vector);
    port (
      --! master clock
      CLK     :  in std_logic;
      RST       :  in std_logic;
      --! Low bit:\n
      --! Steps forward when is '1'. This is to control the frequency\n
      --! Other bits:\n
      --! Since it is a state machine that acts "sometimes"
      --!   it can handle more than 1 machine, and stores the idle.\n
      --! This tells which machine.
      --! The number of machines should be a power of 2.
      --! The size of the vector should be the same as parameter machine\n
      --! There is no done output signal nor ready for next signal
      --! as it is always completed in one CLK cycle
      EN_ADDR :  in std_logic_vector;
      parameter_data :  in std_logic_vector( 15 downto 0 );
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      parameter_write_prefix : in std_logic_vector;
      --! Low bit:\n
      --! Write enable of a pramameter\n
      --! Other bits:\n
      --! Tells which chanel
      parmeter_chanel :  in std_logic_vector;
      --! writes only one parameter at a time
      --! 0100=set frequency, TODO steps and limits
      --! 0101=set high hold, 0110=set low hold, TODO steps and limits
      --! 0111=shift the phase 4 low bits for a shift of N times PI/8 
      --! 1000=set/reset the phase
      which_parameter :  in std_logic_vector( 3 downto 0 );
      --! This signal is set one CLK cycle on each pass through the 0 angle
      start_cycle : out std_logic;
      --! This return the exact angle
      angle_out   : out std_logic_vector( 23 downto 0 ));
  end component frequency_handler;
end package ampl_freq_pacs;


