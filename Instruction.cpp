
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "memory.h"
#include "processor.h"
#include "Instruction.h"

using namespace std;



uint32_t getOPCODE(uint32_t ir){
  return ir & 0x0000007f;
}
uint32_t getFUNC3(uint32_t ir){
  return ((ir & 0x00007000)>>12);
}
uint32_t getFUNC7(uint32_t ir){
    return (ir >> 25);
}
uint32_t getRS1(uint32_t ir){
    return (ir >> 15) & 0x1f;
}
uint32_t getRS2(uint32_t ir){
    return ((ir & 0x01f00000)>>20);
}
uint32_t getRD(uint32_t ir){
    return (ir >> 7) & 0x1f;
}
uint32_t getImm_I(uint32_t ir){
  uint32_t temp = (ir>>20);
  return temp;
}
uint32_t getImm_B(uint32_t ir){
  return (((ir >> 8) & 0xf) << 1) | (((ir >> 25) & 0x3f) << 5) | ((ir & 0x80) << 4) | ((ir & 0x80000000) >> 19);

}

uint32_t getImm_U(uint32_t ir){
  return ((ir & 0xfffff000) >> 12);
}

uint32_t getImm_UJ(uint32_t ir){
    return ((ir & 0x7fe00000) >> 20) | (ir & 0x000ff000) | ((ir & 0x00100000) >> 9) | ((ir & 0x80000000) >> 11);
}

uint32_t getImm_S(uint32_t ir){
    return ((ir >> 7) & 0x1f) | (ir >> 25 << 5);
}



int checkType(uint32_t ir){
  ir = getOPCODE(ir);
  if(ir == 0x03 || ir == 0x0f || ir == 0x13 || ir == 0x1b || ir == 0x67 || ir == 0x73){
    return 1;
  }else if(ir == 0x17 || ir == 0x37){
    return 2;
  }else if(ir == 0x23){
    return 3;
  }else if(ir == 0x33 || ir == 0x3b){
    return 4;
  }else if(ir == 0x63){
    return 5;
  }else if(ir == 0x6f){
    return 6;
  }
  return 0;
}

void EXE_ADDI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd= getRD(ir);
    int rs1 = getRS1(ir);
    uint32_t imm = getImm_I(ir);

    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }

    if(verbose){
        cout << dec <<"addi: rd = " <<rd<< ", rs1 = " << rs1 << ", immed_I = " << setw(8)<<setfill('0')<<hex<<imm<<endl;
    }

    if((imm & 0x00000800) == 0x00000800){
        imm = ~imm;
        imm +=1;
        cpu->set_reg(rd,cpu->get_reg(rs1) - imm);
    }else {
        cpu->set_reg(rd,cpu->get_reg(rs1) + imm);
    }
    cpu->count_cycle(1);
}

void EXE_BNE(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_B(ir);

    //uint32_t rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    if((imm & 0x00001000) == 0x00001000){
        imm = imm | 0xffffe000;
    }

    uint32_t ret = (int)imm + cpu->get_pc();

    if(cpu->get_reg(rs1) != cpu->get_reg(rs2))
	{
		cpu->set_pc(ret -4 );
		//cpu->add_cycle(1);
		cpu->count_cycle(1);
	}


    if(verbose){
        cout<< dec  <<"bne: rs1 = " << getRS1(ir)<< ", rs2 = " << getRS2(ir)  << ", immed_B = " << setw(8) << setfill('0') << hex << imm << endl;
    }

    cpu->count_cycle(1);
}

void EXE_JALR(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    int rs1 = getRS1(ir);

    uint32_t imm = getImm_I(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    uint32_t ret = imm + cpu->get_reg(rs1);

    int32_t tmp = cpu->get_pc();
	cpu->set_pc((ret & 0xfffffffe) -4);
	cpu->set_reg(rd,tmp+4);


/*
    //X[rd] = PC +4;
    cpu->set_reg(rd,cpu->get_pc() + 4);
    uint32_t imm = getImm_I(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = ~imm;
        imm +=1;
        if((imm % 2) != 0){
            imm += 1;
        }
        //PC = X[getRS1(ir)] - imm;
        cpu->set_pc(X[getRS1(ir)] - imm -4);
    }else {
        if((imm % 2) != 0){
            imm -= 1;
        }
        //PC = X[getRS1(ir)] + imm;
        //cout << "-----" << X[getRS1(ir)] << endl;
        cpu->set_pc((X[getRS1(ir)] + imm - 4));
    }*/


    if(verbose){
        cout<< dec  <<"jalr: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(8)<<setfill('0') << hex <<getImm_I(ir)<<endl;
    }




    cpu->count_cycle(2); 
}



void EXE_LUI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = (getImm_U(ir) << 12);
    int rd = getRD(ir);
    uint32_t ret =  imm ;
    //cout << hex << "LUI------ rd " << rd << " ret " << ret << " ir " << ir  << endl;
    if(verbose){
        cout<< dec  <<"lui: rd = " <<rd << ", immed_U = " << setw(8)<<setfill('0') << hex <<imm <<endl;
    }
    //X[rd] = imm;
    cpu->set_reg(rd,ret);

    cpu->count_cycle(1);

}

void EXE_AUIPC(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_U(ir) << 12;
    int rd = getRD(ir);
    if(verbose){
        cout<< dec  <<"auipc: rd = " <<rd << ", immed_U = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }
    uint32_t ret = imm + cpu->get_pc();
    //X[rd] = ret;
    cpu->set_reg(rd,ret); 
    cpu->count_cycle(1);
}

void EXE_JAL(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_UJ(ir);
    int rd = getRD(ir);
    if((imm & 0x00010000) == 0x00010000){
        imm = imm | 0xfffe0000;
    }
    if(verbose){
        cout << dec <<"jal: rd = " <<rd << ", immed_J = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }
    uint32_t ret = imm + cpu->get_pc();
    uint32_t temp = cpu->get_pc();
    //PC = ret - 4;
    cpu->set_pc(ret - 4);
    //X[rd] = temp + 4;
    cpu->set_reg(rd,temp + 4); 
    cpu->count_cycle(2);
}

void EXE_BEQ(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_B(ir);
    //int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = 0x00;
    if((imm & 0x00001000) == 0x00001000){
        imm = imm | 0xffffe000;
    }
    if(verbose){
        cout << dec  <<"beq: rs1 = " <<rs1<< ", rs2 = " << rs2 << ", immed_B = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }
    if((imm & 0x00001000) == 0x00001000){
        imm = imm | 0xffffe000;
    }
    ret = imm + cpu->get_pc();
    if(X[rs1] == X[rs2]){
        //PC = ret - 4;
        cpu->set_pc(ret - 4);
        cpu->count_cycle(1);
    }
    cpu->count_cycle(1);
}


void EXE_BLT(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_B(ir);
    //int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    if((imm & 0x00001000) == 0x00001000){
        imm = imm | 0xffffe000;
    }
    if(verbose){
        cout<< dec  <<"blt: rs1 = " <<rs1  <<", rs2 = " << rs2 << ", immed_B = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }



    uint32_t ret = imm + cpu->get_pc();
    if((int)cpu->get_reg(rs1) < (int)cpu->get_reg(rs2)){
        //PC = ret - 4;
        cpu->set_pc(ret - 4);
        cpu->count_cycle(1);
    }
    cpu->count_cycle(1);
}


void EXE_BGE(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_B(ir);
    //int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    if((imm & 0x00001000) == 0x00001000){
        imm = imm | 0xffffe000;
    }
    if(verbose){
        cout<< dec  <<"bge: rs1 = " <<rs1 <<", rs2 = " << rs2 << ", immed_B = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }  


    uint32_t ret = imm + cpu->get_pc();

    if((int)X[rs1] >= (int)X[rs2]){
        //PC = ret - 4;
        cpu->set_pc(ret - 4);
        cpu->count_cycle(1);
    }
    cpu->count_cycle(1);
}




void EXE_BLTU(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_B(ir);
    //int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    if((imm & 0x00001000) == 0x00001000){
        imm = imm | 0xffffe000;
    }
    if(verbose){
        cout<< dec  <<"bltu: rs1 = "  << rs1 <<", rs2 = " << rs2 << ", immed_B = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }  


    uint32_t ret = imm + cpu->get_pc();
    if(X[rs1] < X[rs2]){
        //PC = ret - 4;
        cpu->set_pc(ret - 4);
        cpu->count_cycle(1);
    }
    cpu->count_cycle(1);
}

void EXE_BGEU(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_B(ir);
    //int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    if((imm & 0x00001000) == 0x00001000){
        imm = imm | 0xffffe000;
    }
    if(verbose){
        cout << dec <<"bgeu: rs1 = " << rs1 <<", rs2 = " << rs2 << ", immed_B = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }  
    uint32_t ret = imm + cpu->get_pc();

    if(cpu->get_reg(rs1) >= cpu->get_reg(rs2)){
        //PC = ret - 4;
        cpu->set_pc(ret - 4);
        cpu->count_cycle(1);
    }
    cpu->count_cycle(1);
}

void EXE_LB(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"lb: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }  


    uint32_t p = imm + X[rs1];

    uint32_t ret = mem->read_word(p>>2<<2);
    uint32_t f = (p % 4)*8;

    ret = (ret&(0xff << f))>>f;

    if((ret & 0x00000080) == 0x00000080){
        ret = ret | 0xffffff00;
    }  
    //X[rd] = ret;
    cpu->set_reg(rd,ret);
    cpu->count_cycle(3);
    mem->count_cycle();

}

void EXE_LH(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"lh: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }  

    uint32_t p = imm + cpu->get_reg(rs1);
    uint32_t ret = 0x00;
    uint32_t f = p % 4;

    uint32_t d = mem->read_word((p>>2<<2));
	unsigned char* p1 = (unsigned char*)(&d);
	unsigned char* p2 = (unsigned char*)(&ret);

    p2[0] = p1[f];
    if(f==3){
        uint32_t d2 = mem->read_word((p>>2<<2)+4);
        unsigned char* p3 = (unsigned char*)(&d2);
        p2[1] = p3[0];
    }else{
        p2[1] = p1[f+1];
    }

    uint32_t tmp = ret&0xffff;

    if((tmp & 0x00008000) == 0x00008000){
        tmp = tmp | 0xffff0000;
    }
    X[rd] = tmp;
    cpu->count_cycle(p%4==3 ? 5 : 3);
    mem->count_cycle();
    if(p%4 == 3){
        mem->count_cycle();
    }

    

}


void EXE_LW(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"lw: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }  

    uint32_t p = imm + X[rs1];

    uint32_t ret = 0x00;

    if(p % 4 == 0){
        ret = mem->read_word(p);
    }else{
        uint32_t add = p>>2<<2;
		uint32_t t1 = mem->read_word(add);
		uint32_t t2 = mem->read_word(add+4);
		int f = p % 4;
		unsigned char* p1 =(unsigned char*)(&ret);
		unsigned char* p2 = (unsigned char*)(&t1);
		unsigned char* p3 = (unsigned char*)(&t2);
		int cnt = 0;
		for(int i=f;i<4;++i)
		{
			p1[cnt++]=p2[i];
		}
		for(int i=0;cnt<4;++i)
		{
			p1[cnt++]=p3[i];
		}
    }

    X[rd] = ret;
    cpu->count_cycle(p%4 ? 5 : 3);
    mem->count_cycle();
    if(p%4){
        mem->count_cycle();
    }

}

void EXE_LBU(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"lbu: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }  


    uint32_t p = imm + X[rs1];

    uint32_t ret = mem->read_word(p>>2<<2);
    uint32_t f = (p % 4)*8;

    ret = (ret&(0xff << f))>>f;
    X[rd] = ret;
    cpu->count_cycle(3);
    mem->count_cycle();
}


void EXE_LHU(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout << dec <<"lhu: rd = " <<rd << ", rs1 = " << rs1 << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }  

    uint32_t p = imm + X[rs1];

    uint32_t ret = 0x00;
    uint32_t f = p % 4;

    uint32_t d = mem->read_word((p>>2<<2));
	unsigned char* p1 = (unsigned char*)(&d);
	unsigned char* p2 = (unsigned char*)(&ret);

    p2[0] = p1[f];
    if(f==3){
        uint32_t d2 = mem->read_word((p>>2<<2)+4);
        unsigned char* p3 = (unsigned char*)(&d2);
        p2[1] = p3[0];
    }else{
        p2[1] = p1[f+1];
    }

    uint32_t tmp = ret&0xffff;
    X[rd] = tmp;
    cpu->count_cycle(p % 4==3 ? 5 : 3);
    mem->count_cycle();
	if(p % 4==3)
		mem->count_cycle();



}

void EXE_SB(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_S(ir);
    //int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"sb: rs1 = "<< rs1 << ", rs2 = "<< rs2 << ", immed_S = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    } 


    uint32_t ret = imm;
    ret = imm + X[rs1];
    uint32_t mask = 0xff;
    uint32_t f = (8*(ret%4));
    mem->write_word(ret>>2<<2,X[rs2]<<f,mask<<f);

    mem->count_cycle();



    cpu->count_cycle(2);
}


void EXE_SH(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_S(ir);
    //int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"sh: rs1 = "<< rs1 << ", rs2 = "<< rs2 << ", immed_S = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    } 
    uint32_t ret = imm;
    ret = imm + X[rs1];
    uint32_t mask = 0xffff;


    mem->count_cycle();

    if(ret%4 == 3){
        mem->write_word(ret-3,(X[rs2]&0x000000ff)<<24,0xff000000);
 		mem->write_word(ret+1,(X[rs2]&0xffffff00)>>8,0x000000ff);

            mem->count_cycle();

    }else{
 		mem->write_word(ret>>2<<2,X[rs2]<<(8*(ret%4)),mask<<(8*(ret%4)));
 	}

     cpu->count_cycle(ret % 4==3 ? 4 : 2);
}

void EXE_SW(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_S(ir);
    //int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout << dec <<"sw: rs1 = "<< rs1 << ", rs2 = "<< rs2 << ", immed_S = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    } 

    uint32_t ret = imm;
    ret = imm + X[rs1];
    uint32_t mask = 0xffffffff;
    uint32_t f = ret % 4;
 	mem->write_word(ret>>2<<2,(X[rs2]&(0xffffffff>>(f*8)))<<(f*8),mask<<(f*8));

    mem->count_cycle();

 	if(f)
 	{
 		mem->write_word((ret>>2<<2)+4,(X[rs2]&(0xffffffff<<(32-f*8)))>>(32-f*8),mask>>(32-f*8));
        mem->count_cycle();
 	}
     cpu->count_cycle(ret % 4 ? 4 : 2);
}

void EXE_SLTI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);

    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"slti: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    } 

    uint32_t ret = (int)X[rs1] < (int)imm ? 1 : 0;
    X[rd] = ret;
    cpu->count_cycle(1);
}

void EXE_SLTIU(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);

    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"sltiu: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    } 

    uint32_t ret = (uint32_t)X[rs1] < (uint32_t)imm ? 1 : 0;
    X[rd] = ret;
    cpu->count_cycle(1);
}

void EXE_XORI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);

    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout << dec <<"xori: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    } 

    uint32_t ret = X[rs1] ^ imm;

    X[rd] = ret;
    cpu->count_cycle(1);

}


void EXE_ORI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"ori: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_I = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }    

    uint32_t ret = X[rs1] | imm;

    X[rd] = ret;
    cpu->count_cycle(1);
    

}

void EXE_ANDI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout << dec <<"andi: rd = " <<rd << ", rs1 = "<< rs1  << ", immed_slti = " << setw(8)<<setfill('0') << hex <<imm<<endl;
    }    

    uint32_t ret = X[rs1] & imm;
    X[rd] = ret;
    cpu->count_cycle(1);
}


void EXE_SLLI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"slli: rd = " <<rd << ", rs1 = "<< rs1  << ", shamt = " << dec <<imm<<endl;
    }    
    uint32_t ret = X[rs1] << getImm_I(ir);
    X[rd] = ret;
    cpu->count_cycle(1);
}

void EXE_SRLI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
    if(verbose){
        cout<< dec  <<"srli: rd = " <<rd << ", rs1 = "<< rs1  << ", shamt = " << dec<<imm<<endl;
    }    
    uint32_t ret = X[rs1] >> getImm_I(ir);
    X[rd] = ret;
    cpu->count_cycle(1);
}

void EXE_SRAI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    uint32_t imm = getImm_I(ir);
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    if((imm & 0x00000800) == 0x00000800){
        imm = imm | 0xfffff000;
    }
  

    uint32_t ret = X[rs1];
    uint32_t bs = imm %32;
    int s = ret & 0x80000000;

    for(int i=0;i<bs;i++){
        ret >>= 1;
        if(s){
            ret |= 0x80000000;
        }
    }

    X[rd] = ret;

    if(verbose){
        cout << dec <<"srai: rd = " <<rd << ", rs1 = "<< rs1  << ", shamt = " << dec <<(imm&0x000000ff)<<endl;
    }  
    cpu->count_cycle(1);

}


void EXE_ADD(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1] + X[rs2];
    X[rd] = ret;
    //cout << hex << "add------ rd " << rd << "  rs1  "<< rs1 << "  Xrs1 " << X[rs1] << "  rs2  "<< rs2 << " X{rs2}  "<<X[rs2]<< " ret " << ret << "  ir  " << ir << endl;  
    if(verbose){
        cout << dec <<"add: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    } 
    cpu->count_cycle(1);

}

void EXE_SUB(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1] - X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"sub: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    } 
    cpu->count_cycle(1);
}
void EXE_SLL(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1] << X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"sll: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }  
    cpu->count_cycle(1);
}

void EXE_SLT(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = (int)X[rs1] < (int)X[rs2] ? 1 : 0;
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"slt: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    } 
    cpu->count_cycle(1);
}

void EXE_SLTU(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1] < X[rs2] ? 1 : 0;
    X[rd] = ret;
    if(verbose){
        cout << dec <<"sltu: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    } 
    cpu->count_cycle(1);
}

void EXE_XOR(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1] ^ X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"xor: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    } 
    cpu->count_cycle(1);
}


void EXE_SRL(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1] >> X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"srl: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }    
    cpu->count_cycle(1);
}

void EXE_SRA(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1];

    uint32_t bs = X[rs2] % 32;
    int s = ret & 0x80000000;

    for(int i=0;i<bs;i++){
        ret >>= 1;
        if(s) ret |= 0x80000000;
    }

    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"sra: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    } 
    cpu->count_cycle(1);
}


void EXE_OR(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1] | X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"or: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    } 
    cpu->count_cycle(1);
}
void EXE_AND(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    int rd = getRD(ir);
    uint32_t rs1 = getRS1(ir);
    uint32_t rs2 = getRS2(ir);
    uint32_t ret = X[rs1] & X[rs2];
    X[rd] = ret;
    if(verbose){
        cout<< dec  <<"and: rd = " <<rd << ", rs1 = "<< rs1 << ", rs2 = " << rs2  <<endl;
    }
    cpu->count_cycle(1);
}

void EXE_FENCE(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    if(verbose){
        cout <<" fence: no operation "<<endl;
    }
    cpu->count_cycle(1);
}

void EXE_FENCE_DOT_I(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    if(verbose){
        cout<< dec  <<" fence.i: no operation "<<endl;
    }
    cpu->count_cycle(1);
}

void EXE_ECALL(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
  
    cout<< dec  <<"ecall: unimplemented instruction"<<endl;

   // cpu->count_cycle(1);
}
void EXE_EBREAK(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){

        cout <<"ebreak: unimplemented instruction"<<endl;
    
    //cpu->count_cycle(1);
}
void EXE_CSRRW(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){

        cout <<"csrrw: unimplemented instruction"<<endl;
    
   // cpu->count_cycle(1);
}
void EXE_CSRRS(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){

        cout <<"csrrs: unimplemented instruction"<<endl;
    
  //  cpu->count_cycle(1);
}
void EXE_CSRRC(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){

        cout <<"csrrc: unimplemented instruction"<<endl;
    
  //  cpu->count_cycle(1);
}
void EXE_CSRRWI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){

        cout <<"csrrwi: unimplemented instruction"<<endl;
    
  //  cpu->count_cycle(1);
}
void EXE_CSRRSI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){

        cout <<"csrrsi: unimplemented instruction"<<endl;
    
  //  cpu->count_cycle(1);
}
void EXE_CSRRCI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){

        cout <<"csrrci: unimplemented instruction"<<endl;
    
  //  cpu->count_cycle(1);
}


void EXE_PI(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    
}

void EXE_PD(memory_accessible* mem,processor* cpu,uint32_t ir, uint32_t* X,uint32_t& PC,bool verbose){
    
}


uint32_t checkInstruction(memory_accessible* icache,processor* cpu,uint32_t* X,uint32_t& PC,uint32_t ir,bool verbose){
    if(verbose){
       // cout << "Memory read word: address = " << hex << setw(8) << setfill('0') << PC  << ", data = " << setw(8) << setfill('0') << hex << icache->read_word(PC) << endl; 
        cout << "Fetch: pc = " <<  setw(8) << setfill('0') << hex << PC << ", ir = " <<  setw(8) << setfill('0') << hex << ir << endl; 

    }
   // cout << "ir   ----  " << endl;

    if(1){
        if(getOPCODE(ir) == 0x00000013 && getFUNC3(ir) == 0x00000000){
            EXE_ADDI(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x00000063 && getFUNC3(ir) == 0x00000001){
            EXE_BNE(icache,cpu,ir,X,PC,verbose);     
        }else if(getOPCODE(ir) == 0x00000067 && getFUNC3(ir) == 0x00000000){
            EXE_JALR(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x00000037 ){
            EXE_LUI(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x00000017){
            EXE_AUIPC(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x6f){
            EXE_JAL(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x00000000){
            EXE_BEQ(icache,cpu,ir,X,PC,verbose);  
        }else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x04){
            EXE_BLT(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x05){
            EXE_BGE(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x06){
            EXE_BLTU(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x63 && getFUNC3(ir) == 0x07){
            EXE_BGEU(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x00){
            EXE_LB(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x01){
            EXE_LH(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x02){
            EXE_LW(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x04){
            EXE_LBU(icache,cpu,ir,X,PC,verbose);  
        }else if(getOPCODE(ir) == 0x03 && getFUNC3(ir) == 0x05){
            EXE_LHU(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x23 && getFUNC3(ir) == 0x00){
            EXE_SB(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x23 && getFUNC3(ir) == 0x01){
            EXE_SH(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x23 && getFUNC3(ir) == 0x02){
            EXE_SW(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x02){
            EXE_SLTI(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x03){
            EXE_SLTIU(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x04){
            EXE_XORI(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x06){
            EXE_ORI(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x07){
            EXE_ANDI(icache,cpu,ir,X,PC,verbose);  
        }else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x01 && getFUNC7(ir) == 0x00){
            EXE_SLLI(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x00){
            EXE_SRLI(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x13 && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x20){
            EXE_SRAI(icache,cpu,ir,X,PC,verbose);  
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x00){
            EXE_ADD(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x20){
            EXE_SUB(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x01 && getFUNC7(ir) == 0x00){
            EXE_SLL(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x02 && getFUNC7(ir) == 0x00){
            EXE_SLT(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x03 && getFUNC7(ir) == 0x00){
            EXE_SLTU(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x04 && getFUNC7(ir) == 0x00){
            EXE_XOR(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x00){
            EXE_SRL(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x05 && getFUNC7(ir) == 0x20){
            EXE_SRA(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x06 && getFUNC7(ir) == 0x00){
            EXE_OR(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x33 && getFUNC3(ir) == 0x07 && getFUNC7(ir) == 0x00){
            EXE_AND(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x0f && getFUNC3(ir) == 0x00){
            EXE_FENCE(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x0f && getFUNC3(ir) == 0x01){
            EXE_FENCE_DOT_I(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x00){
            EXE_ECALL(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x00 && getFUNC7(ir) == 0x01){
            EXE_EBREAK(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x01){
            EXE_CSRRW(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x02){
            EXE_CSRRS(icache,cpu,ir,X,PC,verbose);  
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x03){
            EXE_CSRRC(icache,cpu,ir,X,PC,verbose); 
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x05){
            EXE_CSRRWI(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x06){
            EXE_CSRRSI(icache,cpu,ir,X,PC,verbose);
        }else if(getOPCODE(ir) == 0x73 && getFUNC3(ir) == 0x07){
            EXE_CSRRCI(icache,cpu,ir,X,PC,verbose);
        }else{
            cout << "Illegal instruction: " << setw(8) << setfill('0') << ir  <<  endl;
           // PC += 4;
        }
    }else{
        cout << "error instruction" << endl;
        
    }
}