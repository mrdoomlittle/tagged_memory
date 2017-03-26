# include <tagged_memory.hpp>
# include <iostream>
# include <chrono>
// functions for tmem_t. need more look at src/tagged_memory.hpp
/*
    .does_mem_name_exist(char const *, bool &)
    .add_mem_tag(char const *, char const *, size_t, bool &)
    .set_mem_name(char const *, char const *, bool &)
    .set_mem_value(char const *, char const *, bool &)
    .get_mem_val(char const *, bool &)
    .get_list_length(char const *, bool &)
*/

int main() {
	boost::int8_t any_error = TMEM_SUCCESS;

	mdl::tagged_memory::extra_opt_t options;
	mdl::tmem_t example(128, {'{', ';', '}'}, options, true);

	if (example.stack_dump(";{example_0<2>;NULL,21299}        {example_1<2>;a,b}") != TMEM_SUCCESS) {
		return 0;
	}

	example.analyze_stack(any_error);
	if (any_error != TMEM_SUCCESS) {
		return 0;
	}

	char *mem_val = example.get_mem_val("example_0<1>", mdl::null_idc, any_error);
	printf("\n");
	printf("got string: %s\n", mem_val);
}

