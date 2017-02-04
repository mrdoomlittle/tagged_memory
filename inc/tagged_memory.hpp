# ifndef __tagged__memory__hpp
# define __tagged__memory__hpp
# include <boost/numeric/ublas/vector.hpp>

# include <boost/cstdlib.hpp>
# include <boost/cstdint.hpp>
# include <initializer_list>

# define MEM_BEGIN_TAG '{'
# define MEM_END_TAG '}'

# define MEM_MIDDLE_TAG ';'

# define LIST_LEN_BTAG '<'
# define LIST_LEN_ETAG '>'

# define MEM_LIST_TAG ','
// note this does not work
# define BLANK_MEMORY ' '
# define USED_MEMORY '@'
// NOTE: the STR Begin and End are '
# define STR_BEGIN_TAG 0x27
# define STR_END_TAG 0x27

// NOTE: you can change the size 
constexpr char COMMENT_BTAG[2] = {'/', '*'};
constexpr char COMMENT_ETAG[2] = {'*', '/'};
//# include <bitset>
/* this takes the extra space given to 
* elements when added, so insted of shifting
* all the elements in the stack forward by 1
* it will use the free space that has nothing
* assigned to it. e.g. :ex_0~test;@@@@@:ex_1~test;
* @ is free space.
*/
# define RESIZE_TO_FIT false
# define DEF_MSTACK_FILE "mem_stack.dat"
# define DEF_MINFO_FILE "mem_info.dat"
# define DEF_MADDRS_FILE "mem_addrs.dat"

# define MEM_FREE 0x0
# define MEM_USED 0x1

# define MEM_MOVF 0x0
# define MEM_MOVB 0x1

/* check for illegal chars
*/
# define ILLEGAL_CHARS false

# include <string.h>
# include <eint_t.hpp>
# include <intlen.hpp>
# include <fstream>
# include <boost/array.hpp>
namespace ublas = boost::numeric::ublas;

//# include <eint_t.hpp>
// note to remove this and uncomment above
//typedef boost::uint16_t uint_t;

namespace mdl { class tagged_memory
{
    public:
    // this will replace the bool error part
    typedef struct {
        bool fatal_error = false;
    } error_info_t;

    typedef struct {        
        bool fdirect_rw = false;
        bool fcontains_data = false;
        char *mem_info_file = '\0';
        char *mem_addrs_file = '\0';
    } extra_options_t;

    typedef struct {
        bool caching = false;

		/* the id of the memory
		*/
        std::size_t mem_id = 0;

		/* the addr of the memory
		*/
        uint_t mem_addr = 0;

		/* how many times will it be called
		*/
        std::size_t call_amount = 0;

		/* starts from 0
		*/
        std::size_t call_count = 0;

		/* length of the mem name
		*/
		std::size_t mem_nme_len = 0;

		bool mem_addr_ok = false;

		char *mem_name = nullptr;
		std::size_t list_pointer = 0;

		std::size_t ltaddr_b, ltaddr_e;
		bool locked_list = false;
    } id_cache_t;

    id_cache_t null_idc;

    // NOTE: to clean this up
    struct arc {
        arc(FILE *__fs, char __typ) : fs(__fs), typ(__typ) {}

        void operator&(std::size_t __size) {
            if (typ == 's')
                this-> size += __size;
            else
                this-> size = __size;
        }

        void operator<<(void *__obj) {
            switch(typ) {
                case 'r':
                    fread(__obj, size, 1, fs);
                break;
                case 'w':
                    fwrite(__obj, size, 1, fs);
                break;
            }
        }

        void operator|(char __typ) {
            size = 0;
            typ = __typ;
        }

        std::size_t size = 0;
        char typ = '\0';
        FILE * fs;
    };

    public:
    tagged_memory(uint_t __allocated_memory,
        std::initializer_list<boost::uint8_t> __seporator_tags, extra_options_t __extra_options, bool __debug_logging = true);

	std::size_t find_free_memory(uint_t __mem_eaddr, uint_t& __end_point) {
		bool found_free_mem = false;
		std::size_t free_mem_count = 0, o = __mem_eaddr + 2;
		while(true) {
			printf("%c --- \n", this-> mem_stack_get(o));
			if (this-> mem_stack_get(o) != BLANK_MEMORY) {
				__end_point = o;
				break;
			}

			if (this-> mem_stack_get(o) == BLANK_MEMORY) {
				free_mem_count ++;
			}
			o ++;
		}
		__end_point = o;

		return free_mem_count;
	}

	bool is_there_illegals(char const *__chars) {
		std::size_t char_count = strlen(__chars), o = 0;
		while (o != char_count)
		{
			for (std::size_t i = 0; i != 3; i ++) {
				if (__chars[o] == this-> seporator_tags[i]) {
					return true;
				}
			}

			if (__chars[o] == MEM_LIST_TAG) return true;

			o ++;
		}
	}

	/* dump the memory stack to the terminal
	*/
    char *dump_stack_memory(bool __return = false);

	/* combine 2 strings into one string
	*/
    char *combine_strings(char const *__string_0, char const *__string_1);

    void analyze_stack_memory(bool& __error);

    void dump_into_stack(char const *__memory);

    void dump_into_stack(ublas::vector<char> __memory);

    uint_t get_mem_addr(char const *__name, bool& __error);

    std::size_t get_list_length(char const *__name, bool& __error);

	void add_to_list(char const *__mem_name, std::size_t __amount, bool __append = true, std::size_t __list_id = 0);

	void del_from_list(char const *__mem_name, std::size_t __list_id);

    void save_mem_addrs();
    void save_mem_addrs(char const *__file_path, char const *__file_name);

    void load_mem_addrs();
    void load_mem_addrs(char const *__file_path, char const *__file_name);

    /* save the memory info to a file on the system
    */
    void save_mem_info();
    void save_mem_info(char const *__file_path, char const *__file_name);

    /* load the memory info from a file on the system
    */
    void load_mem_info();
    void load_mem_info(char const *__file_path, char const *__file_name);

    /* check the memory stack for a var thats matches a name
    */
    bool does_mem_name_exist(char const *__mem_name, bool& __error);

    bool compare_strings(char const *__string_0, char const *__string_1);

    /* compare the value of 2 pices of memory
    */
    bool compare_mem_values(char const *__mem_name_0, char const *__mem_name_1, bool& __error);

    char *create_mem_tag(char const *__name, char const *__value = "\0");
 
    void add_mem_tag(char const * __name, char const * __value, size_t __null_space, bool& __error);

    /* set the name of the memory */
    void set_mem_name(char const *__current_name, char const *__name, bool& __error);

    void set_mem_value(char const *__name, char const *__value, bool& __error, uint_t __list_addr);

    /* set the value of the memory */
    void set_mem_value(char const *__name, char const *__value, id_cache_t& __id_cache, bool& __error);

    /* load the memory stack from a file */
    void load_mem_stack_from_file(char const *__file_path, char const *__file_name);
    /* save the memory stack to a file */
    void save_mem_stack_to_file(char const *__file_path, char const *__file_name);

    /* get the name of the memory from a id/address */
    char *get_mem_name(uint_t __addr, bool& __error);

    char *get_mem_value(char const *__name, id_cache_t& __id_cache, bool& __error, uint_t __list_addr = 0, bool __no_list = true);

    /* get the value of the memory from a id/address */
    char *get_mem_value(uint_t __addr, std::size_t __ad, std::size_t __mem_nm_len, bool& __error, uint_t __list_addr, bool __no_list = true);

    /* find the address corresponding to the one passed thru and return the amount
    * that the iterator should be iterated
    */
    std::size_t find_mem_addr_it_pos(uint_t __addr, bool& __error);

    std::size_t  get_mem_id(uint_t __addr, bool& __error) {
        return this-> find_mem_addr_it_pos(__addr, __error);
    } 

    /* starting from the start get the length from { to :
    */
    std::size_t get_mem_name_len(uint_t __addr, bool& __error);

    /* see if we can find the address passed thru in 'mem_addrs' vector at arr pos 0
    * if there is a match then we are returning true else false for no match
    */

    bool is_mem_addr_ok(uint_t __addr);

    /* insert a char into the memory stack. the memory thats allready there will be shifted forward
    * this includes all the memory after the address
    */
    void insert_into_mem_stack(char __mem, uint_t __addr, bool& __error);

    /* remove a pice of memory from the stack and then shift all the memory in the stack after the address
    * to fill in the free space.
    */
    void uninsert_from_mem_stack(uint_t __addr, bool& __error);

    char *extract_list_addr(char const *__name, std::size_t& list_pointer, std::size_t __ltaddr_b, std::size_t __ltaddr_e);

	void set_mem_list_len(char const *__mem_name, std::size_t __list_addr, bool& __is_error);
	std::size_t get_mem_list_len(char const *__mem_name, bool __cached_ver, bool& __is_error);

	bool get_mem_list_addrs(uint_t __mem_addr, std::size_t __mem_id, std::size_t *__list_addrs);

    typedef struct __mem_t {
        __mem_t(std::size_t __element_id, tagged_memory *__this, error_info_t *__error_info) :
        	element_id(__element_id), _this(__this), error_info(__error_info)
		{
            addr = __this-> mem_addrs[this-> element_id][0];
            this-> len = (__this-> mem_addrs[this-> element_id][1] - (_this-> get_mem_name_len(this-> addr, this-> error) + this-> addr)) - 1;
        }

        bool is_addr_in_range(uint_t __addr) {
            if (__addr <  0 || __addr > this-> len) return false;
            return true;
        }

        void operator>>(std::size_t __amount) {
            _this-> mem_mov(element_id, __amount, MEM_MOVF);
        }

        void operator<<(std::size_t __amount) {
            _this-> mem_mov(element_id, __amount, MEM_MOVB);
        }

        char *operator=(char const *__mem) {
            return nullptr;
        }

        boost::uint8_t operator[](std::size_t __addr) {
            if (!this-> is_addr_in_range(__addr)) { this-> error_info-> fatal_error = true; }

            uint_t addr = ((_this-> mem_addrs[this-> element_id][0] + 1) + __addr) + 
                _this-> get_mem_name_len(this-> addr, this-> error_info-> fatal_error) + 1;

            return _this-> mem_stack_get(addr);

        }

        std::size_t get_len(){
            return this-> len;
        }

        error_info_t *error_info;
        uint_t addr = 0;
        bool error = false;
        std::size_t len = 0;
        std::size_t const element_id = 0;
        tagged_memory *_this = nullptr;
    } mem_t;

    void mem_alloc(std::size_t __malloc, bool __fill = false) {
        mem_info.resize(mem_info.size() + 1);

        uint_t before = mem_addrs.size() - 1;
        mem_addrs.resize(mem_addrs.size() + 1);
        uint_t after = mem_addrs.size() - 1;

        mem_addrs[after][0] = mem_addrs[before][1] + 2;
        mem_addrs[after][1] = mem_addrs[after][0] + __malloc;

        if (__fill)
            for (std::size_t i = mem_addrs[after][0] ; i != mem_addrs[after][0] + __malloc; i ++)
                this-> mem_stack_set(USED_MEMORY, i);
    }

    void mem_free(char const *__mem_name, bool& __error, bool __clean = false) {
        uint_t addr = this-> get_mem_addr(__mem_name, __error);
        uint_t pos = this-> find_mem_addr_it_pos(addr, __error);

        this-> mem_free(pos, __clean);

    }

    void mem_free(uint_t __mem_id, bool __clean = false) {
        if (__clean)
            for (std::size_t i = mem_addrs[__mem_id][0]; i != mem_addrs[__mem_id][1] + 2; i ++)
                mem_stack_set(BLANK_MEMORY, i);

        for (std::size_t i = __mem_id ; i != mem_info.size() - 1; i ++) {
            mem_info[i] = mem_info[i + 1];
        }

        mem_info.resize(mem_info.size() - 1);

        for (std::size_t i = __mem_id ; i != mem_addrs.size() - 1; i ++) {
            mem_addrs[i].swap(mem_addrs[i + 1]);
        }

        mem_addrs.resize(mem_addrs.size() - 1);
    }

    void mem_mov(char const *__mem_name, uint_t __mov_amount, boost::uint8_t __direction, bool& __error) {
        uint_t addr = this-> get_mem_addr(__mem_name, __error);
        uint_t pos = this-> find_mem_addr_it_pos(addr, __error);

        this-> mem_mov(pos, __mov_amount, __direction);
    }

    void mem_mov(uint_t __mem_id, uint_t __mov_amount, boost::uint8_t __direction) {
        if (__direction == MEM_MOVF) {
            for (std::size_t i = mem_addrs[__mem_id][1]+1; i != mem_addrs[__mem_id][0]; i --) {
                this-> mem_stack_set(this-> mem_stack_get(i), i + __mov_amount);
                this-> mem_stack_set(BLANK_MEMORY, i);
            }

            this-> mem_stack_set(this-> mem_stack_get(mem_addrs[__mem_id][0]), mem_addrs[__mem_id][0] + __mov_amount);
            this-> mem_stack_set(BLANK_MEMORY, mem_addrs[__mem_id][0]);

            mem_addrs[__mem_id][0] += __mov_amount;
            mem_addrs[__mem_id][1] += __mov_amount;
        }

        if (__direction == MEM_MOVB) {
            for (std::size_t i = mem_addrs[__mem_id][0]; i != mem_addrs[__mem_id][1] + 2; i ++) {
                this-> mem_stack_set(this-> mem_stack_get(i), i - __mov_amount);
                this-> mem_stack_set(BLANK_MEMORY, i);
            }

            mem_addrs[__mem_id][0] -= __mov_amount;
            mem_addrs[__mem_id][1] -= __mov_amount;
        }

        for (std::size_t i = 0; i != mem_info[__mem_id].list_elength.size(); i ++) {
            switch (__direction) {
                case MEM_MOVF:
                    mem_info[__mem_id].list_elength(i) += __mov_amount;
                break;
                case MEM_MOVB:
                    mem_info[__mem_id].list_elength(i) -= __mov_amount;
                break;
            }
        }

        for (std::size_t i = 0; i != mem_info[__mem_id].list_points.size(); i ++) {
            switch (__direction) {
                case MEM_MOVF:
                    mem_info[__mem_id].list_points(i) += __mov_amount;
                break;
                case MEM_MOVB:
                    mem_info[__mem_id].list_points(i) -= __mov_amount;
                break;
            }
        }
    }

    private:
    extra_options_t extra_options;

    /* NOTE: need to up update this.
    */
    enum sp_t : boost::uint8_t { __mem_begin, __mem_middle, __mem_end };
        
    bool debug_logging = false;

    /* each tag will be stored in this array,
    * NOTE: any changes to this will be automaticly used.
    * NOTE: need to update this
    */
    boost::array<char, 6> seporator_tags;

    /* NOTE: need to get this working
    */
    uint_t used_mem = 0, free_mem = 0;

    
    typedef struct {
        // this will indicate that its in a list type format
        bool is_list_type = false;

        boost::uint8_t mem_state = MEM_FREE;

        char * cached_mem_name = nullptr;
        
        std::size_t len_of_list = 0;

        /* e.g. example [255] <- the length from [ to ] in chars */
        std::size_t len_of_tag = 0;

        std::size_t vec_lens[2] = {0};

        void __arc(arc & ac) {
            bool s = true;

            if (ac.typ == 's') s = false;

            ac & sizeof(bool);
            if (s) ac << &is_list_type;

            ac & sizeof(std::size_t);
            if (s) ac << &len_of_list;

            ac & sizeof(std::size_t);
            if (s) ac << &len_of_tag;

            ac & sizeof(std::size_t);
            if (s) ac << &vec_lens[0];

            ac & sizeof(std::size_t);
            if (s) ac << &vec_lens[1];
        }

        /* hear we will store where the starting point of each
        * list element, this allows getting and setting each
        * element to be much faster when using large amounts of data
        */
        ublas::vector<uint_t> list_elength;
        ublas::vector<uint_t> list_points;
    } mem_info_t;
   
    /* for each variable we store, any information about it will be stored hear.
    */
    ublas::vector<mem_info_t> mem_info;

    /* for each variable that we are storing in tagged format
    * we need to know where the beginning address is, so we
    * wont need to analyze the stack every time we try to change something
    */
    ublas::vector<boost::array<uint_t, 2>> mem_addrs;
    
    /* every char that makes up each variable will be stored in this vector.
    * NOTE: this might change later as using this method can be slow.
    */

    ublas::vector<boost::uint8_t> mem_stack;
    
    public:
    void mem_stack_set(boost::uint8_t __mem, uint_t __mem_addr);
    boost::uint8_t mem_stack_get(uint_t __mem_addr);
} ;
    static tagged_memory::id_cache_t null_idc;
    typedef tagged_memory tmem_t;
}


# endif /*__tagged__memory__hpp*/
