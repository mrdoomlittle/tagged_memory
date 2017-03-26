# include "tagged_memory.hpp"
/* NOTE: i am passing char * into const char *
* i dont know if it will cause any error/s.
*/

mdl::tagged_memory::tagged_memory(uint_t __allocated_memory,
    std::initializer_list<echar_t> __sep_tags, extra_opt_t __extra_opt, bool __debug_enabled)
{
    /* check if the list size is grater then 0 if so then
    * we can get the data in the list for that seporator else
    * we will use the default seporator. this is the same for
    * all but list pos number is incremented.
    */
    this-> sep_tags[0] = __sep_tags.size() > 0? *(__sep_tags.begin()) : MEM_BEGIN_TAG;
    this-> sep_tags[1] = __sep_tags.size() > 1? *(__sep_tags.begin() + 1) : MEM_MID_TAG;
    this-> sep_tags[2] = __sep_tags.size() > 2? *(__sep_tags.begin() + 2) : MEM_END_TAG;

    this-> debug_enabled = __debug_enabled == true? false : true;

    /* set all elements in vector to \0 or 0x0 as strlen
    * uses it to determine the length of a char * array
    */
    this-> mem_stack.resize(__allocated_memory);
    for (std::size_t i = 0; i != __allocated_memory; i ++) {
        if (!__extra_opt.fcontains_data && __extra_opt.fdirect_rw)
            this-> mem_stack_set(NULL_MEM, i);
		else
			if (! __extra_opt.fdirect_rw)
				this-> mem_stack_set(NULL_MEM, i);
	}

    this-> extra_opt = __extra_opt;
}

bool mdl::tagged_memory::get_mem_list_addrs(uint_t __mem_addr, std::size_t __mem_id, std::size_t *__list_addrs) {
	if (! this-> mem_info[__mem_id].is_list_type) return false;

	bool found_begin_tag = false;
	boost::int8_t any_error = false;
	uint_t begin_addr, end_addr;

	std::size_t len_of_name = this-> get_mem_name_len(__mem_addr, any_error);

	std::size_t o = __mem_addr + 1;
	while(o != (__mem_addr + len_of_name) + 1) {
		debug_print(this-> debug_enabled, "list addr char: %c\n", this-> mem_stack_get(o));

		if (this-> mem_stack_get(o) == LIST_LEN_BTAG && !found_begin_tag) {
			begin_addr = o;
//			printf("begin: %d\n", o);
			found_begin_tag = true;
		}

		if (this-> mem_stack_get(o) == LIST_LEN_ETAG && o != begin_addr && found_begin_tag) {
			end_addr = o;
			__list_addrs[0] = begin_addr;
			__list_addrs[1] = end_addr;
			return true;
		}

		o ++;
	}

	return false;
}

void mdl::tagged_memory::set_mem_list_len(echar_t const *__mem_name, std::size_t __list_len, boost::int8_t& __is_error) {
	bool is_error = false;
    uint_t mem_addr = this-> get_mem_addr(__mem_name, __is_error);
    std::size_t mem_id = this-> get_mem_id(mem_addr, __is_error);

	echar_t * list_len = to_string(__list_len);

	if (this-> mem_info[mem_id].len_of_list == __list_len) {
		__is_error = true;
		std::free(list_len);
		return;
	}

	std::size_t l1 = intlen(this-> mem_info[mem_id].len_of_list), l2 = intlen(__list_len);

	std::size_t list_addrs[2] = {0};
    if (!this-> get_mem_list_addrs(mem_addr, mem_id, list_addrs)) {
        __is_error = true;
		std::free(list_len);
        return;
    }

	if (this-> mem_info[mem_id].len_of_list > __list_len) {
		std::size_t amount_to_shrink = (this-> mem_info[mem_id].len_of_list - __list_len);
		this-> mem_info[mem_id].len_of_list -= amount_to_shrink;
	}
	else if (this-> mem_info[mem_id].len_of_list < __list_len) {
		std::size_t amount_to_grow = (__list_len - this-> mem_info[mem_id].len_of_list);
		this-> mem_info[mem_id].len_of_list += amount_to_grow;
	}

	std::size_t o = list_addrs[0] + 1, i = 0, change = 0, sv = 0;

	uint_t free_mem_ending = 0;
	std::size_t free_memory = this-> find_free_memory(this-> mem_addrs[mem_id][1], free_mem_ending);
	free_mem_ending --;

//	printf("%d free memory. id %d, addr: %d, ex: %d\n", free_memory, mem_id, mem_addr, list_addrs[0]);
	bool alloc_space = false, moved_stack = false, smove_amount = 0;
	if (l1 == l2) {
//		printf("same size in digits\n");
    	while(o != list_addrs[1]) {
        	this-> mem_stack_set(list_len[i], o);
       		i ++;
			o ++;
		}
		std::free(list_len);
		//std::free((char *)list_len);
		return;
	}
	else if (l1 < l2) {
		change = l2 - l1;
		if (RESIZE_TO_FIT) {
			while (o != list_addrs[1] + change) {
				if (this-> mem_stack_get(o) != LIST_LEN_ETAG) {
					this-> mem_stack_set(list_len[i], o);
					sv = o + 1;
				} else {
					this-> insert_into_mem_stack(list_len[i], sv, __is_error);
					if (!moved_stack) moved_stack = true;
					smove_amount ++;
				}

				o ++;
				i ++;
			}
		} else {
//			printf("working %d\n", change);
			while(o != list_addrs[1] + change) {

				if (this-> mem_stack_get(o) != LIST_LEN_ETAG) {
					this-> mem_stack_set(list_len[i], o);
				} else {
					if (!alloc_space) {
						if (free_memory < l2) {
							while (free_memory != l2-1) {
								this-> insert_into_mem_stack(BLANK_MEMORY, this-> mem_addrs[mem_id][1] + 2, __is_error);
								free_memory ++;
								if (!moved_stack) moved_stack = true;
			smove_amount ++;
							}
						}

						for (std::size_t l = (this-> mem_addrs[mem_id][1] + 1); l != list_addrs[1] -1; l --) {
//							printf("--: %c addr: %d to addr %d.\n", this-> mem_stack_get(l), l, l + change);
							this-> mem_stack_set(this-> mem_stack_get(l), (l + change));
							this-> mem_stack_set(BLANK_MEMORY, l);
						}

						alloc_space = true;
					}

					this-> mem_stack_set(list_len[i], o);
				}

				i ++;
            	o ++;
			}
		}

//		if (this-> debug_enabled)
//			std::cout << "change: " << change << std::endl;
		for (std::size_t c = 0; c != this-> mem_info[mem_id].list_points.size(); c ++) {
			this-> mem_info[mem_id].list_points[c] += change;
		}

		this-> mem_addrs[mem_id][1] += change;

		if (moved_stack) {
			for (std::size_t a = mem_id + 1; a != this-> mem_info.size(); a ++) {
				for (std::size_t c = 0; c != this-> mem_info[a].list_points.size(); c ++) {
					this-> mem_info[a].list_points[c] += smove_amount;
				}
			}

			for (std::size_t a = mem_id + 1; a != this-> mem_addrs.size(); a ++) {
				this-> mem_addrs[a][0] += smove_amount;
				this-> mem_addrs[a][1] += smove_amount;
			}
		}
	}
	else if (l1 > l2) {
		change = (l1 - l2);
		if (RESIZE_TO_FIT) {
			while(o != list_addrs[1]) {
				if (i == change) {
					for (std::size_t l = 0; l != l2; l ++)
						this-> mem_stack_set(list_len[l], (list_addrs[0] + 1) + l);

					break;
				}

				this-> uninsert_from_mem_stack(o, __is_error);
				if (!moved_stack) moved_stack = true;
                smove_amount ++;
				i ++;
				o ++;
			}

			while(this-> mem_stack_get(o-1) != LIST_LEN_ETAG) {
				this-> uninsert_from_mem_stack(o-1, __is_error);
				if (!moved_stack) moved_stack = true;
                smove_amount ++;
			}

		} else {
			while(o != list_addrs[1] - change) {
				this-> mem_stack_set(list_len[i], o);
				i ++;
                o ++;
			}

			while (this-> mem_stack_get(o) != LIST_LEN_ETAG) {
				this-> mem_stack_set(BLANK_MEMORY, o);
				o ++;
			}
		}

		if (RESIZE_TO_FIT) {

			for (std::size_t c = mem_id + 1; c != this-> mem_info[mem_id].list_points.size(); c ++) {
    	        this-> mem_info[mem_id].list_points[c] -= change;
	       	}

			if (moved_stack) {
				for (std::size_t a = mem_id + 1; a != this-> mem_info.size(); a ++) {
        	        for (std::size_t c = 0; c != this-> mem_info[a].list_points.size(); c ++) {
            	        this-> mem_info[a].list_points[c] -= smove_amount;
	               	}
	            }

	            for (std::size_t a = mem_id + 1; a != this-> mem_addrs.size(); a ++) {
    	            this-> mem_addrs[a][0] -= smove_amount;
        	        this-> mem_addrs[a][1] -= smove_amount;
     	       	}
			}
		}
	}

	std::free(list_len);
}

std::size_t mdl::tagged_memory::get_mem_list_len(echar_t const *__mem_name, bool __cached_ver, boost::int8_t& __is_error) {
    uint_t mem_addr = this-> get_mem_addr(__mem_name, __is_error);
    std::size_t mem_id = this-> get_mem_id(mem_id, __is_error);

	if (__cached_ver) return this-> mem_info[mem_id].len_of_list;

	std::size_t list_addrs[2] = {0};

	if (!this-> get_mem_list_addrs(mem_addr, mem_id, list_addrs)) {
		__is_error = true;
		return 0;
	}

	std::size_t len_buff_size = ((list_addrs[1] - 1) - list_addrs[0]) + 1;

	echar_t * len_buff = static_cast<echar_t *>(malloc(len_buff_size * sizeof(echar_t)));
	memset(len_buff, '\0', len_buff_size * sizeof(echar_t));

	std::size_t o = list_addrs[0] + 1, i = 0;
	while(o != list_addrs[1]) {
		len_buff[i] = this-> mem_stack_get(o);
		i ++;
		o ++;
	}

	std::size_t list_len = atoi(len_buff);
	std::free(len_buff);

	return list_len;
}

void mdl::tagged_memory::get_addr_spoints(uint_t __addr, std::size_t& __mem_id, std::size_t& __list_id, boost::int8_t& __is_error) {
	std::size_t mem_id = 0, list_id = 0;
	bool found_mem_id = false;

    for (std::size_t o = 0; o != this-> mem_addrs.size(); o ++) {
        if ((__addr >= this-> mem_addrs[o][0] && __addr <= (this-> mem_addrs[o][1] + 1)) && !found_mem_id) {
            mem_id = o;
            found_mem_id = true;
        }

        if (this-> mem_info[o].is_list_type) {
        if (found_mem_id) {
            for (std::size_t i = this-> mem_info[o].list_points.size() - 1; i != 0; i --) {
//				printf("%d >= %d\n", __addr, this-> mem_info[o].list_points[i]);
                if (__addr >= this-> mem_info[o].list_points[i]) {
//					printf("found list id - %d\n", i);
list_id = i;
break;
                }
            }
            break;
        }
        } else {
            if (found_mem_id) break;
        }
    }

	if (!found_mem_id) {
		__is_error = true;
	}

	__mem_id = mem_id;
	__list_id = list_id;
}

void mdl::tagged_memory::mem_stack_insert(echar_t __mem, uint_t __addr) {
	std::size_t mem_id, list_id;

	boost::int8_t is_error = false;
	bool hard_insert = false;
	this-> get_addr_spoints(__addr, mem_id, list_id, is_error);

	debug_print(this-> debug_enabled, "mem_id: %d, list_id: %d, mem_addr: %d\n", mem_id, list_id, __addr);

	if (is_error) return;

	if (RESIZE_TO_FIT) {
		// need to work on this
	} else {
		if (this-> mem_stack_get(this-> mem_addrs[mem_id][1]+2) == BLANK_MEMORY) {
			std::size_t o = this-> mem_addrs[mem_id][1] + 1;
			while(o != __addr-1) {

				this-> mem_stack_set(this-> mem_stack_get(o), o + 1);
				this-> mem_stack_set(BLANK_MEMORY, o);
				o --;
			}
		} else {
			if (!hard_insert) hard_insert = true;
			this-> insert_into_mem_stack(__mem, __addr, is_error);
		}
	}

	if (this-> mem_info[mem_id].list_points[list_id] == __addr)
		for (std::size_t o = list_id; o != this-> mem_info[mem_id].list_points.size(); o ++)
			this-> mem_info[mem_id].list_points[o] ++;
	else
		for (std::size_t o = list_id + 1; o != this-> mem_info[mem_id].list_points.size(); o ++)
			this-> mem_info[mem_id].list_points[o] ++;

	if (this-> mem_addrs[mem_id][0] == __addr) {
		this-> mem_addrs[mem_id][0] ++;
	} else {
        this-> mem_addrs[mem_id][1] ++;
	}

	if (hard_insert) {
		for (std::size_t l = mem_id + 1; l != this-> mem_info.size(); l ++) {
			this-> mem_addrs[l][0] ++;
			this-> mem_addrs[l][1] ++;
			for (std::size_t o = 0; o != this-> mem_info[l].list_points.size(); o ++)
				this-> mem_info[l].list_points[o] ++;
		}
	}

	if (!hard_insert) this-> mem_stack_set(__mem, __addr);
}

void mdl::tagged_memory::mem_stack_uninsert(uint_t __addr) {
	std::size_t mem_id, list_id;
	boost::int8_t is_error = false;
    bool hard_uninsert = false;
    this-> get_addr_spoints(__addr, mem_id, list_id, is_error);

    if (is_error) return;

	if (RESIZE_TO_FIT) {
        // need to work on this
    } else {
		std::size_t o = __addr;
		while(o != this-> mem_addrs[mem_id][1] + 1) {
			this-> mem_stack_set(this-> mem_stack_get(o + 1), o);
			this-> mem_stack_set(BLANK_MEMORY, o + 1);
			o ++;
		}
    }

	for (std::size_t o = list_id; o != this-> mem_info[mem_id].list_points.size(); o ++)
        this-> mem_info[mem_id].list_points[o] --;

    if (this-> mem_addrs[mem_id][0] == __addr) {
        this-> mem_addrs[mem_id][0] --;
    } else {
        this-> mem_addrs[mem_id][1] --;
    }
}

void mdl::tagged_memory::add_to_list(echar_t const *__mem_name, std::size_t __amount, boost::int8_t& __is_error) {
	uint_t mem_addr = this-> get_mem_addr(__mem_name, __is_error);
    std::size_t mem_id = this-> get_mem_id(mem_id, __is_error);

	if (!this-> mem_info[mem_id].is_list_type) return;

	this-> set_mem_list_len(__mem_name, this-> mem_info[mem_id].len_of_list + __amount, __is_error);

	uint_t to_insert_addr = this-> mem_addrs[mem_id][1] + 1;

	for (std::size_t o = 0; o != __amount; o ++)
		this-> mem_stack_insert(MEM_LIST_TAG, to_insert_addr);

	std::size_t pre_lsize = this-> mem_info[mem_id].list_points.size();

    this-> mem_info[mem_id].list_points.resize(
        this-> mem_info[mem_id].list_points.size() + __amount);

    this-> mem_info[mem_id].list_elength.resize(
        this-> mem_info[mem_id].list_elength.size() + __amount);

	std::size_t l = pre_lsize;
	for (std::size_t o = 0; o != __amount; o ++, l ++) {
		this-> mem_info[mem_id].list_points[l] = to_insert_addr + o;
        this-> mem_info[mem_id].list_elength[l] = 0;
	}
}

void mdl::tagged_memory::del_from_list(echar_t const *__mem_name, std::size_t __list_id, boost::int8_t& __is_error) {
	uint_t mem_addr = this-> get_mem_addr(__mem_name, __is_error);
    std::size_t mem_id = this-> get_mem_id(mem_id, __is_error);

	if (!this-> mem_info[mem_id].is_list_type) return;

	this-> set_mem_list_len(__mem_name, this-> mem_info[mem_id].len_of_list - 1, __is_error);
}

void mdl::tagged_memory::mem_stack_set(echar_t __mem, uint_t __mem_addr) {
    if (extra_opt.fdirect_rw) {
        FILE *ofile = fopen(DEF_MSTACK_FILE, "r+b");

        fseek(ofile, (__mem_addr * sizeof(boost::uint8_t)), SEEK_SET);

        fwrite(&__mem, sizeof(boost::uint8_t), 1, ofile);

        rewind(ofile);

        fclose(ofile);
    } else
        this-> mem_stack(__mem_addr) = __mem;
}

mdl::echar_t mdl::tagged_memory::mem_stack_get(uint_t __mem_addr) {
    if (extra_opt.fdirect_rw) {
        FILE *ifile = fopen(DEF_MSTACK_FILE, "rb");

        fseek(ifile, (__mem_addr * sizeof(boost::uint8_t)), SEEK_SET);

        static echar_t temp = '\0';

        fread(&temp, sizeof(boost::uint8_t), 1, ifile);

        rewind(ifile);

        fclose(ifile);

        return temp;
    } else
        return this-> mem_stack[__mem_addr];

    return '\0';
}

void mdl::tagged_memory::save_mem_addrs() {
    this-> save_mem_addrs("", extra_opt.mem_addrs_file != '\0'? extra_opt.mem_addrs_file : DEF_MADDRS_FILE);
}

void mdl::tagged_memory::save_mem_addrs(echar_t const *__file_path, echar_t const *__file_name) {
    echar_t *full_path = this-> combine_strings(__file_path, __file_name);

    FILE *ofile = fopen(full_path, "wb");

    for (std::size_t i = 0; i != this-> mem_addrs.size(); i ++) {
        // beginning address
        fwrite(&this-> mem_addrs[i][0], sizeof(uint_t), 1, ofile);
        // ending address
        fwrite(&this-> mem_addrs[i][1], sizeof(uint_t), 1, ofile);
    }

    fclose(ofile);
    std::free(full_path);
}

void mdl::tagged_memory::load_mem_addrs() {
    this-> load_mem_addrs("", extra_opt.mem_addrs_file != '\0'? extra_opt.mem_addrs_file : DEF_MADDRS_FILE);
}

void mdl::tagged_memory::load_mem_addrs(echar_t const *__file_path, echar_t const *__file_name) {
    echar_t *full_path = this-> combine_strings(__file_path, __file_name);

    FILE *ifile = fopen(full_path, "rb");

    long file_size = 0;

    fseek(ifile, 0, SEEK_END);

    file_size = ftell(ifile);

    rewind(ifile);

    std::size_t addr_count = (file_size / (sizeof(uint_t) * 2));

    this-> mem_addrs.resize(addr_count);

    for (std::size_t i = 0; i != this-> mem_addrs.size(); i ++) {
        // beginning address
        fread(&this-> mem_addrs[i][0], sizeof(uint_t), 1, ifile);
        // ending address
        fread(&this-> mem_addrs[i][1], sizeof(uint_t), 1, ifile);
    }

    fclose(ifile);
    std::free(full_path);
}

void mdl::tagged_memory::save_mem_info() {
    this-> save_mem_info("", extra_opt.mem_info_file != '\0'? extra_opt.mem_info_file : DEF_MINFO_FILE);
}

void mdl::tagged_memory::save_mem_info(echar_t const *__file_path, echar_t const *__file_name) {
    echar_t *full_path = this-> combine_strings(__file_path, __file_name);
    echar_t *efile_name = this-> combine_strings(".", __file_name);

    echar_t *efull_path = this-> combine_strings(__file_path, efile_name);
    std::free(efile_name);

    FILE *ofile = fopen(full_path, "wb");
    FILE *eofile = fopen(efull_path, "wb");

	if (this-> debug_enabled)
    	std::cout << sizeof(tagged_memory::mem_info_t) << std::endl;

    arc ac(ofile, 'w');
    for (std::size_t i = 0; i != this-> mem_info.size(); i ++) {
//        fwrite(&this-> mem_info[i], sizeof(tagged_memory::mem_info_t), 1, ofile);

        this-> mem_info[i].vec_lens[0] = this-> mem_info[i].list_elength.size();
        this-> mem_info[i].vec_lens[1] = this-> mem_info[i].list_points.size();
        this-> mem_info[i].__arc(ac);

        for (std::size_t o = 0; o != this-> mem_info[i].list_elength.size(); o ++) {
            fwrite(&this-> mem_info[i].list_elength[o], sizeof(uint_t), 1, eofile);
        }

        for (std::size_t o = 0; o != this-> mem_info[i].list_points.size(); o ++) {
            fwrite(&this-> mem_info[i].list_points[o], sizeof(uint_t), 1, eofile);
        }
    }

    fclose(ofile);
    fclose(eofile);
    std::free(full_path);
    std::free(efull_path);
}

void mdl::tagged_memory::load_mem_info() {
    this-> load_mem_info("", extra_opt.mem_info_file != '\0'? extra_opt.mem_info_file : DEF_MINFO_FILE);
}

void mdl::tagged_memory::load_mem_info(echar_t const *__file_path, echar_t const *__file_name) {
    echar_t *full_path = this-> combine_strings(__file_path, __file_name);
    echar_t *efile_name = this-> combine_strings(".", __file_name);

    echar_t *efull_path = this-> combine_strings(__file_path, efile_name);
    std::free(efile_name);

    FILE *ifile = fopen(full_path, "rb");
    FILE *eifile = fopen(efull_path, "rb");

    if (ifile == NULL || eifile == NULL)
        printf("error.\n");

    long file_size = 0;

    fseek(ifile, 0, SEEK_END);

    file_size = ftell(ifile);

    rewind(ifile);

    arc ac(ifile, 's');
    // get a example size
    this-> mem_info.resize(1);
    this-> mem_info[0].__arc(ac);
    std::size_t es = ac.size;

    std::size_t mem_info_vc = (file_size / es);

    this-> mem_info.resize(mem_info_vc);

    // change to read
    ac | 'r';

    for (std::size_t i = 0; i != mem_info_vc; i ++) {
        //fread(&this-> mem_info[i], sizeof(tagged_memory::mem_info_t), 1, ifile);
        this-> mem_info[i].__arc(ac);

        std::size_t s = 0;
        s = this-> mem_info[i].vec_lens[0];
        this-> mem_info[i].list_elength.resize(s);

        for (std::size_t o = 0; o != s; o ++) {
            fread(&this-> mem_info[i].list_elength[o], sizeof(uint_t), 1, eifile);
        }

        s = 0;
        s = this-> mem_info[i].vec_lens[1];
        this-> mem_info[i].list_points.resize(s);

        for (std::size_t o = 0; o != s; o ++) {
            fread(&this-> mem_info[i].list_points[o], sizeof(uint_t), 1, eifile);
        }
    }

    fclose(ifile);
    fclose(eifile);
    std::free(full_path);
    std::free(efull_path);
}

void mdl::tagged_memory::set_mem_val(echar_t const *__name, echar_t const *__value, id_cache_t& __id_cache, boost::int8_t& __any_error, uint_t __list_addr)
{
    /* NOTE: when setting a list element it does not directly edit the stack, but gets the value of the
    * variable changes it and then use the set_mem_val without list enabled
    */

    /* get the current value of the pice of memory
    */
	echar_t *tmp = nullptr;

	/* calulate the length of the string
    */
	std::size_t nx_length = 0;

	/* get the memory location e.g. the beginning address
    */
    uint_t mem_location = 0;

	/* how many itorator ++ is there needed for that pice of memory
    */
    std::size_t ad = 0;

	nx_length = strlen(__value);

	tmp = this-> get_mem_val(__name, null_idc, __any_error, 0, true);

	nx_length = strlen(__value);

	mem_location = this-> get_mem_addr(__name, __any_error);

	ad = find_mem_addr_it_pos(mem_location, __any_error);
	if (__list_addr < 0 || __list_addr >= this-> mem_info[ad].len_of_list) {
		__any_error = TMEM_FAILURE;
		return;
	}

	/* get the length of the current value not the one passed thru function
    */
    std::size_t tlen = strlen(tmp);


    /* what is the length of the list
    */
    std::size_t curr_length = this-> mem_info[ad].list_elength[__list_addr];

    std::size_t change = 0;

    /* get the starting addr of the list element
    */
    uint_t start = this-> mem_info[ad].list_points[__list_addr];

    uint_t after_len = 0;

    std::size_t o = 0;
    if (curr_length == nx_length) {
        for (std::size_t i = start+1; i != start + curr_length +1; i ++) {
            this-> mem_stack_set(__value[o], i);
            o ++;
        }

		if (__id_cache.caching) {
			if (__id_cache.call_count == __id_cache.call_amount) {
				__id_cache.call_count = 0;
				std::free(tmp);
			 } else
				__id_cache.call_count += 1;
		} else {

        std::free(tmp);
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

    echar_t *aft = static_cast<echar_t *>(malloc(after_len * sizeof(echar_t)));
    memset(aft, '\0', after_len * sizeof(echar_t));

    uint_t nmlen = this-> get_mem_name_len(mem_location, __any_error);

    bool found_addr = false;
    std::size_t extra = 0, el = 0;

    if (g) extra = change;
    if (l) el = change;

    std::size_t j = 0, skip = 0; o = 0;
    for (std::size_t i = mem_location + nmlen; i != ((this-> mem_addrs[ad][1] + extra) - el) - 1; i ++) {
        if (i == (start - 1) || found_addr) {
            aft[o] = __value[j];
            skip = change;

            if (found_addr == false && j == 0) found_addr = true;
            if (j == nx_length - 1) found_addr = false; else j ++;
        } else {
            if (g) aft[o] = tmp[o - skip];
            if (l) aft[o] = tmp[o + skip];
        }

        o ++;
    }

	//printf("hello");
    this-> set_mem_val(__name, aft, __id_cache, __any_error);

    for (std::size_t i = __list_addr + 1; i != this-> mem_info[ad].list_points.size(); i ++) {
        if (g) this-> mem_info[ad].list_points(i) += change;
        if (l) this-> mem_info[ad].list_points(i) -= change;
    }

    if (g) this-> mem_info[ad].list_elength(__list_addr) += change;
    if (l) this-> mem_info[ad].list_elength(__list_addr) -= change;
//	if (!__id_cache.caching)
    	 std::free(tmp);
//	else {
//		if (__id_cache.call_count == 0) std::free(tmp);
//	}

    std::free(aft);
}

bool mdl::tagged_memory::does_mem_name_exist(echar_t const *__mem_name, boost::int8_t& __error)
{
    ublas::vector<boost::array<uint_t, 2>>::iterator it = this-> mem_addrs.begin();

    /* iterate thru the address vector and compare the name of each variable until there's a match.
    */
    for (; it != this-> mem_addrs.end() ; ++ it)
        if (this-> compare_strings(__mem_name, this-> get_mem_name((* it)[0], __error))) return true;

    /* if no match was found then return 'false' */
    return false;
}

bool mdl::tagged_memory::compare_mem_values(echar_t const *__mem_name_0, echar_t const *__mem_name_1, boost::int8_t& __error)
{
	echar_t *mem_value_0 = this-> get_mem_val(__mem_name_0, null_idc, __error, 0, true);
	echar_t *mem_value_1 = this-> get_mem_val(__mem_name_1, null_idc, __error, 0, true);

    /* use the compare strings function to comapre the mem values */
	bool compare_result = this-> compare_strings(mem_value_0, mem_value_1);

	/* free the memory that the mem values used.
	*/
	std::free(mem_value_0);
	std::free(mem_value_1);

	// return the result.
	return compare_result;
}

bool mdl::tagged_memory::compare_strings(echar_t const *__string_0, echar_t const *__string_1)
{
    /* get the length of each string as we will need them later */
    std::size_t len_of_string_0 = strlen(__string_0);
    std::size_t len_of_string_1 = strlen(__string_1);

    /* this will allow us to keep track on how many chars we have matched */
    std::size_t matching_char_set = 0;

    /* if there not the same length then there not going to be the same so return false */
    if (len_of_string_0 != len_of_string_1) return false;

    /* compare each char of both string, if they match then add 1 to 'matching_char_set'
    * if theres no match move on to the next set of chars.
    */
    for (std::size_t i = 0 ; i != len_of_string_0 ; i ++) {
        if (__string_0[i] == __string_1[i])
            matching_char_set ++;
        else
            /* if theres a char thats does not match then we already know
            * the string isn't going to be the same.
            */
            break;

        /*  check the end of the string for no match
		* NOTE: this is hear to speed things up
        */
        std::size_t o = (len_of_string_0 - i);
        if (__string_0[o] != __string_1[o]) break;
    }

    /* if we have matched the same amount of chars compared to the length.
    * then matching was a success, so return true
    */
    if (matching_char_set == len_of_string_0) return true;

    // return false for defualt if matching failed
    return false;
}

mdl::echar_t *mdl::tagged_memory::dump_stack_memory(bool __return)
{
    /* allocate memory <- this is where we will store the stack memory
    */
    echar_t *re = static_cast<echar_t *>(malloc(this-> mem_stack.size()));
    memset(re, '\0', this-> mem_stack.size());

    /* if debugging is enabled then we will print the stack to the terminal */
    for (size_t i = 0; i != this-> mem_stack.size(); i ++) {
        if (this-> mem_stack_get(i) == '\0') break;
        re[i] = this-> mem_stack_get(i);

    	debug_print(this-> debug_enabled, "\x1B[0m%c\x1B[32m|\x1B[0m", this-> mem_stack_get(i));
    }

	debug_print(this-> debug_enabled, "\n");

    /* if we are only wanting it to print to the terminal then __return = false
    */
    if (__return == false) {
        std::free(re);
        return nullptr;
    }

    return re;
}

void mdl::tagged_memory::load_mem_stack_from_file(echar_t const *__file_path, echar_t const *__file_name)
{
    std::string each_line;
    echar_t *full_path = this-> combine_strings(__file_path, __file_name);

    std::ifstream ifile(full_path);
    std::free(full_path);

    size_t point = 0, o = 0;

    debug_print(this-> debug_enabled, "\x1B[34mloading memory stack from '%s'\x1B[0m\n", __file_path);

    while ( std::getline (ifile, each_line) ) {
        echar_t const *tmp = each_line.c_str();
        for (size_t i = point; i != point + strlen(tmp); i++) {
            //this-> mem_stack(i) = tmp[o];
            this-> mem_stack_set(tmp[o], i);
            o++;
        }

        o = 0;
        point += strlen(tmp);
    }
}

mdl::echar_t *mdl::tagged_memory::create_mem_tag(echar_t const *__name, echar_t const *__value)
{
    std::size_t length_of_name = strlen(__name);
    std::size_t length_of_value = strlen(__value);

    std::size_t total_size = (length_of_name + length_of_value + 3/* this includes the ending, starting and mid seporator*/);

    total_size ++;

    echar_t *tmp = static_cast<echar_t *>(malloc(total_size * sizeof(echar_t)));
    memset(tmp, '\0', total_size * sizeof(echar_t));

    std::size_t current_addr = 0, o = 0, q = 0;
    tmp[current_addr] = this-> sep_tags[sp_t::__mem_begin];

    current_addr ++;

    for (std::size_t i = current_addr; i != (current_addr + length_of_name); i ++) {
        tmp[i] = __name[o];
        o ++;
        q ++;
    } current_addr += q;

    tmp[current_addr] = this-> sep_tags[sp_t::__mem_middle];
    current_addr ++;

    o = 0; q = 0;
    for (std::size_t i = current_addr; i != (current_addr + length_of_value); i ++) {
        tmp[i] = __value[o];
        o ++;
        q ++;
    } current_addr += q;

    tmp[current_addr] = this-> sep_tags[sp_t::__mem_end];

    return tmp;
}

mdl::echar_t *mdl::tagged_memory::get_mem_val(echar_t const *__name, id_cache_t& __id_cache, boost::int8_t& __any_error, uint_t __list_addr, bool __no_list)
{
	if (ILLEGAL_CHARS) {
		if (this-> is_there_illegals(__name)) {
			fprintf(stderr, "can't get mem val, because the name contains illegals.\n");
			__any_error = TMEM_FAILURE;
			return nullptr;
		}
	}

	uint_t mem_addr = 0;
	std::size_t mem_id = 0, mem_nme_len = 0;

	echar_t *mem_name = nullptr;

	std::size_t ltaddr_b = 0, ltaddr_e = 0;

	bool fb_list = false, fe_list = false;

	std::size_t list_pointer = 0;
	if (__no_list) {
        for (std::size_t i = 0; i != strlen(__name); i++ ) {
            if (__name[i] == LIST_LEN_BTAG) {
                fb_list = true;
                ltaddr_b = i;
            }

			if (__name[i] == LIST_LEN_ETAG) {
				if (i != ltaddr_b && fb_list) {
					fe_list = true;
					ltaddr_e = i;
                }
			}
		}
	}

	if ((fb_list && fe_list) && __no_list)
		mem_name = this-> extract_list_addr(__name, list_pointer, ltaddr_b, ltaddr_e);
	else
		mem_name = const_cast<char *>(__name);

	mem_addr = this-> get_mem_addr(mem_name, __any_error);
    mem_id = this-> find_mem_addr_it_pos(mem_addr, __any_error);
    mem_nme_len = this-> get_mem_name_len(mem_addr, __any_error);

	if (!__no_list) {
		if (__list_addr < 0 || __list_addr >= this-> mem_info[mem_id].len_of_list) {
			__any_error = true;
			return nullptr;
		}
	}

	if ((fb_list && fe_list) && __no_list) {
		echar_t *tmp = this-> get_mem_val(mem_name, __id_cache, __any_error, list_pointer, false);
		std::free(mem_name);
		return tmp;
	}

	return this-> get_mem_val(mem_addr, mem_id, mem_nme_len, __any_error, __list_addr, __no_list);
}

void mdl::tagged_memory::add_mem_tag(echar_t const *__name, echar_t const *__value, std::size_t __null_space, boost::int8_t& __error)
{
	if (this-> does_mem_name_exist(__name, __error)) {
		fprintf(stderr, "can't add mem tag to stack, because it already exists!\n");
		return;
	}

    uint_t insert_addr = 0;

    ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.end();

    if (this-> mem_addrs.size() != 0) {
        --itor;
        insert_addr = ((* itor)[1] + 2);
    }

    debug_print(this-> debug_enabled, "adding memory tag at addr %d\n", insert_addr);

    echar_t *tmp = this-> create_mem_tag(__name, __value);

    std::size_t length_of_mem = strlen(tmp);

    this-> mem_addrs.resize(this-> mem_addrs.size() + 1);
    this-> mem_info.resize(this-> mem_info.size() + 1);

    tagged_memory::mem_info_t& lo = this-> mem_info[this-> mem_addrs.size() -1];

    ublas::vector<uint_t> list_elength;
    ublas::vector<uint_t> list_points;

    bool got_list_be_tags = false;
    bool found_list_begin = false;
    uint_t lb_tag_addr = 0;
    bool found_list_end = false;
    uint_t le_tag_addr = 0;
    bool found_middle_tag = false;
    std::size_t list_sep_tcount = 0;
    uint_t mid_tag_addr = 0;
    uint_t last_lpoint = 0;
    bool is_value_str_type = false;
    uint_t str_bt_addr = 0;
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
                echar_t *len_of_list = static_cast<echar_t *>(malloc((lo.len_of_tag - 1) * sizeof(echar_t)));
                memset(len_of_list, '\0', (lo.len_of_tag - 1) * sizeof(echar_t));
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
            if (tmp[i] == this-> sep_tags[sp_t::__mem_middle] && !found_middle_tag) {
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

    itor = this-> mem_addrs.end();

    --itor;

    (* itor)[0] = insert_addr;
    (* itor)[1] = (insert_addr + length_of_mem) - 2;/*this seems to leave the ending tag at the end of the value output, but this fixed might need to look into it*/
    std::size_t o = 0;
    for (std::size_t i = insert_addr; i != insert_addr + length_of_mem; i ++) {
        //this-> mem_stack(i) = tmp[o];
        this-> mem_stack_set(tmp[o], i);
        o++;
    }

    for (std::size_t i = 0; i != list_points.size(); i ++) {
        list_points(i) = (insert_addr + list_points[i]);
    }

    lo.list_elength.swap(list_elength);
    lo.list_points.swap(list_points);
    std::size_t st = insert_addr + length_of_mem;

    for (std::size_t i = st; i != st + __null_space; i ++)
        this-> mem_stack_set(BLANK_MEMORY, i);

    std::free(tmp);

   //printf("%s\n", this-> create_mem_tag("test", "test"));
}


mdl::echar_t *mdl::tagged_memory::combine_strings(echar_t const *__string_0, echar_t const *__string_1) {
    std::size_t string_0_len = strlen(__string_0);
    std::size_t string_1_len = strlen(__string_1);
    std::size_t full_len = (string_0_len + string_1_len);

    echar_t *tmp = static_cast<echar_t *>(std::malloc(full_len+1 * sizeof(echar_t)));
    memset(tmp, '\0', full_len+1 * sizeof(echar_t));

    std::size_t m = 0;
    for (std::size_t i = 0; i != string_0_len; i ++) {
        tmp[m] = __string_0[i];
        m ++;
    }

    for (std::size_t i = 0; i != string_1_len; i ++) {
        tmp[m] = __string_1[i];
        m ++;
    }

    return tmp;
}

void mdl::tagged_memory::save_mem_stack_to_file(echar_t const *__file_path, echar_t const *__file_name)
{
    debug_print(this-> debug_enabled, "\x1B[34msaving memory stack to '%s'\x1B[0m\n", __file_path);

    std::ofstream ofile;
    echar_t *full_path = this-> combine_strings(__file_path, __file_name);
    ofile.open(full_path);

    std::free(full_path);

    if (ofile.is_open()) {
        debug_print(this-> debug_enabled, "\x1B[32m -- successfully opened file.\x1B[0m\n");

    } else {
        debug_print(this-> debug_enabled, "\x1B[31m -- failed to open file.\x1B[0m\n");
    }

    echar_t *memory = dump_stack_memory(true);

    ofile << memory;
    ofile.close();

    std::free(memory);
}

std::size_t mdl::tagged_memory::find_mem_addr_it_pos(uint_t __addr, boost::int8_t& __error) {
    ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

    std::size_t iterated_amount = 0;
    for (;itor != this-> mem_addrs.end(); ++itor) {
        if ((* itor)[0] == __addr) return iterated_amount;
        iterated_amount ++;
    }

    __error = true;
    return 0;
}

std::size_t mdl::tagged_memory::get_mem_name_len(uint_t __addr, boost::int8_t& __error) {
    if (this-> mem_stack_get(__addr) != this-> sep_tags[sp_t::__mem_begin]) {__error = true; return 0;}

    std::size_t length_of_name = 0;
    for(std::size_t i = (__addr + 1);; i++) {
        if (this-> mem_stack_get(i) == this-> sep_tags[sp_t::__mem_middle]) break;
        length_of_name ++;
    }

    return length_of_name;
}

bool mdl::tagged_memory::is_mem_addr_ok(uint_t __addr) {
    ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

    debug_print(this-> debug_enabled, "\x1B[36mchecking if memory address is ok.\x1B[0m\n");

    for (; itor != this-> mem_addrs.end(); ++itor) {
        if ((* itor)[0] == __addr) {
            debug_print(this-> debug_enabled, "\x1B[35msuccessfully found address %d\x1B[0m\n", __addr);

            return true;
        }
    }

    debug_print(this-> debug_enabled, "\x1B[35mfailed to find address %d\x1B[0m\n", __addr);

    return false;
}

mdl::echar_t *mdl::tagged_memory::get_mem_name(uint_t __addr, boost::int8_t& __error)
{
    if (! this-> is_mem_addr_ok(__addr)) { __error = true; return '\0';}
    ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

    std::size_t ad = find_mem_addr_it_pos(__addr, __error);

    itor += ad;

    std::size_t length_of_name = get_mem_name_len(__addr, __error);

    debug_print(this-> debug_enabled, "\x1B[37mlen of mem name at addr %d is %ld bytes\x1B[0m\n", __addr, length_of_name);

    // havent tested this
    echar_t *__name = static_cast<echar_t *>(malloc((length_of_name + 1) * sizeof(echar_t)));
	memset(__name, '\0', (length_of_name + 1) * sizeof(echar_t));

    std::size_t reduction = 0;

    if (this-> mem_info[ad].is_list_type)
        reduction = this-> mem_info[ad].len_of_tag;

    std::size_t o = 0;
    for (std::size_t i = (__addr + 1) ; i != ((__addr + 1) + length_of_name) - reduction; i ++) {
        __name[o] = this-> mem_stack_get(i);
        o ++;
    }

    __name[o] = '\0';

    return __name;
}

void mdl::tagged_memory::insert_into_mem_stack(echar_t __mem, uint_t __addr, boost::int8_t& __error)
{
    debug_print(this-> debug_enabled, "\x1B[33minserting '%c' into stack at addr %d\x1B[0m\n", __mem, __addr);

    for (std::size_t i = (this-> mem_stack.size() - 1); i != __addr; i --) {
        if (this-> mem_stack_get(i - 1) == '\0') continue;
        debug_print(this-> debug_enabled, "\x1B[35mmoving memory at addr %ld to %ld\x1B[0m\n", (i - 1), i);

        this-> mem_stack_set(this-> mem_stack_get(i - 1), i);
        this-> mem_stack_set(BLANK_MEMORY, i - 1);
    }

    this-> mem_stack_set(__mem, __addr);
}

void mdl::tagged_memory::uninsert_from_mem_stack(uint_t __addr, boost::int8_t& __error)
{
    debug_print(this-> debug_enabled, "\x1B[33muninserting '%c' from stack at addr %d\x1B[0m\n", this-> mem_stack_get(__addr), __addr);

    this-> mem_stack(__addr) = ' ';

    for (size_t i = __addr; i != (this-> mem_stack.size() - 1); i ++) {
        if (this-> mem_stack_get(i) == '\0') continue;

        debug_print(this-> debug_enabled, "\x1B[35mmoving memory at addr %ld to %ld\x1B[0m\n", (i + 1), i);

        this-> mem_stack_set(this-> mem_stack_get(i + 1), i);
    }
}


mdl::uint_t mdl::tagged_memory::get_mem_addr(echar_t const *__name, boost::int8_t& __error)
{
    std::size_t length_of_name = strlen(__name);

    std::size_t mem_addrs_pos = 0;
    std::size_t mem_match_count = 0;
    std::size_t name_char_pos = 0;

    ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

    std::size_t reduction = 0;

    while(mem_addrs_pos != mem_addrs.size())
    {
        if (this-> mem_info[mem_addrs_pos].is_list_type)
            reduction = this-> mem_info[mem_addrs_pos].len_of_tag;
        else
            reduction = 0;

        debug_print(this-> debug_enabled, "\x1B[34msearching mem addr book at id %ld for '%s'\x1B[0m\n", mem_addrs_pos, __name);
        for (size_t i = ((* itor)[0] + 1); i != (* itor)[1] - reduction; i ++) {
            /* if the current memory stack char equals the current name char
            * then add 1 decamal place
            */

            debug_print(this-> debug_enabled, "\x1B[33mmatching char '%c' and '%c'\x1B[0m\n", this-> mem_stack_get(i), __name[name_char_pos]);
            if (this-> mem_stack_get(i) == __name[name_char_pos]) {
                mem_match_count ++;
                debug_print(this-> debug_enabled, "\x1B[42mmatch found at addr %ld\x1B[0m\n", i);
            } else {
                debug_print(this-> debug_enabled, "\x1B[41mmatch not found at addr %ld\x1B[0m\n", i);
            }

            if (this-> mem_stack_get(i) == this-> sep_tags[sp_t::__mem_middle]) {
                /* this was added in to fix a placment error but if any errors occur after this edit it might be because of this*/
                mem_match_count = 0;
                name_char_pos = 0;
                break;
            }

            /* if we have got the the end of the name then there is no point of tracking thru
            * the stack anymore, so we are going to move to the next tag if exists
            */
            if ((name_char_pos + 1) == length_of_name) {

                bool is_next_seporator = this-> mem_stack_get(i + 1) == this-> sep_tags[sp_t::__mem_middle]? true : false;

                /* if it's in a list type format then the next char will not be the nm seporator it will
                * be the beginning of the list size
                */
                if (this-> mem_info[mem_addrs_pos].is_list_type) {
is_next_seporator = this-> mem_stack_get(i + 1) == LIST_LEN_BTAG? true : false;
                }

                if (mem_match_count == length_of_name && is_next_seporator) {
debug_print(this-> debug_enabled, "\x1B[36mmatching success. %ld out of %ld correct.\x1B[0m\n", mem_match_count, length_of_name);
__error = false;
return (* itor)[0];
                }
                __error = true;
               	debug_print(this-> debug_enabled, "\x1B[36mmatching failed. %ld out of %ld correct.\x1B[0m\n", mem_match_count, length_of_name);

            	if (!is_next_seporator) {
            		debug_print(this-> debug_enabled, "\x1B[31mnext char dose not equal '%c' but equals '%c'\x1B[0m\n", this-> sep_tags[sp_t::__mem_middle], this-> mem_stack_get(i + 1));
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

void mdl::tagged_memory::set_mem_name(echar_t const *__current_name, echar_t const *__name, boost::int8_t& __error)
{
    uint_t mem_location = this-> get_mem_addr(__current_name, __error);

    bool resize_to_fit = RESIZE_TO_FIT;

    debug_print(this-> debug_enabled, "\x1B[35msetting mem name at addr %d. from '%s' to '%s'\x1B[0m\n", mem_location, __current_name, __name);

    ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

    std::size_t ad = find_mem_addr_it_pos(mem_location, __error);

    itor += ad;

    std::size_t len_of_name = this-> get_mem_name_len(mem_location, __error);

    if (__error == true) return;

    /* count how much extra space there is after the ending address
    */
    std::size_t free_space = 0;
    for (std::size_t i = (* itor)[1] + 2;; i ++) {
        if (this-> mem_stack_get(i) == BLANK_MEMORY) free_space ++; else break;
    }

    bool is_same_len = false;
    std::size_t o = 0, rm = 0;
    bool less = false, grater = false;
    std::size_t amount_changed = 0;
    std::size_t extra_frm = 0;
    for (std::size_t i = ((* itor)[0] + 1);; i ++) {
        if (this-> mem_stack_get(rm) == this-> sep_tags[sp_t::__mem_middle] && !grater && !is_same_len) {
            if (resize_to_fit) {
                if (free_space == 0) {
this-> insert_into_mem_stack(BLANK_MEMORY, rm, __error);
amount_changed ++;
                } else {
for (std::size_t l = (* itor)[1] + 1; l != (rm - 1); l --) {
    this-> mem_stack_set(this-> mem_stack_get(l), l + 1);
    //this-> mem_stack[l + 1] = this-> mem_stack[l];
    this-> mem_stack_set(BLANK_MEMORY, l);
}

free_space --;
                }
            } else this-> insert_into_mem_stack(BLANK_MEMORY, rm, __error);

            (* itor)[1] ++;
            less = true;
        }

        this-> mem_stack_set(__name[o], i);
//        this-> mem_stack(i) = __name[o];

        if (o == (strlen(__name) - 1))  {
            if (is_same_len) break;
            if (this-> mem_stack_get(rm) != this-> sep_tags[sp_t::__mem_middle] && !less) {
                this-> uninsert_from_mem_stack(rm, __error);
                (* itor)[1] --;
                amount_changed ++;
                grater = true;
            } else break;
        } else { o ++; rm = i+1;}
    }

    if (itor == (this-> mem_addrs.end() - 1) || amount_changed == 0) return;

    if (amount_changed == 0) return;

    std::size_t cid = (ad + 1);

    ++itor;
    for (; itor != this-> mem_addrs.end(); ++itor) {
        if (this-> mem_info[cid].is_list_type) {
            for (std::size_t i = 0; i != this-> mem_info[cid].list_points.size(); i ++) {
                if (grater) this-> mem_info[cid].list_points(i) -= amount_changed;
                if (less) this-> mem_info[cid].list_points(i) += amount_changed;
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

mdl::echar_t *mdl::tagged_memory::extract_list_addr(echar_t const * __name,
    std::size_t& __list_pointer, std::size_t __ltaddr_b, std::size_t __ltaddr_e) {

    echar_t *list_point = static_cast<echar_t *>(malloc((__ltaddr_e - __ltaddr_b) * sizeof(echar_t)));
	memset(list_point, '\0', (__ltaddr_e - __ltaddr_b) * sizeof(echar_t));
    std::size_t _o = 0;
    for (std::size_t i = __ltaddr_b+1; i != __ltaddr_e; i++ ) {
        list_point[_o] = __name[i];
        _o ++;
    }

    __list_pointer = atoi(list_point);

    std::free(list_point);

	// NOTE: any error/s add * sizeof(echar_t) at the end of this.
    std::size_t length_of_name = ((strlen(__name) - (__ltaddr_e - __ltaddr_b)) - 1);
    echar_t * name = static_cast<echar_t *>(malloc(length_of_name + 1 * sizeof(echar_t)));
    memset(name, '\0', length_of_name + 1 * sizeof(echar_t));

    for (std::size_t i = 0; i != (strlen(__name) - (__ltaddr_e - __ltaddr_b)) - 1; i ++)
        name[i] = __name[i];

    return name;
}

void mdl::tagged_memory::set_mem_val(echar_t const *__name, echar_t const *__value, id_cache_t& __id_cache, boost::int8_t& __error)
{
    bool resize_to_fit = RESIZE_TO_FIT;

    std::size_t ltaddr_b = 0;
    std::size_t ltaddr_e = 0;
    bool fblist = false, fe_list = false;

    for (std::size_t i = 0; i != strlen(__name); i++ ) {
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
        echar_t * name = this-> extract_list_addr(__name, list_pointer, ltaddr_b, ltaddr_e);

		//printf("%d\n", __id_cache.call_count);
        this-> set_mem_val(name, __value, __id_cache, __error, list_pointer);

        std::free(name);

        return;
    }

    uint_t mem_location = 0;
# ifndef NO_CACHING
    if (! __id_cache.caching)
        mem_location = this-> get_mem_addr(__name, __error);
    else {
        if (__id_cache.call_count == 0) {
			mem_location = this-> get_mem_addr(__name, __error);

            if (__id_cache.same_mem_addr)
				__id_cache.mem_addr = mem_location;
        } else
			if (__id_cache.same_mem_addr)
            	mem_location = __id_cache.mem_addr;
			else
				mem_location = this-> get_mem_addr(__name, __error);
    }
# else
	mem_location = this-> get_mem_addr(__name, __error);
# endif
    bool found_list_indec_tags = false;

    ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

    std::size_t ad = 0;

    if (! __id_cache.caching)
        ad = find_mem_addr_it_pos(mem_location, __error);
    else {
        if (__id_cache.call_count == 0) {
            ad = find_mem_addr_it_pos(mem_location, __error);
			if (__id_cache.same_mem_id)
            	__id_cache.mem_id = ad;
        } else {
			if (__id_cache.same_mem_id)
            	ad = __id_cache.mem_id;
			else
				ad = find_mem_addr_it_pos(mem_location, __error);
		}
    }

    itor += ad;

    std::size_t length_of_name = get_mem_name_len(mem_location, __error);

    std::size_t length_of_value = ((* itor)[1] - (((* itor)[0] + 1) + (length_of_name + 1)));
    bool did_addrs_change = false;
    uint_t last_end_addr = (* itor)[1];
    length_of_value ++;
    bool len = false, tsmall = false;
    std::size_t o = 0, extra = 1, rm = 0;

    bool bypass = strlen(__value) == length_of_value? true : false;

    std::size_t extra_frm = 0;
    for (std::size_t i = ((* itor)[0] + 1) + (length_of_name + 1);; i++ ) {
        if (this-> mem_stack_get(i) == this-> sep_tags[sp_t::__mem_begin] && !len && !tsmall) extra ++;

        if (o == strlen(__value)) {
            if (tsmall || bypass) break;

            if (i == (* itor)[1] /* - 1*/ && !len)  {
                len = true;
                i = ((* itor)[0] + 1) + (length_of_name + 1);
            }

            if (len) {
                if (this-> mem_stack_get(rm) == this-> sep_tags[sp_t::__mem_end] && extra == 0) {
break;
                }
                else
                {
if (!resize_to_fit)
{
    this-> mem_stack_set(this-> mem_stack_get((rm + extra_frm) + 1), rm);

    this-> mem_stack_set(BLANK_MEMORY, (rm + extra_frm) + 1);

    if (this-> mem_stack_get(rm) == this-> sep_tags[sp_t::__mem_end]) extra --;
    extra_frm ++;
}

if (resize_to_fit)
{
    this-> uninsert_from_mem_stack(rm, __error);

    if (this-> mem_stack_get(rm) == this-> sep_tags[sp_t::__mem_end]) extra --;
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
if (!resize_to_fit)
{
    if (this-> mem_stack_get(i + 1) != BLANK_MEMORY) {
        this-> insert_into_mem_stack(BLANK_MEMORY, i, __error);
        did_addrs_change = true;
    } else
        this-> mem_stack_set(this-> mem_stack_get(i), i + 1);

} else {
    this-> insert_into_mem_stack(BLANK_MEMORY, i, __error);
    did_addrs_change = true;
}
                }
            }

            this-> mem_stack_set(__value[o], i);

            o ++;
            rm = i+1;

        }
    }
# ifndef NO_CACHING
    if (__id_cache.caching) {
        if (__id_cache.call_count == __id_cache.call_amount)
            __id_cache.call_count = 0;
        else
            __id_cache.call_count += 1;
	}
# endif
    std::size_t before = (* itor)[1];
    (* itor)[1] = ((* itor)[0] + strlen(__value) + (length_of_name + 1));

    std::size_t change = 0;
    bool g = false, l = false;

    if (before > (* itor)[1]) {
        change = (before - (* itor)[1]);
        l = true;
    }

    if (before < (* itor)[1]) {
        change = ((* itor)[1] - before);
        g = true;
    }

    if (before == (* itor)[1] || ! did_addrs_change) return;

    std::size_t cid = (ad + 1);

    if (itor != this-> mem_addrs.end()) ++itor;
    for (; itor != this-> mem_addrs.end(); ++itor)
    {
        if (this-> mem_info[cid].is_list_type) {
            for (std::size_t i = 0; i != this-> mem_info[cid].list_points.size(); i ++) {
                if (g) this-> mem_info[cid].list_points(i) += change;
                if (l) this-> mem_info[cid].list_points(i) -= change;
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

mdl::echar_t *mdl::tagged_memory::get_mem_val(uint_t __addr, std::size_t __ad, std::size_t __mem_nm_len, boost::int8_t& __any_error, uint_t __list_addr, bool __no_list)
{
    if (! this-> is_mem_addr_ok(__addr)) { __any_error = TMEM_FAILURE; return '\0'; }

	debug_print(this-> debug_enabled, "addr returned = %ld", __addr);

    ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

    std::size_t ad = __ad, ext = 0;
    itor += ad;

    std::size_t length_of_name = __mem_nm_len; //get_mem_name_len(__addr, __error);

    std::size_t length_of_value = 0;
    if (this-> mem_info[ad].is_list_type == false || __no_list)
        length_of_value = ((* itor)[1] - (((* itor)[0] + 1) + (length_of_name + 1)));
    else {
        length_of_value = this-> mem_info[ad].list_elength[__list_addr];
        ext = 1;
    }

    uint_t list_sep_point = 0;
    if (this-> mem_info[ad].is_list_type)
        list_sep_point = this-> mem_info[ad].list_points[__list_addr] + 1;

    if (this-> mem_info[ad].is_list_type == false || __no_list)
        length_of_value += 2;

    echar_t *tmp = static_cast<echar_t *>(malloc((length_of_value + ext) * sizeof(echar_t)));
    memset(tmp, '\0', (length_of_value + ext) * sizeof(echar_t));

    std::size_t o = 0;
    if (this-> mem_info[ad].is_list_type && __no_list == false) {
       for (std::size_t i = list_sep_point ; i != (list_sep_point + length_of_value) ; i ++ ) {
            tmp[o] = this-> mem_stack_get(i);
            o ++;
        }

        return tmp;
    }

    for (std::size_t i = ((* itor)[0] + 1) + (length_of_name + 1); i != (* itor)[1] + 1; i++ ) {
        tmp[o] = this-> mem_stack_get(i);
        o ++;
    }

    return tmp;
}

std::size_t mdl::tagged_memory::get_list_length(echar_t const *__name, boost::int8_t& __error)
{
    uint_t mem_location = this-> get_mem_addr(__name, __error);
    std::size_t ad = find_mem_addr_it_pos(mem_location, __error);

    if (! this-> mem_info[ad].is_list_type) { __error = true; return 0; }
    return this-> mem_info[ad].len_of_list;
}

void mdl::tagged_memory::analyze_stack(boost::int8_t& __any_error)
{
    if (this-> mem_stack.size() == 0) return;

    bool found_mem_begin_ts = false;
    bool found_mem_end_ts = false;
    std::size_t extra_mem_begin_tss = 0;
    uint_t mem_begin_ts_addr = 0;
    uint_t mem_end_ts_addr = 0;

    std::size_t mem_stack_pos = 0, waround = 0;
    bool found_list_begin = false, is_list_type = false;

    ublas::vector<uint_t> list_elength;
    ublas::vector<uint_t> list_points;

    uint_t stack_length = this-> mem_stack.size();
    std::size_t last_lpoint = 0;
    float one_precent = (100.00/stack_length);
    uint_t mid_tag_addr = 0;
    bool found_mid_tag = false;
    std::size_t list_sep_tcount = 0;
    uint_t lb_tag_addr = 0, le_tag_addr = 0;
    float finished_precentage = 0.00;
    bool is_value_str_type = false;
    uint_t str_begin_tag_addr = 0;

    bool comment_begin_tag = false;
    uint_t comment_begin_eaddr = 0;
    debug_print(this-> debug_enabled, "\x1B[36manalysing the memory stack, please wait ...\x1B[0m\n");
    do
    {
        debug_print(this-> debug_enabled, "\x1B[32m%.2f%%\x1B[34m - analysing ...\x1B[0m\n", finished_precentage);

        if (this-> mem_stack_get(mem_stack_pos) == '\0') {
            debug_print(this-> debug_enabled, "\x1B[31mbreaking from loop. next char equls '\\0'\x1B[0m\n");
            break;
        }

        /* NOTE: make this more efficient
        */
        if (!comment_begin_tag) {
            uint_t match_count = 0;
            for (std::size_t i = 0 ; i != sizeof(COMMENT_BTAG) ; i ++)
                if (this-> mem_stack_get(mem_stack_pos + i) == COMMENT_BTAG[i]) match_count ++;

            if (match_count == sizeof(COMMENT_BTAG)) {
                comment_begin_eaddr = (mem_stack_pos + sizeof(COMMENT_BTAG));
                comment_begin_tag = true;
                debug_print(this-> debug_enabled, "found the beginning tag for a comment, addr: %ld.\n", mem_stack_pos);
            }
        }


        if (comment_begin_tag && mem_stack_pos > comment_begin_eaddr) {
            uint_t match_count = 0;
            for (std::size_t i = 0 ; i != sizeof(COMMENT_ETAG) ; i ++)
                if (this-> mem_stack_get(mem_stack_pos + i) == COMMENT_ETAG[i]) match_count ++;

            if (match_count == sizeof(COMMENT_ETAG)) {
                comment_begin_tag = false;
                comment_begin_eaddr = 0;
                debug_print(this-> debug_enabled, "found the ending tag for a comment, addr: %ld.\n", mem_stack_pos);
            } else
                /* skip to the end of the loop, NOTE: dont use continue as we need to run the code at the end of this loop
                */
                goto end;
        }

        if (this-> mem_stack_get(mem_stack_pos) == this-> sep_tags[sp_t::__mem_begin] && !is_value_str_type)
        {
            debug_print(this-> debug_enabled, "\x1B[35mfound beginning seporator as addr %ld\x1B[0m\n", mem_stack_pos);

            /* if the next char in the memory stack is the ending tag there is no point of this, and
            * it would cause errors as theres to middle tag so that means no name or value.
            */
            if (this-> mem_stack_get(mem_stack_pos + 1) != this-> sep_tags[sp_t::__mem_end]) {
                if (! found_mem_begin_ts) {
					debug_print(this-> debug_enabled, "\x1B[33mmemory begin has been set at addr %ld\x1B[0m\n", mem_stack_pos);

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
            if (this-> mem_stack_get(mem_stack_pos) == STR_BEGIN_TAG && found_mid_tag && !is_value_str_type) {
                debug_print(this-> debug_enabled, "\x1B[35mfound string beginning tag at addr %ld\x1B[0m\n", mem_stack_pos);

                /* set to true, this will cause every other tag to be ignored e.g. memory beginning tag etc
                */
                is_value_str_type = true;
                str_begin_tag_addr = mem_stack_pos;
            }

            if (is_value_str_type && (this-> mem_stack_get(mem_stack_pos) == this-> mem_stack_get(str_begin_tag_addr)) && mem_stack_pos != str_begin_tag_addr) {
                debug_print(this-> debug_enabled, "\x1B[35mfound string ending tag at addr %ld\x1B[0m\n", mem_stack_pos);

                /* when we change it back to false it will allow other tags to be interpreted
                */
                is_value_str_type = false;
                str_begin_tag_addr = 0;
            }

            if (this-> mem_stack_get(mem_stack_pos) == this-> sep_tags[sp_t::__mem_middle] && !is_value_str_type) {
                debug_print(this-> debug_enabled, "found middle tag at addr %ld\n", mem_stack_pos);

                mid_tag_addr = mem_stack_pos;
                found_mid_tag = true;
            }

            // this is before the mid tag
            if (this-> mem_stack_get(mem_stack_pos) == LIST_LEN_BTAG && !found_mid_tag) {
                debug_print(this-> debug_enabled, "\x1B[35mfound list beginning tag at addr %ld\x1B[0m\n", mem_stack_pos);

                found_list_begin = true;
                lb_tag_addr = mem_stack_pos;
            }

            if (found_mem_begin_ts && this-> mem_stack_get(mem_stack_pos) == MEM_LIST_TAG && !is_value_str_type) {
                list_elength.resize(list_elength.size() + 1);
                list_elength(list_sep_tcount) = (mem_stack_pos - (list_sep_tcount == 0? mid_tag_addr : last_lpoint)) - 1;

                list_points.resize(list_points.size() + 1);
                list_points(list_sep_tcount) = list_sep_tcount == 0? mid_tag_addr : last_lpoint;

                list_sep_tcount ++;
                last_lpoint = mem_stack_pos;
            }

            if (this-> mem_stack_get(mem_stack_pos) == LIST_LEN_ETAG && ! found_mid_tag && !is_value_str_type) {
                if (found_list_begin && mem_stack_pos != lb_tag_addr) {
					debug_print(this-> debug_enabled, "\x1B[35mfound list ending tag at addr %ld\x1B[0m\n", mem_stack_pos);

					found_list_begin = false;
					is_list_type = true;
					le_tag_addr = mem_stack_pos;
                }
            }
        }

        if (this-> mem_stack_get(mem_stack_pos) == this-> sep_tags[sp_t::__mem_end] && found_mem_begin_ts && mem_stack_pos != mem_begin_ts_addr && !is_value_str_type)
        {
            debug_print(this-> debug_enabled, "\x1B[35mfound ending seporator as addr %ld\x1B[0m\n", mem_stack_pos);

            if (extra_mem_begin_tss != 0)
                extra_mem_begin_tss --;
            else {
            	debug_print(this-> debug_enabled, "\x1B[33mmemory end has been set at addr %ld\x1B[0m\n", mem_stack_pos);

                mem_end_ts_addr = mem_stack_pos;

                this-> mem_addrs.resize(this-> mem_addrs.size() + 1);
                this-> mem_info.resize(this-> mem_info.size() + 1);

                tagged_memory::mem_info_t & lo = this-> mem_info[this-> mem_addrs.size() -1];

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
					echar_t *list_len = static_cast<char *>(malloc((lo.len_of_tag - 2) * sizeof(echar_t)));
					memset(list_len, NULL_MEM, (lo.len_of_tag - 2) * sizeof(echar_t));

					/* from [ to ] extract whats there.
					*/
					std::size_t j = 0;
					for (std::size_t p = lb_tag_addr + 1; p != le_tag_addr; p ++, j++)
						list_len[j] = this-> mem_stack_get(p);

					lo.len_of_list = atoi(list_len);

					/* free the memory that we used */
					std::free(list_len);
                }

                //printf("---------------------> %ld -- %ld\n", lo.len_of_tag, lo.len_of_list);

                ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

                itor += (this-> mem_addrs.size() - 1);

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

        end:
        finished_precentage += one_precent;
        mem_stack_pos++;
    } while (mem_stack_pos != this-> mem_stack.size());

	__any_error = TMEM_SUCCESS;

    debug_print(this-> debug_enabled, "\x1B[36mfinished analysing.\x1B[0m\n");
}

boost::int8_t mdl::tagged_memory::stack_dump(ublas::vector<char> __memory) {
	debug_print(this-> debug_enabled, "\x1B[37mdumping '%s' into mem stack.\x1B[0m\n", __memory);
	ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

	uint_t stack_begin_addr = 0;
	if (this-> mem_addrs.size() != 0) {
		itor += this-> mem_addrs.size() - 1;
		stack_begin_addr = (* itor)[1] + 1;
	}

	std::size_t mem_len = __memory.size();
	if (mem_len) return TMEM_NOP;

	for (std::size_t i = stack_begin_addr; i != mem_len + stack_begin_addr; i ++)
		this-> mem_stack_set(__memory[i - stack_begin_addr], i);
	return TMEM_SUCCESS;
}

boost::int8_t mdl::tagged_memory::stack_dump(echar_t const *__memory) {
	debug_print(this-> debug_enabled, "\x1B[37mdumping '%s' into mem stack.\x1B[0m\n", __memory);
	ublas::vector<boost::array<uint_t, 2>>::iterator itor = this-> mem_addrs.begin();

	uint_t stack_begin_addr = 0;
	if (this-> mem_addrs.size() != 0) {
		itor += this-> mem_addrs.size() - 1;
		stack_begin_addr = (* itor)[1] + 1;
	}

	std::size_t mem_len = strlen(__memory);
	if (!mem_len) return TMEM_NOP;

	for (std::size_t i = stack_begin_addr; i != mem_len + stack_begin_addr; i ++)
		this-> mem_stack_set(__memory[i - stack_begin_addr], i);
	return TMEM_SUCCESS;
}
