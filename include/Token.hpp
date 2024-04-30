#pragma once
#include <string>
#include <map>
#include <glaze/glaze.hpp>

// Token types
enum class TokenType{
    // Special tokens
    EOF_, 
    ILLEGAL,

    // Data types
    IDENT,
    INT,
    FLOAT,

    // Arithmetic operators
    PLUS,
    MINUS,
    ASTERISK,
    SLASH,
    POW,
    MODULUS,

    // Assignment
    EQ,

    // Comparission symbols
    LT,
    GT,
    EQ_EQ,
    NOT_EQ,
    LT_EQ,
    GT_EQ,

    // Symbols
    COLON,
    SEMICOLON,
    COMMA,
    LPAREN,
    RPAREN,
    ARROW,
    LBRACE,
    RBRACE,

    // Keywords
    LET,
    DEF,
    RETURN,
    IF,
    DO,
    ELSE,
    TRUE,
    FALSE,

    // Typing
    TYPE



};

// map token type to string
std::map<TokenType, std::string> token_type_map = {
    {TokenType::EOF_, "EOF"},
    {TokenType::ILLEGAL, "ILLEGAL"},
    
    {TokenType::IDENT, "IDENT"},
    {TokenType::INT, "INT"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::PLUS, "PLUS"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::ASTERISK, "ASTERISK"},
    {TokenType::SLASH, "SLASH"},
    {TokenType::POW, "POW"},
    {TokenType::MODULUS, "MODULUS"},

    {TokenType::EQ, "EQ"},

    {TokenType::LT, "<"},
    {TokenType::GT, ">"},
    {TokenType::EQ_EQ, "=="},
    {TokenType::NOT_EQ, "!="},
    {TokenType::LT_EQ, "<="},
    {TokenType::GT_EQ, ">="},


    {TokenType::COLON, "COLON"},
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::LPAREN, "LPAREN"},
    {TokenType::RPAREN, "RPAREN"},
    {TokenType::ARROW, "ARROW"},
    {TokenType::LBRACE, "LBRACE"},
    {TokenType::RBRACE, "RBRACE"},
    
    {TokenType::LET, "LET"},
    {TokenType::DEF, "DEF"},
    {TokenType::RETURN, "RETURN"},
    {TokenType::IF, "IF"},
    {TokenType::DO, "DO"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::TRUE, "TRUE"},
    {TokenType::FALSE, "FALSE"},


    {TokenType::TYPE, "TYPE"}
};

// Reserved keywords
std::map<std::string, TokenType> KEYWORDS = {
    {"let", TokenType::LET},
    {"def", TokenType::DEF},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"do", TokenType::DO},
    {"else", TokenType::ELSE},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE}
};

// Reserved type keywords
std::vector<std::string> TYPE_KEYWORDS = {
    "int",
    "float",
    "bool"
};


// Lookup identifier if it is a reserved keyword
TokenType lookup_ident(std::string ident){
    if (KEYWORDS.find(ident) != KEYWORDS.end()){
        return KEYWORDS[ident];
    }
    
    if (std::find(TYPE_KEYWORDS.begin(), TYPE_KEYWORDS.end(), ident) != TYPE_KEYWORDS.end()){
        return TokenType::TYPE;
    }

    return TokenType::IDENT;

}

// Token class
class Token{
    public:
        TokenType type;
        // literal can be of type int, float, string
        std::string literal;
        int line_no;
        int col_no;

        // constructor
        Token(TokenType type, std::string literal, int line_no, int col_no) : type(type), literal(literal), line_no(line_no), col_no(col_no){}
        

        // to_string
        std::string to_string(){
            
            return "Token[" + token_type_map[type] + ", " + literal + ", " + std::to_string(line_no) + ", " + std::to_string(col_no) + "]";}
};