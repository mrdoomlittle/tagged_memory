# include <tagged_memory.hpp>
# include <iostream>

int main()
{
    // some functions take a bool ref in and will set it to true if there was a problem doing somthing
    bool error = false;
   
    std::cout << "debugging has started" << std::endl;

    mdl::tmem_t example(128/*size is chars/bytes*/, {':', ';', '~'}/*tags B/E/S*/, true/*this turns on debugging*/);

    example.dump_into_stack(":HOHO[2]~HELL,HEVEN;:hello[2]~A,B;");
    example.analyze_stack_memory(error);

    
   // example.set_mem_value("hello[0]", "HELLO_WORLD", error);

    std::cout << example.get_mem_value("HOHO", error, 0, true) << std::endl;
    example.set_mem_value("HOHO", "MOMO", error);

    std::cout << example.get_mem_value("hello[0]", error, 0, true) << std::endl;    

    
 
//    example.load_mem_stack_from_file("output.dat");
 

   
    example.set_mem_value("hello[0]", "HELLO_WORLD", error);
  
    //example.set_mem_name("example_1", "by_mrdoomlittle", error);
  
    for (std::size_t i = 0; i != example.get_list_length("hello", error); i ++) 
        std::cout << "----------> " << example.get_mem_value("hello", error, i, false) << std::endl;  
  
    std::cout << "----------> " << example.get_mem_value("hello[0]", error, 0, true) << std::endl;
 
    //example.save_mem_stack_to_file("output.dat");
    example.dump_stack_memory();

    //std::cout << example.compare_mem_value("example_1", "test_var", error) << std::endl;
}
