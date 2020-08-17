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

bool alg::Binary::evaluate(flt_t& value) {
	flt_t left = 0.L, right = 0.L;
	
	if (!_left->evaluate(left))
		return false;
	
	if (!_right->evaluate(right))
		return false;
	
	switch (_op) {
		case '+': value = left + right;
			break;
		case '-': value = left - right;
			break;
		case '*': value = left * right;
			break;
		case '/': value = left / right;
			break;
		case '^': value = pow(left, right);
			break;
	}
	
	return true;
}


alg::Unary::Unary(char op, alg::expr_ptr right):
	_op(op), _right(std::move(right)) {}
	
alg::Unary::~Unary() {}

bool alg::Unary::evaluate(flt_t& value) {
	flt_t right = 0.L;
	
	if (!_right->evaluate(right))
		return false;
	
	switch (_op) {
		case '-': value = -right;
			break;
	}
	
	return true;
}


alg::Reference::Reference(ref_t& ptr):
	_ptr(ptr) {}
	
alg::Reference::~Reference() {}

bool alg::Reference::evaluate(flt_t& value) {
	if (_ptr == nullptr)
		return false;
	
	value = *_ptr;
	return true;
}


alg::Terminal::Terminal(flt_t payload):
	_payload(payload) {}
	
alg::Terminal::~Terminal() {}

bool alg::Terminal::evaluate(flt_t& value) {
	value = _payload;
	return true;
}


alg::Functional::Functional(const std::string& name):
	_name(name) {}

alg::Functional::~Functional() {}

bool alg::Functional::evaluate(flt_t& value) {
	args_t args;
	flt_t temp;
	
	for (auto& arg : _arguments) {
		if (!arg->evaluate(temp))
			return false;
		
		args.push_back(temp);
	}
	
	auto overloads = _functions.at(_name);
	value = overloads.at(args.size())(args);
	return true;
}

alg::Functional& alg::Functional::push(alg::expr_ptr argument) {
	_arguments.push_back(std::move(argument));
	return *this;
}

size_t alg::Functional::argc() const {
	return _arguments.size();
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


alg::parse::Exception::Exception(const std::string& msg):
	_msg(msg) {}
	
const std::string& alg::parse::Exception::msg() const {
	return _msg;
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

bool alg::parse::BookKeep::new_error(alg::expr_ptr& tree, const std::string& msg) {
	fprintf(stdout, "Error: %s\n", msg.c_str());
	return false;
}

bool alg::parse::BookKeep::new_terminal(alg::expr_ptr& tree) {
	auto result = std::make_unique<Terminal>(lex::terminal());
	lex::next_token();
	tree = std::move(result);
	return true;
}

bool alg::parse::BookKeep::new_group(alg::expr_ptr& tree) {
	expr_ptr node = nullptr;
	lex::next_token();
	
	if (!new_expression(node))
		return false;
	
	if (!node) {
		tree = nullptr;
		return true;
	}
	
	if ((Tokens)lex::token() != Tokens::CLOSE_GROUP) {
		return new_error(tree, "expected ')'");
	}
	
	lex::next_token();
	tree = std::move(node);
	return true;
}

bool alg::parse::BookKeep::new_identifier(alg::expr_ptr& tree) {
	std::string name = lex::identifier();
	lex::next_token();
	
	if ((Tokens)lex::token() != Tokens::OPEN_GROUP) {
		auto name_it = _named_values.find(name);
		
		if (name_it == _named_values.end())
			return new_error(tree, std::string("no binding with name '" + name + "' is declared in this scope"));
		
		// Note: `std::map::find` doesn't enumerate the values in the map;
		// it enumerates the key-value pairs in the map.
		tree = std::make_unique<Reference>(name_it->second);
		return true;
	}
	
	lex::next_token();
	auto function_it = _functions.find(name);
	
	if (function_it == _functions.end())
		return new_error(tree, std::string("no definition found with the name '" + name + "'"));
	
	auto functionCall = std::make_unique<Functional>(name);
	expr_ptr arg = nullptr;
	
	while ((Tokens)lex::token() != Tokens::CLOSE_GROUP) {
		if (!new_expression(arg))
			return false;
			
		if (arg)
			functionCall->push(std::move(arg));
	}
	
	// Note: `std::map::find` doesn't enumerate the values in the map;
	// it enumerates the key-value pairs in the map.
	if (function_it->second.find(functionCall->argc()) == overloads.end())
		return new_error(
			tree,
			std::string(
				"no overload found for definition '"
				+ name
				+ "' that takes "
				+ std::to_string(functionCall->argc())
				+ " arguments"
			)
		);
	
	lex::next_token();
	tree = std::move(functionCall);
	return true;
}

bool alg::parse::BookKeep::new_primary(alg::expr_ptr& tree) {
	int op;
	expr_ptr node = nullptr;
	
	switch ((Tokens)lex::token()) {
	default:
		op = lex::token();
		lex::next_token();
		
		if (!new_primary(node))
			return false;
		
		tree = std::move(std::make_unique<Unary>(op, std::move(node)));
		break;
	case Tokens::IDENTIFIER:
		if (!new_identifier(node))
			return false;
		
		tree = std::move(node);
		break;
	case Tokens::TERMINAL:
		if (!new_terminal(node))
			return false;
		
		tree = std::move(node);
		break;
	case Tokens::OPEN_GROUP:
		if (!new_group(node))
			return false;
		
		tree = std::move(node);
		break;
	case Tokens::CLOSE_GROUP:
	case Tokens::DELIMITER:
		lex::next_token();
		tree = nullptr;
		break;
	}
	
	return true;
	// return new_error(tree, "unknown token when expecting an expression");
}

bool alg::parse::BookKeep::new_expression(alg::expr_ptr& tree) {
	expr_ptr left = nullptr;
	
	if (!new_primary(left))
		return false;
	
	if (!left) {
		tree = nullptr;
		return true;
	}
	
	if (!new_binary_right(tree, 0, std::move(left)))
		return false;
	
	return true;
}

bool alg::parse::BookKeep::new_binary_right(alg::expr_ptr& tree, int exprPrec, expr_ptr left) {
	while (lex::any()) {
		int prec = current_precedence();
		
		if (prec < exprPrec) {
			tree = std::move(left);
			return true;
		}
		
		int op = lex::token();
		lex::next_token();
		expr_ptr right = nullptr;
		
		if (!new_primary(right))
			return false;
		
		if (!right) {
			tree = nullptr;
			return true;
		}
		
		if (prec <= current_precedence()) {
			expr_ptr temp = std::move(right);
			
			if (!new_binary_right(right, prec, std::move(temp)))
				return false;
			
			if (!right) {
				tree = nullptr;
				return true;
			}
		}
		
		left = std::make_unique<Binary>(op, std::move(left), std::move(right));
	}
	
	tree = std::move(left);
	return true;
}

bool alg::parse::BookKeep::new_tree(alg::expr_ptr& tree, const std::string& buf) {
	start(buf);
	return new_expression(tree);
}
