# include "tagged_memory.hpp"
/* NOTE: i am passing char * into const char * 
* i dont know if it will cause any error/s.
*/

mdl::tagged_memory::tagged_memory(boost::uint16_t __allocated_memory,
    std::initializer_list<char> __seporator_tags, bool __debug_logging)
{
    this-> seporator_tags[0] = __seporator_tags.size() > 0? 
        *(__seporator_tags.begin()) : MEM_BEGIN_TAG;
    this-> seporator_tags[1] = __seporator_tags.size() > 1?
        *(__seporator_tags.begin() + 1) : MEM_MIDDLE_TAG;
    this-> seporator_tags[2] = __seporator_tags.size() > 2?
        *(__seporator_tags.begin() + 2) : MEM_END_TAG;

    this-> debug_logging = __debug_logging;

    /* set all elements in vector to \0 as strlen uses it to determine the length of a chr * array
    */
    this-> memory_stack.resize(__allocated_memory);
    for (size_t i = 0; i != __allocated_memory; i ++)
        this-> memory_stack(i) = '\0';
}

void mdl::tagged_memory::set_mem_value(char const * __name, char const * __value, bool & __error, boost::uint16_t __list_addr)
{
    /* NOTE: when setting a list element it does not directly edit the stack, but gets the value of the
    * variable change it and then use the set_mem_value without list enabled
    */

    char * tmp = this-> get_mem_value(__name, __error, 0, true);

    std::size_t nx_length = strlen(__value);
    
    boost::uint16_t mem_location = this-> get_mem_addr(__name, __error);

    std::size_t ad = find_mem_addr_it_pos(mem_location, __error);

    std::size_t curr_length = this-> infomation[ad].list_elength[__list_addr];

    std::size_t change = 0;

    boost::uint16_t start = this-> infomation[ad].list_points[__list_addr];
    std::size_t tlen = strlen(tmp);
    boost::uint16_t after_len = 0;
    std::size_t o = 0;
    if (curr_length == nx_length) {

        for (std::size_t i = start; i != start + curr_length; i ++) {
            this-> memory_stack[i] = __value[o];
            o ++;
        }

        return;
    }

    bool g = false, l = false;
    if (curr_length > nx_length) {
        change = (curr_length - nx_length);
        after_len = (tlen - change); 
        l = true;
    }

    if (curr_length < nx_length) {
        change = (nx_length - curr_length);
        after_len = (tlen + change);
        g = true;
    }
 
    std::cout << "af" << after_len << std::endl;

    char * aft = static_cast<char *>(malloc(after_len * sizeof(char)));
    memset(aft, '\0', after_len * sizeof(char));
    std::cout << "change: " << change << std::endl;

    boost::uint16_t nmlen = this-> get_mem_name_len(mem_location, __error);
    o = 0;
    bool found_addr = false;

    std::size_t extra = 0, el = 0;
    if (g) extra = change;
    if (l) el = change;
    std::cout << "extra: " << extra << std::endl;

    std::size_t j = 0, skip = 0;
    for (std::size_t i = mem_location + nmlen; i != ((this-> memory_addrs[ad][1] + extra) - el) - 1; i ++) {
        if (i == (start - 1) || found_addr) {
            aft[o] =  __value[j];

            std::cout << "@ : " << aft[o] << std::endl;         
            skip = change;
            if (found_addr == false && j == 0) found_addr = true;
            if (j == nx_length - 1) found_addr = false; else j ++;
        } else {
            if (g) aft[o] = tmp[o - skip];
            if (l) aft[o] = tmp[o + skip];
            std::cout << "@ : " << aft[o] << std::endl;
        }
        o ++;
        
        std::cout << "TICK-" << o-1 << std::endl;
    }    

    std::cout << aft << std::endl;

    this-> set_mem_value(__name, aft, __error);    

    for (std::size_t i = __list_addr + 1; i != this-> infomation[ad].list_points.size(); i ++) {
        if (g) this-> infomation[ad].list_points(i) += change; 
        if (l) this-> infomation[ad].list_points(i) -= change;
    }

    if (g) this-> infomation[ad].list_elength(__list_addr) += change;
    if (l) this-> infomation[ad].list_elength(__list_addr) -= change;

    std::free(tmp);
    std::free(aft);
}

bool mdl::tagged_memory::does_mem_name_exist(char const * __mem_name, bool & __error)
{
    ublas::vector<boost::array<boost::uint16_t, 2>>::iterator it = this-> memory_addrs.begin();

    /* iterate thru the address vector and compare the name of each variable until there's a match.
    */
    for (; it != this-> memory_addrs.end() ; ++ it)
        if (this-> compare_strings(__mem_name, this-> get_mem_name((* it)[0], __error))) return true;
 
    /* if no match was found then return 'false' */
    return false;
}

bool mdl::tagged_memory::compare_mem_values(char const * __mem_name_0, char const * __mem_name_1, bool & __error)
{
    /* use the compare strings function and then return the result
    * NOTE: the only way to comapre list elements is to pass somthing like this as the name 'example[ list pos ]'
    */
    return this-> compare_strings(this-> get_mem_value(__mem_name_0, __error, 0, true), 
        this-> get_mem_value(__mem_name_1, __error, 0, true));
}

bool mdl::tagged_memory::compare_strings(char const * __string_0, char const * __string_1)
{
    /* get the length of each string as we will need them later */
    size_t len_of_string_0 = strlen(__string_0);
    size_t len_of_string_1 = strlen(__string_1);

    /* this will allow us to keep track on how many chars we have matched */
    size_t matching_char_set = 0;

    // NOTE: remove this later if not needed as it was for debugging
    std::cout << __string_0 << ", " << __string_1  << std::endl;

    /* if there not the same length then there not going to be the same so return false */
    if (len_of_string_0 != len_of_string_1) return false;

    /* compare each char of both string, if they match then add 1 to 'matching_char_set'
    * if theres no match move on to the next set of chars.
    */
    for (size_t i = 0 ; i != len_of_string_0 ; i ++)
        if (__string_0[i] == __string_1[i]) matching_char_set ++;

    /* if we have matched the same amount of chars compared to the length.
    * then matching was a success, so return true
    */
    if (matching_char_set == len_of_string_0) return true;

    // return false for defualt if matching failed
    return false;
}

char * mdl::tagged_memory::dump_stack_memory()
{
    char * re = static_cast<char *>(malloc(this-> memory_stack.size()));

    memset(re, '\0', this-> memory_stack.size());

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
    tmp[current_addr] = this-> seporator_tags[sp_t::__mem_begin];

    current_addr ++;

    for (size_t i = current_addr; i != (current_addr + length_of_name); i ++) {
        tmp[i] = __name[o];
        o ++;
        q ++;
    } current_addr += q;

    tmp[current_addr] = this-> seporator_tags[sp_t::__mem_middle];
    current_addr ++;

    o = 0; q = 0;
    for (size_t i = current_addr; i != (current_addr + length_of_value); i ++) {
        tmp[i] = __value[o];
        o ++;
        q ++;
    } current_addr += q;

    tmp[current_addr] = this-> seporator_tags[sp_t::__mem_end];

    return tmp;
}

char * mdl::tagged_memory::get_mem_value(char const * __name, bool & __error, boost::uint16_t __list_addr, bool __no_list)
{
    boost::uint16_t mem_addr = this-> get_mem_addr(__name, __error);

    if (__no_list == true) {
        
        std::size_t ltaddr_b = 0;
        std::size_t ltaddr_e = 0;
        bool fblist = false, fe_list = false;

        for (size_t i = 0; i != strlen(__name); i++ ) {
            if (__name[i] == LIST_LEN_BTAG) {
                fblist = true;
                ltaddr_b = i;
            }

            if (__name[i] == LIST_LEN_ETAG) {
                if (i != ltaddr_b && fblist) {
                    fe_list = true;
                    ltaddr_e = i;
                }
            }
        }

        if (fblist && fe_list) {
        
            std::size_t list_pointer = 0;
            char * name = this-> extract_list_addr(__name, list_pointer, ltaddr_b, ltaddr_e);
          
            std::cout << "name = " << name << std::endl; 

            //this-> set_mem_value(name, __value, __error, list_pointer);

            char * tmp = this-> get_mem_value(name, __error, list_pointer, false);            

            std::free(name);
           
            return tmp;
        }

    }
    std::cout << "--------------@" << __no_list << std::endl; 
    return this-> get_mem_value(mem_addr, __error, __list_addr, __no_list);
}

void mdl::tagged_memory::add_mem_tag(char const * __name, char const * __value, size_t __null_space, bool & __error)
{
    if (this-> does_mem_name_exist(__name, __error)) {
        printf("cant add memory tag to stack because it allready exists!\n");
        return;
    }

    boost::uint16_t insert_addr = 0;

    ublas::vector<boost::array<boost::uint16_t, 2>>
            ::iterator itor = this-> memory_addrs.end();

    if (this-> memory_addrs.size() != 0) {
        --itor;

        insert_addr = ((* itor)[1] + 2);
    }
   
    if (this-> debug_logging)
        printf("adding memory tag at addr %d\n", insert_addr);

    char * tmp = this-> create_mem_tag(__name, __value);

    std::size_t length_of_mem = strlen(tmp);

    this-> memory_addrs.resize(this-> memory_addrs.size() + 1);
    this-> infomation.resize(this-> infomation.size() + 1);

    tagged_memory::__o & lo = this-> infomation[this-> memory_addrs.size() -1];

    ublas::vector<boost::uint16_t> list_elength;
    ublas::vector<boost::uint16_t> list_points;

    bool got_list_be_tags = false;
    bool found_list_begin = false;
    boost::uint16_t lb_tag_addr = 0;
    bool found_list_end = false;
    boost::uint16_t le_tag_addr = 0;
    bool found_middle_tag = false;
    std::size_t list_sep_tcount = 0;
    boost::uint16_t mid_tag_addr = 0;
    boost::uint16_t last_lpoint = 0;
    bool is_value_str_type = false;
    boost::uint16_t str_bt_addr = 0;
    for (std::size_t i = 0; i != length_of_mem; i ++) {
        if (tmp[i] == STR_BEGIN_TAG && !is_value_str_type) {
            is_value_str_type = true;
            str_bt_addr = i;
        }
        if (tmp[i] == STR_END_TAG && i != str_bt_addr && is_value_str_type) is_value_str_type = false;

        if (found_middle_tag == false && !is_value_str_type) {
            if (tmp[i] == LIST_LEN_BTAG && !found_list_begin && !got_list_be_tags && !found_list_end) {
                found_list_begin = true;
                lb_tag_addr = i;
            }

            if (tmp[i] == LIST_LEN_ETAG && found_list_begin && i != lb_tag_addr && !got_list_be_tags && !found_list_end) {
                found_list_end = true;
                le_tag_addr = i;
            }

            if (found_list_begin && found_list_end && !got_list_be_tags) {
                got_list_be_tags = true;
                lo.len_of_tag = (le_tag_addr - lb_tag_addr) + 1;
                lo.is_list_type = true;
                char * len_of_list = static_cast<char *>(malloc((lo.len_of_tag - 1) * sizeof(char)));
                memset(len_of_list, '\0', (lo.len_of_tag - 1) * sizeof(char));
                std::size_t l = 0;
                for (std::size_t p = lb_tag_addr + 1; p != le_tag_addr; p ++) {
                    len_of_list[l] = tmp[p];
                    l ++;
                }

                lo.len_of_list = atoi(len_of_list);

                std::free(len_of_list);
            } 
        }

        if (!is_value_str_type) {
            if (tmp[i] == this-> seporator_tags[sp_t::__mem_middle] && !found_middle_tag) {
                found_middle_tag = true;
                mid_tag_addr = i;
            }

            if (found_middle_tag && tmp[i] == MEM_LIST_TAG) {
                list_elength.resize(list_elength.size() + 1);

                list_elength(list_sep_tcount) = (i - (list_sep_tcount == 0? mid_tag_addr : last_lpoint)) - 1; 

                list_points.resize(list_points.size() + 1);

                list_points(list_sep_tcount) = list_sep_tcount == 0? mid_tag_addr : last_lpoint;

                list_sep_tcount++;
                last_lpoint = i;
            }
        }
    }

    list_elength.resize(list_elength.size() + 1);
    list_points.resize(list_points.size() + 1);

    list_elength(list_sep_tcount) = ((length_of_mem - 1) - last_lpoint) - 1;
    list_points(list_sep_tcount) = last_lpoint;

    itor = this-> memory_addrs.end();

    --itor;

    (* itor)[0] = insert_addr;
    (* itor)[1] = (insert_addr + length_of_mem) - 2;/*this seems to leave the ending tag at the end of the value output, but this fixed might need to look into it*/
    size_t o = 0;
    for (size_t i = insert_addr; i != insert_addr + length_of_mem; i ++) {
        this-> memory_stack(i) = tmp[o];

        o++;
    }
  
    for (std::size_t i = 0; i != list_points.size(); i ++) {
        list_points(i) = (insert_addr + list_points[i]);
    }

    lo.list_elength.swap(list_elength);
    lo.list_points.swap(list_points);
    std::size_t st = insert_addr + length_of_mem;

    for (std::size_t i = st; i != st + __null_space; i ++) {
        this-> memory_stack(i) = BLANK_MEMORY;
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
    if (this-> memory_stack[__addr] != this-> seporator_tags[sp_t::__mem_begin]) {__error = true; return 0;}
 
    size_t length_of_name = 0;
    for(size_t i = (__addr +1);; i++) {
        if (this-> memory_stack[i] == this-> seporator_tags[sp_t::__mem_middle]) break;
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

    std::size_t ad = find_mem_addr_it_pos(__addr, __error); 

    itor += ad;

    std::size_t length_of_name = get_mem_name_len(__addr, __error);

    if (this-> debug_logging)
        printf("\x1B[37mlen of mem name at addr %d is %ld bytes\x1B[0m\n", __addr, length_of_name); 

    // havent tested this 
    char * __name = static_cast<char *>(malloc(length_of_name));

    std::size_t reduction = 0;

    if (this-> infomation[ad].is_list_type)
        reduction = this-> infomation[ad].len_of_tag;

    std::size_t o = 0;
    for (std::size_t i = (__addr + 1) ; i != ((__addr + 1) + length_of_name) - reduction; i ++) {
        __name[o] = this-> memory_stack[i];
        o ++; 
    }
    __name[o] = '\0';

    std::cout << "get name = " << __name << std::endl;
   
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
  
    std::cout << "len ~ " << length_of_name << std::endl;

    size_t mem_addrs_pos = 0;
    size_t mem_match_count = 0;
    size_t name_char_pos = 0;

    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();
   
    std::size_t reduction = 0;

    while(mem_addrs_pos != memory_addrs.size())
    {
        if (this-> infomation[mem_addrs_pos].is_list_type)
            reduction = this-> infomation[mem_addrs_pos].len_of_tag;
        else
            reduction = 0;

        std::cout << "reduc : " << reduction << std::endl;
        if (this-> debug_logging)
            printf("\x1B[34msearching mem addr book at id %ld for '%s'\x1B[0m\n", mem_addrs_pos, __name);
        for (size_t i = ((* itor)[0] + 1); i != (* itor)[1] - reduction; i ++) {
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

            if (this-> memory_stack[i] == this-> seporator_tags[sp_t::__mem_middle]) {
                /* this was added in to fix a placment error but if any errors occur after this edit it might be because of this*/
                mem_match_count = 0;
                name_char_pos = 0;
                break;    
            }
            
            /* if we have got the the end of the name then there is no point of tracking thru
            * the stack anymore, so we are going to move to the next tag if exists
            */
            if ((name_char_pos + 1) == length_of_name) {
                
                bool is_next_seporator = this-> memory_stack[i + 1] == this-> seporator_tags[sp_t::__mem_middle]? true : false; 

                /* if it's in a list type format then the next char will not be the nm seporator it will
                * be the beginning of the list size
                */ 
                if (this-> infomation[mem_addrs_pos].is_list_type) {
                    is_next_seporator = this-> memory_stack[i + 1] == LIST_LEN_BTAG? true : false;
                }

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
                        printf("\x1B[31mnext char dose not equal '%c' but equals '%c'\x1B[0m\n", this-> seporator_tags[sp_t::__mem_middle], this-> memory_stack[i + 1]);
                    }
                }
            
                mem_match_count = 0;
                name_char_pos = 0;
                break;
            } else
                name_char_pos ++;
        }
        mem_match_count = 0;
        name_char_pos = 0; 

        ++itor;
        mem_addrs_pos ++;
    }
    return 0;
}

void mdl::tagged_memory::set_mem_name(char const * __current_name, char const * __name, bool & __error)
{
    boost::uint16_t mem_location = this-> get_mem_addr(__current_name, __error);

    bool resize_to_fit = RESIZE_TO_FIT;

    if (this-> debug_logging)
        printf("\x1B[35msetting mem name at addr %d. from '%s' to '%s'\x1B[0m\n", mem_location, __current_name, __name);
   
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    std::size_t ad = find_mem_addr_it_pos(mem_location, __error);
    //MARK
    itor += ad;

    size_t len_of_name = this-> get_mem_name_len(mem_location, __error);
    
    if (__error == true) return;   

    /* count how much extra space there is after the ending address
    */
    std::size_t free_space = 0;
    for (std::size_t i = (* itor)[1] + 2;; i ++) {
        if (this-> memory_stack[i] == BLANK_MEMORY) free_space ++; else break;
    }

    bool is_same_len = false;
    size_t o = 0, rm = 0;
    bool less = false, grater = false;
    size_t amount_changed = 0;
    std::size_t extra_frm = 0;
    for (size_t i = ((* itor)[0] + 1);; i ++) {
        if (this-> memory_stack[rm] == this-> seporator_tags[sp_t::__mem_middle] && !grater && !is_same_len) {
            if (resize_to_fit) {
                if (free_space == 0) {
                    this-> insert_into_mem_stack(BLANK_MEMORY, rm, __error);
                    amount_changed ++;
                } else {
                    for (std::size_t l = (* itor)[1] + 1; l != (rm - 1); l --) {
                        this-> memory_stack[l + 1] = this-> memory_stack[l];
                        this-> memory_stack[l] = BLANK_MEMORY;
                    }
                
                    free_space --;
                }
            } else this-> insert_into_mem_stack(BLANK_MEMORY, rm, __error);

            (* itor)[1] ++;
            less = true;
        }

        this-> memory_stack(i) = __name[o]; 

        if (o == (strlen(__name) - 1))  {
            if (is_same_len) break;
            if (this-> memory_stack[rm] != this-> seporator_tags[sp_t::__mem_middle] && !less) {
                this-> uninsert_from_mem_stack(rm, __error);
                (* itor)[1] --;
                amount_changed ++;
                grater = true;
            } else break;
        } else { o ++; rm = i+1;}
    }
    
    if (itor == (this-> memory_addrs.end() - 1) || amount_changed == 0) return;
    
    if (amount_changed == 0) return;

    std::size_t cid = (ad + 1);

    ++itor;
    for (; itor != this-> memory_addrs.end(); ++itor) {
        if (this-> infomation[cid].is_list_type) {
            for (std::size_t i = 0; i != this-> infomation[cid].list_points.size(); i ++) {      
                if (grater) this-> infomation[cid].list_points(i) -= amount_changed;
                if (less) this-> infomation[cid].list_points(i) += amount_changed;
            }
        }

        if (less) {
            (* itor)[0] += amount_changed;
            (* itor)[1] += amount_changed;
            
        }

        if (grater) {
            (* itor)[0] -= amount_changed;
            (* itor)[1] -= amount_changed;
        }
        cid ++;
    }
}

char * mdl::tagged_memory::extract_list_addr(char const * __name, 
    std::size_t & __list_pointer, std::size_t __ltaddr_b, std::size_t __ltaddr_e) {
    char * list_point = static_cast<char *>(malloc((__ltaddr_e - __ltaddr_b) * sizeof(char)));

    std::size_t _o = 0;
    for (size_t i = __ltaddr_b+1; i != __ltaddr_e; i++ ) {
        list_point[_o] = __name[i];
        _o ++;
    }

    __list_pointer = atoi(list_point); 

    std::free(list_point);

    std::size_t length_of_name = ((strlen(__name) - (__ltaddr_e - __ltaddr_b)) - 1) * sizeof(char);

    char * name = static_cast<char *>(malloc(length_of_name + 1 * sizeof(char)));
    memset(name, '\0', length_of_name + 1 * sizeof(char));

    for (std::size_t i = 0; i != (strlen(__name) - (__ltaddr_e - __ltaddr_b)) - 1; i ++) {
        name[i] = __name[i];
    }

    std::cout << "ename = " << name << std::endl; 

    return name;
}

void mdl::tagged_memory::set_mem_value(char const * __name, char const * __value, bool & __error)
{
    bool resize_to_fit = RESIZE_TO_FIT;
   
    std::size_t ltaddr_b = 0;
    std::size_t ltaddr_e = 0;
    bool fblist = false, fe_list = false;

    for (size_t i = 0; i != strlen(__name); i++ ) {
        if (__name[i] == LIST_LEN_BTAG) {
            fblist = true;
            ltaddr_b = i;
        }

        if (__name[i] == LIST_LEN_ETAG) {
            if (i != ltaddr_b && fblist) {
                fe_list = true;
                ltaddr_e = i;
            }
        }
    }

    if (fblist && fe_list) {

        std::size_t list_pointer = 0;
        char * name = this-> extract_list_addr(__name, list_pointer, ltaddr_b, ltaddr_e); 
  
        this-> set_mem_value(name, __value, __error, list_pointer);        

        std::free(name);

        return;
    }

    boost::uint16_t mem_location = this-> get_mem_addr(__name, __error);

    bool found_list_indec_tags = false;

    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    std::size_t ad = find_mem_addr_it_pos(mem_location, __error);
    itor += ad;

    size_t length_of_name = get_mem_name_len(mem_location, __error); 
 
    size_t length_of_value = ((* itor)[1] - (((* itor)[0] + 1) + (length_of_name + 1)));
    bool did_addrs_change = false;
    boost::uint16_t last_end_addr = (* itor)[1];
    length_of_value ++;
    bool len = false, tsmall = false;
    size_t o = 0, extra = 1, rm = 0;
    bool bypass = strlen(__value) == length_of_value? true : false;
    std::size_t extra_frm = 0;
    for (size_t i = ((* itor)[0] + 1) + (length_of_name + 1);; i++ ) {
        if (this-> memory_stack[i] == this-> seporator_tags[sp_t::__mem_begin] && !len && !tsmall) extra ++;

        if (o == strlen(__value)) {
            if (tsmall || bypass) break;

            if (i == (* itor)[1] /* - 1*/ && !len)  {
                len = true;
                i = ((* itor)[0] + 1) + (length_of_name + 1);
            }

            if (len) {
                if (this-> memory_stack[rm] == this-> seporator_tags[sp_t::__mem_end] && extra == 0) {
                    break;
                }
                else
                {
                    if (!resize_to_fit)
                    { 
                        this-> memory_stack[rm] = this-> memory_stack[(rm + extra_frm) + 1];
                        this-> memory_stack[(rm + extra_frm) + 1] = ' ';

                        if (this-> memory_stack[rm] == this-> seporator_tags[sp_t::__mem_end]) extra --;
                        extra_frm ++;
                    }

                    if (resize_to_fit)
                    { 
                        this-> uninsert_from_mem_stack(rm, __error); 

                        if (this-> memory_stack[rm] == this-> seporator_tags[sp_t::__mem_end]) extra --;
                    } 
                }
            } 
       
        } else {
            if (!bypass) {
                if (i == ((* itor)[1] + 1) && !tsmall) tsmall = true; 
             
                if (tsmall) 
                {
                    /* if we are dealing a large amount of data you should first specify
                    * how much memory its going to take up, so when it comes to updating
                    * that pice of memory we dont need to push all the elements in the stack
                    * up by one.
                    */
                    if (resize_to_fit)
                    {
                        if (this-> memory_stack[i + 1] != ' ') {
                            this-> insert_into_mem_stack(' ', i, __error);
                            did_addrs_change = true;
                        } else  
                            this-> memory_stack[i + 1] = this-> memory_stack[i];

                    } else {
                        this-> insert_into_mem_stack(' ', i, __error);
                        did_addrs_change = true;
                    }
                }
            }
            this-> memory_stack(i) = __value[o];

            o ++;
            rm = i+1;

        }
    }

    size_t before = (* itor)[1];
    (* itor)[1] = ((* itor)[0] + strlen(__value) + (length_of_name + 1));

    size_t change = 0;
    bool g = false, l = false;

    if (before > (* itor)[1]) {
        change = (before - (* itor)[1]);
        l = true;
    }

    if (before < (* itor)[1]) {
        change = ((* itor)[1] - before);   
        g = true;
    }

    if (before == (* itor)[1]) return;
    
    std::size_t cid = (ad + 1);

    if (itor != this-> memory_addrs.end()) ++itor;
    for (; itor != this-> memory_addrs.end(); ++itor)
    {
        if (this-> infomation[cid].is_list_type) {
            for (std::size_t i = 0; i != this-> infomation[cid].list_points.size(); i ++) {            
                if (g) this-> infomation[cid].list_points(i) += change;
                if (l) this-> infomation[cid].list_points(i) -= change;
            }
            if (did_addrs_change) return;
        }

        if (did_addrs_change) {
            if (l) {
                (* itor)[0] -= change;
                (* itor)[1] -= change;
            }

            if (g) {
                (* itor)[0] += change;
                (* itor)[1] += change;
            }
        }

        cid ++;
    } 
}

char * mdl::tagged_memory::get_mem_value(boost::uint16_t __addr, bool & __error, boost::uint16_t __list_addr, bool __no_list)
{
    if (! this-> does_mem_addr_ok(__addr)) { __error = true; return '\0';}

    std::cout << "addr returned = " << __addr << std::endl;   
 
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    std::size_t ad = find_mem_addr_it_pos(__addr, __error), ext = 0;
    itor += ad;
    
    std::size_t length_of_name = get_mem_name_len(__addr, __error);

    std::size_t length_of_value = 0;
    if (this-> infomation[ad].is_list_type == false || __no_list) 
        length_of_value = ((* itor)[1] - (((* itor)[0] + 1) + (length_of_name + 1)));
    else {
        length_of_value = this-> infomation[ad].list_elength[__list_addr];
        ext = 1;
    }

    boost::uint16_t list_sep_point = 0; 
    if (this-> infomation[ad].is_list_type)
        list_sep_point = this-> infomation[ad].list_points[__list_addr] + 1;

    if (this-> infomation[ad].is_list_type == false || __no_list)
        length_of_value += 2;

    char * tmp = static_cast<char *>(malloc(length_of_value + ext * sizeof(char)));
    memset(tmp, '\0', length_of_value + ext * sizeof(char));

    
    std::size_t o = 0;
    if (this-> infomation[ad].is_list_type && __no_list == false) {
       for (std::size_t i = list_sep_point ; i != (list_sep_point + length_of_value) ; i ++ ) {
            tmp[o] = this-> memory_stack[i];
            o ++;
        }
       
        return tmp;
    }
    
 
    for (std::size_t i = ((* itor)[0] + 1) + (length_of_name + 1); i != (* itor)[1] + 1; i++ ) {
        tmp[o] = this-> memory_stack[i];
        o ++;
    }
   
    return tmp;
}

std::size_t mdl::tagged_memory::get_list_length(char const * __name, bool & __error)
{
    boost::uint16_t mem_location = this-> get_mem_addr(__name, __error);
    std::size_t ad = find_mem_addr_it_pos(mem_location, __error);
 
    if (! this-> infomation[ad].is_list_type) { __error = true; return 0; }
    return this-> infomation[ad].len_of_list;
}

void mdl::tagged_memory::analyze_stack_memory(bool & __error)
{
    /* if theres no data to analyze, then dont analyze */
    if (this-> memory_stack.size() == 0) return;

    bool found_mem_begin_ts = false;
    bool found_mem_end_ts = false;
    size_t extra_mem_begin_tss = 0
;
    boost::uint16_t mem_begin_ts_addr = 0;
    boost::uint16_t mem_end_ts_addr = 0;

    size_t mem_stack_pos = 0, waround = 0;
    bool found_list_begin = false, is_list_type = false;

    ublas::vector<boost::uint16_t> list_elength;
    ublas::vector<boost::uint16_t> list_points;

    boost::uint16_t stack_length = this-> memory_stack.size();
    std::size_t last_lpoint = 0;
    float one_precent = (100.00/stack_length); 
    boost::uint16_t mid_tag_addr = 0;
    bool found_mid_tag = false;
    std::size_t list_sep_tcount = 0;
    boost::uint16_t lb_tag_addr = 0, le_tag_addr = 0;
    float finished_precentage = 0.00;
    bool is_value_str_type = false;
    boost::uint16_t str_begin_tag_addr = 0;
    if (this-> debug_logging)
        printf("\x1B[36manalysing the memory stack, please wait ...\x1B[0m\n");
    do
    {
        if (this-> debug_logging) {
            printf("\x1B[32m%.2f%%\x1B[34m - analysing ...\x1B[0m\n", finished_precentage);            
        }

        /* ther is no point of analyzing the full stack so we will just analyze untill
        * we have found a \0 in the vector as all the vector elements get set to that
        * in the constructor.
        */
        if (this-> memory_stack[mem_stack_pos] == '\0') { 
            if (this-> debug_logging)
                printf("\x1B[31mbreaking from loop. next char equls '\\0'\x1B[0m\n");

            break; 
        }

        if (this-> memory_stack[mem_stack_pos] == this-> seporator_tags[sp_t::__mem_begin] && !is_value_str_type)
        {
            if (this-> debug_logging)
                printf("\x1B[35mfound beginning seporator as addr %ld\x1B[0m\n", mem_stack_pos);

            /* if the next char in the memory stack is the ending tag there is no point of this, and
            * it would cause errors as theres to middle tag so that means no name or value.
            */
            if (this-> memory_stack[mem_stack_pos + 1] != this-> seporator_tags[sp_t::__mem_end]) {
                if (! found_mem_begin_ts) {
                    if (this-> debug_logging)
                        printf("\x1B[33mmemory begin has been set at addr %ld\x1B[0m\n", mem_stack_pos);

                    found_mem_begin_ts = true;
                    mem_begin_ts_addr = mem_stack_pos;
                } else {
                    extra_mem_begin_tss ++;
                    waround ++;
                }
            }
        }

        if (found_mem_begin_ts) 
        {   
            /* check if the current pos in the memory stack equls the string beginning tag
            */
            if (this-> memory_stack[mem_stack_pos] == STR_BEGIN_TAG && found_mid_tag && !is_value_str_type) {
                if (this-> debug_logging)
                    printf("\x1B[35mfound string beginning tag at addr %ld\x1B[0m\n", mem_stack_pos);

                /* set to true, this will cause every other tag to be ignored e.g. memory beginning tag etc
                */
                is_value_str_type = true;
                str_begin_tag_addr = mem_stack_pos;
            }

            if (is_value_str_type && (this-> memory_stack[mem_stack_pos] == this-> memory_stack[str_begin_tag_addr]) && mem_stack_pos != str_begin_tag_addr) {
                if (this-> debug_logging)
                    printf("\x1B[35mfound string ending tag at addr %ld\x1B[0m\n", mem_stack_pos);

                /* when we change it back to false it will allow other tags to be interpreted
                */
                is_value_str_type = false;
                str_begin_tag_addr = 0;
            }
 
            if (this-> memory_stack[mem_stack_pos] == this-> seporator_tags[sp_t::__mem_middle] && !is_value_str_type) { 
                if (this-> debug_logging)
                    printf("found middle tag at addr %ld\n", mem_stack_pos);

                mid_tag_addr = mem_stack_pos;
                found_mid_tag = true;
            }

            // this is before the mid tag 
            if (this-> memory_stack[mem_stack_pos] == LIST_LEN_BTAG && !found_mid_tag) {
                if (this-> debug_logging)
                    printf("\x1B[35mfound list beginning tag at addr %ld\x1B[0m\n", mem_stack_pos);

                found_list_begin = true;
                lb_tag_addr = mem_stack_pos; 
            }
     
            if (found_mem_begin_ts && this-> memory_stack[mem_stack_pos] == MEM_LIST_TAG && !is_value_str_type) { 
                list_elength.resize(list_elength.size() + 1);
                list_elength(list_sep_tcount) = (mem_stack_pos - (list_sep_tcount == 0? mid_tag_addr : last_lpoint)) - 1;
 
                list_points.resize(list_points.size() + 1);
                list_points(list_sep_tcount) = list_sep_tcount == 0? mid_tag_addr : last_lpoint;

                list_sep_tcount ++;
                last_lpoint = mem_stack_pos;
            }

            if (this-> memory_stack[mem_stack_pos] == LIST_LEN_ETAG && ! found_mid_tag && !is_value_str_type) {
                if (found_list_begin && mem_stack_pos != lb_tag_addr) {
                    if (this-> debug_logging)
                        printf("\x1B[35mfound list ending tag at addr %ld\x1B[0m\n", mem_stack_pos);

                    found_list_begin = false;
                    is_list_type = true;
                    le_tag_addr = mem_stack_pos;
                } 
            }
        }

        if (this-> memory_stack[mem_stack_pos] == this-> seporator_tags[sp_t::__mem_end] && found_mem_begin_ts && mem_stack_pos != mem_begin_ts_addr && !is_value_str_type)
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
                this-> infomation.resize(this-> infomation.size() + 1);

                tagged_memory::__o & lo = this-> infomation[this-> memory_addrs.size() -1];
               
                lo.is_list_type = is_list_type;

                /* check if it a list type, if so then we can continue
                */
                if (lo.is_list_type) 
                {
                    // the extra 1 is because it startes to cout from 0
                    /* length of the tag from [ to ], we will need this later
                    * when extracting the name of the memory.
                    */
                    lo.len_of_tag = (le_tag_addr - lb_tag_addr) + 1;

                    /* because these vectors size change every time we find a list seporator,
                    * we are going to be off by 1 element. e.g. (: , , , , ;) list sep count = 4
                    * NOTE: need to explain.
                    */
                    list_elength.resize(list_elength.size() + 1);
                    list_points.resize(list_points.size() + 1);

                    list_elength(list_sep_tcount) = (mem_stack_pos - last_lpoint) - 1;
                    list_points(list_sep_tcount) = last_lpoint; 

                    /* swap the contents of the vectors.
                    */
                    lo.list_points.swap(list_points);
                    lo.list_elength.swap(list_elength); 
                
                    /* this is where we will store the data from [ to ] by without the []
                    */ 
                    char * list_len = static_cast<char *>(malloc((lo.len_of_tag - 2) * sizeof(char)));
                    memset(list_len, '\0', (lo.len_of_tag - 2) * sizeof(char));    
 
                    /* from [ to ] extract whats there.
                    */
                    std::size_t j = 0;
                    for (std::size_t p = lb_tag_addr + 1; p != le_tag_addr; p ++) {
                        list_len[j] = this-> memory_stack[p];
                        j++;
                    }

                    /* convirt to a integer
                    */
                    lo.len_of_list = atoi(list_len);

                    /* free the memory that we used */
                    std::free(list_len);
                }

                //printf("---------------------> %ld -- %ld\n", lo.len_of_tag, lo.len_of_list);
  
                ublas::vector<boost::array<boost::uint16_t, 2>>
                    ::iterator itor = this-> memory_addrs.begin();
            
                itor += (this-> memory_addrs.size() - 1);
             
                (* itor)[0] = mem_begin_ts_addr;
                (* itor)[1] = mem_end_ts_addr - 1;

                //std::cout << (* itor)[0] << ", " << (* itor)[1] << std::endl;
                is_list_type = false;
                found_mem_begin_ts = false;
                found_mem_end_ts = false;
                extra_mem_begin_tss = 0;
                waround = 0;
                le_tag_addr = 0;
                lb_tag_addr = 0;
                list_sep_tcount = 0;
                list_points.clear();
                list_points.resize(0);
                list_elength.clear();
                list_elength.resize(0);
                last_lpoint = 0;
                found_list_begin = false;
                found_mid_tag = false;  
                is_value_str_type = false;
                str_begin_tag_addr = 0;               
            }
        }

        finished_precentage += one_precent;
        mem_stack_pos++;
    } while (mem_stack_pos != this-> memory_stack.size());

    if (this-> debug_logging)
        printf("\x1B[36mfinished analysing.\x1B[0m\n");
}

void mdl::tagged_memory::dump_into_stack(ublas::vector<char> __memory)
{
    ublas::vector<boost::array<boost::uint16_t, 2>>
        ::iterator itor = this-> memory_addrs.begin();

    boost::uint16_t stack_begin_address = 0;

    if (this-> memory_addrs.size() != 0) {
        itor += this-> memory_addrs.size() - 1;
        stack_begin_address = (* itor)[1] + 1;
    }

    size_t length_of_string = __memory.size();

    for (size_t i = stack_begin_address; i != length_of_string + stack_begin_address; i ++)
        this-> memory_stack(i) = __memory[i - stack_begin_address];
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
