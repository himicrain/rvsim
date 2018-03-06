#ifndef PROCESSOR_H
#define PROCESSOR_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2017

   Class for processor

**************************************************************** */

#include "memory_accessible.h"
#include <set>
using namespace std;

class processor {

 private:
    uint32_t X[32];
    uint32_t PC ;
    uint32_t BreakPoint;
    uint32_t Instruction_count;
    set<uint32_t> breakpoints;
    memory_accessible* icache;
	  memory_accessible* dcache;
    bool verbose;
    uint32_t temp_reg;
    int cycles;


  // TODO: Add private members here

 public:


  //memory_accessible* icache;
  // Consructor
  uint32_t getIcache(uint32_t address){
    return this->icache->read_word(address);
  }
  processor(memory_accessible* icache,
	    memory_accessible* dcache,
	    bool verbose);

  // Display PC value
  void show_pc();

  // Set PC to new value
  void set_pc(uint32_t new_pc);
  uint32_t get_pc();
  // Display register value
  void show_reg(unsigned int reg_num);

  // Set register to new value
  void set_reg(unsigned int reg_num, uint32_t new_value);
  uint32_t get_reg(uint32_t num);
  // Execute a number of instructions
  void execute(unsigned int num, bool breakpoint_check);

  // Clear breakpoint
  void clear_breakpoint();

  // Set breakpoint at an address
  void set_breakpoint(uint32_t address);

  // Probe the instruction cache interface for an address
  void probe_instruction (uint32_t address);

  // Probe the data cache interface for an address
  void probe_data (uint32_t address);

  uint64_t get_instruction_count();

  uint64_t get_cycle_count();
  void count_cycle(int cycle);

};

#endif
