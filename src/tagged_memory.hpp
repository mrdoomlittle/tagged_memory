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
# define MEM_LIST_TAG ','
# define MEM_CHAR_TAG '/'
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
    boost::uint16_t get_mem_addr(char const * __name, bool & __error);

    /* check the memory stack for a var thats matches a name
    */
    bool does_mem_name_exist(char const * __name);

    bool compare_strings(char const * __value_0, char const * __value_1);

    /* compare the value of 2 pices of memory
    */
    bool compare_mem_value(char const * __name_0, char const * __name_1, bool & __error);

    char * create_mem_tag(char const * __name, char const * __value = "\0");
 
    void add_mem_tag(char const * __name, char const * __value = "\0", size_t __null_space = 0);

    /* set the name of the memory */
    void set_mem_name(char const * __current_name, char const * __name, bool & __error);

    /* set the value of the memory */
    void set_mem_value(char const * __name, char const * __value, bool & __error);

    /* load the memory stack from a file */
    void load_mem_stack_from_file(char const * __file_path);
    /* save the memory stack to a file */
    void save_mem_stack_to_file(char const * __file_path);

    /* get the name of the memory from a id/address */
    char * get_mem_name(boost::uint16_t __addr, bool & __error);

    char * get_mem_value(char const * __name, bool & __error);

    /* get the value of the memory from a id/address */
    char * get_mem_value(boost::uint16_t __addr, bool & __error);

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
    
    private:
    enum sp_t : boost::uint8_t { __begin, __end, __seporator, __list };
    
    bool debug_logging = false;

    boost::array<char, 4> seporator_tags;
    boost::uint16_t used_memory, free_memory;
    ublas::vector<boost::array
        <boost::uint16_t, 2>> memory_addrs;
    ublas::vector<char> memory_stack;
} ;
//    template class tagged_memory<char>;
    typedef tagged_memory tmem_t;
}


# endif /*__tagged__memory__hpp*/
