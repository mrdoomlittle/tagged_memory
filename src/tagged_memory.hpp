# ifndef __tagged__memory__hpp
# define __tagged__memory__hpp
# include <boost/numeric/ublas/vector.hpp>
# include <boost/cstdlib.hpp>
# include <boost/cstdint.hpp>
# include <initializer_list>
# define MEM_BEGIN_TAG '{'
# define MEM_END_TAG '}'
# define MEM_SEP_TAG ':'
/* this is fore lists 
*/
# define LIST_LEN_BTAG '['
# define LIST_LEN_ETAG ']'
# define MEM_LIST_TAG ','

# include <string.h>
# include <fstream>
# include <boost/array.hpp>
namespace ublas = boost::numeric::ublas;
namespace mdl { class tagged_memory
{
    public:
    tagged_memory(boost::uint16_t __allocated_memory,
        std::initializer_list<char> __seporator_tags, bool __debug_logging = true);

    char * dump_stack_memory();

    void analyze_stack_memory(bool & __error);

    void dump_into_stack(char const * __memory);

    void dump_into_stack(ublas::vector<char> __memory);

    boost::uint16_t get_mem_addr(char const * __name, bool & __error);

    std::size_t get_list_length(char const * __name, bool & __error);

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

    void set_mem_value(char const * __name, char const * __value, bool & __error, boost::uint16_t __list_addr);

    /* set the value of the memory */
    void set_mem_value(char const * __name, char const * __value, bool & __error);

    /* load the memory stack from a file */
    void load_mem_stack_from_file(char const * __file_path);
    /* save the memory stack to a file */
    void save_mem_stack_to_file(char const * __file_path);

    /* get the name of the memory from a id/address */
    char * get_mem_name(boost::uint16_t __addr, bool & __error);

    char * get_mem_value(char const * __name, bool & __error, boost::uint16_t __list_addr = 0, bool __no_list = true);

    /* get the value of the memory from a id/address */
    char * get_mem_value(boost::uint16_t __addr, bool & __error, boost::uint16_t __list_addr, bool __no_list = true);

    /* find the address corresponding to the one passed thru and return the amount
    * that the iterator should be iterated
    */
    size_t find_mem_addr_it_pos(boost::uint16_t __addr, bool & __error);

    /* starting from the start get the length from { to :
    */
    size_t get_mem_name_len(boost::uint16_t __addr, bool & __error);

    /* see if we can find the address passed thru in 'memory_addrs' vector at arr pos 0
    * if there is a match then we are returning true else false for no match
    */
    // NOTE: change to 'is' and not 'does' :|
    bool does_mem_addr_ok(boost::uint16_t __addr);

    /* insert a char into the memory stack. the memory thats allready there will be shifted forward
    * this includes all the memory after the address
    */
    void insert_into_mem_stack(char __mem, boost::uint16_t __addr, bool & __error);

    /* remove a pice of memory from the stack and then shift all the memory in the stack after the address
    * to fill in the free space 
    */
    void uninsert_from_mem_stack(boost::uint16_t __addr, bool & __error);
    
    char * extract_list_addr(char const * __name, std::size_t & list_pointer,
        std::size_t __ltaddr_b, std::size_t __ltaddr_e);

    private:
    /* NOTE: need to up update this.
    */
    enum sp_t : boost::uint8_t { __begin, __end, __seporator, __list };
    
    bool debug_logging = false;

    /* each tag will be stored in this array,
    * NOTE: any changes to this will be automaticly used.
    */
    boost::array<char, 6> seporator_tags;

    /* NOTE: need to get this working
    */
    boost::uint16_t used_memory, free_memory;
 
    typedef struct {
        // this will indicate that its in a list type format
        bool is_list_type = false;
        
        std::size_t len_of_list = 0;
        std::size_t len_of_tag = 0;

        /* hear we will store where the starting point of each
        * list element, this allows getting and setting each
        * element to be much faster when using large amounts of data
        */
        ublas::vector<boost::uint16_t> list_elength;
        ublas::vector<boost::uint16_t> list_points;
    } __o;

    /* for each variable we store, any information about it will be stored hear.
    */
    ublas::vector<__o> infomation;

    /* for each variable that we are storing in tagged format
    * we need to know where the beginning address is, so we
    * wont need to analyze the stack every time we try to change something
    */
    ublas::vector<boost::array<boost::uint16_t, 2>> memory_addrs;

    /* every char that makes up each variable will be stored in this vector.
    * NOTE: this might change later as using this method can be slow.
    */
    ublas::vector<char> memory_stack;
} ;
    typedef tagged_memory tmem_t;
}


# endif /*__tagged__memory__hpp*/
