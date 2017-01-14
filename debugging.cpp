# include <tagged_memory.hpp>
# include <iostream>

// functions for tmem_t. need more look at src/tagged_memory.hpp
/*
    .does_mem_name_exist(char const *, bool &)
    .add_mem_tag(char const *, char const *, size_t, bool &)
    .set_mem_name(char const *, char const *, bool &)
    .set_mem_value(char const *, char const *, bool &)
    .get_mem_value(char const *, bool &)
    .get_list_length(char const *, bool &)
*/

int main()
{
    bool error = false;

    /* NOTE: comments can only be used in the file or when you dump data into the stack 
    * and analyze, this will not work with the function 'add_mem_tag' and comments only
    * work when not inside a var e.g. below
    */

    mdl::tagged_memory::eoptions_t eo;

    // NOTE: implement this   
    eo.fdirect_rw = true;
    eo.fcontains_data = true;
    eo.mem_info_file = "mem_info.dat";
    eo.mem_addrs_file = "mem_addrs.dat";

    /* create */
    mdl::tmem_t example(128, {'{', ';', '}'}, eo, true/*debug info*/);

    /* dump charset into the stack */
    //example.dump_into_stack("/*NOTE: the tag < & > are for list length*/{example_0<2>;ex[0], ex[0][1]}{example_1<3>;ex[1],P,a}");

    /* analyze the charset that was put into the stack */
    //example.analyze_stack_memory(error);
    
    //example.save_mem_stack_to_file("", DEF_MSTACK_FILE);

    /* NOTE: save_mem_info creates 2 files .filename, filename
    */
    mdl::tagged_memory::error_info_t error_info;
    /* save the memory infomation e.g. tag address etc
    */
    //example.save_mem_info();
    //example.save_mem_addrs();

  
    // NOTE: need to work on this
    /* load the memory infomation from file
    */
    example.load_mem_info();
    example.load_mem_addrs(); 

    /* mdl::tagged_memory::mem_t
    * allows you to access the data at a point in the stack
    * without needing to use a function.
    */
    mdl::tagged_memory::mem_t * ex[2];

    ex[0] = new mdl::tagged_memory::mem_t(0/*id*/, (& example), &error_info);
    ex[1] = new mdl::tagged_memory::mem_t(1/*id*/, (& example), &error_info);
    
    /* print the data to terminal
    */
    for (std::size_t o = 0; o != 2; o ++) { 
        std::cout << "ID: " << o << ", ";
        for (std::size_t i = 0; i != ex[o]-> get_len(); i ++)
            std::cout << (* ex[o])[i];

        std::cout << "\n";
    }

    /* NOTE: if you want illegal char's then use the '' string tags
    * note the end resault will output the val with the tags.
    */

    /* example set
    */
    example.set_mem_value("example_0<0>", "hello", error);

    /* list example: mem_name<list_addr> */
    char * tmp = example.get_mem_value("example_0<0>", error);
    
    // or example.get_mem_value("example_0", error, list_addr, false);

    printf("output: %s\n", tmp);

    std::free(tmp);

    return 0;
}
