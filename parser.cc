
/*
 *
 * Author: Sophia Nordlie
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include <map>
#include <string>
#include "parser.h"

using namespace std;

int next_available = 0;
int mem[2000];
int input_line[50];
int current_input = 0;
int input_size = 0;	// counter

void Parser::syntax_error() {
	cout << "SYNTAX ERROR\n";
	exit(1);
}

std::map<std::string, int> symbol_table;	// map for symbol table

struct statement_node {
	int statement_type;
	int LHS;
	int optr;
	int op1;
	int op2;
	struct statement_node* next;
}; statement_node *s_head = NULL;
statement_node *s_tail = NULL;

Token Parser::expect(TokenType expected_type) {
	Token t = lexer.GetToken();
	if(t.token_type != expected_type) {
		syntax_error();
	}
	return t;
}

Token Parser::peek() {
	Token t = lexer.GetToken();
	lexer.UngetToken(t);
	return t;
}

bool Parser::search_table(Token t) {
	if(symbol_table.find(t.lexeme) != symbol_table.end()) {
		return true;	// symbol already exists
	}
	else {
		return false;	// symbol needs to be added
	}
}

void Parser::new_symbol(Token t) {
        symbol_table.insert(std::make_pair(t.lexeme, next_available));
	mem[next_available] = 0;
        next_available++;
}

void Parser::new_statement(int type, int lhs, int optr, int op1, int op2) {
	statement_node *temp = new statement_node;
	temp->statement_type = type;
	temp->LHS = lhs;
	temp->optr = optr;
	temp->op1 = op1;
	temp->op2 = op2;
	
	if(s_head == NULL){	// statement list empty
	
		s_head = temp;
		s_tail = temp;	
	}
	else {			// add statement to end
	
		s_tail->next = temp;
		s_tail = s_tail->next;
	}
}

void Parser::parse_input() {
	// input --> program inputs
	Token t = peek();
	if(t.token_type == PROC || t.token_type == MAIN) {
		parse_program();
		t = peek();
		if(t.token_type == NUM) {
			parse_inputs();
		}
		else {
			syntax_error();
		}
	}
	else {
		syntax_error();
	}
}

void Parser::parse_program() {
	// program --> main || proc_decl_section main
	Token t = peek();
	if(t.token_type == PROC) {
		parse_proc_decl_section();
		t = peek();
		if(t.token_type == MAIN) {
			parse_main();
		}
		else {
			syntax_error();
		}
	}
	else if(t.token_type == MAIN) {
		parse_main();
	}
	else {
		syntax_error();
	}

}

void Parser::parse_proc_decl_section() {
        // proc_decl_section --> proc_decl || proc_decl proc_decl_section
	parse_proc_decl();
	Token t = peek();
	if(t.token_type == PROC) {
		parse_proc_decl_section();
	}
}

void Parser::parse_proc_decl() {
        // proc_decl --> PROC procedure_name procedure_body
        Token t = expect(PROC);
        t = peek();
        if(t.token_type == ID || t.token_type == NUM) {
                parse_procedure_name();
                t = peek();
                if(t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID
                                || t.token_type == NUM || t.token_type == DO) {
                        parse_procedure_body();
                }
                else {
                        syntax_error();
                }
        }
        else {
                syntax_error();
        }
}

void Parser::parse_procedure_name() {
	// procedure_name --> ID || NUM
	Token t = peek();
	if(t.token_type == ID || t.token_type == NUM) {
		t = lexer.GetToken();
		// look up token in table
	}
	else {
		syntax_error();
	}
}

void Parser::parse_procedure_body() {
	// procedure_body--> statement_list
	Token t = peek();
	if(t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID ||
		       	t.token_type == NUM || t.token_type == DO) {
		parse_statement_list();
	}
	else {
		syntax_error();
	}
}

void Parser::parse_statement_list() {
        // statement_list--> statement || statement statement_list
        //parse_statement();
        Token t_1 = lexer.GetToken();
	Token t_2 = lexer.GetToken();
	if(t_1.token_type == ENDPROC || (t_1.token_type == NUM && t_2.token_type == NUM) || (t_1.token_type == NUM && t_2.token_type == END_OF_FILE)) {
		return;
	}
        if(t_1.token_type == INPUT || t_1.token_type == OUTPUT || t_1.token_type == ID ||
		       	(t_1.token_type == NUM && t_2.token_type == SEMICOLON) || t_1.token_type == DO) {	
		lexer.UngetToken(t_2);
		lexer.UngetToken(t_1);
		parse_statement();
                parse_statement_list();
        }
}

void Parser::parse_statement() {
	// statement--> input_statement || output_statement || procedure_invocation
	// || do_statement || assign_statement
	Token t_1 = lexer.GetToken();
	Token t_2 = lexer.GetToken();
	lexer.UngetToken(t_2);
	lexer.UngetToken(t_1);

	if((t_1.token_type == ID || t_1.token_type == NUM) && (t_2.token_type == SEMICOLON)) {
		parse_procedure_invocation();
	}
	else if((t_1.token_type == ID) && (t_2.token_type == EQUAL)) {
		parse_assign_statement();
	}
	else if((t_1.token_type == DO)) {
		parse_do_statement();
	}
	else if(t_1.token_type == INPUT) {
		parse_input_statement();
	}
	else if(t_1.token_type == OUTPUT) {
		parse_output_statement();
	}
	else {
		syntax_error();
	}
}

void Parser::parse_input_statement() {
        // input_statement--> INPUT ID SEMICOLON
	int var = 0;
        Token t = expect(INPUT);
	
        t = expect(ID);
        // assign ID if necessary
	
	if(!search_table(t)){		// symbol not found
		new_symbol(t);
	}
	var = symbol_table[t.lexeme];	// get index of variable
	
        t = expect(SEMICOLON);
	
        // build statement node
	new_statement(INPUT, 0, 0, var, 0);	// INPUT
}

void Parser::parse_output_statement() {
        // output_statement--> OUTPUT ID SEMICOLON
	int var = 0;
        Token t = expect(OUTPUT);
        
        t = expect(ID);
        // assign ID if necessary
	
	if(!search_table(t)) {	// symbol not found
		new_symbol(t);
	}
	var = symbol_table[t.lexeme];

        t = expect(SEMICOLON);
        
	// build statement node
	new_statement(OUTPUT, 0, 0, var, 0);	// OUTPUT
}

void Parser::parse_procedure_invocation() {
        // procedure_invocation-->procedure_name SEMICOLON
        Token t = peek();
        if(t.token_type == ID || t.token_type== NUM) {
                parse_procedure_name();

                t = expect(SEMICOLON);
                // put semicolon
        }
        else {
                syntax_error();
        }
}

void Parser::parse_do_statement() {
        // do_statement--> DO ID procedure_invocation
        Token t = expect(DO);   
        t = expect(ID);
        // check for ID and alloc if necessary
	if(!search_table(t)) {	// symbol not found
		new_symbol(t);
	}
        t = lexer.GetToken();
        if(t.token_type == ID || t.token_type == NUM) {
                parse_procedure_invocation();
        }
        else {
                syntax_error();
        }
}

void Parser::parse_assign_statement() {
        // assign_statement--> ID EQUAL expr SEMICOLON
	
	int lhs, op1, op2, optr;
	
        Token t = expect(ID);
        // check for ID and alloc if necessary
	
	if(!search_table(t)) {	// symbol not found
		new_symbol(t);
	}
	lhs = symbol_table[t.lexeme];	// assign index of lhs
        t = expect(EQUAL);
        t = peek();
        if(t.token_type == ID || t.token_type == NUM) {
		std::tie(op1, op2, optr) = parse_expr();   // assign optr, op1, op2
        }
	else {
		syntax_error();
	}

        t = expect(SEMICOLON);
       
       	// build statement node
	new_statement(ASSIGN, lhs, optr, op1, op2);	// ASSIGN
}

std::tuple<int, int, int>Parser::parse_expr() {
        // expr--> primary || primary operator primary
	int optr, op1, op2 = 0;
        Token t = peek();
        if(t.token_type == ID) {
		parse_primary();
		op1 = symbol_table[t.lexeme];	// index of symbol
	}
	else if(t.token_type == NUM) {
		parse_primary();
		op1 = next_available - 1;	// index of num
	}
	t = peek();
	if(t.token_type == SEMICOLON) {
		optr = EQUAL;
		op2 = -1;
	}
	else if(t.token_type == PLUS || t.token_type == MINUS || t.token_type == MULT ||
		       	t.token_type == DIV) {		// optional operator primary
		optr = parse_operator();		// storing operator

		t = peek();
		if(t.token_type == ID) {
			parse_primary();
			op2 = symbol_table[t.lexeme];	// index of symbol
		}
		else if(t.token_type == NUM) {
			parse_primary();
			op2 = next_available - 1;	// index of num
		}
		else {
			syntax_error();
		}
	}

	return std::make_tuple(op1, op2, optr);		// return expression parts
}

int Parser::parse_operator() {
        Token t = lexer.GetToken();
	int op_num = 0;
        switch (t.token_type) {
                case PLUS:
                        op_num = 6;
		        return op_num;
		break;
                case MINUS:
                        op_num = 7;
			return op_num;
		break;
                case MULT:
                        op_num = 8;
			return op_num;
		break;
                case DIV:
                        op_num = 9;
			return op_num;
		break;
                default:
                        syntax_error();
        }

}

void Parser::parse_primary() {
	Token t = lexer.GetToken();
	if(t.token_type == ID) {
		if(!search_table(t)) {
			new_symbol(t);
		}
	}
	else if(t.token_type == NUM) {
		int prim = stoi(t.lexeme);
		mem[next_available] = prim;
		next_available++;
	}
	else {
		syntax_error();
	}
}

void Parser::parse_main() {
	//main --> MAIN procedure_body
        Token t = expect(MAIN);
	// store MAIN token?
	t = peek();
	if(t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID ||
		       	t.token_type == NUM || t.token_type == DO) {
       		 parse_procedure_body();
	}
	else {
		syntax_error();
	}
}

void Parser::parse_inputs() {
	Token t = expect(NUM);
	t = peek();

	if(t.token_type == END_OF_FILE) {
		return;
	}
	else if(t.token_type == NUM) {	// assign num
		input_line[current_input] = stoi(t.lexeme);
       		current_input++;
       		input_size++;
		parse_inputs();
	}
	
}

void execute_program(struct statement_node* s_head) {
	struct statement_node* counter;
	counter = s_head;
	
	while(counter != NULL) {
		switch(counter->statement_type)	{
			case ASSIGN:	switch(counter->optr) {	// case ASSIGN
					
					case PLUS: mem[counter->LHS] = mem[counter->op1] +
					       	mem[counter->op2];	// case plus
						break;
					case MINUS: mem[counter->LHS] = mem[counter->op1] -
						mem[counter->op2];	// case minus
						break;
					case MULT: mem[counter->LHS] = mem[counter->op1] *
						mem[counter->op2];	// case mult
						break;
					case DIV: mem[counter->LHS] = mem[counter->op1] /
						mem[counter->op2];	// case div
						break;
					case EQUAL:
						mem[counter->LHS] = mem[counter->op1];
						break;
				}
			case INPUT: mem[counter->op1] = input_line[current_input]; // case INPUT
				current_input++;	
				break;
			case OUTPUT: cout << mem[counter->op1];	// case OUTPUT
				break;
			case DO: 
				break;
			case PROC_INV:
				break;
		}
		counter = counter->next;
	}
}

int main() {
	Parser parser;
	for(int i = 0; i < 2000; i++) {
		mem[i] = 0;
	}
	parser.parse_input();
	execute_program(s_head);
//	LexicalAnalyzer lexer;
//	Token token;

//	token = lexer.GetToken();
//	token.Print();
//	while(token.token_type != END_OF_FILE)
//	{
//		token = lexer.GetToken();
//		token.Print();
//	}
}

