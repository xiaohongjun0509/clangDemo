
#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/DeclObjC.h"

using namespace clang;
using namespace std;
using namespace llvm;
using namespace clang::ast_matchers;

namespace XHJPlugin {
    
  
    
    class HJHandler : public MatchFinder::MatchCallback {
    private:
        CompilerInstance &ci;
        
    public:
        HJHandler(CompilerInstance &ci) :ci(ci) {}
        
        bool isUserSourceCode (SourceManager &mgr, Decl *decl){

//            std::string filename = ci.getSourceManager().getFilename(decl->getSourceRange().getBegin()).str();
//            std::string filename = mgr.getFilename(decl->getSourceRange().getBegin()).str();
            
//            std::string filename1 = mgr.getFilename(decl->getLocation()).str();
//            DiagnosticsEngine &D = ci.getDiagnostics();
//            D.Report(D.getCustomDiagID(DiagnosticsEngine::Warning, "fileName %0"))<< filename;
//            if (filename.empty())
//                return false;
            // /Applications/Xcode.app/xxx
//            if(filename.find("/Applications/Xcode.app/") == 0)
//                return false;
            
            return true;
        }
        
        //用前缀的方法来判断
        bool startsWith( const string &s, const string &t )
        {
            return s.size() >= t.size() &&
            std::equal( t.begin(), t.end(), s.begin() );
        }
        
        void run(const MatchFinder::MatchResult &Result) {
            // 当前遍历的是类的声明
            if (const ObjCInterfaceDecl *decl = Result.Nodes.getNodeAs<ObjCInterfaceDecl>("ObjCInterfaceDecl")) {
                string className= decl->getNameAsString();
                bool hasPrefix = startsWith(className, "KS");
                size_t pos = className.find('_');
                DiagnosticsEngine &D = ci.getDiagnostics();
                SourceManager &mgr = Result.Context->getSourceManager();
                SourceLocation location = decl->getLocation();
                if (pos != StringRef::npos) {
                    SourceLocation loc = decl->getLocation().getLocWithOffset(pos);
                    D.Report(loc, D.getCustomDiagID(DiagnosticsEngine::Error, "类名中不能带有下划线"));
                }
                
                //   检查类名是不是符合规范:iOS规范中，类名首字母必须大写并且不能有下划线
                const unsigned char *nameFirstChar = decl->getName().bytes_begin();
                const unsigned char *aChar = (const unsigned char *)"a";
                const unsigned char *zChar = (const unsigned char *)"z";
                if (((int)(*aChar)<= (int)(*nameFirstChar)) && (int)(*nameFirstChar)<= (int)(*zChar)) {
                    SourceLocation loc = decl->getLocation().getLocWithOffset(0);
                    D.Report(loc, D.getCustomDiagID(DiagnosticsEngine::Error, "类名不符合规范, 首字母必须小写"));
                }
            }
            // 当前遍历的是属性的声明
            if (const ObjCPropertyDecl *decl = Result.Nodes.getNodeAs<ObjCPropertyDecl>("ObjCPropertyDecl")) {
                DiagnosticsEngine &D = ci.getDiagnostics();
                SourceLocation loc = decl->getLocation();
                string type_string = decl->getType().getAsString();
                // NSArray 和 NSString 等在使用strong修饰的时候，给出编译警告⚠️
                if ("NSArray *" == type_string || "NSString *" == type_string) {
                    ObjCPropertyDecl::PropertyAttributeKind kind = decl->getPropertyAttributes();
                    if (kind && ObjCPropertyDecl::OBJC_PR_strong != 0) {
                        D.Report(loc, D.getCustomDiagID(DiagnosticsEngine::Warning, " %0 : %1 属性修饰错误"))<<decl->getNameAsString()<<type_string;
                    }
                }
            }
        }
    };
    
    class HJASTConsumer: public ASTConsumer {
    private:
        MatchFinder matcher;
        HJHandler handler;
        
    public:
        HJASTConsumer(CompilerInstance &ci) :handler(ci) {
            //添加时间的监听
            matcher.addMatcher(objcInterfaceDecl().bind("ObjCInterfaceDecl"), &handler);
            matcher.addMatcher(objcPropertyDecl().bind("ObjCPropertyDecl"), &handler);
        }
        
        void HandleTranslationUnit(ASTContext &context) {
            //处理语法树
            matcher.matchAST(context);
        }
    };
    
    class HJASTAction: public PluginASTAction {
    public:
        unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci, StringRef iFile) {
            //创建语法树的消费者
            return unique_ptr<HJASTConsumer> (new HJASTConsumer(ci));
        }
        
        bool ParseArgs(const CompilerInstance &ci, const vector<string> &args) {
            return true;
        }
    };
}

/*
 插件的注册的入口
 */
static FrontendPluginRegistry::Add<XHJPlugin::HJASTAction>
X("XHJPlugin", "The XHJPlugin is my first clang-plugin.");

