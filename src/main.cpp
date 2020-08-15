#include "MathParser.h"
#include <iomanip>
#include <iostream>

#ifdef DEBUG
#define pass(name, value) std::cout << name << ": [" << value << "]\n"
#define pause(name, value) pass(name, value); std::cin.get()
#endif

bool strequal(const std::string& first, const std::string& secnd);

int main(int argc, char** argv) {
	if (argc > 2) {
		if (strequal(argv[1], "lex")) {
			alg::parse::start(argv[2]);
			
			while (alg::lex::token() != (int)alg::Tokens::BUFFER_EOF) {
				alg::test_lexer(std::cout);
				alg::lex::next_token();
			}
		}
		else
		if (strequal(argv[1], "parse")) {
			std::cout << std::setprecision(15);
			alg::expr_ptr tree;
			
			if (alg::parse::new_tree(tree, argv[2])) {
				flt_t value;
				
				if (tree->evaluate(value)) {
					std::cout << value << '\n';
				}
			}
		}
	}
	
	return 0;
}

bool strequal(const std::string& first, const std::string& secnd) {
	if (first.length() != secnd.length())
		return false;
	
	for (int i = 0; i < first.length(); ++i)
		if (tolower(first[i]) != tolower(secnd[i]))
			return false;
	
	return true;
}
