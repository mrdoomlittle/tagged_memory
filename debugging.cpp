# include <tagged_memory.hpp>
# include <iostream>

int main()
{
    bool error = false;
    mdl::tmem_t example(128/*size is chars/bytes*/, {':', '~', ';'}/*tags B/M/E*/, true/*this turns on debugging*/);

    example.dump_into_stack(":dan~test;   :tmp~loser;");

    example.analyze_stack_memory(error);

    example.set_mem_name("dan", "hello12", error);

    std::cout << "-> " << example.get_mem_value("tmp", error, 0, true) << std::endl;

//    example.add_mem_tag("hello[22283636]", "", 0, error);

    example.add_mem_tag("ex[2]", "'Hello,People',pep", 12, error);
    std::cout << "-> " << example.get_mem_value("ex[0]", error, 0, true) << std::endl;
    example.dump_stack_memory();
return 0;
/*
    // some functions take a bool ref in and will set it to true if there was a problem doing somthing
    bool error = false;
   
    std::cout << "debugging has started" << std::endl;

    example.dump_into_stack(":dan[2]~A,B;:config~:mrdoomlittle~Dx20;;:william_druss[2]~B,A;");
    example.analyze_stack_memory(error);

    //example.set_mem_value("mrdoomlittle[0]", "?", error);
    std::cout << "-:> --  " << example.get_mem_value("config", error, 0, true) << std::endl;
    std::cout << "-:> --  " << example.get_mem_value("william_druss[0]", error, 0, true) << std::endl;
    std::cout << "-:> -- " << example.get_mem_value("william_druss", error, 0, true) << std::endl;
    std::cout << "-:> " << example.get_mem_addr("william_druss", error) << std::endl;

    example.set_mem_value("config", "configr", error);

    example.dump_stack_memory();
return 0;  
   // example.set_mem_value("hello[0]", "HELLO_WORLD", error);

    std::cout << example.get_mem_value("HOHO", error, 0, true) << std::endl;
    example.set_mem_value("HOHO[0]", "MOMO", error);

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
*/
}
