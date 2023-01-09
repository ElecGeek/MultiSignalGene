library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

--! More information is in the files located in the cpp_version folder
package frequency_handler_pac is

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
      RST       :  in std_logic;
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
end package frequency_handler_pac;

library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

entity frequency_handler is
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
    --! Low bit:\n
    --! Steps forward when is '1'. This is to control the frequency\n
    --! Other bits:\n
    --! Since it is a state machine that acts "sometimes"
    --!   it can handle more than 1 machine, and stores the idle.\n
    --! This tells which machine.
    --! The number of machines should be a power of 2.
    --! The size of the vector should be the same as parameter machine\n
    --! There is no done output signal as it is always completed in one CLK cycle
    EN_ADDR :  in std_logic_vector;
    RST       :  in std_logic;
    parameter_data :  in std_logic_vector( 15 downto 0 );
    --! Since the frequency handler can be instanciated more than one time
    --! for a given channel, this is a predecode
    parameter_write_prefix : in std_logic_vector;
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
end entity frequency_handler;

architecture arch of frequency_handler is
  signal frequency : std_logic_vector( 23 downto 0 ) := ( others => 'L' );
  signal hold_count : std_logic_vector( 15 + 4 + sample_rate_id_pwr2 + division_rate_pwr2 downto 0 );
  signal high_hold  : std_logic_vector( hold_count'range );
  signal low_hold  : std_logic_vector( hold_count'range );
  signal angle : std_logic_vector( frequency'range );
  constant padding_low_PS : std_logic_vector( angle'high - 4 downto angle'low ) := ( others => '0' );
begin
  assert EN_ADDR'length = parmeter_chanel'length
    report "EN_ADDR and parmeter_chanel should have the same size"
      severity failure;
  assert sample_rate_id_pwr2 /= 3 report "Sample rate is 384, has never been tested" severity warning; 
  angle_out <= angle;
  
  main_proc : process( CLK )
    variable angle_v : std_logic_vector( angle'range );
    variable angle_count_v : boolean;
    variable quadrants : std_logic_vector( 3 downto 0 );
  begin
    if rising_edge( CLK ) then
      RST_IF : if RST = '0' then
        angle_v := angle;
        param_compo : if parmeter_chanel( parmeter_chanel'low ) = '1' and
                         parameter_write_prefix = write_prefix then
          case which_parameter is
            when "0100" =>
              for ind in parameter_data'length - 1 downto 0 loop
                if ( ind + 5 - sample_rate_id_pwr2 - division_rate_pwr2 ) > 0 then
                  frequency( frequency'low + ind + 5 - sample_rate_id_pwr2 - division_rate_pwr2 ) <=
                    parameter_data( parameter_data'low + ind );
                end if;
              end loop;
            when "0101" =>
              if hi_low_hold_always_0 = false then
                high_hold( high_hold'high downto high_hold'high + 1 - parameter_data'length ) <= parameter_data;
                high_hold( high_hold'high - parameter_data'length downto high_hold'low )      <= ( others => '0' );
              end if;
            when "0110" =>
              if hi_low_hold_always_0 = false then
                low_hold( low_hold'high downto low_hold'high + 1 - parameter_data'length ) <= parameter_data;
                low_hold( low_hold'high + 1 - parameter_data'length downto low_hold'low ) <= ( others => '0' );
              end if;
            when "0111" =>
              angle_v := std_logic_vector( unsigned( angle ) +
                                           unsigned( parameter_data( parameter_data'low + 3 downto parameter_data'low ) & padding_low_PS ));
            when "1000" =>
              angle_v := std_logic_vector( unsigned( parameter_data( parameter_data'low + 3 downto parameter_data'low ) & padding_low_PS ));
            when others => null;
          end case;
        end if param_compo;

        EN_IF : if EN_ADDR( EN_ADDR'low ) = '1' then
          if hold_count /= std_logic_vector( to_unsigned ( 0 , hold_count'length )) then
            hold_count <=std_logic_vector( unsigned( hold_count ) - 1 );
          else
            angle_v := std_logic_vector( unsigned( angle_v ) + unsigned( frequency ));
          end if;
          
          quadrants := angle( angle_v'high downto angle_v'high - 1 ) &
                       angle_v( angle_v'high downto angle_v'high - 1 );
          case quadrants is
            when "1100" =>
              start_cycle <= '1';
            when "0001" =>
              if hi_low_hold_always_0 = false then
                hold_count <= high_hold;
              end if;
              start_cycle <= '0';
            when "1011" =>
              if hi_low_hold_always_0 = false then
                hold_count <= low_hold;
              end if;
              start_cycle <= '0';
            when others =>
            start_cycle <= '0';
          end case;
          angle <= angle_v;
        end if EN_IF;
      else
        high_hold <= ( others => '0' );
        low_hold <= ( others => '0' );
        hold_count <= ( others => '0' );
        frequency <= ( others => '0' );
        angle <= ( others => '0' );
      end if RST_IF;
    end if;
  end process main_proc;

end architecture arch;
