#include <iostream>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "json.hpp"
#include "Compiler.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

#include <fstream>


int main(int argc, char** argv)
{
    bool LEXER_DEBUG = false;
    bool PARSER_DEBUG = false;
    bool COMPILER_DEBUG = true;
    bool RUN_CODE = false;

    // read source file
    std::string source = "";
    std::string line;
    std::ifstream file("/home/pirate/projects/ccp_lang_cmake/source.ligma");
    if (file.is_open()){
        while (getline(file, line)){
            source += line + "\n";
        }
        file.close();
    } else {
        std::cout << "Unable to open file" << std::endl;
        return 1;
    }

    Lexer lexer = Lexer(source);

    if (LEXER_DEBUG){
        Lexer lexer = Lexer(source);
        while (lexer.current_char != '\0'){
            Token token = lexer.next_token();
            std::cout << token.to_string() << std::endl;
        }
    }


    if (PARSER_DEBUG){
        Parser parser = Parser(lexer);
        Program program = parser.parse_program();
    
        if (parser.errors.size() > 0){
            for (std::string error : parser.errors){
                std::cout << error << std::endl;
            }
            return 1;
        }
        nlohmann::json program_json = program.json();

        // dump json to file
        std::string program_json_str = program_json.dump(4);
        std::ofstream out("program.json");
        out << program_json_str;
        out.close();
    }


    if (COMPILER_DEBUG){
        Parser parser = Parser(lexer);
        Program program = parser.parse_program();


        Compiler compiler = Compiler();

        compiler.compile(&program);

        auto module = compiler.get_module();
        std::error_code EC;
        llvm::raw_fd_ostream OS("module.ll", EC);
        module->print(OS, nullptr);
        OS.flush();
    }

    // define mcjit execution engine
    if (RUN_CODE){
        Parser parser = Parser(lexer);
        Program program = parser.parse_program();

        Compiler compiler = Compiler();
        compiler.compile(&program);

        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();

        llvm::ExecutionEngine* engine = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(compiler.get_module())).create();
        llvm::Function* main = compiler.get_module()->getFunction("main");
        if (!main){
            std::cout << "Function main not found" << std::endl;
            return 1;
        }

        std::vector<llvm::GenericValue> args;
        llvm::GenericValue result = engine->runFunction(main, args);
        std::cout << result.IntVal.getLimitedValue() << std::endl;
    }
    
    return 0;
}