# Code Example
```
// compile: g++ -std=c++11 -Iinc -Llib -DARC64 -o main main.cpp -ltagged_memory -lintlen
// must install intlen url: http://github.com/mrdoomlittle/intlen/

# include <tagged_memory.hpp>
# include <cstdio>
int main() {
	bool is_error = false;

	mdl::tagged_memory::extra_options_t eo;

	mdl::tmem_t example(
		/*stack size in bytes*/128,
		/*tags that will be used*/{'{', ';', '}'},
		eo,
		/*print debug info to terminal*/false
	);

	example.dump_into_stack("{ex_0;testing} {ex_1<2>;test,hello}");

	example.analyze_stack_memory(is_error);

    char *temp = example.get_mem_value("ex_0", mdl::null_idc, is_error);

    printf("output: %s\n", temp);
    std::free(temp);

	// example.get_mem_value("/*name of mem*/", mdl::null_idc, is_error); <- returns char *

	// example.set_mem_value("/*name of mem*/", "/*value to set*/", mdl::null_idc, is_error);

	// example.free_mem("/*name of mem*/, is_error, true/*set mem to blank*/");
    return 0;
}
```
