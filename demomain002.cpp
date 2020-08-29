#include "MathParser.h"
#include <iomanip>
#include <iostream>

#ifdef DEBUG
#define pass(name, value) std::cout << name << ": [" << value << "]\n"
#define pause(name, value) pass(name, value); std::cin.get()
#endif

int main(int argc, char** argv) {
	alg::expr_ptr tree;
	alg::parse::BookKeep myBookKeep;
	
	myBookKeep._named_values["what"] = std::make_shared<flt_t>(11.7889);
	
	if (myBookKeep.new_tree(tree, "2 + 3 * (13.2 - 1) - what")) {
		flt_t value;
		
		if (tree->evaluate(value)) {
			std::cout << value << '\n';
		}
	}
	
	return 0;
}
