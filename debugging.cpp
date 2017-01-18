# include <tagged_memory.hpp>
# include <iostream>
# include <chrono>
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

    mdl::tagged_memory::extra_options_t eo;

    /* if fdirect_rw is true then it will read directly from a file
	* and not from memory.
 	*/ 
    eo.fdirect_rw = false;

	// does the file contain dataw
    eo.fcontains_data = false;

	// if the file is not specified when calling
	// the function to save or load then  it will
	// defualt to this else  DEF_MINFO_FILE / DEF MADDRS_FILE
    eo.mem_info_file = "mem_info.dat";
    eo.mem_addrs_file = "mem_addrs.dat";

    /* create */
    mdl::tmem_t example(128, {'{', ';', '}'}, eo, true/*debug info*/);

    /* dump charset into the stack */
    //example.dump_into_stack("/*NOTE: the tag < & > are for list length*/{example_0<2>;ex[0], ex[0][1]}{example_1<3>;ex[1],P,a}");

    /* NOTE: when setting elements it will be slow if you havent no extra space to the right
    * e.g. {example_0;}@@@@@@@@@@@{example_1;} 
    * @ = space
    */
    example.dump_into_stack("{example_0;21299}    {example_1<2>;a,b}");
    /* analyze the charset that was put into the stack */
    example.analyze_stack_memory(error);
    
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
    //example.load_mem_info();
    //example.load_mem_addrs(); 


    //example.mem_alloc(2, true);

    //example.mem_mov(0, 2, MEM_MOVB);

    //example.mem_free("example_0", error, true);


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

    auto begin = std::chrono::high_resolution_clock::now();

    /* if we are going to the the same mem value for e.g. 255 times
    * using this it will only check / find the name the the mem once.
    */
    mdl::tagged_memory::id_cache_t id_cache;
    id_cache.caching = true;
    id_cache.call_amount = 255;

    /* example set
    */
    for (std::size_t i = 0; i != 256; i ++) {
        std::string s = std::to_string(i);
        example.set_mem_value("example_0", s.c_str(), id_cache, error);
    }

    /* list example: mem_name<list_addr> */
    char * tmp = example.get_mem_value("example_1<0>", error);
    
    // or example.get_mem_value("example_0", error, list_addr, false);

    printf("output: %s\n", tmp);

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);

    std::cout << time_span.count() << std::endl;    

    example.dump_stack_memory();

    std::free(tmp);

    return 0;
}
