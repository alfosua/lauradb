#if !defined(LAURADB_PARSING_SQL_H_)
#define LAURADB_PARSING_SQL_H_

#include "core.hpp"
#include <string>
#include <vector>

namespace lauradb::parsing::sql {
using lauradb::parsing::core::make_input;
using lauradb::parsing::core::string_parsable;

inline auto select_query_parser(const string_parsable &input) {
    using lauradb::parsing::core::one_character;
    using lauradb::parsing::core::tag;

    auto column_separator = one_character(L',');
    auto select_tag = tag(L"select");
    auto from_tag = tag(L"from");
}

} // namespace lauradb::parsing::sql

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

#endif