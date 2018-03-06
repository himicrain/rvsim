/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2017

   Class members for memory

**************************************************************** */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <cstdio>
#include <algorithm>
#include "memory.h"
#include <stdio.h>
#include <cmath>
#include <string.h>
using namespace std;


// Constructor
memory::memory(bool verbose,
	       bool cycle_reporting,
	       unsigned long int address_cycles,
	       unsigned long int data_cycles) {
  // TODO: ...
    this->cycle_reporting = cycle_reporting;
    this->verbose = verbose;
    this->address_cycles = address_cycles;
    this->data_cycles = data_cycles;
    this->cycles = 0;
    this->name = "Memory";

}

memory::~memory(){

}

uint32_t memory::read_byte (uint32_t address) {
  uint32_t temp;
  int block_num = (int)(address/(BLOCK_SIZE*4));
  int offset = (int)(address%(BLOCK_SIZE*4));
  int offset_ = (int)(offset/4);
  int offset_in = offset%4;

  map<int,uint32_t*>::iterator it;
  it = Blocks.find(block_num);
  uint32_t temp_ ;

  if(it == Blocks.end()){
     Blocks.insert(pair<int,uint32_t*>(block_num,new uint32_t[BLOCK_SIZE]{0}));
  }

  temp_ = Blocks[block_num][offset_] ;

  int t = offset_in*8;

  uint32_t m1 = 0xff000000 >> t;

  temp = (temp_ & m1) >> (24-t);

  return temp;



}
// Read a word of data from an address
uint32_t memory::read_word (uint32_t address) {
  // TODO: ...
  uint32_t temp;
  int block_num = (int)(address/(BLOCK_SIZE*4));
  int offset = (int)(address%(BLOCK_SIZE*4));

  int offset_ = (int)(offset/4);

  //int current_size = Blocks.size();
  map<int,uint32_t*>::iterator it;
  it = Blocks.find(block_num);
  if(it == Blocks.end()){
      Blocks.insert(pair<int,uint32_t*>(block_num,new uint32_t[BLOCK_SIZE]{0}));
   }

  temp = Blocks[block_num][offset_];
 
  return temp;


}
void memory::write_byte(uint32_t address, uint32_t data, uint32_t mask) {
  
  int block_num = (int)(address/(BLOCK_SIZE*4));

  int offset = (int)(address%(BLOCK_SIZE*4));
  int offset_ = (int)(offset/4);
  int offset_in = offset%4;

  map<int,uint32_t*>::iterator it;
  it = Blocks.find(block_num);

  uint32_t ms = 0xff000000 >> (24-offset_in*8);
  uint32_t ms1 = ~ms;

  uint32_t md = data << ((offset_in*8));

  if(it == Blocks.end()){
     Blocks.insert(pair<int,uint32_t*>(block_num,new uint32_t[BLOCK_SIZE]{0}));
  }


  uint32_t old_data = Blocks[block_num][offset_];
  uint32_t new_word = (old_data & ms1) | md;

  Blocks[block_num][offset_] = new_word;

   //Blocks[block_num][offset] = (new_word & 0x000000ff);

}
// Write a word of data to an address, mask contains 1s for bytes to be updated
void memory::write_word (uint32_t address, uint32_t data, uint32_t mask) {

  // TODO: ...
  
  uint32_t temp_data = (data & mask);
  bool tb = this->verbose;
  this->verbose = false;
  uint32_t temp_old_data = (this->read_word(address) & (~mask));
  this->verbose = tb;
  uint32_t new_word = temp_data | temp_old_data;

  int offset = (int)(address%(BLOCK_SIZE*4));
  int offset_ = (int)(offset/4);

  int block_num = (int)(address/(BLOCK_SIZE*4));

  map<int,uint32_t*>::iterator it;
  it = Blocks.find(block_num);
  if(it != Blocks.end()){
        Blocks[block_num][offset_] = new_word;
   }
}

// Read a block of data of a given size, starting at an address
// Data size is a power of 2 number of words, and address is block aligned.
void memory::read_block (uint32_t address, uint32_t data[], unsigned int data_size) {

  if(this->cycle_reporting){
    this->calculate_cycle(data_size);
  }

  for(int i=0;i<data_size;i++){
    data[i] = this->read_word(address+i*4);
  }
  if(this->verbose){
      cout << "Memory read block: address = " << hex << setw(8) << setfill('0') << address << ", " << dec <<  data_size << " words" << endl;
  }
  

  // TODO: ...
}

// Write a block of data of a given size, starting at an address
// Data size is a power of 2 number of words, and address is block aligned.
void memory::write_block (uint32_t address, uint32_t data[], unsigned int data_size) {
  // TODO: ...
  for(int i=0;i<data_size;i++){
      this->write_word(address+i*4,data[i], 0xffffffff);
  }
  if(this->verbose){
         cout << "Memory write block: address = " << hex << setw(8) << setfill('0') << address  << ", " << dec << data_size << " words" << endl; 
  }

}

// Display on cout whether the address is present, and if so, display the data
void memory::probe_address (uint32_t address) {
  // TODO: ...
  show_address(address);
}

// Report access stats
void memory::report_accesses () {
  // TODO: ...
}

// Display the word of data at an address
void memory::show_address (uint32_t address) {
  // TODO: ...
  //char r[10];
  uint32_t t = read_word(address);
  cout  << "Memory: "  <<hex << setw(8) << setfill('0') <<  t << endl;

}

// Set the word of data at an address
void memory::set_address (uint32_t address, uint32_t data) {
  // TODO: ...
    write_word(address, data, 0xffffffff);
}

// Return the accumulated number of cycles for read accesses
unsigned long int memory::get_read_cycle_count () {
  // TODO: ...
  return this->cycles;
}

void memory::calculate_cycle(int n){
  int t = (this->address_cycles + this->data_cycles*n) ;
      if(t==0){
          this->cycles ++;
      }else{
          this->cycles += t;
      }
}
void memory::count_cycle(){
  //this->cycles += (this->data_cycles + this->address_cycles);
  //this->calculate_cycle();
    
}

// Load a hex image file
bool memory::load_file(string file_name, uint32_t &start_address) {
  ifstream input_file(file_name);
  string input;
  unsigned long int line_count = 0;
  unsigned long int byte_count = 0;
  char record_start;
  char byte_string[3];
  char halfword_string[5];
  uint32_t record_length;
  uint32_t record_address;
  uint32_t record_type;
  uint32_t record_data;
  uint32_t record_checksum;
  bool end_of_file_record = false;
  uint32_t load_address;
  uint32_t load_data;
  uint32_t load_mask;
  uint32_t load_base_address = 0x00000000;
  start_address = 0x00000000;
  if (input_file.is_open()) {
    while (true) {
      line_count++;
      input_file >> record_start;
      if (record_start != ':') {
	cout << "Input line " << dec << line_count << " does not start with colon character" << endl;
	return false;
      }
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_length);
      input_file.get(halfword_string, 5);
      sscanf(halfword_string, "%x", &record_address);
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_type);
      switch (record_type) {
      case 0x00:  // Data record
	for (uint32_t i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
    load_address = (load_base_address | record_address) + i;
    //load_address = (start_address | record_address) + i;
    //cout << load_base_address << "    " << record_address << endl;
	  // TODO: code to store data byte in memory.
	  //   - load_address is the byte address at which to store
	  //   - record_data is the byte of data to store
	  // ...


    /**
    * code to load data
    */
   //cout << hex << "load_address  " << load_address<< " record_data " << record_data << " mask " << load_mask << endl;
    write_byte(load_address,record_data,0x000000ff );
   // cout << hex << "load_address  " << load_address<< " read   " << read_byte(load_address) << endl;

	  byte_count++;
    //sscanf(byte_string, "%x", &record_data);
	}


	break;
      case 0x01:  // End of file
	end_of_file_record = true;
	break;
      case 0x02:  // Extended segment address (set bits 19:4 of load base address)
	load_base_address = 0x00000000;
	for (uint32_t i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	  load_base_address = (load_base_address << 8) | (record_data << 4);
	}
	break;
      case 0x03:  // Start segment address (ignored)
	for (uint32_t i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	}
	break;
      case 0x04:  // Extended linear address (set upper halfword of load base address)
	load_base_address = 0x00000000;
	for (uint32_t i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	  load_base_address = (load_base_address << 8) | (record_data << 16);
	}
	break;
      case 0x05:  // Start linear address (set execution start address)
	start_address = 0x00000000;
	for (uint32_t i = 0; i < record_length; i++) {
	  input_file.get(byte_string, 3);
	  sscanf(byte_string, "%x", &record_data);
	  start_address = (start_address << 8) | record_data;
	}
	break;
      }
      input_file.get(byte_string, 3);
      sscanf(byte_string, "%x", &record_checksum);
      input_file.ignore();
      if (end_of_file_record)
	break;
    }
    input_file.close();
    cout << dec << byte_count << " bytes loaded, start address = "
	 << setw(8) << setfill('0') << hex << start_address << endl;
    return true;
  }
  else {
    cout << "Failed to open file" << endl;
    return false;
  }
}

/*
int main(){
  memory m(true,true,0x00,0x00);
  uint32_t a = 0;
  m.load_file("command_test_l.hex",a);
  
  
  int p0;
  cin >>hex>> p0;
  cout <<hex<< p0 << endl;
  uint32_t t= m.read_word(p0);
  cout <<hex << t << endl;
}*/