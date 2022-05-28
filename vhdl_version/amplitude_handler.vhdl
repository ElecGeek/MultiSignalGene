library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

--! More information is in the files located in the cpp_version folder
package amplitude_handler_pac is
  component amplitude_handler is
    generic (
      sample_rate_id_pwr2 : integer range 0 to 3);
    port (
      --! master clock
      CLK     :  in std_logic;
      --! Steps forward when is '1'. This is to control the amplitude
      --! The result should be latched no later than one clock cycle
      --! after the EN
      EN      :  in std_logic; 
      RST       :  in std_logic;
      master_volume :  in std_logic_vector( 7 downto 0 );
      amplitude : in std_logic_vector( 7 downto 0 );
      parameter :  in std_logic_vector( 15 downto 0 );
      --! write the slewrate parameter
      --! 1= set the slewrate
      write_param : in std_logic;
      amplitude_out   : out std_logic_vector);
    end component amplitude_handler;
end package amplitude_handler_pac;

library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

entity amplitude_handler is
  generic (
    sample_rate_id_pwr2 : integer range 0 to 3);
  port (
     --! master clock
    CLK     :  in std_logic;
    --! Steps forward when is '1'. This is to control the amplitude
    --! The result should be latched no later than one clock cycle
    --! after the EN
    EN      :  in std_logic; 
    RST       :  in std_logic;
    master_volume :  in std_logic_vector;
    amplitude : in std_logic_vector;
    parameter :  in std_logic_vector( 15 downto 0 );
    --! write the slewrate parameter
    --! 1= set the slewrate
    write_param : in std_logic;

    amplitude_out   : out std_logic_vector);
end entity amplitude_handler;

architecture arch of amplitude_handler is
  --! The multiplications, addition and compare are done sequencially.
  --! After each EN pulse, the system computes the next value,
  --! and keep it ready for reading.\n
  --! This adds a latency of maximum 2 read cycles.
  signal state : std_logic_vector( 3 downto 0 ) := "1111";
  signal req_volume : std_logic_vector( master_volume'range );
  signal req_amplitude : std_logic_vector( amplitude'length + master_volume'length - 1  downto 0 );
  signal target_amplitude : std_logic_vector( req_amplitude'high + 2 - 1 downto req_amplitude'low );
  signal amplitude_run : std_logic_vector( 23 + 2 downto 0 ) := ( others => 'L' );
  signal slewrate : std_logic_vector( 15 downto 0 );
begin
  assert sample_rate_id_pwr2 /= 3 report "Sample rate is 384, has never been tested" severity warning; 
  assert master_volume'length > 2 and master_volume'length < 13
    report "Size of master_volume (" & integer'image( master_volume'length ) & ") should be between 3 and 12"
      severity failure;
  assert master_volume'length = amplitude'length
    report "Size of master_volume (" & integer'image( master_volume'length ) & ") " &
      "and amplitude (" & integer'image( amplitude'length ) & ") should be the same"
    severity failure;
  assert master_volume'length = amplitude'length
    report "Size of amplitude_out (" & integer'image( amplitude_out'length ) & ") " &
      "should be twice the size of the amplitude (" & integer'image( amplitude'length ) & ")"
    severity failure;
  assert master_volume'length = 8
    report "Size of master_volume (" & integer'image( master_volume'length ) &
    ") has never been tested for values other than 8"
      severity warning;

  amplitude_out <= amplitude_run( amplitude_run'high - 2 downto amplitude_run'high - 2 + 1 - amplitude_out'length );
  
  main_proc : process( CLK )
    variable padded_slewrate : std_logic_vector( amplitude_run'range );
    variable req_amplitude_v : std_logic_vector( req_amplitude'range );
  begin
    if rising_edge( CLK ) then
      RST_IF : if RST = '0' then
        PARAM_IF : if write_param = '1' then
          slewrate <= parameter;
        end if PARAM_IF;
        STATE_IF : if state = "1111" then
          if EN = '1' then
            state <= "0000";
          end if;
          req_amplitude <= ( others => '0' );
          req_volume <= master_volume;
        elsif unsigned( state ) < to_unsigned( req_volume'length, state'length ) then
          -- Multiply the master volume with the amplitude
          req_amplitude_v( req_amplitude_v'high downto req_amplitude_v'low + 1 ) :=
            req_amplitude( req_amplitude'high - 1 downto req_amplitude'low );
          req_amplitude_v( req_amplitude_v'low ) := '0';
          if req_volume( req_volume'high ) = '1' then
            req_amplitude_v := std_logic_vector( unsigned( req_amplitude_v ) + unsigned( amplitude ));
          end if;
          req_amplitude <= req_amplitude_v;
          req_volume( req_volume'high downto req_volume'low + 1 ) <=
            req_volume( req_volume'high - 1 downto req_volume'low );
          req_volume( req_volume'low ) <= '-';
          state <= std_logic_vector( unsigned( state ) + 1 );
        elsif state = std_logic_vector( to_unsigned( req_volume'length, state'length )) then
          -- If amplitude_run is different than req_amplitude, compute the slew rate
          padded_slewrate := ( others => '0' );
          padded_slewrate( padded_slewrate'low + slewrate'length - 1 downto padded_slewrate'low ) := slewrate;
          if sample_rate_id_pwr2 = 3 then
            padded_slewrate( padded_slewrate'high - 1 downto padded_slewrate'low ) :=
              padded_slewrate( padded_slewrate'high downto padded_slewrate'low + 1 );
          elsif sample_rate_id_pwr2 = 1 then
            padded_slewrate( padded_slewrate'high downto padded_slewrate'low + 1 ) :=
              padded_slewrate( padded_slewrate'high - 1 downto padded_slewrate'low );
            padded_slewrate( padded_slewrate'low downto padded_slewrate'low ) := "0";
          elsif sample_rate_id_pwr2 = 0 then
            padded_slewrate( padded_slewrate'high downto padded_slewrate'low + 2 ) :=
              padded_slewrate( padded_slewrate'high - 2 downto padded_slewrate'low );
            padded_slewrate( padded_slewrate'low + 1 downto padded_slewrate'low ) := "00";
          end if;
          if unsigned( req_amplitude ) > unsigned( amplitude_run( amplitude_run'high - 2 downto amplitude_run'high - 2 - req_amplitude'length + 1 )) then
            amplitude_run <= std_logic_vector( signed( amplitude_run ) + signed( padded_slewrate ));
          elsif unsigned( req_amplitude ) < unsigned( amplitude_run( amplitude_run'high - 2 downto amplitude_run'high - 2 - req_amplitude'length + 1 )) then
            amplitude_run <= std_logic_vector( signed( amplitude_run ) - signed( padded_slewrate ));
          end if;
          state <= std_logic_vector( unsigned( state ) + 1 );
        elsif state = std_logic_vector( to_unsigned( req_volume'length + 1, state'length )) then
          -- Check for limits (not negative, not aver 24 bits
          case amplitude_run( amplitude_run'high downto amplitude_run'high - 1 ) is
            when "10" | "11" =>
              amplitude_run <= ( others => '0' );
            when "01" =>
              amplitude_run( amplitude_run'high downto amplitude_run'high - 1 ) <= "00";
              amplitude_run(amplitude_run'high - 2 downto amplitude_run'low ) <= ( others => '1' );
            when others =>
              null;
          end case;          
          state <= std_logic_vector( unsigned( state ) + 1 );
        else
          -- go to the end
          state <= std_logic_vector( unsigned( state ) + 1 );
        end if STATE_IF;
      else
        STATE <= "1111";
        amplitude_run <= ( others => '0' );
      end if RST_IF;
    end if;
  end process main_proc;

end architecture arch;
