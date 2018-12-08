#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;

namespace MyPluginSpace
{
    
    class MyPluginVisitor : public RecursiveASTVisitor<MyPluginVisitor>
    {
    private:
        CompilerInstance &Instance;
        ASTContext *Context;
        
    public:
        
        void setASTContext (ASTContext &context)
        {
            this -> Context = &context;
        }
        
        MyPluginVisitor (CompilerInstance &Instance)
        :Instance(Instance)
        {
            
        }
    };
    
    class MyPluginConsumer : public ASTConsumer
    {
        CompilerInstance &Instance;
        std::set<std::string> ParsedTemplates;
    public:
        MyPluginConsumer(CompilerInstance &Instance,
                         std::set<std::string> ParsedTemplates)
        : Instance(Instance), ParsedTemplates(ParsedTemplates), visitor(Instance) {}
        
        bool HandleTopLevelDecl(DeclGroupRef DG) override
        {
            return true;
        }
        
        void HandleTranslationUnit(ASTContext& context) override
        {
            visitor.setASTContext(context);
            visitor.TraverseDecl(context.getTranslationUnitDecl());
        }
    private:
        MyPluginVisitor visitor;
        
    };
    
    class MyPluginASTAction : public ASTFrontendAction
    {
        std::set<std::string> ParsedTemplates;
    protected:
        std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                       llvm::StringRef) override
        {
            return llvm::make_unique<MyPluginConsumer>(CI, ParsedTemplates);
        }
        
//        bool ParseArgs(const CompilerInstance &CI,
//                       const std::vector<std::string> &args) override {
//            return true;
//        }
    };
}

using namespace MyPluginSpace;
using namespace clang::tooling;

static llvm::cl::OptionCategory OptionCategory("MyPlugin");
int main(int argc, const char **argv) {
    tooling::CommonOptionsParser Options(argc, argv, OptionCategory);
    ClangTool Tool(Options.getCompilations(), Options.getSourcePathList());
    int result = Tool.run(tooling::newFrontendActionFactory<MyPluginASTAction>().get());
    return result;
}

//static clang::FrontendPluginRegistry::Add<MyPluginASTAction>
//X("MyPlugin", "My plugin");
