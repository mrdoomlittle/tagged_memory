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
	mdl::echar_t * temp = nullptr;
	mdl::tagged_memory::id_cache_t id_cache;
	id_cache.caching = true;
    id_cache.call_amount = 99;

	mdl::tagged_memory::extra_options_t extra_options;

	mdl::tmem_t example(128, {'{', ';', '}'}, extra_options, false);

	// benchmark test 0
	sw_begin_set();

	example.dump_into_stack("{benchmark_list;LIST,LIST,LIST,LIST}{benchmark_string;NULL}");

	sw_end_set();
	sw_cal_time();
	print_result("dump_info_stack", "{benchmark_list;LIST,LIST,LIST,LIST}{benchmark_string;NULL}");

	// benchmark test 1
	sw_begin_set();

	example.analyze_stack_memory(is_error);

	sw_end_set();
    sw_cal_time();
	print_result("analyze_stack_memory", "");

	printf("\n");

	// benchmark test 2
	sw_begin_set();

	example.set_mem_value("benchmark_list<2>", "Hello World!", mdl::null_idc, is_error);

	sw_end_set();
    sw_cal_time();
    print_result("set_mem_value", "benchmark_list<2> = Hello World!");

	// benchmark test 3
	sw_begin_set();

    temp = example.get_mem_value("benchmark_list<2>", mdl::null_idc, is_error);

    sw_end_set();
    sw_cal_time();

	std::free(temp);

    print_result("get_mem_value", "benchmark_list<2>");

	printf("\n");

	// benchmark test 4
	sw_begin_set();

    example.set_mem_value("benchmark_string", "Hello World!", mdl::null_idc, is_error);

    sw_end_set();
    sw_cal_time();
    print_result("set_mem_value", "benchmark_string = Hello World!");


	// benchmark test 5
	sw_begin_set();

    temp = example.get_mem_value("benchmark_string", mdl::null_idc, is_error);

    sw_end_set();
    sw_cal_time();

	std::free(temp);

	print_result("get_mem_value", "benchmark_string");

	printf("\n");

	std::size_t o = 0;

	// benchmark test 6
	sw_begin_set();

	while(o != 100) {
		example.set_mem_value("benchmark_list<2>", "Hello World!", id_cache, is_error);
		o ++;
	}

	sw_end_set();
    sw_cal_time();

	o = 0;
	id_cache.call_count = 0;

    print_result("set_mem_value", "benchmark_list<2> = Hello World! {100}");

	// benchmark test 7
    sw_begin_set();

    while(o != 100) {
        temp = example.get_mem_value("benchmark_list<2>", id_cache, is_error);
		std::free(temp);
        o ++;
    }

	o = 0;
    id_cache.call_count = 0;

    sw_end_set();
    sw_cal_time();

	print_result("get_mem_value", "benchmark_list<2> {100}");

	printf("\n");

	// benchmark test 8
	sw_begin_set();

    while(o != 100) {
        example.set_mem_value("benchmark_string", "Hello World!", id_cache, is_error);
        o ++;
    }

    sw_end_set();
    sw_cal_time();

    o = 0;
    id_cache.call_count = 0;

    print_result("set_mem_value", "benchmark_string = Hello World! {100}");

	// benchmark test 9
	sw_begin_set();

    while(o != 100) {
        temp = example.get_mem_value("benchmark_string", id_cache, is_error);
        std::free(temp);
        o ++;
    }

    o = 0;
    id_cache.call_count = 0;

    sw_end_set();
    sw_cal_time();

    print_result("get_mem_value", "benchmark_string {100}");
}
