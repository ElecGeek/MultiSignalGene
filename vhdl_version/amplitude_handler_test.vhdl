--! Use standard library
library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all,
  ieee.math_real.all,
  work.amplitude_handler_pac.all;

--! @brief This module runs a single test of amplitude_handler
--!
--! \n
--! For long simulation a progress data is sent regularly\n
--! \n
--! It can be used in batch mode or in stand alone.
--! The batch mode provides signal to publish the reports after all the
--! instantiations has terminated. This is to avoid to mix the progress with the
--! reported data.
entity amplitude_handler_test is
  generic (
    sample_rate_id_pwr2 : integer range 0 to 3 := 0
    );
  port (
    --! Tells the simulation is over. It is used (with an and-reduce) in batch mode to start all the reporting
    simul_over : out std_logic;
    --! Controls the report. 0 = wait, 1 = do it, U = do it after the simulation is completed (stand alone)
    display_in :  in std_logic;
    --! Pass the event to the next instantiation after the report is completed (batch mode)
    display_out: out std_logic
  );
end entity amplitude_handler_test;

architecture arch of amplitude_handler_test is
  signal CLK : std_logic := '0';
  signal EN : std_logic;
  signal RST : std_logic_vector( 5 downto 0 ) := ( others => '1' );
  signal main_counter_a : std_logic_vector( 3 downto 0 ) := ( others => '0' );
  constant main_counter_a_max : std_logic_vector( main_counter_a'range ) := ( others=> '1' );
  signal main_counter_mv : std_logic_vector( 3 downto 0 ) := ( others => '0' );
  constant main_counter_mv_max : std_logic_vector( main_counter_mv'range ) := ( others=> '1' );
  signal sub_counter : std_logic_vector( 12 downto 0 ) := ( others => '0' );
  signal sub_counter_max : std_logic_vector( sub_counter'range ) := ( others => '1' );
  signal start_cycle : std_logic;
  signal val_param : std_logic_vector( 15 downto 0) := "1000000000000000";
  signal master_volume : std_logic_vector( 7 downto 0 );
  signal amplitude : std_logic_vector( 7 downto 0 );
  signal amplitude_out : std_logic_vector( 15 downto 0 );
  signal delta_out : integer;
  signal write_param : std_logic;
  signal simul_over_s : std_logic := '0';
  signal display_out_s : std_logic := '0';
begin
  simul_over <= simul_over_s;
  
  main_proc : process
    variable ind_counter : integer;
  begin
      if main_counter_a /= main_counter_a_max or
         main_counter_mv /= main_counter_mv_max or
         sub_counter /= sub_counter_max then
        CLK_1 : if CLK = '1' then
          RST( RST'high - 1 downto RST'low ) <= RST( RST'high downto RST'low + 1 );
          RST( RST'high ) <= '0';
          RST_IF : if RST = std_logic_vector( to_unsigned( 0 , RST'length )) then
          --        counter <= std_logic_vector( unsigned( counter ) + 1 );
              if sub_counter( sub_counter'low + 4 downto sub_counter'low ) = "00000" then
                ind_counter := main_counter_a'high;
                for ind_a in amplitude'high downto amplitude'low loop
                  amplitude( ind_a ) <= main_counter_a( ind_counter );
                  if ind_counter > main_counter_a'low then
                    ind_counter := ind_counter - 1;
                  else
                    ind_counter := main_counter_a'high;
                  end if;
                end loop;
                ind_counter := main_counter_mv'high;
                for ind_a in master_volume'high downto master_volume'low loop
                  master_volume( ind_a ) <= main_counter_mv( ind_counter );
                  if ind_counter > main_counter_mv'low then
                    ind_counter := ind_counter - 1;
                  else
                    ind_counter := main_counter_mv'high;
                  end if;
                end loop;
                delta_out <= to_integer( unsigned( amplitude_out )) -
                             to_integer( unsigned( amplitude )) * to_integer( unsigned( master_volume ));
              elsif sub_counter( sub_counter'low + 4 downto sub_counter'low ) = "00001" then
                write_param <= '1';
              elsif sub_counter( sub_counter'low + 4 downto sub_counter'low ) = "00010" then
                write_param <= '0';
                EN <= '1';
              else
                EN <= '0';
              end if;
              if sub_counter = sub_counter_max then
                sub_counter <= ( others => '0' );
                if main_counter_mv = main_counter_mv_max then
                  main_counter_mv <= ( others => '0' );
                  if main_counter_a = main_counter_a_max then
                    main_counter_a <= ( others => '0' );
                  else
                    main_counter_a <= std_logic_vector( unsigned( main_counter_a ) + 1 );
                  end if;
                  assert false report "Done: " &
                    integer'image( to_integer( unsigned( main_counter_a ))) &
                    "/" &
                    integer'image( 2 ** main_counter_a'length - 1 )
                    severity note;
                else
                  main_counter_mv <= std_logic_vector( unsigned( main_counter_mv ) + 1 );
                end if;
              else
                sub_counter <= std_logic_vector( unsigned( sub_counter ) + 1 );
              end if;
            end if RST_IF;
        end if CLK_1;
        CLK <= not CLK;
        wait for 20 nS;
      else
        report "Simulation is over" severity note;
        simul_over_s <= '1';
        wait;
      end if;
    end process main_proc;

    display : process
    begin
      wait until ( display_in = '1' or ( display_in = 'U' and simul_over_s = '1' ));
      display_out_s <= '1';
    end process display;

    amplitude_handler_instanc : amplitude_handler generic map (
      sample_rate_id_pwr2 => sample_rate_id_pwr2
      )
      port map (
      CLK => CLK,
      EN => EN,
      RST => RST( RST'low ),
      parameter => val_param,
      write_param => write_param,
      master_volume => master_volume,
      amplitude => amplitude,
      amplitude_out => amplitude_out);
        
end architecture arch;
