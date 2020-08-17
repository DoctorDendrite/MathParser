#pragma once

#include "types.h"
#include <map>
#include <math.h>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace alg
{
	typedef std::vector<flt_t> args_t;
	
	#define carg_ref const args_t &
	
	typedef flt_t (*math_f)(carg_ref);
	typedef std::map<size_t, math_f> overloads_t;
	
	static const std::map<std::string, overloads_t>
	_functions = {
		  { "sin",   { { 1, [](carg_ref a) { return sinl(a[0]); } } } }  // SINE
		, { "cos",   { { 1, [](carg_ref a) { return cosl(a[0]); } } } }  // COSINE
		, { "tan",   { { 1, [](carg_ref a) { return tanl(a[0]); } } } }  // TANGENT
		, { "sec",   { { 1, [](carg_ref a) { return 1.L/cosl(a[0]); } } } }  // SECANT
		, { "csc",   { { 1, [](carg_ref a) { return 1.L/sinl(a[0]); } } } }  // COSECANT
		, { "cot",   { { 1, [](carg_ref a) { return 1.L/tanl(a[0]); } } } }  // COTANGENT
		, { "asin",  { { 1, [](carg_ref a) { return asinl(a[0]); } } } }  // ARCSINE
		, { "acos",  { { 1, [](carg_ref a) { return acosl(a[0]); } } } }  // ARCCOSINE
		, { "atan",  { { 1, [](carg_ref a) { return atanl(a[0]); } } } }  // ARCTANGENT
		, { "ln",    { { 1, [](carg_ref a) { return logl(a[0]); } } } }  // NATURAL_LOGARITHM
		, { "lg",    { { 1, [](carg_ref a) { return log2l(a[0]); } } } }  // BINARY_LOGARITHM
		, { "log",   { { 1, [](carg_ref a) { return log10l(a[0]); } } } }  // DECIMAL_LOGARITHM
		, { "exp",   { { 1, [](carg_ref a) { return expl(a[0]); } } } }  // NATURAL_EXPONENT
		, { "sqrt",  { { 1, [](carg_ref a) { return sqrtl(a[0]); } } } }  // SQUARE_ROOT
		, { "floor", { { 1, [](carg_ref a) { return floorl(a[0]); } } } }  // FLOOR
		, { "ceil",  { { 1, [](carg_ref a) { return ceill(a[0]); } } } }  // CEILING
		, { "abs",   { { 1, [](carg_ref a) { return fabsl(a[0]); } } } }  // ABSOLUTE_VALUE
		, { "sign",  { { 1, [](carg_ref a) { return a[0] < 0 ? -1.L : 1.L; } } } }  // SIGN
		, { "min",   { { 2, [](carg_ref a) { return fminl(a[0], a[1]); } } } }  // MINIMUM
		, { "max",   { { 2, [](carg_ref a) { return fmaxl(a[0], a[1]); } } } }  // MAXIMUM
		, { "pow",   { { 2, [](carg_ref a) { return powl(a[0], a[1]); } } } }  // POWER
	};
	
	#undef carg_ref
	
	class Expression {
	public:
		virtual bool evaluate(flt_t&) = 0;
		virtual ~Expression() = default;
	};
	
	typedef std::unique_ptr<alg::Expression> expr_ptr;
	
	class Binary: public Expression {
	private:
		char _op;
		expr_ptr _left;
		expr_ptr _right;
	public:
		Binary(char op, expr_ptr left, expr_ptr right);
		virtual ~Binary() override;
		virtual bool evaluate(flt_t&) override;
	};
	
	class Unary: public Expression {
	private:
		char _op;
		expr_ptr _right;
	public:
		Unary(char op, expr_ptr right);
		virtual ~Unary() override;
		virtual bool evaluate(flt_t&) override;
	};
	
	typedef std::shared_ptr<flt_t> ref_t;
	
	class Reference: public Expression {
	private:
		ref_t _ptr;
	public:
		Reference(ref_t& ptr);
		virtual ~Reference() override;
		virtual bool evaluate(flt_t&) override;
	};
	
	class Terminal: public Expression {
	private:
		flt_t _payload;
	public:
		Terminal(flt_t payload);
		virtual ~Terminal() override;
		virtual bool evaluate(flt_t&) override;
	};
	
	class Functional: public Expression {
	private:
		std::string _name;
		std::vector<expr_ptr> _arguments;
	public:
		Functional(const std::string& name);
		virtual ~Functional() override;
		virtual bool evaluate(flt_t&) override;
		Functional& push(expr_ptr argument);
		size_t argc() const;
	};
	
	enum class Tokens {
		  BUFFER_EOF = -1
		, IDENTIFIER = -2
		, TERMINAL = -3
		, OPEN_GROUP = -4
		, CLOSE_GROUP = -5
		, DELIMITER = -6
	};
	
	const static std::map<char, int> _precedence = {
		  {'+', 20}
		, {'-', 20}
		, {'*', 40}
		, {'/', 40}
		, {'^', 60}
	};
	
	class lex
	{
	private:
		static size_t _index;
		static char _element;
		static int _token;
		static flt_t _terminal;
		static std::string _identifier;
		static std::string _buf;
		static std::map<std::string, flt_t> _named_values;
	public:
		static bool any();
		static char next_element();
		static char start(const std::string& expression);
		static int next_token();
		
		static int token();
		static flt_t terminal();
		static const std::string& identifier();
		static flt_t& named_value(const std::string& name);
	};
	
	namespace parse
	{
		class Exception {
		private:
			std::string _msg;
		public:
			Exception(const std::string& msg);
			const std::string& msg() const;
		};
		
		void start(const std::string& buf);
		
		int current_precedence();
		
		class BookKeep {
		public:
			typedef std::map<std::string, ref_t> names_t;
		private:
			names_t _named_values;
		public:
			BookKeep() = default;
			virtual ~BookKeep() = default;
			
			bool new_error(expr_ptr& tree, const std::string& msg);
			bool new_terminal(expr_ptr& tree);
			bool new_group(expr_ptr& tree);
			bool new_identifier(expr_ptr& tree);
			bool new_primary(expr_ptr& tree);
			bool new_expression(expr_ptr& tree);
			bool new_binary_right(expr_ptr& tree, int exprPrec, expr_ptr left);
			bool new_tree(expr_ptr& tree, const std::string& buf);
		};
	};
	
	void test_lexer(std::ostream& out);
};
