#include <celero/Celero.h>

#include "cmdline.hpp"
#include "bench_rbtree.hpp"

int main(int argc, char ** argv)
{
	Cmdline & cmd = Cmdline::get();
	cmd.init(argc, argv);
	
	celero::Run(cmd.get_remaining_argc(), cmd.get_remaining_argv());                                                                                          
	
	return 0;
}
