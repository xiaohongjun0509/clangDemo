set(LLVM_LINK_COMPONENTS
    Support
)
add_clang_executable(MyPlugin
    MyPlugin.cpp
)

target_link_libraries(MyPlugin PRIVATE
    clangAST
    clangDriver
    clangFormat
    clangLex
    clangParse
    clangBasic
    clangFrontend
    clangSema
    clangTooling
    clangToolingCore
    clangRewrite
)


if(UNIX)
    set(CLANGXX_LINK_OR_COPY create_symlink)
else()
    set(CLANGXX_LINK_OR_COPY copy)
endif()

