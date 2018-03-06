#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "processor.h"
#include "Instruction.h"
using namespace std;



 processor::processor(memory_accessible* icache,
	    memory_accessible* dcache,
	    bool verbose){
        this->icache = icache;
        this->dcache = dcache;
        this->verbose = verbose;
        this->PC = 0x00;
        memset(X,0,sizeof(X));
        BreakPoint = 0xffffffff;
        this->Instruction_count = 0;

      }

  // Display PC value
  void  processor::show_pc(){
    uint32_t t = this->PC;
    cout << hex << setw(8) << setfill('0') << t << endl;
  
  }
  uint32_t processor::get_pc(){
    return this->PC;
  }
  // Set PC to new value
  void processor::set_pc(uint32_t new_pc){
      this->PC = new_pc;
  }

  // Display register value
  void processor::show_reg(unsigned int reg_num){
    //cout << hex << X[reg_num] << endl;
    char r[10];
    uint32_t t ;
    if(reg_num==0){
      t = 0x00000000;
    }else
      t = X[reg_num];
    cout << hex << setw(8) << setfill('0') <<  t << endl;
  }

  // Set register to new value
  void processor::set_reg(unsigned int reg_num, uint32_t new_value){
    this->temp_reg = reg_num;
    if(reg_num != 0){
      this->X[reg_num] = new_value;
    }else if(reg_num == 0){
      this->X[reg_num] = 0x00;
    }
  }

uint32_t processor::get_reg( uint32_t num){
  if(num == 0 ){
    return 0x00;
  }else{
    return X[num];
  }
  
}
  // Execute a number of instructions
  void processor::execute(unsigned int num, bool breakpoint_check){
    
      for(int i=0;i<num;i++){
        if(this->PC % 4 != 0){
          cout << "Unaligned pc: "<< setw(8) << setfill('0') << hex<< PC << endl;
          return ;
        }

       // cout << hex <<"  pre  PC  " << PC  << "    pk  " << BreakPoint<<  endl;
        if(breakpoint_check &&( BreakPoint == this->PC)){
            cout << "Breakpoint reached at " << setw(8) << setfill('0') << hex << PC << endl;
            BreakPoint = 0xffffffff;
            break ;
        }
        //int jjj=0;
        //cout << "this->icache->    " << this->icache->read_word(this->PC) << endl;
        //cin >> jjj;

        this->icache->count_cycle();

        //bool save = this->dcache->verbose;
        //this->dcache->verbose = false;
        uint32_t ret = checkInstruction(this->dcache,this,this->X,this->PC,this->icache->read_word(this->PC),this->verbose);
       // this->dcache->verbose = save;
        
        //uint32_t ret = checkInstruction(this->dcache,this,this->X,this->PC,0x00000013,this->verbose);
       // cout << hex <<"   PC  " << PC <<  endl;
       //this->dcache->verbose = save;
        

        if(breakpoint_check &&( BreakPoint == this->PC)){
            cout << "Breakpoint reached at " << setw(8) << setfill('0') << hex << PC << endl;
            //breakpoints.erase(PC);
            BreakPoint = 0xffffffff;
            break ;
        }

        this->Instruction_count ++ ;
        this->PC += 4;


      }
  }

  // Clear breakpoint
  void processor::clear_breakpoint(){
    BreakPoint = 0xffffffff;
  }

  // Set breakpoint at an address
  void processor::set_breakpoint(uint32_t address){
    if(this->verbose){
      cout << "Breakpoint set at " << hex << setw(8) << setfill('0') << address << endl;
    }
    BreakPoint = address;
  }

  // Probe the instruction cache interface for an address
  void processor::probe_instruction (uint32_t address){

		this->icache->probe_address(address);

  }

  // Probe the data cache interface for an address
  void  processor::probe_data (uint32_t address){

		this->dcache->probe_address(address);


  }

  uint64_t  processor::get_instruction_count(){
    return this->Instruction_count;
  }

  uint64_t  processor::get_cycle_count(){
    return this->cycles;
  }

  void processor::count_cycle(int cycle){
    this->cycles += cycle;
  }
