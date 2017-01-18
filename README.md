NOTE: this was only for fun. example code in debugging.cpp

if i had a string like this "{example<2>;a,b}" and i want to change the first element to "hello" i can do this:
/*instance of tmem_t*/.set_mem_value("example<0>", "hello", /* bool & 'any error/s it will change to true' */);
OUTPUT: "{example<2>;hello,b}"
