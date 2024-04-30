#pragma once
#include <iostream>
#include <string>
#include "Token.hpp"

class Lexer{
    public:

        std::string source;
        int pos;
        int read_pos; // whats next?
        int line_no;
        char current_char;
    
        Lexer(std::string source)
            : source(source), pos(-1), read_pos(0), line_no(1), current_char(source[0]) {
                read_char();
            }

        Token next_token(){ // get next token

            Token tok = Token(TokenType::ILLEGAL, "", this->line_no, this->pos);

            skip_whitespace();
            
            switch (this->current_char)
            {
            case '+':{
                tok = create_token(TokenType::PLUS, "+");
                break;
            }
            case '-':{
                // handle arrow
                if (peek_char() == '>'){
                    read_char();
                    tok = create_token(TokenType::ARROW, "->");
                } else {
                    tok = create_token(TokenType::MINUS, "-");
                }
                break;
            }
            case '*':{
                tok = create_token(TokenType::ASTERISK, "*");
                break;
            }
            case '/':{
                tok = create_token(TokenType::SLASH, "/");
                break;
            }
            case '^':{
                tok = create_token(TokenType::POW, "^");
                break;
            }
            case '%':{
                tok = create_token(TokenType::MODULUS, "%");
                break;
            }
            case '<':{
                if (peek_char() == '='){
                    read_char();
                    tok = create_token(TokenType::LT_EQ, "<=");
                } else {
                    tok = create_token(TokenType::LT, "<");
                }
                break;
            }
            case '>':{
                if (peek_char() == '='){
                    read_char();
                    tok = create_token(TokenType::GT_EQ, ">=");
                } else {
                    tok = create_token(TokenType::GT, ">");
                }
                break;
            }
            case '=':{
                if (peek_char() == '='){
                    read_char();
                    tok = create_token(TokenType::EQ_EQ, "==");
                } else {
                    tok = create_token(TokenType::EQ, "=");
                }
                break;
            }
            case '!':{
                if (peek_char() == '='){
                    read_char();
                    tok = create_token(TokenType::NOT_EQ, "!=");
                } else {
                    // TODO
                    tok = create_token(TokenType::ILLEGAL, "!");
                }
                break;
            }
            case ':':{
                tok = create_token(TokenType::COLON, ":");
                break;
            }
            case ';':{
                tok = create_token(TokenType::SEMICOLON, ";");
                break;
            }
            case ',':{
                tok = create_token(TokenType::COMMA, ",");
                break;
            }
            case '(':{
                tok = create_token(TokenType::LPAREN, "(");
                break;

            }
            case ')':{
                tok = create_token(TokenType::RPAREN, ")");
                break;
            }
            case '{':{
                tok = create_token(TokenType::LBRACE, "{");
                break;
            }
            case '}':{
                tok = create_token(TokenType::RBRACE, "}");
                break;
            }
            case '\0':{
                tok = create_token(TokenType::EOF_, "");
                break;
            }
            
            default:
                // check if its a letter
                if (isalpha(this->current_char)){
                    std::string literal = read_ident();
                    TokenType type = lookup_ident(literal); // check if its a reserved keyword
                    tok = create_token(type, literal);
                    return tok;

                // check if its a number
                } else if(isdigit(this->current_char)){
                    tok = read_number();
                    return tok;
                
                } else { // illegal token
                    tok = create_token(TokenType::ILLEGAL, std::string(1, this->current_char));
                }
                break;
            }

            read_char();
            return tok;
        }

    private:
        
        void read_char(){ // read next character
            
            // check if we reached the end of the source
            if(this->read_pos >= this->source.length()){
                this->current_char = '\0';
            } else {
                this->current_char = this->source[this->read_pos];
            }

            // update pos and read_pos
            this->pos = this->read_pos;
            this->read_pos += 1;
        }

        char peek_char(){ // what's the next character?
            if(this->read_pos >= this->source.length()){
                return '\0';
            } else {
                return this->source[this->read_pos];
            }
        }

        Token read_number(){ // tokinize numbers

            int start_pos = this->pos;
            int dot_count = 0; // for float numbers
            std::string output = "";

            // while is digit or dot
            while (isdigit(this->current_char) || this->current_char == '.'){

                
                if(this->current_char == '.'){ 
                    dot_count += 1;
                }

                if(dot_count > 1){ // invalid float number

                    std::string invalid_number = source.substr(start_pos, this->pos - start_pos);

                    while (this->current_char != ' ' && this->current_char != '\t' && this->current_char != '\n' && this->current_char != '\r'){
                        invalid_number += this->current_char;
                        read_char();
                    }

                    std::cout << "Invalid number: " << invalid_number << " at line: " << this->line_no << " pos: " << this->pos << std::endl;
                    return create_token(TokenType::ILLEGAL, invalid_number);
                }

                output += this->current_char;
                read_char();

                if (this->current_char == '\0'){
                    break;
                }
            }

            if (dot_count == 0){ // integer
                return create_token(TokenType::INT, output);
            } else { // float
                return create_token(TokenType::FLOAT, output);
            }
        }


        void skip_whitespace(){ // skip whitespaces
            while (this->current_char == ' ' || this->current_char == '\t' || this->current_char == '\n' || this->current_char == '\r'){
                if(this->current_char == '\n'){
                    this->line_no += 1;
                }
                read_char();
            }
        }

        Token create_token(TokenType type, std::string literal){
            return Token(type, literal, this->line_no, this->pos);
        }

        std::string read_ident(){ // read identifiers

            int start_pos = this->pos;
            
            // reading variable names
            while (isalpha(this->current_char) || isalnum(this->current_char) || this->current_char == '_'){
                read_char();
            }
            return source.substr(start_pos, this->pos - start_pos);
        }


};