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

# include <string.h>
# include <fstream>
# include <boost/array.hpp>
namespace ublas = boost::numeric::ublas;

/* note need to rename this */
typedef boost::uint16_t uint_t;

namespace mdl { class tagged_memory
{
    public:
    // this will replace the bool error part
    typedef struct {
        bool fatal_error = false;
    } error_info_t;

    typedef struct {        
        bool fdirect_reading = false;
        bool fcontains_data = false;
        char * mem_info_file = '\0';
        char * mem_addrs_file = '\0';
    } eoptions_t;

    // NOTE: to clean this up
    struct arc {
        arc(FILE * __fs, char __typ) : fs(__fs), typ(__typ) {}

        arc operator&(std::size_t __size) {
            if (typ == 's')
                this-> size += __size;
            else
                this-> size = __size;
        }

        arc operator<<(void * __obj) {
            switch(typ) {
                case 'r':
                    fread(__obj, size, 1, fs);
                break;
                case 'w':
                    fwrite(__obj, size, 1, fs);
                break;
            } 
        }
    
        arc operator|(char __typ) {
            size = 0;
            typ = __typ;
        }
     
        std::size_t size = 0;
        char typ = '\0';
        FILE * fs;
    };

    public:
    tagged_memory(uint_t __allocated_memory,
        std::initializer_list<boost::uint8_t> __seporator_tags, eoptions_t __extra_options, bool __debug_logging = true);

    char * dump_stack_memory(bool __return = false);

    char * combine_strings(char const * __string_0, char const * __string_1);

    void analyze_stack_memory(bool & __error);

    void dump_into_stack(char const * __memory);

    void dump_into_stack(ublas::vector<char> __memory);

    uint_t get_mem_addr(char const * __name, bool & __error);

    std::size_t get_list_length(char const * __name, bool & __error);

    void save_mem_addrs();
    void save_mem_addrs(char const * __file_path, char const * __file_name);

    void load_mem_addrs();
    void load_mem_addrs(char const * __file_path, char const * __file_name);

    /* save the memory info to a file on the system
    */
    void save_mem_info();
    void save_mem_info(char const * __file_path, char const * __file_name);

    /* load the memory info from a file on the system
    */
    void load_mem_info();
    void load_mem_info(char const * __file_path, char const * __file_name);

    /* check the memory stack for a var thats matches a name
    */
    bool does_mem_name_exist(char const * __mem_name, bool & __error);

    bool compare_strings(char const * __string_0, char const * __string_1);

    /* compare the value of 2 pices of memory
    */
    bool compare_mem_values(char const * __mem_name_0, char const * __mem_name_1, bool & __error);

    char * create_mem_tag(char const * __name, char const * __value = "\0");
 
    void add_mem_tag(char const * __name, char const * __value, size_t __null_space, bool & __error);

    /* set the name of the memory */
    void set_mem_name(char const * __current_name, char const * __name, bool & __error);

    void set_mem_value(char const * __name, char const * __value, bool & __error, uint_t __list_addr);

    /* set the value of the memory */
    void set_mem_value(char const * __name, char const * __value, bool & __error);

    /* load the memory stack from a file */
    void load_mem_stack_from_file(char const * __file_path, char const * __file_name);
    /* save the memory stack to a file */
    void save_mem_stack_to_file(char const * __file_path, char const * __file_name);

    /* get the name of the memory from a id/address */
    char * get_mem_name(uint_t __addr, bool & __error);

    char * get_mem_value(char const * __name, bool & __error, uint_t __list_addr = 0, bool __no_list = true);

    /* get the value of the memory from a id/address */
    char * get_mem_value(uint_t __addr, bool & __error, uint_t __list_addr, bool __no_list = true);

    /* find the address corresponding to the one passed thru and return the amount
    * that the iterator should be iterated
    */
    size_t find_mem_addr_it_pos(uint_t __addr, bool & __error);

    /* starting from the start get the length from { to :
    */
    size_t get_mem_name_len(uint_t __addr, bool & __error);

    /* see if we can find the address passed thru in 'mem_addrs' vector at arr pos 0
    * if there is a match then we are returning true else false for no match
    */

    bool is_mem_addr_ok(uint_t __addr);

    /* insert a char into the memory stack. the memory thats allready there will be shifted forward
    * this includes all the memory after the address
    */
    void insert_into_mem_stack(char __mem, uint_t __addr, bool & __error);

    /* remove a pice of memory from the stack and then shift all the memory in the stack after the address
    * to fill in the free space 
    */
    void uninsert_from_mem_stack(uint_t __addr, bool & __error);
    
    char * extract_list_addr(char const * __name, std::size_t & list_pointer,
        std::size_t __ltaddr_b, std::size_t __ltaddr_e);

    typedef struct __mem_t {
        __mem_t(std::size_t __element_id, tagged_memory * __this, error_info_t * __error_info) : 
        element_id(__element_id), _this(__this), error_info(__error_info) { 
            addr = __this-> mem_addrs[this-> element_id][0];
            this-> len = (__this-> mem_addrs[this-> element_id][1] - (_this-> get_mem_name_len(this-> addr, this-> error) + this-> addr)) - 1;
        }

        bool is_addr_in_range(uint_t __addr) {
            if (__addr <  0 || __addr > this-> len) return false;
            return true;
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

        error_info_t * error_info;
        uint_t addr = 0;
        bool error = false;
        std::size_t len = 0;
        std::size_t const element_id = 0;
        tagged_memory * _this = nullptr;
    } mem_t;

    // this is for later
    void mem_alloc() {
        
    }

    void mem_free();

    private:
    eoptions_t extra_options;

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
    public:
    void mem_stack_set(boost::uint8_t __mem, std::size_t __mem_addr);
    boost::uint8_t mem_stack_get(std::size_t __mem_addr);

    ublas::vector<boost::uint8_t> mem_stack;
} ;
    typedef tagged_memory tmem_t;
}


# endif /*__tagged__memory__hpp*/
