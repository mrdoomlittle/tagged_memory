# include "tagged_memory.hpp"


mdl::tagged_memory::tagged_memory(boost::uint16_t __allocated_memory,
    std::initializer_list<char> __seporator_tags, bool __debug_logging)
{
    this-> seporator_tags[0] = __seporator_tags.size() > 0? 
        *(__seporator_tags.begin()) : MEM_BEGIN_TAG;
    this-> seporator_tags[1] = __seporator_tags.size() > 1? 
        *(__seporator_tags.begin() + 1) : MEM_END_TAG;
    this-> seporator_tags[2] = __seporator_tags.size() > 2?
        *(__seporator_tags.begin() + 2) : MEM_SEP_TAG;
    this-> seporator_tags[3] = __seporator_tags.size() > 3? 
        *(__seporator_tags.begin() + 3) : MEM_LIST_TAG;

    this-> debug_logging = __debug_logging;

    /* set all elements in vector to \0 as strlen uses it to determine the length of a chr * array
    */
    this-> memory_stack.resize(__allocated_memory);
    for (size_t i = 0; i != __allocated_memory; i ++)
        this-> memory_stack(i) = '\0';
}

bool mdl::tagged_memory::compare_mem_value(char const * __name_0, char const * __name_1, bool & __error)
{
  

    return this-> compare_strings(
        this-> get_mem_value(__name_0, __error),
        this-> get_mem_value(__name_1, __error)
    );
}

bool mdl::tagged_memory::compare_strings(char const * __value_0, char const * __value_1)
{
    size_t len_of_v0 = strlen(__value_0);
    size_t len_of_v1 = strlen(__value_1);
    size_t matching_char_c = 0;

    std::cout << __value_0 << ", " << __value_1  << std::endl;

    if (len_of_v0 != len_of_v1) return false;

    for (size_t i = 0; i != len_of_v0; i ++)
        if (__value_0[i] == __value_1[i]) matching_char_c ++;

    if (matching_char_c == len_of_v0) return true;
    return false;
}

char * mdl::tagged_memory::dump_stack_memory()
{
    char * re = static_cast<char *>(malloc(this-> memory_stack.size()));

    for (size_t i = 0; i != this-> memory_stack.size(); i ++) {
        if (this-> memory_stack[i] == '\0') break;
        re[i] = this-> memory_stack[i];
        if (this-> debug_logging)
            printf("\x1B[0m%c\x1B[32m|\x1B[0m", this-> memory_stack[i]);
    } printf("\n");

    return re;   
}


void mdl::tagged_memory::load_mem_stack_from_file(char const * __file_path)
{
    std::string each_line;
    std::ifstream ifile(__file_path);
    size_t point = 0, o = 0;

    if (this-> debug_logging)
        printf("\x1B[34mloading memory stack from '%s'\x1B[0m\n", __file_path);

    while ( std::getline (ifile, each_line) ) {
        char const * tmp = each_line.c_str();

        for (size_t i = point; i != point + strlen(tmp); i++) {
            this-> memory_stack(i)  = tmp[o];
            o++;
        }

    }
}

char * mdl::tagged_memory::create_mem_tag(char const * __name, char const * __value)
{
    size_t length_of_name = strlen(__name);
    size_t length_of_value = strlen(__value);

    size_t total_size = (length_of_name + length_of_value + 3/* this includes the ending, starting and mid seporator*/);
 
    total_size ++;

    char * tmp = static_cast<char *>(malloc(sizeof(char) * total_size));

    memset(tmp, '\0', total_size);

    size_t current_addr = 0, o = 0, q = 0;
    tmp[current_addr] = this-> seporator_tags[sp_t::__begin];

    current_addr ++;

    for (size_t i = current_addr; i != (current_addr + length_of_name); i ++) {
        tmp[i] = __name[o];
        o ++;
        q ++;
    } current_addr += q;

    tmp[current_addr] = this-> seporator_tags[sp_t::__seporator];
    current_addr ++;

    o = 0; q = 0;
    for (size_t i = current_addr; i != (current_addr + length_of_value); i ++) {
        tmp[i] = __value[o];
        o ++;
        q ++;
    } current_addr += q;

    tmp[current_addr] = this-> seporator_tags[sp_t::__end];

    return tmp;
}

char * mdl::tagged_memory::get_mem_value(char const * __name, bool & __error)
{
    boost::uint16_t mem_addr = this-> get_mem_addr(__name, __error);

    return this-> get_mem_value(mem_addr, __error);
}

void mdl::tagged_memory::add_mem_tag(char const * __name, char const * __value, size_t __null_space)
{
    boost::uint16_t insert_addr = 0;

    ublas::vector<boost::array<boost::uint16_t, 2>>
            ::iterator itor = this-> memory_addrs.end();

    if (this-> memory_addrs.size() != 0) {
        --itor;

        insert_addr = ((* itor)[1] + 2);
    }
   
    if (this-> debug_logging)
        printf("adding memory tag into addr %d\n", insert_addr);

    char * tmp = this-> create_mem_tag(__name, __value);

    size_t length_of_mem = strlen(tmp);

    this-> memory_addrs.resize(this-> memory_addrs.size() + 1);

    itor = this-> memory_addrs.end();

    --itor;

    (* itor)[0] = insert_addr;
    (* itor)[1] = (insert_addr + length_of_mem) - 2;/*this seems to leave the ending tag at the end of the value output, but this fixed might need to look into it*/
    size_t o = 0;
    for (size_t i = insert_addr; i != insert_addr + length_of_mem; i ++) {
        this-> memory_stack(i) = tmp[o];

        o++;
    }

    std::free(tmp);
 
   //printf("%s\n", this-> create_mem_tag("test", "test")); 
 
}

void mdl::tagged_memory::save_mem_stack_to_file(char const * __file_path)
{
    if (this-> debug_logging)
        printf("\x1B[34msaving memory stack to '%s'\x1B[0m\n", __file_path);

    std::ofstream ofile;
    ofile.open(__file_path);

    if (ofile.is_open()) {
        if (this-> debug_logging)
            printf("\x1B[32m -- successfully opened file.\x1B[0m\n");

    } else {
        if (this-> debug_logging)
            printf("\x1B[31m -- failed to open file.\x1B[0m\n");
    }

    char * memory = dump_stack_memory();

    ofile << memory;
    ofile.close();

    std::free(memory);
}



size_t mdl::tagged_memory::find_mem_addr_it_pos(boost::uint16_t __addr, bool & __error) {
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    size_t iterated_amount = 0;
    for (;itor != this-> memory_addrs.end(); ++itor) {
        if ((* itor)[0] == __addr) return iterated_amount;
        iterated_amount ++; 
    }

    __error = true;
    return 0;
}

size_t mdl::tagged_memory::get_mem_name_len(boost::uint16_t __addr, bool & __error) {
    if (this-> memory_stack[__addr] != this-> seporator_tags[sp_t::__begin]) {__error = true; return 0;}
 
    size_t length_of_name = 0;
    for(size_t i = (__addr +1);; i++) {
        if (this-> memory_stack[i] == this-> seporator_tags[sp_t::__seporator]) break;
        length_of_name++; 
    }

    return length_of_name;
}

bool mdl::tagged_memory::does_mem_addr_ok(boost::uint16_t __addr) {
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();
    if (this-> debug_logging)
        printf("\x1B[36mchecking if memory address is ok.\x1B[0m\n");

    for (; itor != this-> memory_addrs.end(); ++itor) {
        if ((* itor)[0] == __addr) {
            if (this-> debug_logging)
                printf("\x1B[35msuccessfully found address %d\x1B[0m\n", __addr);
            return true;

        }
    }
    if (this-> debug_logging)
        printf("\x1B[35mfailed to find address %d\x1B[0m\n", __addr);
    return false;
}

char * mdl::tagged_memory::get_mem_name(boost::uint16_t __addr, bool & __error)
{
    if (! this-> does_mem_addr_ok(__addr)) { __error = true; return '\0';}
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    itor += find_mem_addr_it_pos(__addr, __error); 

    size_t length_of_name = get_mem_name_len(__addr, __error);

    if (this-> debug_logging)
        printf("\x1B[37mlen of mem name at addr %d is %ld bytes\x1B[0m\n", __addr, length_of_name); 

    // havent tested this 
    char * __name = static_cast<char *>(malloc(length_of_name));

    size_t o = 0;
    for (size_t i = (__addr +1); i != (__addr +1) + length_of_name; i ++) {
        __name[o] = this-> memory_stack[i];
        o ++; 
    }
    __name[o] = '\0';

   
    return __name; 
}


void mdl::tagged_memory::insert_into_mem_stack(char __mem, boost::uint16_t __addr, bool & __error)
{
    if (this-> debug_logging)
        printf("\x1B[33minserting '%c' into stack at addr %d\x1B[0m\n", __mem, __addr);

    for (size_t i = (this-> memory_stack.size() - 1); i != __addr; i --) {
        if (this-> memory_stack[i-1] == '\0') continue;
        if (this-> debug_logging)
            printf("\x1B[35mmoving memory at addr %ld to %ld\x1B[0m\n", (i - 1), i);
        this-> memory_stack(i) = this-> memory_stack[i - 1];
        this-> memory_stack(i - 1) = ' ';
    }

    this-> memory_stack(__addr) = __mem;
}

void mdl::tagged_memory::uninsert_from_mem_stack(boost::uint16_t __addr, bool & __error)
{
    if (this-> debug_logging)
        printf("\x1B[33muninserting '%c' from stack at addr %d\x1B[0m\n", this-> memory_stack[__addr], __addr);

    this-> memory_stack(__addr) = ' ';

    for (size_t i = __addr; i != (this-> memory_stack.size() - 1); i ++) {
        if (this-> memory_stack[i] == '\0') continue;
        if (this-> debug_logging)
            printf("\x1B[35mmoving memory at addr %ld to %ld\x1B[0m\n", (i + 1), i);
        this-> memory_stack(i) = this-> memory_stack[i + 1];
    }
}


boost::uint16_t mdl::tagged_memory::get_mem_addr(char const * __name, bool & __error)
{
    size_t length_of_name = strlen(__name);

    size_t mem_addrs_pos = 0;
    size_t mem_match_count = 0;
    size_t name_char_pos = 0;

    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();
    
    while(mem_addrs_pos != memory_addrs.size())
    {
        if (this-> debug_logging)
            printf("\x1B[34msearching mem addr book at id %ld for '%s'\x1B[0m\n", mem_addrs_pos, __name);
        for (size_t i = ((* itor)[0] + 1); i != (* itor)[1]; i ++) {
            /* if the current memory stack char equals the current name char 
            * then add 1 decamal place
            */

            if (this-> debug_logging)
                printf("\x1B[33mmatching char '%c' and '%c'\x1B[0m\n", this-> memory_stack[i], __name[name_char_pos]);
            if (this-> memory_stack[i] == __name[name_char_pos]) {
                mem_match_count ++;
                if (this-> debug_logging)
                    printf("\x1B[42mmatch found at addr %ld\x1B[0m\n", i);
            } else {
                if (this-> debug_logging)
                    printf("\x1B[41mmatch not found at addr %ld\x1B[0m\n", i);
            }

            if (this-> memory_stack[i] == this-> seporator_tags[sp_t::__seporator]) {
                break;    
            }
            
            /* if we have got the the end of the name then there is no point of tracking thru
            * the stack anymore, so we are going to move to the next tag if exists
            */
            if ((name_char_pos + 1) == length_of_name) {

                bool is_next_seporator = this-> memory_stack[i + 1] == this-> seporator_tags[sp_t::__seporator]? true : false;
                if (mem_match_count == length_of_name && is_next_seporator) {
                    if (this-> debug_logging)
                        printf("\x1B[36mmatching success. %ld out of %ld correct.\x1B[0m\n", mem_match_count, length_of_name);
                    __error = false;
                    return (* itor)[0];
                }
                __error = true;
                if (this-> debug_logging) {
                    printf("\x1B[36mmatching failed. %ld out of %ld correct.\x1B[0m\n", mem_match_count, length_of_name);

                    if (!is_next_seporator) {
                        printf("\x1B[31mnext char dose not equal '%c' but equals '%c'\x1B[0m\n", this-> seporator_tags[sp_t::__seporator], this-> memory_stack[i + 1]);
                    }
                }
                mem_match_count = 0;
                name_char_pos = 0;
                break;
            } else
                name_char_pos ++;
        } 

        ++itor;
        mem_addrs_pos ++;
    }
    return 0;
}

void mdl::tagged_memory::set_mem_name(char const * __current_name, char const * __name, bool & __error)
{
    boost::uint16_t mem_location = this-> get_mem_addr(__current_name, __error);

    if (this-> debug_logging)
        printf("\x1B[35msetting mem name at addr %d. from '%s' to '%s'\x1B[0m\n", mem_location, __current_name, __name);
   
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    itor += find_mem_addr_it_pos(mem_location, __error);

    size_t len_of_name = this-> get_mem_name_len(mem_location, __error);
    
    if (__error == true) return;   
 
    bool is_same_len = false;
    size_t o = 0, rm = 0;
    bool less = false, grater = false;
    for (size_t i = ((* itor)[0] + 1);; i ++) {
        if (this-> memory_stack[rm] == this-> seporator_tags[sp_t::__seporator] && !grater && !is_same_len) {
            this-> insert_into_mem_stack(' ', rm, __error);
            (* itor)[1] ++;
            less = true;
        }

        this-> memory_stack(i) = __name[o]; 

        if (o == (strlen(__name) - 1))  {
            if (is_same_len) break;
            if (this-> memory_stack[rm] != this-> seporator_tags[sp_t::__seporator] && !less) {
                this-> uninsert_from_mem_stack(rm, __error);
                (* itor)[1] --;
                grater = true;
            } else break;
        } else { o ++; rm = i+1;}
    }
}

void mdl::tagged_memory::set_mem_value(char const * __name, char const * __value, bool & __error)
{

    boost::uint16_t mem_location = this-> get_mem_addr(__name, __error);

    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    itor += find_mem_addr_it_pos(mem_location, __error);

    size_t length_of_name = get_mem_name_len(mem_location, __error);
    
    size_t length_of_value = ((* itor)[1] - (((* itor)[0] + 1) + (length_of_name + 1)));
 
    length_of_value ++;
    bool len = false, tsmall = false;
    size_t o = 0, extra = 1, rm = 0;
    bool bypass = strlen(__value) == length_of_value? true : false;
    
    for (size_t i = ((* itor)[0] + 1) + (length_of_name + 1);; i++ ) {
        if (this-> memory_stack[i] == this-> seporator_tags[sp_t::__begin] && !len && !tsmall) extra ++;

        if (o == strlen(__value)) {
            if (tsmall || bypass) break;

            if (i == (* itor)[1] /* - 1*/ && !len)  {
                len = true;
                i = ((* itor)[0] + 1) + (length_of_name + 1);
            }

            if (len) {
                if (this-> memory_stack[rm] == this-> seporator_tags[sp_t::__end] && extra == 0) {
                    break;
                }
                else
                {
                    this-> uninsert_from_mem_stack(rm, __error); 
                    if (this-> memory_stack[rm] == this-> seporator_tags[sp_t::__end])
                        extra --;
                
                }
            } 
       
        } else {
            if (!bypass) {
            if (i == ((* itor)[1] + 1) && !tsmall) {
                std::cout << this-> memory_stack[i] << std::endl;
        
                tsmall = true;
            }
         
            if (tsmall) this-> insert_into_mem_stack(' ', i, __error);
}
            this-> memory_stack(i) = __value[o];

            o ++;
            rm = i+1;

        }
    }

    size_t before = (* itor)[1];
    (* itor)[1] = ((* itor)[0] + strlen(__value) + (length_of_name + 1));

    size_t change = 0;
    
    if (before > (* itor)[1])
        change = (before - (* itor)[1]);

    if (before < (* itor)[1])
        change = ((* itor)[1] - before);
   
    if (before == (* itor)[1]) return;
    
    if (itor != this-> memory_addrs.end()) ++itor;
    for (; itor != this-> memory_addrs.end(); ++itor)
    {
        if (before > (* itor)[1]) {
            (* itor)[0] -= change;
            (* itor)[1] -= change;
        }

        if (before < (* itor)[1]) {
            (* itor)[0] += change;
            (* itor)[1] += change;
        }
    } 
}

char * mdl::tagged_memory::get_mem_value(boost::uint16_t __addr, bool & __error)
{
    if (! this-> does_mem_addr_ok(__addr)) { __error = true; return '\0';}
    
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    itor += find_mem_addr_it_pos(__addr, __error);
   
    size_t length_of_name = get_mem_name_len(__addr, __error);
 
    size_t length_of_value = ((* itor)[1] - (((* itor)[0] + 1) + (length_of_name + 1)));

    length_of_value += 2;
    char * tmp = static_cast<char *>(malloc(length_of_value));
    memset(tmp, '\0', length_of_value);

    size_t o = 0;
    for (size_t i = ((* itor)[0] + 1) + (length_of_name + 1); i != (* itor)[1] + 1; i++ ) {
        tmp[o] = this-> memory_stack[i];
        o ++;
    }

    return tmp;
}

void mdl::tagged_memory::analyze_stack_memory(bool & __error)
{
    bool found_mem_begin_ts = false;
    bool found_mem_end_ts = false;
    size_t extra_mem_begin_tss = 0;
    boost::uint16_t mem_begin_ts_addr = 0;
    boost::uint16_t mem_end_ts_addr = 0;
    size_t mem_stack_pos = 0, waround = 0;

    boost::uint16_t stack_length = this-> memory_stack.size();

    float one_precent = (100.00/stack_length); 
 
    float finished_precentage = 0.00;
    if (this-> debug_logging)
        printf("\x1B[36manalysing the memory stack, please wait ...\x1B[0m\n");
    do
    {
        if (this-> debug_logging) {
            printf("\x1B[32m%.2f%%\x1B[34m - analysing ...\x1B[0m\n", finished_precentage);
            
        }

        if (this-> memory_stack[mem_stack_pos] == '\0') { 
            if (this-> debug_logging)
                printf("\x1B[31mbreaking from loop. next char equls '\\0'\x1B[0m\n");
            break; 
        }

        if (this-> memory_stack[mem_stack_pos] == this-> seporator_tags[sp_t::__begin])
        {
            if (this-> debug_logging)
                printf("\x1B[35mfound beginning seporator as addr %ld\x1B[0m\n", mem_stack_pos);
           // if (this-> memory_stack[mem_stack_pos + 1] != this-> seporator_tags[sp_t::__end]) {
                if (! found_mem_begin_ts) {
                    if (this-> debug_logging)
                        printf("\x1B[33mmemory begin has been set at addr %ld\x1B[0m\n", mem_stack_pos);
                    found_mem_begin_ts = true;
                    mem_begin_ts_addr = mem_stack_pos;
                } else {
                    extra_mem_begin_tss ++;
                    waround ++;
                }
          //  }
        }

        if (this-> memory_stack[mem_stack_pos] == this-> seporator_tags[sp_t::__end] && mem_stack_pos != mem_begin_ts_addr && found_mem_begin_ts)
        {
            if (this-> debug_logging)
                printf("\x1B[35mfound ending seporator as addr %ld\x1B[0m\n", mem_stack_pos);
            if (extra_mem_begin_tss != 0)
                extra_mem_begin_tss --;
            else {
                if (this-> debug_logging)
                    printf("\x1B[33mmemory end has been set at addr %ld\x1B[0m\n", mem_stack_pos); 
                mem_end_ts_addr = mem_stack_pos;
               
                this-> memory_addrs.resize(this-> memory_addrs.size() + 1);

                ublas::vector<boost::array<boost::uint16_t, 2>>
                    ::iterator itor = this-> memory_addrs.begin();
            
                itor += (this-> memory_addrs.size() - 1);
             
                (* itor)[0] = mem_begin_ts_addr;
                (* itor)[1] = mem_end_ts_addr - 1;

                //std::cout << (* itor)[0] << ", " << (* itor)[1] << std::endl;

                found_mem_begin_ts = false;
                found_mem_end_ts = false;
                extra_mem_begin_tss = 0;
                waround = 0;
            }
        }

        finished_precentage += one_precent;
        mem_stack_pos++;
    } while (mem_stack_pos != this-> memory_stack.size());

    if (this-> debug_logging)
        printf("\x1B[36mfinished analysing.\x1B[0m\n");
}

void mdl::tagged_memory::dump_into_stack(char const * __memory)
{
    if (this-> debug_logging)
        printf("\x1B[37mdumping '%s' into memory stack.\x1B[0m\n", __memory);
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    /* where will begin putting the memory passed thru the function into the memory stack
    */
    boost::uint16_t stack_begin_address = 0;

    if (this-> memory_addrs.size() != 0) {
        itor += this-> memory_addrs.size() - 1;
        stack_begin_address = (* itor)[1] + 1;
    }

    /* the length of the char array thats been passed thru the function
    */
    size_t length_of_string = strlen(__memory);

    for (size_t i = stack_begin_address; i != length_of_string + stack_begin_address; i ++)
        this-> memory_stack(i) = __memory[i - stack_begin_address];
}
