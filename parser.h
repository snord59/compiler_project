/*
 *
 *
 *
 *
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <tuple>
#include "lexer.h"

//typedef std::tuple<int, int, string, string, string>;


class Parser {
	private:
		LexicalAnalyzer lexer;

		void syntax_error();
		Token expect(TokenType expected_type);
		Token peek();
//		void allocate_mem(ID x);
//		void parse_input();
		void parse_program();
		void parse_proc_decl_section();
		void parse_proc_decl();
		void parse_procedure_name();
		void parse_procedure_body();
		void parse_statement_list();
		void parse_statement();
		void parse_input_statement();
		void parse_output_statement();
		void parse_procedure_invocation();
		void parse_do_statement();
		void parse_assign_statement();
		std::tuple<int, int, int> parse_expr();
		int parse_operator();
		void parse_primary();
		void parse_main();
		void parse_inputs();		
		bool search_table(Token t);
		void new_symbol(Token t);	
		void new_statement(int type, int lhs, int optr, int op1, int op2);
	public:
		int main();
		void parse_input();
		void execute_program(struct statement_node* s_head);
};

#endif
