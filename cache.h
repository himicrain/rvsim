#ifndef CACHE_H
#define CACHE_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2017

   Class for cache

**************************************************************** */

#include "memory_accessible.h"

using namespace std;

class cache: public memory_accessible {

 public:

  enum write_strategy_t { READ_ONLY, WRITE_THROUGH, WRITE_BACK };

  // Constructor
  cache(string name,
	memory_accessible* backing_store,
	unsigned long int block_size,
	unsigned long int size,
	unsigned long int associativity,
	write_strategy_t write_strategy,
	bool verbose,
	bool cycle_reporting,
	unsigned long int address_cycles,
	unsigned long int data_cycles);
  	 
  // Read a word of data from an address
  uint32_t read_word (uint32_t address);

  // Write a word of data to an address, mask contains 1s for bytes to be updated
  void write_word (uint32_t address, uint32_t data, uint32_t mask);

  // Read a block of data of a given size, starting at an address
  // Data size is a power of 2 number of words, and address is block aligned.
  void read_block (uint32_t address, uint32_t data[], unsigned int data_size);

  // Write a block of data of a given size, starting at an address
  // Data size is a power of 2 number of words, and address is block aligned.
  void write_block (uint32_t address, uint32_t data[], unsigned int data_size);

  // Display on cout whether the address is present, and if so, display the data
  void probe_address (uint32_t address);

  // Return the accumulated number of cycles for read accesses
  unsigned long int get_read_cycle_count ();

  // Report access stats
  void report_accesses ();
  void count_cycle();

 private:

  // TODO: Add private members here

};

#endif
