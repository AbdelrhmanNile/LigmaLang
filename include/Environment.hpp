#pragma once

#include <string>
#include <map>
#include <tuple>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

class Environment{ // Environment class to store variables within a scope

    public:
        std::map<std::string, std::tuple<llvm::Value*, llvm::Type*>> records = {};
        Environment* parent = nullptr;
        std::string name = "";

        Environment(Environment* parent, std::string name) : parent(parent), name(name){}
        Environment(){}


        llvm::Value* define(std::string name, llvm::Value* value, llvm::Type* type){
            this->records[name] = std::make_tuple(value, type);
            return value;
        }

        std::tuple<llvm::Value*, llvm::Type*> lookup(std::string name){
            return this->resolve(name);
        }

        // function to print the environment variables
        void print(){
            for (auto const& [key, val] : this->records){
                std::cout << key << std::endl;
            }
        }
    
    private:
        std::tuple<llvm::Value*, llvm::Type*> resolve(std::string name){
            if (this->records.find(name) != this->records.end()){
                return this->records[name];
            } else if (this->parent != nullptr){
                return this->parent->resolve(name);
            } else {
                return std::make_tuple(nullptr, nullptr);
            }
        }
};