#pragma once
#include <string>
#include <map>
#include "json.hpp"
#include <iostream>

enum class NodeType{
    Program,

    // Statements
    ExpressionStatement,
    LetStatement,
    BlockStatement,
    FunctionStatement,
    ReturnStatement,
    AssignStatement,
    IfStatement,
    ElseStatement,

    // Expressions
    InfixExpression,
    CallExpression,

    // Literals
    IntegerLiteral,
    FloatLiteral,
    IdentifierLiteral,
    BooleanLiteral,

    // Helper
    FunctionParameter,
};

std::map<NodeType, std::string> node_type_map = {
    {NodeType::Program, "Program"},
    {NodeType::ExpressionStatement, "ExpressionStatement"},
    {NodeType::LetStatement, "LetStatement"},
    {NodeType::BlockStatement, "BlockStatement"},
    {NodeType::FunctionStatement, "FunctionStatement"},
    {NodeType::ReturnStatement, "ReturnStatement"},
    {NodeType::AssignStatement, "AssignStatement"},
    {NodeType::IfStatement, "IfStatement"},
    {NodeType::ElseStatement, "ElseStatement"},



    {NodeType::InfixExpression, "InfixExpression"},
    {NodeType::CallExpression, "CallExpression"},


    {NodeType::IntegerLiteral, "IntegerLiteral"},
    {NodeType::FloatLiteral, "FloatLiteral"},
    {NodeType::IdentifierLiteral, "IdentifierLiteral"},
    {NodeType::BooleanLiteral, "BooleanLiteral"},

    {NodeType::FunctionParameter, "FunctionParameter"},
};


// abstract Node class with two methods: type and json
class Node{
    public:
        virtual std::string type() = 0;
        virtual NodeType type_enum() = 0;
        virtual nlohmann::json json() = 0;
        virtual ~Node() {}
};

class Statement : public Node{};

// expression node
class Expression : public Node{};


class Program : public Node{
    public:
        std::vector<Statement*> statements;

        std::string type(){
            return node_type_map[NodeType::Program];
        }

        NodeType type_enum(){
            return NodeType::Program;
        }

        nlohmann::json json(){
           nlohmann::json::array_t stmts_json;
              for (Statement* stmt : this->statements){
                stmts_json.push_back({stmt->type(), stmt->json()});
              }
            
            nlohmann::json j = {
                {"statements", stmts_json},
                {"type", this->type()}
                
            };

            return j;
        }
};

class FunctionParameter : public Expression{
    public:
        std::string name;
        std::string value_type;

        FunctionParameter(std::string name, std::string value_type) : name(name), value_type(value_type){}
        FunctionParameter(std::string name) : name(name), value_type(""){}

        std::string type(){
            return node_type_map[NodeType::FunctionParameter];
        }

        NodeType type_enum(){
            return NodeType::FunctionParameter;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"type", this->type()},
                {"name", this->name},
                {"value_type", this->value_type}
            };

            return j;
        }
};


/*Statements*/
class ExpressionStatement : public Statement{
    public:
        Expression* expr;

        ExpressionStatement(Expression* expr) : expr(expr){}

        std::string type(){
            return node_type_map[NodeType::ExpressionStatement];
        }

        NodeType type_enum(){
            return NodeType::ExpressionStatement;
        } 

        nlohmann::json json(){
            nlohmann::json j {
                {"expr", this->expr->json()},
                {"type", this->type()}
            };

            return j;
        }        
};

class LetStatement : public Statement{
    public:
        Expression* name;
        Expression* value;
        std::string value_type;

        // all of them are optional
        LetStatement(std::optional<Expression*> name, std::optional<Expression*> value, std::optional<std::string> value_type) : name(name.value_or(nullptr)), value(value.value_or(nullptr)), value_type(value_type.value_or("")){}
        LetStatement() : name(nullptr), value(nullptr), value_type(""){}

        std::string type(){
            return node_type_map[NodeType::LetStatement];
        }

        NodeType type_enum(){
            return NodeType::LetStatement;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"value_type", this->value_type},
                {"value", this->value->json()},
                {"name", this->name->json()},
                {"type", this->type()}
            };

            return j;
        }
};

class BlockStatement : public Statement{
    public:
        std::vector<Statement*> statements;

        // constructor with default value 
        BlockStatement(std::vector<Statement*> statements = {}) : statements(statements){}

        std::string type(){
            return node_type_map[NodeType::BlockStatement];
        }

        NodeType type_enum(){
            return NodeType::BlockStatement;
        }

        nlohmann::json json(){
            nlohmann::json::array_t stmts_json;
            for (Statement* stmt : this->statements){
                stmts_json.push_back({stmt->type(), stmt->json()});
            }

            nlohmann::json j {
                {"statements", stmts_json},
                {"type", this->type()}
            };

            return j;
        }
};

class IdentifierLiteral : public Expression{
    public:
        std::string value;

        IdentifierLiteral(std::string value) : value(value){}

        std::string type(){
            return node_type_map[NodeType::IdentifierLiteral];
        }

        NodeType type_enum(){
            return NodeType::IdentifierLiteral;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"value", this->value},
                {"type", this->type()}
            };

            return j;
        }
};

class FunctionStatement : public Statement{
    public:
        std::vector<FunctionParameter*> params;
        BlockStatement* body;
        IdentifierLiteral* name;
        std::string return_type;

        FunctionStatement(std::vector<FunctionParameter*> params, BlockStatement* body, IdentifierLiteral* name, std::string return_type) : params(params), body(body), name(name), return_type(return_type){}
        FunctionStatement() : params({}), body(nullptr), name(nullptr), return_type(""){}

        std::string type(){
            return node_type_map[NodeType::FunctionStatement];
        }

        NodeType type_enum(){
            return NodeType::FunctionStatement;
        }

        nlohmann::json json(){
            nlohmann::json::array_t params_json;
            for (Expression* param : this->params){
                params_json.push_back(param->json());
            }

            nlohmann::json j {
                {"return_type", this->return_type},
                {"name", this->name->json()},
                {"body", this->body->json()},
                {"params", params_json},
                {"type", this->type()}
            };

            return j;
        }
};

class ReturnStatement : public Statement{
    public:
        Expression* return_value;

        ReturnStatement(Expression* return_value) : return_value(return_value){}
        ReturnStatement() : return_value(nullptr){}

        std::string type(){
            return node_type_map[NodeType::ReturnStatement];
        }

        NodeType type_enum(){
            return NodeType::ReturnStatement;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"return_value", this->return_value->json()},
                {"type", this->type()}
            };

            return j;
        }
};

class AssignStatement : public Statement{
    public:
        IdentifierLiteral* ident;
        Expression* right_value;

        AssignStatement(IdentifierLiteral* ident, Expression* right_value) : ident(ident), right_value(right_value){}
        AssignStatement() : ident(nullptr), right_value(nullptr){}

        std::string type(){
            return node_type_map[NodeType::AssignStatement];
        }

        NodeType type_enum(){
            return NodeType::AssignStatement;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"right_value", this->right_value->json()},
                {"type", this->type()},
                {"ident", this->ident->json()}
            };

            return j;
        }


};

class IfStatement : public Statement{
    public:
        Expression* condition;
        BlockStatement* concequence;
        BlockStatement* alternative;

        IfStatement(Expression* condition, BlockStatement* concequence, BlockStatement* alternative) : condition(condition), concequence(concequence), alternative(alternative){}
        IfStatement() : condition(nullptr), concequence(nullptr), alternative(nullptr){}

        std::string type(){
            return node_type_map[NodeType::IfStatement];
        }

        NodeType type_enum(){
            return NodeType::IfStatement;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"alternative", (this->alternative == nullptr) ? "None" : this->alternative->json()},
                {"concequence", this->concequence->json()},
                {"condition", this->condition->json()},
                {"type", this->type()},
            };

            return j;
        }
};


class InfixExpression : public Expression{
    public:
        Expression* left;
        std::string op;
        // right node can be null
        Expression* right;

        InfixExpression(Expression* left, std::string op) : left(left), op(op){}
        std::string type(){
            return node_type_map[NodeType::InfixExpression];
        }

        NodeType type_enum(){
            return NodeType::InfixExpression;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"right", this->right->json()},
                {"op", this->op},
                {"left", this->left->json()},
                {"type", this->type()}
            };

            return j;
        }
};

class CallExpression : public Expression{
    public:
        IdentifierLiteral* Function;
        std::vector<Expression*> arguments = {};

        CallExpression(IdentifierLiteral* Function, std::vector<Expression*> arguments) : Function(Function), arguments(arguments){}
        CallExpression(IdentifierLiteral* Function) : Function(Function), arguments({}){}

        std::string type(){
            return node_type_map[NodeType::CallExpression];
        }

        NodeType type_enum(){
            return NodeType::CallExpression;
        }

        nlohmann::json json(){
            nlohmann::json::array_t args_json;
            for (Expression* arg : this->arguments){
                args_json.push_back(arg->json());
            }

            nlohmann::json j {
                {"arguments", args_json},
                {"Function", this->Function->json()},
                {"type", this->type()}
            };

            return j;
        }

};

class IntegerLiteral : public Expression{
    public:
        int value;

        IntegerLiteral(int value) : value(value){}

        std::string type(){
            return node_type_map[NodeType::IntegerLiteral];
        }

        NodeType type_enum(){
            return NodeType::IntegerLiteral;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"value", this->value},
                {"type", this->type()}
                
            };

            return j;
        }
};

class FloatLiteral : public Expression{
    public:
        float value;

        FloatLiteral(float value) : value(value){}

        std::string type(){
            return node_type_map[NodeType::FloatLiteral];
        }

        NodeType type_enum(){
            return NodeType::FloatLiteral;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"value", this->value},
                {"type", this->type()}
            };

            return j;
        }
};

class BooleanLiteral : public Expression{
    public:
        bool value;

        BooleanLiteral(bool value) : value(value){}

        std::string type(){
            return node_type_map[NodeType::BooleanLiteral];
        }

        NodeType type_enum(){
            return NodeType::BooleanLiteral;
        }

        nlohmann::json json(){
            nlohmann::json j {
                {"value", this->value},
                {"type", this->type()}
            };

            return j;
        }
};