#if !defined(LAURADB_PARSING_SQL_H_)
#define LAURADB_PARSING_SQL_H_

#include "core.hpp"
#include <algorithm>
#include <string>
#include <vector>

namespace lauradb::parsing::sql::syntax_tree {

class select_query {
  public:
    const std::wstring &source;
    const std::vector<std::wstring> &columns;

    select_query(const std::wstring &source,
                 const std::vector<std::wstring> &columns)
        : source(source), columns(columns) {}
};

} // namespace lauradb::parsing::sql::syntax_tree

namespace lauradb::parsing::sql {
using namespace lauradb::parsing::sql::syntax_tree;
using namespace lauradb::parsing::core;

inline auto select_query_parser() {
    auto termination = multispacing_or_none();
    auto column_separator = delimited(
        multispacing_or_none(), one_character(L','), multispacing_or_none());
    auto select_tag = tag(L"select");
    auto select_prefix = pair(select_tag, multispacing());

    auto from_tag = tag(L"from");
    auto from_prefix = pair(from_tag, multispacing());

    auto column_list = separated_list(column_separator, alphanumerics());
    auto columns = delimited(select_prefix, column_list, multispacing());

    auto source = delimited(from_prefix, alphanumerics(), termination);

    auto compilation = pair(columns, source);

    auto select_query_map = map(compilation, [](auto compilation) {
        const auto [columns, source] = compilation;
        return select_query{source, columns};
    });

    return select_query_map;
}

} // namespace lauradb::parsing::sql

#endif