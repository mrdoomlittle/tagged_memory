## Code Example
'''
# include <tagged_memory.hpp>
int main() {
	bool is_error = false;

	mdl::tagged_memory::extra_options_t eo;

	mdl::tmem_t example(
		/*stack size in bytes*/128,
		/*tags that will be used*/{'{', ';', '}'},
		eo,
		/*print debug info to terminal*/false
	);
	
	example.dump_into_stack("{ex_0;testing} {ex_0<2>;test,hello}");

	example.analyze_stack_memory(is_error);

	// example.get_mem_value("/*name of mem*/", mdl::null_idc, is_error); <- returns char *

	// example.set_mem_value("/*name of mem*/", "/*value to set*/", mdl::null_idc, is_error);
}
'''
