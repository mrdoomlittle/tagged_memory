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

    /* create */
    mdl::tmem_t example(128, {'{', ';', '}'}, false/*debug info*/);

    /* dump charset into the stack */
    example.dump_into_stack("{example_0;ex[0]}{example_1;ex[1]}");

    /* analyze the charset that was put into the stack */
    example.analyze_stack_memory(error);

    mdl::tagged_memory::error_info_t error_info;

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

    return 0;
}
