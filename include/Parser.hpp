#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

#include "Lexer.hpp"
#include "Token.hpp"
#include "Ast.hpp"



// PrecedenceType is used to determine the order of operations
enum class PrecedenceType{
    LOWEST,
    EQUALS, // ==
    LESSGREATER, // > or <
    SUM,
    PRODUCT, // *
    EXPONENT,
    PREFIX, // -X or !X
    CALL,
    INDEX
};

// Precedence map
std::map<TokenType, PrecedenceType> precedence_map = {
    {TokenType::PLUS, PrecedenceType::SUM},
    {TokenType::MINUS, PrecedenceType::SUM},
    {TokenType::ASTERISK, PrecedenceType::PRODUCT},
    {TokenType::SLASH, PrecedenceType::PRODUCT},
    {TokenType::POW, PrecedenceType::EXPONENT},
    {TokenType::MODULUS, PrecedenceType::PRODUCT},
    
    {TokenType::EQ_EQ, PrecedenceType::EQUALS},
    {TokenType::NOT_EQ, PrecedenceType::EQUALS},
    {TokenType::LT, PrecedenceType::LESSGREATER},
    {TokenType::GT, PrecedenceType::LESSGREATER},
    {TokenType::LT_EQ, PrecedenceType::LESSGREATER},
    {TokenType::GT_EQ, PrecedenceType::LESSGREATER},

    {TokenType::LPAREN, PrecedenceType::CALL},

};

class Parser{ // Pratt parser
    public:
        Lexer lexer;
        std::vector<std::string> errors = {}; // error messages
        Token current_token = Token(TokenType::EOF_, "", 0, 0); // current token
        Token peek_token = Token(TokenType::EOF_, "", 0, 0); // next token
        

        // type alias for prefix parse functions that return an Expression*
        using prefix_func_expr = std::function<Expression*(Parser*)>;

        // type alias for infix parse functions that return an Expression*
        using infix_func_expr = std::function<Expression*(Expression*, Parser*)>;

        // map of prefix parse functions
        std::map<TokenType, prefix_func_expr> prefix_parse_fns = {};
        
        
        // map of infix parse functions
        std::map<TokenType, infix_func_expr> infix_parse_fns = {};

        // constructor
        Parser(Lexer lexer) : lexer(lexer){
            this->next_token();
            this->next_token();

            // prefix parse functions
            this->prefix_parse_fns[TokenType::INT] = [](Parser* p){ return p->parse_integer_literal(); };
            this->prefix_parse_fns[TokenType::FLOAT] = [](Parser* p){ return p->parse_float_literal(); };
            this->prefix_parse_fns[TokenType::LPAREN] = [](Parser* p){ return p->parse_grouped_expression(); };
            this->prefix_parse_fns[TokenType::IDENT] = [](Parser* p){ return p->parse_identifier(); };
            
            //this->prefix_parse_fns[TokenType::IF] = [](Parser* p){ return p->parse_if_statement(); };
            
            this->prefix_parse_fns[TokenType::TRUE] = [](Parser* p){ return p->parse_boolean_literal(); };
            this->prefix_parse_fns[TokenType::FALSE] = [](Parser* p){ return p->parse_boolean_literal(); };
            

            // infix parse functions
            this->infix_parse_fns[TokenType::PLUS] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::MINUS] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::ASTERISK] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::SLASH] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::POW] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::MODULUS] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::EQ_EQ] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::NOT_EQ] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::LT] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::GT] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::LT_EQ] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::GT_EQ] = [](Expression* left, Parser* p){ return p->parse_infix_expression(left); };
            this->infix_parse_fns[TokenType::LPAREN] = [](Expression* left, Parser* p){ return p->parse_call_expression(left); };

        }

        // parse the program
        Program parse_program(){
            Program program = Program();

            while (this->current_token.type != TokenType::EOF_){
                Statement* stmt = this->parse_statement();
                if (stmt != nullptr){
                    program.statements.push_back(stmt);
                }
                this->next_token();
            }

            return program;
        }



    private:
// --------------------------------------- HELPER FUNCTIONS ---------------------------------------
        // advance the current token and peek token
        void next_token(){
            this->current_token = this->peek_token;
            this->peek_token = this->lexer.next_token();
        }

        bool peek_token_is(TokenType type){
            return this->peek_token.type == type;
        }

        bool expect_peek(TokenType type){
            if (this->peek_token_is(type)){
                this->next_token();
                return true;
            } else {
                this->peek_error(type);
                return false;
            }
        }

        bool current_token_is(TokenType type){
            return this->current_token.type == type;
        }

        PrecedenceType current_precedence(){
            if (precedence_map.find(this->current_token.type) != precedence_map.end()){
                return precedence_map[this->current_token.type];
            }
            return PrecedenceType::LOWEST;
        }

        PrecedenceType peek_precedence(){
            if (precedence_map.find(this->peek_token.type) != precedence_map.end()){
                return precedence_map[this->peek_token.type];
            }
            return PrecedenceType::LOWEST;
        }

        void peek_error(TokenType type){
            std::string msg = "expected next token to be " + token_type_map[type] + ", got " + token_type_map[this->peek_token.type] + " instead";
            this->errors.push_back(msg);
        }

        void no_prefix_parse_fn_error(TokenType type){
            std::string msg = "no prefix parse function for " + token_type_map[type] + " found";
            this->errors.push_back(msg);
        }
// --------------------------------------- PARSING STATEMENTS ---------------------------------------
        // parse a statement
        Statement* parse_statement(){

            // if its an assignment statement
            if (this->current_token_is(TokenType::IDENT) && this->peek_token_is(TokenType::EQ)){
                return this->parse_assignment_statement();
            }


            switch(this->current_token.type){
                case TokenType::LET:
                    return this->parse_let_statement();
                
                case TokenType::DEF:
                    return this->parse_function_statement();
                
                case TokenType::RETURN:
                    return this->parse_return_statement();
                
                case TokenType::IF:
                    return this->parse_if_statement();

                default:
                    return this->parse_expression_statement();;
            }
        }

        // parse an expression statement
        ExpressionStatement* parse_expression_statement(){
            Expression* expr = this->parse_expression(PrecedenceType::LOWEST);

            if (this->peek_token_is(TokenType::SEMICOLON)){
                this->next_token();
            }

            return new ExpressionStatement(expr);
        }

        // parse a let statement
        LetStatement* parse_let_statement(){
            // let x:int = 5;
            //  ^

            LetStatement* stmt = new LetStatement();

            // after let expect an identifier
            if (!this->expect_peek(TokenType::IDENT)){
                return nullptr;
            }

            // set the name of the variable
            stmt->name = new IdentifierLiteral(this->current_token.literal);

            // after the identifier expect a colon
            if (!this->expect_peek(TokenType::COLON)){
                return nullptr;
            }

            // after the colon expect a type
            if (!this->expect_peek(TokenType::TYPE)){
                return nullptr;
            }

            // set the type of the variable
            stmt->value_type = this->current_token.literal;

            // after the type expect an equal sign
            if (!this->expect_peek(TokenType::EQ)){
                return nullptr;
            }

            this->next_token();

            // parse the expression
            stmt->value = this->parse_expression(PrecedenceType::LOWEST);

            while (!this->current_token_is(TokenType::SEMICOLON) && !this->current_token_is(TokenType::EOF_)){
                this->next_token();
            }

            return stmt;
        }

        // parse a function statement
        FunctionStatement* parse_function_statement(){
            FunctionStatement* smt = new FunctionStatement();

            // def add() -> int { return 10; }
            //  ^

            // after def expect an identifier
            if (!this->expect_peek(TokenType::IDENT)){
                return nullptr;
            }

            // set the name of the function
            smt->name = new IdentifierLiteral(this->current_token.literal);

            // after the identifier expect a left parenthesis
            if (!this->expect_peek(TokenType::LPAREN)){
                return nullptr;
            }

            // parse the parameters
            smt->params = parse_function_parameters();

            // after the right parenthesis expect an arrow
            if (!this->expect_peek(TokenType::ARROW)){
                return nullptr;
            }

            // after the arrow expect a type
            if (!this->expect_peek(TokenType::TYPE)){
                return nullptr;
            }

            // set the return type
            smt->return_type = this->current_token.literal;

            // after the type expect a left brace
            if (!this->expect_peek(TokenType::LBRACE)){
                return nullptr;
            }

            // parse the body
            smt->body = this->parse_block_statement();

            return smt;
        }

        // parse function parameters
        std::vector<FunctionParameter*> parse_function_parameters(){

            // def add(x:int, y:int) -> int { return 10; }
            //        ^

            std::vector<FunctionParameter*> params = {};

            // no parameters
            if (this->peek_token_is(TokenType::RPAREN)){
                this->next_token();
                return params;
            }

            // skip the left parenthesis
            this->next_token();

            FunctionParameter* first_param = new FunctionParameter(this->current_token.literal);

            // expect a colon after parameter name
            if (!this->expect_peek(TokenType::COLON)){
                return {nullptr};
            }

            // expect a type after colon
            if (!this->expect_peek(TokenType::TYPE)){
                return {nullptr};
            }

            // set the type of the parameter
            first_param->value_type = this->current_token.literal;
            params.push_back(first_param);

            // parse the rest of the parameters if any
            while (this->peek_token_is(TokenType::COMMA)){
                this->next_token();
                this->next_token();
                
                FunctionParameter* param = new FunctionParameter(this->current_token.literal);
                if (!this->expect_peek(TokenType::COLON)){
                    return {nullptr};
                }

                if (!this->expect_peek(TokenType::TYPE)){
                    return {nullptr};
                }

                param->value_type = this->current_token.literal;
                params.push_back(param);
                
            }

            if (!this->expect_peek(TokenType::RPAREN)){
                return {nullptr};
            }

            return params;
        }

        // parse a return statement
        ReturnStatement* parse_return_statement(){
            ReturnStatement* stmt = new ReturnStatement();

            this->next_token(); // skip return token

            stmt->return_value = this->parse_expression(PrecedenceType::LOWEST);

            if (!this->expect_peek(TokenType::SEMICOLON)){ // expect a semicolon
                std::cout << "Error: no semicolon in return" << std::endl;
                return nullptr;
            }

            return stmt;
        }

        // parse a block statement
        BlockStatement* parse_block_statement(){ // inside { }
            
            BlockStatement* block = new BlockStatement();
            this->next_token(); // skip left brace

            // parse all the statements inside the block
            while (!this->current_token_is(TokenType::RBRACE) && !this->current_token_is(TokenType::EOF_)){
                Statement* stmt = this->parse_statement();
                if (stmt != nullptr){
                    block->statements.push_back(stmt);
                }
                this->next_token();
            }

            return block;
        }

        // parse an assignment statement
        AssignStatement* parse_assignment_statement(){
            AssignStatement* stmt = new AssignStatement();

            stmt->ident = new IdentifierLiteral(this->current_token.literal);

            this->next_token(); // skip ident token
            this->next_token(); // skip =

            // parse expression to the right of the =
            stmt->right_value = parse_expression(PrecedenceType::LOWEST);

            this->next_token();

            return stmt;
        }

        IfStatement* parse_if_statement(){

            Expression* condition = nullptr;
            BlockStatement* concequence = nullptr;
            BlockStatement* alternative = nullptr;

            // move past if token
            next_token();

            // parse the condition
            condition = parse_expression(PrecedenceType::LOWEST);

            // expect a do token
            if (!this->expect_peek(TokenType::DO)){
                return nullptr;
            }

            // expect a left brace
            if (!this->expect_peek(TokenType::LBRACE)){
                return nullptr;
            }

            // parse the if block
            concequence = parse_block_statement();

            if (this->peek_token_is(TokenType::ELSE)){
                next_token();

                
                if (!this->expect_peek(TokenType::LBRACE)){
                return nullptr;
                }

                // parse the else block
                alternative = parse_block_statement();
            }

            return new IfStatement(condition, concequence, alternative);

        }

// --------------------------------------- PARSING EXPRESSIONS ---------------------------------------

        // parse an expression
        Expression* parse_expression(PrecedenceType precedence){

            // get the prefix parse function for the current token
            auto prefix = this->prefix_parse_fns[this->current_token.type];
            
            if (prefix == nullptr){
                this->no_prefix_parse_fn_error(this->current_token.type);
                return nullptr;
            }

            // parse the left expression
            Expression* left_exp = prefix(this);
            
            
            // parse infix expressions
            while (!this->peek_token_is(TokenType::SEMICOLON) && precedence < this->peek_precedence()){

                // get the infix parse function for the peek token
                auto infix = this->infix_parse_fns[this->peek_token.type];
                
                if (infix == nullptr){
                    return left_exp;
                }

                this->next_token();

                left_exp = infix(left_exp, this);
            }

            return left_exp;
        }

        // parse an infix expression
        Expression* parse_infix_expression(Expression* left){

            InfixExpression* infix_expr = new InfixExpression(left, this->current_token.literal);
            auto precedence = this->current_precedence();
            this->next_token();

            infix_expr->right = this->parse_expression(precedence);

            return infix_expr;
        }

        // parse a grouped expression (expression inside parentheses)
        Expression* parse_grouped_expression(){
            
            // skip the left parenthesis
            this->next_token();

            // parse the expression inside the parentheses
            Expression* expr = this->parse_expression(PrecedenceType::LOWEST);

            // expect a right parenthesis
            if (!this->expect_peek(TokenType::RPAREN)){
                return nullptr;
            }

            return expr;
        }

        // parse a call expression
        CallExpression* parse_call_expression(Expression* function){
            CallExpression* call_expr = new CallExpression(static_cast<IdentifierLiteral*>(function));
            call_expr->arguments = parse_expression_list(TokenType::RPAREN);

            return call_expr;

        }

        // parse an expression list
        std::vector<Expression*> parse_expression_list(TokenType end){
            std::vector<Expression*> expr_list = {};

            if (this->peek_token_is(end)){
                this->next_token();
                return expr_list;
            }

            this->next_token();
            expr_list.push_back(this->parse_expression(PrecedenceType::LOWEST));

            while (this->peek_token_is(TokenType::COMMA)){
                this->next_token();
                this->next_token();
                expr_list.push_back(this->parse_expression(PrecedenceType::LOWEST));
            }

            if (!this->expect_peek(end)){
                return {nullptr};
            }

            return expr_list;
        }

// --------------------------------------- PARSING LITERALS ---------------------------------------

        // parse an identifier
        Expression* parse_identifier(){
            return new IdentifierLiteral(this->current_token.literal);
        }

        // parse an integer literal
        Expression* parse_integer_literal(){
            int value = std::stoi(this->current_token.literal);
            return new IntegerLiteral(value);
        }

        // parse a float literal
        Expression* parse_float_literal(){
            float value = std::stof(this->current_token.literal);
            return new FloatLiteral(value);
        }

        BooleanLiteral* parse_boolean_literal(){
            bool value = current_token_is(TokenType::TRUE);
            return new BooleanLiteral(value);
        }



};