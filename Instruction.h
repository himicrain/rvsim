

#include "memory.h"
#include "processor.h"

uint32_t getImm_B(uint32_t ir);
uint32_t getImm_U(uint32_t ir);
uint32_t checkInstruction(memory_accessible* icache, processor* cpu, uint32_t* X,uint32_t& PC,uint32_t ir,bool verbose);
int checkType(uint32_t ir);
