#include <clang-c/Index.h>
#include <cstdio>

#include <iostream>
#include <vector>


CXCursor tu_cursor;

struct client_data
{
    CXTranslationUnit tu;
    std::vector<CXCursor> current_namespaces;
    CXCursor current_struct;
};

std::pair<CXToken*, unsigned int>
get_tokens (CXTranslationUnit tu, CXCursor cursor)
{
    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation start = clang_getRangeStart(range);
    CXSourceLocation end = clang_getRangeEnd(range);
    unsigned int start_offset, end_offset;

    std::pair<CXToken*, unsigned int> retval(0, 0);
    clang_tokenize(tu, range, &retval.first, &retval.second);

    return retval;
}

void
free_tokens (CXTranslationUnit tu, std::pair<CXToken*, unsigned int> tokens)
{ clang_disposeTokens(tu, tokens.first, tokens.second); }

void print_tokens (CXTranslationUnit tu, CXCursor cursor)
{
    std::pair<CXToken*, unsigned int> tokens = get_tokens(tu, cursor);

    for (unsigned int i = 0; i < tokens.second; ++i) {
        if (i)
            std::cout << " ";
        CXString spelling = clang_getTokenSpelling(tu, tokens.first[i]);
        std::cout << clang_getCString(spelling);
    }
    std::cout << "\n";

    free_tokens(tu, tokens);
}

bool struct_kind (CXCursorKind kind)
{
    switch (kind) {
    case CXCursor_ClassDecl:
    case CXCursor_StructDecl:
    case CXCursor_ClassTemplate:
    case CXCursor_ClassTemplatePartialSpecialization:
        return true;
    }
    return false;
}

void indent (const client_data& data)
{
    std::size_t size = data.current_namespaces.size();
    std::cout << std::string(size * 4, ' ');
}

void open_struct (const client_data& data, CXCursor struct_cursor)
{
    std::pair<CXToken*, unsigned int> tokens =
        get_tokens(data.tu, struct_cursor);

    std::cout << "\n";
    indent(data);

    const std::string open_brace = "{";
    const std::string struct_ = "struct";
    const std::string class_ = "class";

    for (unsigned int i = 0; i < tokens.second; ++i) {
        CXString spelling = clang_getTokenSpelling(data.tu, tokens.first[i]);

        const char* c_str = clang_getCString(spelling);

        if (c_str == struct_ || c_str == class_) {
            std::cout << "\n";
            indent(data);
        } else if (i) {
            std::cout << " ";
        }

        std::cout << c_str;
        if (c_str == open_brace)
            break;
    }

    free_tokens(data.tu, tokens);
}

void close_struct (const client_data& data)
{
    std::cout << "\n";
    indent(data);
    std::cout << "};\n";
}

void open_namespace (const client_data& data, CXCursor namespace_)
{
    std::cout << "\n";
    indent(data);
    std::cout
        << "namespace "
        << clang_getCString(clang_getCursorSpelling(namespace_))
        << " {";
}

void close_namespace (const client_data& data)
{
    std::cout << "\n";
    indent(data);
    std::cout << "}\n";
}

void dump_cursor (const char* name, CXCursor cursor)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    CXString kind_spelling = clang_getCursorKindSpelling(kind);
    CXString cursor_spelling = clang_getCursorSpelling(cursor);
    std::cout << name << " "
              << clang_getCString(kind_spelling) << " "
              << clang_getCString(cursor_spelling) << " "
              << "\n";
}

CXChildVisitResult
visitor (CXCursor cursor, CXCursor parent, CXClientData data_)
{
    client_data& data = *static_cast<client_data*>(data_);

#if 0
    std::cout << "\n";
    dump_cursor("cursor", cursor);
    dump_cursor("parent", parent);
#endif

    CXCursor null_cursor = clang_getNullCursor();

    // close open namespaces we have left
    CXCursor enclosing_namespace = parent;
    while (!clang_equalCursors(enclosing_namespace, tu_cursor) &&
           clang_getCursorKind(enclosing_namespace) != CXCursor_Namespace) {
        enclosing_namespace =
            clang_getCursorSemanticParent(enclosing_namespace);
    }
#if 0
    dump_cursor("enclosing_namespace", enclosing_namespace);
#endif
    if (!clang_equalCursors(enclosing_namespace, tu_cursor) &&
        clang_getCursorKind(enclosing_namespace) == CXCursor_Namespace) {
        while (!clang_equalCursors(enclosing_namespace,
                                   data.current_namespaces.back())) {
            data.current_namespaces.pop_back();
            close_namespace(data);
        }
    }

    // close open struct if we have left it
    CXCursor enclosing_struct = parent;
    while (!clang_equalCursors(enclosing_struct, tu_cursor) &&
           !struct_kind(clang_getCursorKind(enclosing_struct))) {
        enclosing_struct = clang_getCursorSemanticParent(enclosing_struct);
    }
#if 0
    dump_cursor("enclosing_struct", enclosing_struct);
#endif
    if (!clang_Cursor_isNull(data.current_struct) &&
        !clang_equalCursors(enclosing_struct, data.current_struct)) {
        data.current_struct = null_cursor;
        close_struct(data);
    }

    CXCursorKind kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_Namespace) {
        open_namespace(data, cursor);
        data.current_namespaces.push_back(cursor);
        return CXChildVisit_Recurse;
    } else if (struct_kind(kind)) {
        if (clang_Cursor_isNull(data.current_struct)) {
            data.current_struct = cursor;
            open_struct(data, cursor);
            return CXChildVisit_Recurse;
        }
    }

    return CXChildVisit_Continue;
}

int main (int argc, char* argv[])
{
    CXIndex index = clang_createIndex(0, 1);
    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,
        0,
        argv,
        argc,
        0,
        0,
        CXTranslationUnit_None
    );

    client_data data = {tu, {}, clang_getNullCursor()};

    tu_cursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(tu_cursor, visitor, &data);

    if (!clang_Cursor_isNull(data.current_struct))
        close_struct(data);

    while (!data.current_namespaces.empty()) {
        data.current_namespaces.pop_back();
        close_namespace(data);
    }

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
