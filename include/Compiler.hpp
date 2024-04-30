#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/NoFolder.h"

#include <map>
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <memory> 

#include "Ast.hpp"
#include "Environment.hpp"

enum class BuiltInFunction {
    PRINT,
    INVALID
};

BuiltInFunction get_builtin_function(std::string name){
    const std::map<std::string, BuiltInFunction> builtins = {
        {"print", BuiltInFunction::PRINT}
    };
    
    auto it = builtins.find(name);
    if (it != builtins.end()) {
        return it->second;
    }
    return BuiltInFunction::INVALID;
}

class Compiler {

public:
        // Constructor
    Compiler(){
        this->module = new llvm::Module("main", context);
        this->env = new Environment();
        initialize_builtins();
    }

    // initiate compilation
    void compile(Node* node){
        if (node)
            switch(node->type_enum()){
                case NodeType::Program:
                    visit_program(static_cast<Program*>(node));
                    break;
                case NodeType::ExpressionStatement:
                    visit_expression_statement(static_cast<ExpressionStatement*>(node));
                    break;
                case NodeType::LetStatement:
                    visit_let_statement(static_cast<LetStatement*>(node));
                    break;
                case NodeType::InfixExpression:
                    visit_infix_expression(static_cast<InfixExpression*>(node));
                    break;
                case NodeType::FunctionStatement:
                    visit_function_statement(static_cast<FunctionStatement*>(node));
                    break;
                case NodeType::ReturnStatement:
                    visit_return_statement(static_cast<ReturnStatement*>(node));
                    break;
                case NodeType::BlockStatement:
                    visit_block_statement(static_cast<BlockStatement*>(node));
                    break;
                case NodeType::AssignStatement:
                    visit_assign_statement(static_cast<AssignStatement*>(node));
                    break;
                case NodeType::IfStatement:
                    visit_if_statement(static_cast<IfStatement*>(node));
                    break;
                
                case NodeType::CallExpression:
                    visit_call_expression(static_cast<CallExpression*>(node));
                    break;

                default:
                    std::cout << "Node type: " << node->type() << '\n';
                    std::cerr << "Unknown node type encountered during compilation\n";
            }
    }

    // get module
    llvm::Module* get_module(){
        return this->module;
    }

private:

    // LLVM module
    llvm::Module* module;

    // LLVM context and module
    llvm::LLVMContext context;

    // Intermediate representation builder
    llvm::IRBuilder<> builder{context};

    // Environment for variable tracking
    Environment* env;

    // Errors encountered during compilation
    std::vector<std::string> errors = {};

    // Map for basic types
    std::map<std::string, llvm::Type*> type_map = {
        {"int", llvm::Type::getInt32Ty(context)},
        {"float", llvm::Type::getFloatTy(context)},
        {"bool", llvm::Type::getInt1Ty(context)}
    };

    void initialize_builtins(){ // initialize builtin variables and functions
        
        // initialize booleans
        auto bool_type = type_map["bool"];
        
        auto true_val = llvm::ConstantInt::get(context, llvm::APInt(1, 1, true));
        auto false_val = llvm::ConstantInt::get(context, llvm::APInt(1, 0, true));

        // create global variables for true and false
        llvm::GlobalVariable* true_var = new llvm::GlobalVariable(*module, bool_type, true, llvm::GlobalValue::ExternalLinkage, true_val, "true");
        llvm::GlobalVariable* false_var = new llvm::GlobalVariable(*module, bool_type, true, llvm::GlobalValue::ExternalLinkage, false_val, "false");

        true_var->setConstant(true);
        false_var->setConstant(true);


        env->define("true", true_var, bool_type);
        env->define("false", false_var, bool_type);


    }
    
    // visit the program node
    void visit_program(Program* node){
        // Create main function
        /* std::string func_name = "main";
        std::vector<llvm::Type*> arg_types;
        llvm::Type* return_type = type_map["int"];
        llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, arg_types, false);
        llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, func_name, module.get());
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "main_entry", func);
        builder.SetInsertPoint(entry); */

        // Compile statements inside the program
        for (Statement* stmt : node->statements){
            compile(stmt);
        }

        // Return a constant value
        //builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 69, true)));
    }

    void visit_expression_statement(ExpressionStatement* node){
        compile(node->expr);
    }

    void visit_let_statement(LetStatement* node){
        
        if (node->name && node->value) {

            // variable name
            std::string name = static_cast<IdentifierLiteral*>(node->name)->value;
            
            // value of the variable
            Expression* value = node->value;
            auto [val, type] = resolve_value(value);

            // if variable doesnt exist, create a new variable
            if (this->env->lookup(name) == std::make_tuple(nullptr, nullptr)){
                llvm::AllocaInst* ptr = this->builder.CreateAlloca(type, nullptr, name);
                this->builder.CreateStore(val, ptr);
                this->env->define(name, ptr, type);
            } else {
                auto [ptr, type] = this->env->lookup(name);
                this->builder.CreateStore(val, ptr);
            }
        }
    }

    void visit_block_statement(BlockStatement* node){
        for (Statement* stmt : node->statements){
            compile(stmt);
        }
    }

    void visit_return_statement(ReturnStatement* node){
        auto ret_val = node->return_value;
        auto [val, type] = resolve_value(ret_val);
        this->builder.CreateRet(val);
    }

    void visit_function_statement(FunctionStatement* node){

        // function name
        std::string func_name = static_cast<IdentifierLiteral*>(node->name)->value;


        // function body
        BlockStatement* body = static_cast<BlockStatement*>(node->body);

        // function parameters
        std::vector<FunctionParameter*> params = node->params;

        std::vector<std::string> param_names;
        for (FunctionParameter* param : params){
            param_names.push_back(param->name);
        }

        // function parameter types
        std::vector<llvm::Type*> param_types = [this, &params](){
            std::vector<llvm::Type*> types;
            for (FunctionParameter* param : params){
                types.push_back(type_map[param->value_type]);
            }
            return types;
        }();


        // function return type
        llvm::Type* return_type = type_map[node->return_type];
        llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);

        // create function
        llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, func_name, module);

        // create function block
        llvm::BasicBlock* block = llvm::BasicBlock::Create(context, func_name+"_entry", func);


                
        // store global environment
        auto prev_block = this->builder.GetInsertBlock();
        auto prev_point = this->builder.saveIP();
        auto prev_env = this->env;
        auto prev_env_address = &prev_env;

       

        // set the insert point to the function block
        this->builder.SetInsertPoint(block);

        std::vector<llvm::Value*> params_ptrs = {};
        for (int i = 0; i < param_types.size(); i++){
            
            auto param_type = param_types[i];
            auto param_name = param_names[i];

            llvm::AllocaInst* ptr = this->builder.CreateAlloca(param_type, nullptr, param_name);
            this->builder.CreateStore(func->arg_begin() + i, ptr);

            params_ptrs.push_back(ptr);
        }


         // create new scope for the function, with the previous scope as the parent
        this->env = new Environment(*prev_env);

        for (int i = 0; i < param_names.size(); i++){
            this->env->define(param_names[i], params_ptrs[i], param_types[i]);
        }


        // register the function inside its own scope
        this->env->define(func_name, func, return_type);

        // compile the function body
        compile(body);

        // restore the previous environment
        this->env = prev_env;
        
        // register the function in the global environment
        this->env->define(func_name, func, return_type);

        // restore the insert point
        this->builder.SetInsertPoint(prev_block);
        this->builder.restoreIP(prev_point);

    }


    void visit_assign_statement(AssignStatement* node){

        // variable name
        std::string name = node->ident->value;

        // value of the variable
        Expression* value = node->right_value;
        auto [val, type] = resolve_value(value);

        // if you are trying to assign a value to a variable that has not been defined
        if (this->env->lookup(name) == std::make_tuple(nullptr, nullptr)){
            this->errors.push_back("COMPILE ERROR: Identifier " + name + " has not been defined before its re-assigned");   
        } else {
            auto [ptr, type] = this->env->lookup(name);
            this->builder.CreateStore(val, ptr);
        }
    }

    void visit_if_statement(IfStatement* node){

        auto condition = node->condition;
        auto consequence = node->concequence;
        auto alternative = node->alternative;

        auto [cond_val, cond_type] = resolve_value(condition);
        llvm::Function* func = this->builder.GetInsertBlock()->getParent();
        llvm::BasicBlock* then_block = llvm::BasicBlock::Create(context, "then", func);
        llvm::BasicBlock* else_block = llvm::BasicBlock::Create(context, "else");
        llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context, "ifcont");

        auto ip = this->builder.saveIP();

        this->builder.CreateCondBr(cond_val, then_block, else_block);

        this->builder.SetInsertPoint(then_block);
        compile(consequence);
        this->builder.CreateBr(merge_block);

        func->insert(func->end(), else_block);
        this->builder.SetInsertPoint(else_block);
        compile(alternative);
        this->builder.CreateBr(merge_block);

        func->insert(func->end(), merge_block);
        this->builder.SetInsertPoint(merge_block);
    }


    // infix expressions
    std::tuple<llvm::Value*, llvm::Type*> visit_infix_expression(InfixExpression* node){
        if (node->left && node->right) {
            std::string op = node->op;
            auto [left_value, left_type] = resolve_value(node->left); 
            auto [right_value, right_type] = resolve_value(node->right);
            llvm::Value* result = nullptr;
            llvm::Type* result_type = nullptr;

            // if both left and right values are integers
            if (left_type == type_map["int"] && right_type == type_map["int"]){
                switch (op[0]){
                    case '+':
                        result = this->builder.CreateAdd(left_value, right_value);                    
                        break;
                    case '-':
                        result = builder.CreateSub(left_value, right_value);
                        break;
                    case '*':
                        result = builder.CreateMul(left_value, right_value);
                        break;
                    case '/':
                        result = builder.CreateSDiv(left_value, right_value);
                        break;
                    case '%':
                        result = builder.CreateSRem(left_value, right_value);
                        break;
                    case '^':
                        // TODO
                        break;
                    
                    case '<':
                        // if op length is 1, then it is a less than operator
                        if (op.length() == 1){
                            result = builder.CreateICmpSLT(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        if (op == "<="){
                            result = builder.CreateICmpSLE(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        break;
                    case '>':
                        // if op length is 1, then it is a greater than operator
                        if (op.length() == 1){
                            result = builder.CreateICmpSGT(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        if (op == ">="){
                            result = builder.CreateICmpSGE(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        break;
                    case '=':
                        if (op == "=="){
                            result = builder.CreateICmpEQ(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        break;
                    case '!':
                        if (op == "!="){
                            result = builder.CreateICmpNE(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        break;
                }
            
            // if both left and right values are floats
            } else if (left_type == type_map["float"] && right_type == type_map["float"]){
                switch (op[0]){
                    case '+':
                        result = builder.CreateFAdd(left_value, right_value);
                        break;
                    case '-':
                        result = builder.CreateFSub(left_value, right_value);
                        break;
                    case '*':
                        result = builder.CreateFMul(left_value, right_value);
                        break;
                    case '/':
                        result = builder.CreateFDiv(left_value, right_value);
                        break;
                    case '%':
                        result = builder.CreateFRem(left_value, right_value);
                        break;
                    case '^':
                        // TODO
                        break;
                    case '<':
                        // if op length is 1, then it is a less than operator
                        if (op.length() == 1){
                            result = builder.CreateFCmpOLT(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        if (op == "<="){
                            result = builder.CreateFCmpOLE(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        break;
                    case '>':
                        // if op length is 1, then it is a greater than operator
                        if (op.length() == 1){
                            result = builder.CreateFCmpOGT(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        if (op == ">="){
                            result = builder.CreateFCmpOGE(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        break;
                    case '=':
                        if (op == "=="){
                            result = builder.CreateFCmpOEQ(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        break;
                    case '!':
                        if (op == "!="){
                            result = builder.CreateFCmpONE(left_value, right_value);
                            result_type = llvm::Type::getInt1Ty(context);
                            break;
                        }
                        break;
                }
            }
            if (result != nullptr && result_type != nullptr)
                result_type = result->getType();
            return std::make_tuple(result, result_type);
        }
        return std::make_tuple(nullptr, nullptr);
    }

    // call expressions -> func()
    std::tuple<llvm::Instruction*, llvm::Type*> visit_call_expression(CallExpression* node){

        std::cout << "Visiting call expression\n";
        
        std::string func_name = static_cast<IdentifierLiteral*>(node->Function)->value;
        std::vector<Expression*> params = node->arguments;
        std::vector<llvm::Value*> params_values;
        std::vector<llvm::Type*> params_types;

        if (params.size() > 0){
            for (Expression* param : params){
                auto [p_val, p_type] = resolve_value(param);
                params_values.push_back(p_val);
                params_types.push_back(p_type);
            }
        }

        switch(get_builtin_function(func_name)){
            /* 
            built-in functions here
            */
           
            default: // user defined function
                auto [func, return_type] = this->env->lookup(func_name);
 
                auto func_ = llvm::cast<llvm::Function>(func);


                auto ret = this->builder.CreateCall(func_, params_values);   
                return std::make_tuple(ret, return_type);
        }
    }

    std::tuple<llvm::Value*, llvm::Type*> resolve_value(Expression* node, std::optional<std::string> value_type = std::nullopt){
        if (node) {
            switch(node->type_enum()){
                case NodeType::IntegerLiteral:{
                    auto type_i = type_map["int"];
                    return std::make_tuple(llvm::ConstantInt::get(context, llvm::APInt(32, static_cast<IntegerLiteral*>(node)->value, true)), type_i);
                }
                case NodeType::FloatLiteral:{
                    auto type_f = type_map["float"];
                    return std::make_tuple(llvm::ConstantFP::get(context, llvm::APFloat(static_cast<FloatLiteral*>(node)->value)), type_f);
                }
                case NodeType::IdentifierLiteral:{
                    std::string name = static_cast<IdentifierLiteral*>(node)->value;
                    auto [value, type] = env->lookup(name);
                    if (value)
                        return std::make_tuple(builder.CreateLoad(type, value), type);
                    else
                        std::cerr << "Undefined variable: " << name << '\n';
                    break;
                }
                case NodeType::InfixExpression:{
                    return visit_infix_expression(static_cast<InfixExpression*>(node));
                }
                case NodeType::BooleanLiteral:{
                    auto type_b = llvm::Type::getInt1Ty(context);
                    return std::make_tuple(llvm::ConstantInt::get(context, llvm::APInt(1, static_cast<BooleanLiteral*>(node)->value, true)), type_b);
                }
                case NodeType::CallExpression:{
                    return visit_call_expression(static_cast<CallExpression*>(node));
                }
                default:
                    std::cerr << "Unhandled node type during value resolution\n";
            }
        }
        return std::make_tuple(nullptr, nullptr);
    }
};
