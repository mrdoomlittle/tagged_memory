# include <tagged_memory.hpp>
# include <iostream>

int main()
{
    // some functions take a bool ref in and will set it to true if there was a problem doing somthing
    bool error = false;
   
    std::cout << "debugging has started" << std::endl;

    mdl::tmem_t example(128/*size is chars/bytes*/, {':', ';', '~'}/*tags B/E/S*/, true/*this turns on debugging*/);

    //example.dump_into_stack(":is_running~true;:vid_ids~HELLOX;");
     
    example.load_mem_stack_from_file("output.dat");

    example.analyze_stack_memory(error);

    example.add_mem_tag("test_var", "i_love_programming", 0, error);

    example.set_mem_name("example_1", "by_mrdoomlittle", error);
   
    std::cout << "----------> " << example.get_mem_value("test_var", error) << std::endl;  


    //example.save_mem_stack_to_file("output.dat");
    example.dump_stack_memory();

    //std::cout << example.compare_mem_value("example_1", "test_var", error) << std::endl;
}
