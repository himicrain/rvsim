#ifndef CACHE_H
#define CACHE_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2017

   Class for cache

**************************************************************** */
#include<vector>
#include <map> 
#include <string.h>
#include <vector>
#include "memory_accessible.h"

using namespace std;








class NODE{
    public: 
        int data;
        NODE* next;
        NODE(int data);
};



class STACK{
    private:
        NODE* head;
        int size;
    public: 
        STACK(int size);
        void push(int data);
        int pop(int data,int pos);
        int isExist(int data);
        int upDate(int data);
        int getLen();
        void print();
        ~STACK();
};




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


  int translate(uint32_t address);

  uint32_t getTagMask();
  uint32_t getIndexMask();
  uint32_t getOffestMask();
  int getBlockId(uint32_t address);
  int getOffestInSet(uint32_t address);
  int getOffestInBlock(uint32_t address);
  int getOffestInCache(uint32_t address);
  int getTag(uint32_t address);

  bool isHit(uint32_t address);
  
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
  void calculate_cycle(int n);
  void count_cycle();
  void count_miss();
  void count_dirty();

  int getIndexInSet(int block_id, int offest_set);
  bool isAlign(uint32_t address);


 int blocks_num ;
 int sets_num;
 int block_size;
 int word_num;

 uint32_t tag_mask;
 uint32_t index_mask;
 uint32_t offest_mask;


 private:
          //vector <uint32_t*> Blocks;
      //map<int,map<int, Entry*>*> Blocks;
     // vector< vector<Entry* > > Blocks;
     uint32_t*** Blocks;
     int** DIRTY; // 1 for dirty, 0 for clean
     uint32_t** TAGS;
     int** INVALID;
      //Entry*** Blocks;
      map<int,STACK*> LRUS;
      unsigned long int cycles;
      
      //string name;
      memory_accessible* backing_store;
      
      unsigned long int size;
      unsigned long int associativity;
      write_strategy_t write_strategy;
      bool verbose;
      bool cycle_reporting;
      unsigned long int address_cycles;
      unsigned long int data_cycles;

      unsigned long int both_cycles;

      unsigned long int miss_num;
      unsigned long int dirty_num;




  // TODO: Add private members here

};

#endif
