#include "MathParser.h"


void alg::test_lexer(std::ostream& out) {
	switch ((Tokens)lex::token()) {
	case Tokens::IDENTIFIER:
		out << "Function Name: [" << lex::identifier() << "]\n";
		break;
	case Tokens::TERMINAL:
		out << "Terminal: [" << lex::terminal() << "]\n";
		break;
	case Tokens::OPEN_GROUP:
		out << "Open Group\n";
		break;
	case Tokens::CLOSE_GROUP:
		out << "Close Group\n";
		break;
	case Tokens::DELIMITER:
		out << "Delimiter\n";
		break;
	default:
		out << "Operator: [" << (char)lex::token() << "]\n";
		break;
	}
}


alg::Binary::Binary(char op, alg::expr_ptr left, alg::expr_ptr right):
	_op(op), _left(std::move(left)), _right(std::move(right)) {}
	
alg::Binary::~Binary() {}

flt_t alg::Binary::evaluate() {
	switch (_op) {
		case '+': return _left->evaluate() + _right->evaluate();
		case '-': return _left->evaluate() - _right->evaluate();
		case '*': return _left->evaluate() * _right->evaluate();
		case '/': return _left->evaluate() / _right->evaluate();
		case '^': return pow(_left->evaluate(), _right->evaluate());
	}
	
	return 0;
}


alg::Unary::Unary(char op, alg::expr_ptr right):
	_op(op), _right(std::move(right)) {}
	
alg::Unary::~Unary() {}

flt_t alg::Unary::evaluate() {
	switch (_op) {
		case '-': return -_right->evaluate();
	}
	
	return 0;
}


alg::Name::Name(const std::string& payload):
	_payload(payload) {}
	
alg::Name::~Name() {}

flt_t alg::Name::evaluate() {
	return lex::named_value(_payload);
}


alg::Terminal::Terminal(flt_t payload):
	_payload(payload) {}
	
alg::Terminal::~Terminal() {}

flt_t alg::Terminal::evaluate() {
	return _payload;
}


alg::Functional::Functional(const std::string& name):
	_name(name) {}

alg::Functional::~Functional() {}

flt_t alg::Functional::evaluate() {
	args_t args;
	
	for (auto& arg : _arguments)
		args.push_back(arg->evaluate());
	
	auto overloads = _functions.at(_name);
	
	if (args.size() == 0)
		return 0.L;
	
	return overloads.at(args.size())(args);
}

alg::Functional& alg::Functional::push(alg::expr_ptr argument) {
	_arguments.push_back(std::move(argument));
	return *this;
}


std::map<std::string, flt_t> alg::lex::_named_values;
size_t alg::lex::_index;
char alg::lex::_element;
int alg::lex::_token;
flt_t alg::lex::_terminal = 0.L;
std::string alg::lex::_identifier;
std::string alg::lex::_buf;

int alg::lex::token() {
	return _token;
}

flt_t alg::lex::terminal() {
	return _terminal;
}

const std::string & alg::lex::identifier() {
	return _identifier;
}

flt_t& alg::lex::named_value(const std::string& name) {
	return lex::named_value(name);
}

bool alg::lex::any() {
	return _index <= _buf.length();
}

char alg::lex::next_element() {
	_element = _buf[_index++];
	return _element;
}

char alg::lex::start(const std::string& expression) {
	_buf = expression;
	_index = 0;
	return next_element();
}

int alg::lex::next_token() {
	if (!any()) {
		return _token = (int)Tokens::BUFFER_EOF;
	}
	else {
		while (isspace(_element))
			next_element();
		
		if (_element == ',') {
			next_element();
			_token = (int)Tokens::DELIMITER;
		}
		else if (_element == '(') {
			next_element();
			_token = (int)Tokens::OPEN_GROUP;
		}
		else if (_element == ')') {
			next_element();
			_token = (int)Tokens::CLOSE_GROUP;
		}
		else if (isalpha(_element) || _element == '_') {
			std::string temp;
			
			while (any() && (isalpha(_element) || isdigit(_element) || _element == '_')) {
				temp += _element;
				next_element();
			}
			
			_identifier = temp;
			_token = (int)Tokens::IDENTIFIER;
		}
		else if (isdigit(_element)) {
			bool separator = false;
			std::string temp;
			
			while (any() && (isdigit(_element) || (!separator && _element == '.'))) {
				if (_element == '.')
					separator = true;
				
				temp += _element;
				next_element();
			}
			
			_terminal = std::stold(temp);
			_token = (int)Tokens::TERMINAL;
		}
		else if (_element == '.') {
			std::string temp;
			temp += _element;
			next_element();
			
			while (any() && (isdigit(_element))) {
				temp += _element;
				next_element();
			}
			
			_terminal = std::stold(temp);
			_token = (int)Tokens::TERMINAL;
		}
		else if (ispunct(_element)) {
			auto op = (int)_element;
			next_element();
			_token = op;
		}
		else {
			_token = (int)Tokens::BUFFER_EOF;
		}
	}
	
	#ifdef DEBUG
	test_lexer();
	#endif
	
	return _token;
}


void alg::parse::start(const std::string& buf) {
	alg::lex::start(buf);
	alg::lex::next_token();
}

int alg::parse::current_precedence() {
	if (!isascii(lex::token()))
		return -1;
	
	int prec = _precedence.at(lex::token());
	
	if (prec <= 0)
		return -1;
	
	return prec;
}

alg::expr_ptr alg::parse::new_error(const char* msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return nullptr;
}

alg::expr_ptr alg::parse::new_terminal() {
	auto result = std::make_unique<Terminal>(lex::terminal());
	lex::next_token();
	return std::move(result);
}

alg::expr_ptr alg::parse::new_group() {
	lex::next_token();
	auto ptr = new_expression();
	
	if (!ptr)
		return nullptr;
	
	if ((Tokens)lex::token() != Tokens::CLOSE_GROUP)
		return new_error("expected ')'");
	
	lex::next_token();
	return std::move(ptr);  // TODO: Try `std::move`
}

alg::expr_ptr alg::parse::new_identifier() {
	std::string name = lex::identifier();
	lex::next_token();
	
	if ((Tokens)lex::token() != Tokens::OPEN_GROUP)
		return std::make_unique<Name>(name);
	
	lex::next_token();
	auto functionCall = std::make_unique<Functional>(name);
	
	while ((Tokens)lex::token() != Tokens::CLOSE_GROUP) {
		if (auto arg = new_expression()) {
			functionCall->push(std::move(arg));
		}
	}
	
	lex::next_token();
	return functionCall;
}

alg::expr_ptr alg::parse::new_primary() {
	int op;
	
	switch ((Tokens)lex::token()) {
	default:
		op = lex::token();
		lex::next_token();
		return std::make_unique<Unary>(op, std::move(new_primary()));
	case Tokens::IDENTIFIER:
		return new_identifier();
	case Tokens::TERMINAL:
		return new_terminal();
	case Tokens::OPEN_GROUP:
		return new_group();
	case Tokens::CLOSE_GROUP:
	case Tokens::DELIMITER:
		lex::next_token();
		return nullptr;
	}
	
	// return new_error("unknown token when expecting an expression");
}

alg::expr_ptr alg::parse::new_expression() {
	auto left = new_primary();
	
	if (!left)
		return nullptr;
	
	return new_binary_right(0, std::move(left));
}

alg::expr_ptr alg::parse::new_binary_right(int exprPrec, expr_ptr left) {
	while (lex::any()) {
		int prec = current_precedence();
		
		if (prec < exprPrec)
			return left;
		
		int op = lex::token();
		lex::next_token();
		auto right = new_primary();
		
		if (!right)
			return nullptr;
		
		if (prec <= current_precedence()) {
			right = new_binary_right(prec, std::move(right));
			
			if (!right)
				return nullptr;
		}
		
		left = std::make_unique<Binary>(op, std::move(left), std::move(right));
	}
	
	return left;
}

alg::expr_ptr alg::parse::new_tree(const std::string& buf) {
	start(buf);
	return new_expression();
}
