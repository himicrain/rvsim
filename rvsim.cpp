/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2017

   Main program

**************************************************************** */

#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>


#include "memory.h"
#include "cache.h"
#include "processor.h"
#include "commands.h"

using namespace std;

int main(int argc, char* argv[]) {

    // Values of command line options. 
    string arg;
    bool verbose = false;
    bool cycle_reporting = false;
    unsigned long int level1_block_size = 0, level1_size = 0, level1_associativity = 0;
    unsigned long int level2_block_size = 0, level2_size = 0, level2_associativity = 0;
    unsigned long int level2_address_cycles = 0, level2_data_cycles = 0;
    unsigned long int memory_address_cycles = 0, memory_data_cycles = 0;
    bool level1_full_associativity = false, level2_full_associativity = false;
    bool level1_present = false;
    bool level2_present = false;
    bool options_error = false;

    memory* main_memory;
    memory_accessible* level2_cache;
    memory_accessible* level1_icache;
    memory_accessible* level1_dcache;
    processor* cpu;

    unsigned long int cpu_instruction_count;
    
    for (int i = 1; i < argc; i++) {
	// Process the next option
	arg = string(argv[i]);
	if (arg == "-v")  // Verbose output
	    verbose = true;
	else if (arg == "-c")  // Cycle and instruction reporting enabled
	    cycle_reporting = true;
	else if (arg == "-b1") {  // Level-1 cache block size in bytes
	    i++;
	    level1_block_size = atoi(argv[i]);
	}
	else if (arg == "-s1") {  // Level-1 cache size in Kbytes
	    i++;
	    level1_size = atoi(argv[i]);
	}
	else if (arg == "-a1") {  // Level-1 cache associativity
	    i++;
	    if (string(argv[i]) == "full")  // Calculate later using cache size and block size
		level1_full_associativity = true;
	    else
		level1_associativity = atoi(argv[i]);
	}
	else if (arg == "-b2") {  // Level-2 cache block size in bytes
	    i++;
	    level2_block_size = atoi(argv[i]);
	}
	else if (arg == "-s2") {  // Level-2 cache size in Kbytes
	    i++;
	    level2_size = atoi(argv[i]);
	}
	else if (arg == "-a2") {  // Level-2 cache associativity
	    i++;
	    if (string(argv[i]) == "full")  // Calculate later using cache size and block size
		level2_full_associativity = true;
	    else
		level2_associativity = atoi(argv[i]);
	}
	else if (arg == "-ca2") {  // Cycles for address access in level-2 cache
	    i++;
	    level2_address_cycles = atoi(argv[i]);
	}
	else if (arg == "-cd2") {  // Cycles per data word access in level-2 cache
	    i++;
	    level2_data_cycles = atoi(argv[i]);
	}
	else if (arg == "-cam") {  // Cycles for address access in memory
	    i++;
	    memory_address_cycles = atoi(argv[i]);
	}
	else if (arg == "-cdm") {  // Cycles per data word access in memory
	    i++;
	    memory_data_cycles = atoi(argv[i]);
	}
	else {
	    cout << "Unknown option: " << arg << endl;
	}
    }

    // If associativity is full, calculate using cache size and block size
    if (level1_full_associativity && level1_block_size != 0) {
	level1_associativity = (level1_size * 1024) / level1_block_size;
    }
    if (level2_full_associativity && level2_block_size != 0) {
	level2_associativity = (level2_size * 1024) / level2_block_size;
    }

    // If any level-1 options specified, level-1 caches are present
    level1_present = (level1_block_size != 0 || level1_size != 0 || level1_associativity != 0);

    // If any level-2 options specified, level-2 cache is present
    level2_present = (level2_block_size != 0 || level2_size != 0 || level2_associativity != 0);

    // Check for missing or invalid options
    if (level1_present && (level1_block_size == 0 || level1_size == 0 || level1_associativity == 0)) {
	cout << "Missing option, all of -b1, -s1, -a1 required if level-1 caches are present" << endl;
	options_error = true;
    }
    if (level2_present && (level2_block_size == 0 || level2_size == 0 || level2_associativity == 0)) {
	cout << "Missing option, all of -b2, -s2, -a2 required if level-2 cache is present" << endl;
    }
    if ((level1_present &&
	 ((level1_size & (level1_size - 1)) ||
	  (level1_block_size & (level1_block_size - 1)) ||
	  (level1_associativity & (level1_associativity - 1)))) ||
	(level2_present && 
	 ((level2_size & (level2_size - 1)) ||
	  (level2_block_size & (level2_block_size - 1)) ||
	  (level2_associativity & (level2_associativity - 1))))) {
	cout << "Cache sizes, block sizes and associativity must all be powers of 2" << endl;
	options_error = true;
    }
    if (level1_present && (level1_block_size * level1_associativity > level1_size * 1024)) {
	cout << "Level 1 cache size must be at least block size times associativity" << endl;
	options_error = true;
    }
    if (level2_present && (level2_block_size * level2_associativity > level2_size * 1024)) {
	cout << "Level 2 cache size must be at least block size times associativity" << endl;
	options_error = true;
    }
    if (level2_present && !level1_present) {
	cout << "Level-1 caches must be present if level-2 cache is present" << endl;
    }
    if (level1_present && level2_present && (level1_block_size > level2_block_size)) {
	cout << "Level 1 block size must not be larger than level 2 block size" << endl;
	options_error = true;
    }
    if (cycle_reporting && level2_present &&
	(level2_address_cycles == 0 || level2_data_cycles == 0)) {
	cout << "Missing option, -ca2 and -cd2 required with -c for cycle reporting when level-2 cache is present" << endl;
	options_error = true;
    }
    if (cycle_reporting &&
	(memory_address_cycles == 0 || memory_data_cycles == 0)) {
	cout << "Missing option, -cam and -cdm required with -c for cycle reporting" << endl;
	options_error = true;
    }
    if (options_error) {
	return 1;  // Non-zero exit code indicates error
    }    

    if (verbose) {
	// Display summary of cache options
	if (level1_present)
	    cout << "Level 1 caches: size = " << dec << level1_size <<
		"KB, block size = " << dec << level1_block_size <<
		", associativity = " << dec << level1_associativity << endl;
	if (level2_present)
	    cout << "Level 2 cache: size = " << dec << level2_size <<
		"KB, block size = " << dec << level2_block_size <<
		", associativity = " << dec << level2_associativity << endl;
	if (cycle_reporting) {
	    if (level2_present)
		cout << "Level-2 cache: address access cycles = " << dec << level2_address_cycles <<
		    ", data access cycles = " << dec << level2_data_cycles << endl;
	    cout << "Memory: address access cycles = " << dec << memory_address_cycles <<
		", data access cycles = " << dec << memory_data_cycles << endl;
	}
    }
  
    main_memory = new memory (verbose, cycle_reporting, memory_address_cycles, memory_data_cycles);
    if (level2_present) {
	level2_cache = new cache ("Level-2 cache",
				  main_memory, level2_block_size, level2_size * 1024, level2_associativity,
				  cache::WRITE_BACK,
				  verbose, cycle_reporting, level2_address_cycles, level2_data_cycles);
    } else {
	level2_cache = main_memory;
    }
    if (level1_present) {
	level1_icache = new cache ("Level-1 instruction cache",
				   level2_cache, level1_block_size, level1_size * 1024, level1_associativity,
				   cache::READ_ONLY,
				   verbose, false, 0, 0); // no cycle reporting, since no delay
	level1_dcache = new cache ("Level-1 data cache",
				   level2_cache, level1_block_size, level1_size * 1024, level1_associativity,
				   cache::WRITE_THROUGH,
				   verbose, false, 0, 0); // no cycle reporting, since no delay
    } else {
	level1_icache = main_memory;
	level1_dcache = main_memory;
    }
    cpu = new processor (level1_icache, level1_dcache, verbose);

    interpret_commands(main_memory, cpu, verbose);
    // Report final statistics

    cpu_instruction_count = cpu->get_instruction_count();
    cout << "Instructions executed: " << dec << cpu_instruction_count << endl;

    if (level1_present) {
	level1_icache->report_accesses();
	level1_dcache->report_accesses();
    }
    if (level2_present)
	level2_cache->report_accesses();

    if (cycle_reporting) {
	// Required for postgraduate Computer Architecture course
	unsigned long int cpu_cycle_count;
	unsigned long int level2_cache_cycle_count;
	unsigned long int main_memory_cycle_count;

	cpu_cycle_count = cpu->get_cycle_count();
	if (level2_present)
	    level2_cache_cycle_count = level2_cache->get_read_cycle_count();
	else
	    level2_cache_cycle_count = 0;
	main_memory_cycle_count = main_memory->get_read_cycle_count();

	cout << "CPU cycle count: " << dec << cpu_cycle_count << endl;
	if (level2_present)
	    cout << "Stall cycles accessing level-2 cache: " << dec << level2_cache_cycle_count << endl;
	cout << "Stall cycles accessing memory: " << dec << main_memory_cycle_count << endl;
	cout << "Average CPI (ideal): "
	     << fixed << setprecision(6)
	     << (double)cpu_cycle_count / (double)cpu_instruction_count << endl;
	cout << "Average CPI (actual): "
	     << fixed << setprecision(6)
	     << ((double)cpu_cycle_count + (double)level2_cache_cycle_count + (double)main_memory_cycle_count)
	    / (double)cpu_instruction_count
	     << endl;
    }
}
