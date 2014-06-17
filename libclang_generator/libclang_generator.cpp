#include <clang-c/Index.h>
#include <cstdio>

#include <iostream>
#include <vector>


struct client_data
{
    CXTranslationUnit tu;
    std::vector<std::string> namespaces;
    bool in_struct;
};

CXChildVisitResult visitor (CXCursor cursor, CXCursor parent, CXClientData data_)
{
    client_data& data = *static_cast<client_data*>(data_);

    CXCursorKind kind = clang_getCursorKind(cursor);

    std::cout
        << clang_getCString(clang_getCursorKindSpelling(clang_getCursorKind(cursor))) << " "
        << clang_getCString(clang_getCursorSpelling(cursor)) << " "
        << "\n";

    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation start = clang_getRangeStart(range);
    CXSourceLocation end = clang_getRangeEnd(range);
    unsigned int start_offset, end_offset;

    CXToken* tokens = 0;
    unsigned int num_tokens = 0;
    clang_tokenize(data.tu, range, &tokens, &num_tokens);
    for (unsigned int i = 0; i < num_tokens; ++i) {
        if (i)
            std::cout << " ";
        std::cout << clang_getCString(clang_getTokenSpelling(data.tu, tokens[i]));
    }
    std::cout << "\n";
    clang_disposeTokens(data.tu, tokens, num_tokens);

    return CXChildVisit_Recurse;
}

int main (int argc, char *argv[])
{
    CXIndex index = clang_createIndex(0, 1);
    CXTranslationUnit tu =
        clang_parseTranslationUnit(index, 0, argv, argc, 0, 0, CXTranslationUnit_None);

    client_data data = {tu, {}, false};

    CXCursor cursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(cursor, visitor, &data);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
