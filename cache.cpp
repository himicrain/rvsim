/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2017

   Class members for cache

**************************************************************** */

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>
#include <map>

#include "cache.h"

using namespace std;

NODE::NODE(int data){
    this->data = data;
}

STACK::STACK(int size){
    this->size = size;
    head = new NODE(-1);
    head->next = NULL;

}
STACK::~STACK(){
    NODE* p = NULL;
    p = head;

    while(p){
        p = p->next;
        delete head;
        head = p;
        cout << "release" << endl;
    }

}

void STACK::print(){
    NODE* p = NULL;
    p = head->next;
    //cout << getLen() << " length " << endl;
    while(p){
        cout << p->data << endl;
        p = p->next;
    }

}
int STACK::isExist(int data){
    NODE* p = NULL;
    p = this->head;
    int pos = -1;
    while(p->next){
        pos ++ ;
        if(p->next->data == data){
            return pos;
        }
        p = p->next;
    }
    return -1;
}
void STACK::push(int data){
        NODE* node = new NODE(data);
        node->next = head->next;
        head->next = node;
}

int STACK::pop(int data,int pos){
    NODE* p = NULL;

    p = head;
    int num = 0;

    while(num < pos){
        p = p->next;
        num ++ ;
    }

    NODE* pre = p;
    NODE* mid = p->next;
    NODE* aft = p->next->next;

    pre->next = aft;

    uint32_t temp_data = mid->data;

    //delete mid;

    return temp_data;

}
int STACK::getLen(){
    int len = 0;
    NODE* p = NULL;
    p = head->next;

    while(p){
        len ++ ;
        p = p->next;
    }
    return len;

}
int STACK::upDate(int data){
    int check = this->isExist(data);
    int len = getLen();
    //cout << "lenght " << len << "  check " << check << endl;

    if(len < size){
        if(check == -1){
            //cout << "not exist " << endl;
            push(data);
            //cout << " index  " << 0-len << endl;
            return (int)(0-len);
        }else{
            int pos = pop(data,check);
            push(data);
            return data;
        }
    }else{
        if(check == -1){
            int pos = pop(data,len-1);
            push(data);
            return pos;
        }else{
            //cout << " check " << check << endl;
            //pop(data,len-1);
            int pos = pop(data,check);
            push(data);
            //pos = pop(data,len-1);
            return pos;
        }
    }

}



void cache::count_miss(){
    this->miss_num ++ ;
}

void cache::count_dirty(){
    this->dirty_num ++ ;
}
// Constructors

cache::cache(string name,
	     memory_accessible* backing_store,
	     unsigned long int block_size,
	     unsigned long int size,
	     unsigned long int associativity,
	     write_strategy_t write_strategy,
	     bool verbose,
	     bool cycle_reporting,
	     unsigned long int address_cycles,
	     unsigned long int data_cycles) {
  // TODO: ...

        this->size = size;

		this->backing_store = backing_store;
		this->name = name;
		this->block_size = block_size;
		this->associativity = associativity;
		this->write_strategy =  write_strategy;
		this->verbose = verbose;
		this->cycle_reporting = cycle_reporting;
		this->address_cycles = address_cycles;
		this->data_cycles = data_cycles;

		this->blocks_num = size/block_size;
		this->sets_num = blocks_num/associativity;

        this->word_num = this->block_size/4;

        this->data_cycles = data_cycles;
        this->address_cycles = address_cycles;
        this->cycles = 0;
        this->both_cycles = 0;
        this->miss_num = 0;

        //初始化各mask
        long offest_bit = (int)(log(this->block_size)/log(2));

        this->offest_mask = 0xffffffff  >>  (32- offest_bit);

        long index_bit = (int)(log((this->size/this->block_size)/this->associativity)/log(2));
        this->index_mask = ((0xffffffff >> offest_bit) << (32 - index_bit) ) >> (32 - index_bit - offest_bit);
        long tag_bit = 32 - offest_bit - index_bit;
        this->tag_mask = (0xffffffff >> (32 - tag_bit )) << (32 - tag_bit);



        this->Blocks = new uint32_t**[sets_num];
        this->DIRTY = new int*[sets_num];
        this->TAGS = new uint32_t* [sets_num];
        this->INVALID = new int*[sets_num];
        for(int i=0;i<sets_num;i++){
            this->Blocks[i] = new uint32_t*[this->associativity];
            this->DIRTY[i] = new int[this->associativity];
            this->TAGS[i] = new uint32_t[this->associativity];
            this->INVALID[i] = new int[this->associativity];
        }

        for(int i=0;i<sets_num;i++){
            for(int j=0;j<this->associativity;j++){
                this->Blocks[i][j] = new uint32_t[this->word_num];
                this->DIRTY[i][j] = 0;
                this->TAGS[i][j] = -1;
                this->INVALID[i][j] = 1; //1 for invalid, 0 for valid
            }
        }

        //cout << "init ---- " << name << " size is " << size << " sets num " << sets_num  << "  blocks_num " << blocks_num << " block_size " << block_size << endl;
        //为每一个set创建一个LRU堆栈，用于存储当前每个set的可用block
		for(int i=0;i<sets_num;i++){
			this->LRUS.insert(pair<int,STACK*>(i,new STACK(this->associativity)));
		}

        string ws = "";
        if(this->write_strategy == 0){
            ws = "read only";
        }else if(this->write_strategy == 1){
            ws = "write through";
        }else if(this->write_strategy == 2){
            ws = "write back";
        }

        if(this->verbose){
            cout << hex <<  setw(8) <<setfill('0')  << this->name << ": tag_mask = "  << this->tag_mask << ", set index mask = " << hex <<  setw(8) <<setfill('0')  << this->index_mask  << ", offset mask = " << hex <<  setw(8) <<setfill('0')  << this->offest_mask << ", " << ws << endl;
        }
}



//就是第几个block
int cache::translate(uint32_t address){
	//通过地址运算得出地址所在block的id
	int num = address/block_size;
	return num;
}
//取block_id
int cache::getBlockId(uint32_t address){
    //通过地址运算得出地址所在block的id, 取整，比如 address = 15 ，block_size = 4, 则 block_id = 15 / 4 = 3，则在角标为3 的存储结构中
	int block_id = (int)(address / this->block_size);
	return block_id;
}
//获取在block中的偏移量
int cache::getOffestInBlock(uint32_t address){
    //取余，获取address在block中的偏移量，比如 address = 15 ，block_size = 4, 则 offest = 15 % 4 = 3
    int offest = (address % this->block_size)/4;
    return offest;
}
//获取属于哪个set
int cache::getOffestInCache(uint32_t address){
    //获取当前block在Cache中的偏移量,即是，获取当前block在哪一个set
    int block_id = (int)(address / this->block_size);
    int set_id = block_id % this->sets_num;
    return set_id;  
}
int cache::getTag(uint32_t address){
    int block_id = (int)(address / this->block_size);
    //获取当前block的tag，比如 block_id = 17, sets_num = 4, set_id = 17%4 = 1, tag = 17 / 4 = 4;
    int tag = (int)(block_id/this->sets_num);
    return tag;
}


int cache::getIndexInSet(int tag, int offest_set){

    for(int i=0;i<this->associativity;i++){
        if(this->TAGS[offest_set][i] == tag && this->INVALID[offest_set][i] == 0){
            return i;
        }
    }
    return 0;

}


bool cache::isHit(uint32_t address){
    //获取block_id
    //cout << "--check hit-------"  << endl;
	//int block_id = this->getBlockId(address);
	//取到第几个set里
	//int set_num = block_id % this->sets_num;    
    int offest_set = this->getOffestInCache(address);
    int tag = this->getTag(address);
    //获取该set

    for(int i=0;i<this->associativity;i++){
        if(this->TAGS[offest_set][i] == tag){
            return true;
        }
    }
    return false;

}

bool cache::isAlign(uint32_t address){

    int b = (address % this->block_size)%4;
    if( b != 0){
        return false;
    }else{
        return true;
    }
}

// Read a word of data from an address
uint32_t cache::read_word (uint32_t address) {
  // TODO: ...

    bool hit = this->isHit(address);
    int offest = this->getOffestInBlock(address);
    int offest_set = this->getOffestInCache(address);
    STACK* stack = this->LRUS[offest_set];
    uint32_t** set = this->Blocks[offest_set];
    int tag = this->getTag(address);

    int index = 0;
    int len = stack->getLen();
    int temp_index = stack->upDate(tag);
    

    if(temp_index >=0 ){
            //fuck error
        index = this->getIndexInSet(temp_index,offest_set);
    }else{
        index = 0-temp_index;

    }

    if(this->verbose){

        if(this->write_strategy != cache::write_strategy_t::WRITE_BACK){

            int offest_set = this->getOffestInCache(address);
            cout << this->name << ": read word address = " << hex << setw(8) << setfill('0') << address << endl;
        
            if(hit){
                cout << this->name << ": lookup address = " << hex << setw(8) << setfill('0')<< address << ": tag = " << tag << ", set = " << offest_set << ", offset = " << offest*4 << ", hit, entry = "  <<  index << endl;
            }else{
                cout << this->name << ": lookup address = " << hex << setw(8) << setfill('0')<< address << ": tag = " << tag << ", set = " << offest_set << ", offset = " << offest*4 << ", miss" << endl;
            }

        }

    }

    if(this->write_strategy == cache::write_strategy_t::WRITE_THROUGH){

        if(hit){
            return set[index][offest];
        }else{
       
            int data_size = this->block_size/4;

            uint32_t* TagsInSet = this->TAGS[offest_set];
            int* InvalidInSet = this->INVALID[offest_set];
            int* DirtyInSet = this->DIRTY[offest_set];

            uint32_t* temp_data = new uint32_t[data_size] ;

            if(this->verbose){
                cout << this->name << ": Replacing entry " << index << endl;
            }

            this->count_miss();

            this->backing_store->read_block(address-offest*4,temp_data,data_size);

            TagsInSet[index] = tag;
            InvalidInSet[index] = 0;


            for(int i=0;i<(data_size);i++){
                set[index][i] = temp_data[i];
            }

            return set[index][offest];

        }

        //cout << "write-through" <<endl;
    }else if(this->write_strategy == cache::write_strategy_t::WRITE_BACK){
        //cout << "write-back" << endl;
            int data_size = this->block_size/4;

            this->count_cycle();
            if(hit){
                return set[index][offest];
            }else{    

            uint32_t* TagsInSet = this->TAGS[offest_set];
            int* InvalidInSet = this->INVALID[offest_set];
            int* DirtyInSet = this->DIRTY[offest_set];

            int offest_index = (int)(offest/data_size) * data_size;

            uint32_t* temp_data = new uint32_t[data_size] ;

            int entry_tag = TagsInSet[index];        
            int dirty = DirtyInSet[index];
            int entry_invalid = InvalidInSet[index];      
            if(this->verbose){
                    cout << this->name << ": Replacing entry " << index ;
                    if(dirty){
                        cout << ", dirty address = " << hex << setw(8) << setfill('0') << ((entry_tag* this->sets_num)+offest_set)*this->block_size; 
                    }
                    cout << endl;
            }

            this->count_miss();

            //this->count_cycle();

            if(dirty && entry_invalid == 0){
                this->count_dirty();
                uint32_t former_address = ((entry_tag * this->sets_num)+offest_set)*this->block_size;
                
                this->backing_store->read_block(address,temp_data,this->block_size/4);

                this->backing_store->write_block(former_address,set[index],this->block_size/4);
            }else{
                this->backing_store->read_block(address,temp_data,this->block_size/4);
            }

            for(int i=0;i < this->block_size/4;i++){
                set[index][i] = temp_data[i];
            }

            DirtyInSet[index] = 0;
            InvalidInSet[index] = 0;
            TagsInSet[index] = tag;

            return set[index][offest];
        }

    }else if(this->write_strategy == cache::write_strategy_t::READ_ONLY){

        if(hit){
                return set[index][offest];
        }else{
            int data_size = this->block_size/4;

            uint32_t* TagsInSet = this->TAGS[offest_set];
            int* InvalidInSet = this->INVALID[offest_set];
            int* DirtyInSet = this->DIRTY[offest_set];

            uint32_t* temp_data = new uint32_t[data_size] ;

            if(this->verbose){
                    cout << this->name << ": Replacing entry " << index   << endl;
            }
            this->count_miss();
            this->backing_store->read_block(address-offest*4,temp_data,data_size);

           InvalidInSet[index] = 0;
           TagsInSet[index] = tag;

            for(int i=0;i<(data_size);i++){
                set[index][i] = temp_data[i];
            }
            return set[index][offest];

        }
    }
}

// Write a word of data to an address, mask contains 1s for bytes to be updated
void cache::write_word (uint32_t address, uint32_t data, uint32_t mask) {
    int offest = this->getOffestInBlock(address);
    int offest_set = this->getOffestInCache(address);
    bool hit = this->isHit(address);
    int tag = this->getTag(address);
    STACK* stack = this->LRUS[offest_set];

    uint32_t** set = this->Blocks[offest_set];
    uint32_t* TagsInSet = this->TAGS[offest_set];
    int* InvalidInSet = this->INVALID[offest_set];
    int* DirtyInSet = this->DIRTY[offest_set];

    int data_size = this->block_size/4;

    int index = 0;
    int temp_index = stack->upDate(tag);
    if(temp_index >=0 ){
        //fuck error
        index = this->getIndexInSet(temp_index,offest_set);
    }else{
        index = 0-temp_index;

    }

    if(this->verbose ){
            if( this->write_strategy != cache::write_strategy_t::WRITE_BACK ){
                cout << hex << setw(8) << setfill('0')  << this->name << ": write word address = " << hex << setw(8) << setfill('0') << address << ", data = " << hex << setw(8) << setfill('0') << data << ", mask = " << hex << setw(8) << setfill('0') << mask << endl;
            }
            
            if(hit){
                cout << hex << setw(8) << setfill('0')  << this->name << ": lookup address = " << hex << setw(8) << setfill('0')<< address << ": tag = " << hex <<  tag << ", set = " << offest_set << ", offset = " << offest*4 << ", hit, entry = "  <<  index << endl;
            }else{
                cout << hex << setw(8) << setfill('0')  << this->name << ": lookup address = " << hex << setw(8) << setfill('0')<< address << ": tag = " << hex << tag << ", set = " << offest_set << ", offset = " <<  offest*4 << ", miss" << endl;
            }
    }

    if(this->write_strategy == cache::write_strategy_t::WRITE_THROUGH){

        if(verbose){
            if(this->backing_store->name != "Memory")
                cout << this->backing_store->name << ": write word address = " << hex << setw(8) << setfill('0') << address << ", data = " << hex << setw(8) << setfill('0') << data << ", mask = " << hex << setw(8) << setfill('0') << mask  << endl;
            else
                cout << this->backing_store->name << " write word: address = " << hex << setw(8) << setfill('0') << address << ", data = " << hex << setw(8) << setfill('0') << data << ", mask = " << hex << setw(8) << setfill('0') << mask  << endl;
        }
       
        if(hit){
            
            uint32_t temp_data = (data & mask);
            uint32_t temp_old_data = set[index][offest] & (~mask);
            uint32_t new_word = temp_data | temp_old_data;

            set[index][offest] = new_word;
            this->backing_store->write_word(address,data,mask);
        }else{
            this->count_miss();
            this->backing_store->write_word(address,data,mask);

        }


  }else if(this->write_strategy == cache::write_strategy_t::WRITE_BACK){
       uint32_t* temp_datas = new uint32_t[this->block_size/4] ;

       int entry_tag = TagsInSet[index];
       uint32_t temp_data = (data & mask);
       uint32_t temp_old_data = set[index][offest] & (~mask);
       uint32_t new_word = temp_data | temp_old_data;
       
       this->count_cycle();

      if(hit){
            set[index][offest] = new_word;
      }else{

            this->count_miss();
            int dirty = DirtyInSet[index];
            if(this->verbose){
                    cout << this->name << ": Replacing entry " << index ;
                    if(dirty){
                        cout << ", dirty address = " << hex << setw(8) << setfill('0') <<  ((entry_tag * this->sets_num)+offest_set)*this->block_size; ;
                    }
                    cout << endl;
            }

            if(dirty ){

                this->count_dirty();
                uint32_t former_address = ((entry_tag * this->sets_num)+offest_set)*this->block_size;

                this->backing_store->read_block(address>>2<<2,temp_datas,data_size);

                this->backing_store->write_block(former_address,set[index],data_size);

            }else{
                this->backing_store->read_block(address>>2<<2,temp_datas,data_size);
            }


            int align = address % 4;

            for(int i=0;i<(data_size-align);i++){
                set[index][i] = temp_datas[i];
            }

            if(align){
                this->backing_store->read_block(address>>2<<2 + 4,temp_datas,data_size);
                for(int i=0;i<align;i++){
                    set[index][data_size-align+i] = temp_datas[i];
                }
            }

            
            set[index][offest] = new_word;
            InvalidInSet[index] = 0;
            TagsInSet[index] = tag;
      }

       DirtyInSet[index] = 1;

  }else if(this->write_strategy == cache::write_strategy_t::READ_ONLY){
      
  }

  // TODO: ...
}

// Read a block of data of a given size, starting at an address
// Data size is a power of 2 number of words, and address is block aligned.
void cache::read_block (uint32_t address, uint32_t data[], unsigned int data_size) {
  // TODO: ...
    if(verbose){
        bool hit = this->isHit(address);
        int offest_set = this->getOffestInCache(address);
        //int offest_block = this->getOffestInSet(address);
        int tag = this->getTag(address);
        STACK* stack = this->LRUS[offest_set];
        int offest = this->getOffestInBlock(address);

        int index = 0;
        int len = stack->getLen();
        int temp_index = stack->upDate(tag);
        
        if(temp_index >=0 ){
                //fuck error
            index = this->getIndexInSet(temp_index,offest_set);
        }else{
            index = 0-temp_index;

        }

        cout << this->name << ": read block address = " << hex << setw(8) << setfill('0') << address << ", data size = " << dec << data_size << " words" << endl;
        
        if(hit){
                cout << this->name << ": lookup address = " << hex << setw(8) << setfill('0')<< address << ": tag = " << tag << ", set = " << offest_set << ", offset = " << offest*4 << ", hit, entry = "  <<  index << endl;
            }else{
                cout << this->name << ": lookup address = " << hex << setw(8) << setfill('0')<< address << ": tag = " << tag << ", set = " << offest_set << ", offset = " << offest*4 << ", miss" << endl;
            }
    }

    for(int i=0;i<data_size;i++){
        data[i] = this->read_word(address+i*4);
    }

    this->cycles = this->cycles - (data_size-1);
    int data_ = this->block_size/4;
  
    if(this->cycle_reporting){
       //  this->both_cycles = this->both_cycles - (this->address_cycles + this->data_cycles*data_)*(data_size-1);
        this->calculate_cycle(data_size);
    }

}

// Write a block of data of a given size, starting at an address
// Data size is a power of 2 number of words, and address is block aligned.
void cache::write_block (uint32_t address, uint32_t data[], int unsigned data_size) {
  // TODO: ...  

}

// Display on cout whether the address is present, and if so, display the data
void cache::probe_address (uint32_t address) {
    bool flag = isHit(address);
   
    //哪一个set
    int offest_set = this->getOffestInCache(address);

    uint32_t** set = this->Blocks[offest_set];

    int* DirtyInSet = this->DIRTY[offest_set];

    int tag = this->getTag(address);

    uint32_t* temp_data = new uint32_t[this->block_size/4];

    int offest_ = this->getOffestInBlock(address);

    if(this->verbose){

        if(this->write_strategy == cache::write_strategy_t::READ_ONLY){
            cout << "CPU: probe instruction address = " << hex << setw(8) << setfill('0') << address << endl;
        }else if(this->write_strategy == cache::write_strategy_t::WRITE_THROUGH){
            cout << "CPU: probe data address = " << hex << setw(8) << setfill('0') << address << endl;
        }if(this->write_strategy == cache::write_strategy_t::WRITE_BACK){
            //cout << "CPU: probe instruction --------------------------- address = " << hex << setw(8) << setfill('0') << address << endl;
        }

        cout << this->name << ": probe address = " << hex << setw(8) << setfill('0') << address << endl;
    
    }


    if(flag){
            int index  = (int)this->getIndexInSet(tag,offest_set);

            string dirty_str = DirtyInSet[index] == 0 ? "" : ", dirty";

            if(this->verbose) {
                 cout << this->name << ": lookup address = "<< hex << setw(8) << setfill('0') << address << ": tag = "<< tag << ", set = " << offest_set << ", offset = " << offest_*4 << ", hit, entry = " << index << endl;
            }

            cout << this->name << ": hit, "  << "set = " << hex << offest_set << ", entry = " << dec <<  index << dirty_str << ":" << endl;
            
            for(int i=0;i<this->block_size/4;i++){
                temp_data[i] = set[index][i];
            }

            cout << "    " ;
            for(int i=0;i<(this->block_size/4)-1;i++){
                int f = i % 8;
                if(f == 7 && i != 0){
                    cout << hex << setw(8) << setfill('0') << temp_data[i] ;
                    cout << endl;
                    cout << "    ";
                    continue;
                }
                cout << hex << setw(8) << setfill('0') << temp_data[i] << " " ;
            }
            cout << hex << setw(8) << setfill('0') << temp_data[(this->block_size/4)-1] ;
            cout << endl;

        }else{
            if(this->verbose) {
                 cout << this->name << ": lookup address = "<< hex << setw(8) << setfill('0') << address << ": tag = "<< tag << ", set = " << offest_set << ", offset = " << offest_*4 << ", miss" << endl;
            }

            cout << this->name << ": miss, " << "set = "  << hex  << offest_set << endl;

        }

    this->backing_store->probe_address(address);


}

// Return the accumulated number of cycles for read accesses
unsigned long int cache::get_read_cycle_count () {
  // TODO: ...

  return this->both_cycles;
}

// Report access stats
void cache::report_accesses () {
  // TODO: ...

  cout << this->name << " access count: " << this->cycles << endl;
  if(this->cycles != 0){
        cout << this->name << " miss rate: " << fixed <<setprecision(6) <<  (this->miss_num/(float)this->cycles) << endl;
  }

  if(this->write_strategy == cache::write_strategy_t::WRITE_BACK && this->cycles != 0){
     cout <<this->name <<  " dirty on replacement rate: " << fixed <<setprecision(6) << (this->dirty_num/(float)(this->cycles-1)) << endl;
  }

}
void cache::calculate_cycle(int n){
  int t = (this->address_cycles + this->data_cycles*n) ;

      //if(t==0){
          //this->both_cycles++;
          //cout << "   no oooooooooooooooo " << endl;
     // }else{
  this->both_cycles += t;
    //  }
}
void cache::count_cycle(){

    this->cycles ++;

}
