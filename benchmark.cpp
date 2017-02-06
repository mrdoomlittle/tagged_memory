# include <tagged_memory.hpp>
# include <echar_t.hpp>
# include <cstdio>
# include <chrono>

auto sw_begin = std::chrono::high_resolution_clock::now();
auto sw_end = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> sw_time_span;

void sw_begin_set() {
	sw_begin = std::chrono::high_resolution_clock::now();
}

void sw_end_set() {
	sw_end = std::chrono::high_resolution_clock::now();
}

void sw_cal_time() {
	sw_time_span = std::chrono::duration_cast<std::chrono::duration<double>>(sw_end - sw_begin);
}

void print_result(mdl::echar_t const * __func_name, mdl::echar_t const * __extra_info)
{
	printf("\x1b[32m%s\x1b[0m, \x1b[35m[%lf] \x1b[0m- \x1b[36m%s\n", __func_name, sw_time_span.count(), __extra_info);
}

int main() {
	bool is_error = false;
	mdl::tagged_memory::extra_options_t extra_options;

	mdl::tmem_t example(128, {'{', ';', '}'}, extra_options, false);

	sw_begin_set();

	example.dump_into_stack("{benchmark_list;LIST,LIST,LIST,LIST}{benchmark_string;NULL}");

	sw_end_set();
	sw_cal_time();
	print_result("dump_info_stack", "{benchmark_list;LIST,LIST,LIST,LIST}{benchmark_string;NULL}");

	sw_begin_set();

	example.analyze_stack_memory(is_error);

	sw_end_set();
    sw_cal_time();
	print_result("analyze_stack_memory", "");
}
