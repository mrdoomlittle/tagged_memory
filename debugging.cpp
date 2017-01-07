# include <tagged_memory.hpp>
# include <iostream>

int main()
{
    bool error = false;

    mdl::tmem_t example(128, {'{', ';', '}'}, false/*debug info*/);

    example.dump_into_stack("{example_0;ex[0]}{example_1;ex[1]}");

    example.analyze_stack_memory(error);

    mdl::tagged_memory::error_info_t error_info;

    mdl::tagged_memory::mem_t * ex[2];
    ex[0] = new mdl::tagged_memory::mem_t(0, (& example), &error_info);
    ex[1] = new mdl::tagged_memory::mem_t(1, (& example), &error_info);
    
    for (std::size_t o = 0; o != 2; o ++) { 
        for (std::size_t i = 0; i != ex[o]-> get_len(); i ++)
            std::cout << "----> " << (* ex[o])[i] << std::endl;

        std::cout << "\n";
    }
    example.analyze_stack_memory(error);

    return 0;
}
