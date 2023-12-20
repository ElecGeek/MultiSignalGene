--! Use standard library
library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all,
  ieee.math_real.all,
work.signal_gene.all;

--! @brief Batches the test of the sample_step_sine_test with many value
--!
--! This is an early version
entity sample_step_sine_generic_test is
end entity sample_step_sine_generic_test;

architecture arch of sample_step_sine_generic_test is
  signal simul_over_and_reduce : std_logic;
  component sample_step_sine_test is
    generic (
      sub_counter_size : integer range 2 to 20 := 8;
      limit_calc : integer range 4 to 31 := 7;
      amplitude : integer range 0 to 65535 := 65535;
      rtl_not_fast : boolean);
    port (
      simul_over : out std_logic;
      display_in :  in std_logic;
      display_out: out std_logic);
  end component sample_step_sine_test;
  type test_config_t is record
      sub_counter_size : integer range 2 to 20;
      limit_calc : integer range 4 to 31;
      amplitude : integer range 0 to 65535;
      rtl_not_fast : boolean;
  end record test_config_t;
  type test_config_list_t is array( natural range<> ) of test_config_t;
  constant test_config_list : test_config_list_t := (
    1 => ( sub_counter_size => 7,  limit_calc => 8, amplitude => 65535, rtl_not_fast => true),
    2 => ( sub_counter_size => 4,  limit_calc => 31, amplitude => 65535, rtl_not_fast => true),
    3 => ( sub_counter_size => 9,  limit_calc => 31, amplitude => 65535, rtl_not_fast => true),
    4 => ( sub_counter_size => 4,  limit_calc => 31, amplitude => 255, rtl_not_fast => true),
    5 => ( sub_counter_size => 4,  limit_calc => 31, amplitude => 20, rtl_not_fast => true),
    6 => ( sub_counter_size => 4,  limit_calc => 31, amplitude => 0, rtl_not_fast => true),
    7 => ( sub_counter_size => 4,  limit_calc => 31, amplitude => 0, rtl_not_fast => false)
    );
  signal simul_over : std_logic_vector( test_config_list'range );
  signal display_io : std_logic_vector( test_config_list'low - 1 to test_config_list'high );
  
  signal last_wait : boolean := false;
begin
  and_reduc_over : process is
    variable reduced : std_logic;
    begin
      reduced := '1';
      for ind in simul_over'high downto simul_over'low loop
        reduced := reduced and simul_over( ind );
      end loop;
      simul_over_and_reduce <= reduced;
      wait on simul_over;
  end process and_reduc_over;


  xxx : process is
    begin
      if simul_over_and_reduce = '0' then
        wait for 1 mS;
      elsif last_wait = false then
        last_wait <= true;
        wait for 1 mS;
      else
        wait;
      end if;
    end process xxx;

    display_io( display_io'low ) <= simul_over_and_reduce;
                
    test_config_instanc : for ind in test_config_list'range generate
        sample_step_sine_test_inst : sample_step_sine_test generic map (
          sub_counter_size => test_config_list( ind ).sub_counter_size,
          limit_calc => test_config_list( ind ).limit_calc,
          rtl_not_fast => test_config_list( ind ).rtl_not_fast )
          port map (
            simul_over => simul_over( ind ),
            display_in => display_io( ind - 1) ,
            display_out => display_io( ind ) );
    end generate test_config_instanc;
 
  --sample_step_sine_test_L7 : sample_step_sine_test generic map (
  --  sub_counter_size => 8,
  --  limit_calc => 7)
  --  port map (
  --    simul_over => simul_over( 0 ),
  --    display_in => simul_over_and_reduce ,
  --    display_out => display_io( 0 ) );
  --sample_step_sine_test_Lmax : sample_step_sine_test generic map (
  --  sub_counter_size => 4,
  --  limit_calc => 31)
  --  port map (
  --    simul_over => simul_over( 1 ),
  --    display_in => display_io( 0 ) ,
  --    display_out => display_io( 1 ) );
  --sample_step_sine_test_fine_Lmax : sample_step_sine_test generic map (
  --  sub_counter_size => 9,
  --  limit_calc => 31)
  --  port map (
  --    simul_over => simul_over( 2 ),
  --    display_in => display_io( 1 ) ,
  --    display_out => display_io( 2 ));
  --sample_step_sine_test_low_amp : sample_step_sine_test generic map (
  --  sub_counter_size => 4,
  --  limit_calc => 31,
  --  amplitude => 255)
  --  port map (
  --    simul_over => simul_over( 3 ),
  --    display_in => display_io( 2 ) ,
  --    display_out => display_io( 3 ) );
  --sample_step_sine_test_amp_close_0 : sample_step_sine_test generic map (
  --  sub_counter_size => 4,
  --  limit_calc => 31,
  --  amplitude => 20)
  --  port map (
  --    simul_over => simul_over( 4 ),
  --    display_in => display_io( 3 ) ,
  --    display_out => display_io( 4 ) );
  --sample_step_sine_test_amp_0 : sample_step_sine_test generic map (
  --  sub_counter_size => 4,
  --  limit_calc => 31,
  --  amplitude => 0)
  --  port map (
  --    simul_over => simul_over( 5 ),
  --    display_in => display_io( 4 ) ,
  --    display_out => open );
end architecture arch;





--configuration sample_step_sine_generic_rtl_test of sample_step_sine_generic_test is
--  for arch
--    for all : test_config_instanc
--    use configuration work.sample_step_sine_rtl_test;
--    end for;
--  end for;
--end configuration sample_step_sine_generic_rtl_test;


--configuration sample_step_sine_generic_iobehaviour_test of sample_step_sine_generic_test is
--  for arch
--    for all : test_config_instanc
--    use configuration work.sample_step_sine_iobehaviour_test;
--    end for;
--  end for;
--end configuration sample_step_sine_generic_iobehaviour_test;
