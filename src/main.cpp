#include "MathParser.h"
#include <cstring>
#include <iomanip>
#include <iostream>

#ifdef DEBUG
#define pass(name, value) std::cout << name << ": [" << value << "]\n"
#define pause(name, value) pass(name, value); std::cin.get()
#endif

int main(int argc, char** argv) {
	if (argc > 2) {
		if (!strcmp(argv[1], "lex")) {
			alg::parse::start(argv[2]);
			
			while (alg::lex::token() != (int)alg::Tokens::BUFFER_EOF) {
				alg::test_lexer(std::cout);
				alg::lex::next_token();
			}
		}
		else
		if (!strcmp(argv[1], "parse")) {
			std::cout << std::setprecision(15);
			std::cout << alg::parse::new_tree(argv[2])->evaluate() << '\n';
		}
	}
	
	return 0;
}
