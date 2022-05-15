#if !defined(LAURADB_PARSING_CORE_H_)
#define LAURADB_PARSING_CORE_H_

#include <concepts>
#include <cwctype>
#include <fmt/xchar.h>
#include <functional>
#include <stdexcept>
#include <utility>

namespace lauradb::parsing::core {
class string_parsable;

using string_predicate = std::function<bool(wchar_t &)>;

class string_parsable {
  private:
    std::wstring_view view;

  public:
    constexpr string_parsable(const string_parsable &) = default;
    constexpr string_parsable(string_parsable &&) = default;

    string_parsable &operator=(const string_parsable &) = default;
    string_parsable &operator=(string_parsable &&) = default;

    ~string_parsable() = default;

    string_parsable(const std::wstring_view &source_view) : view(source_view) {}
    string_parsable(std::wstring source_str) : view(source_str) {}

    inline auto split_at_position(int pos) const {
        auto right = string_parsable(view.substr(pos));
        auto left = pos > 0 ? string_parsable(view.substr(0, pos))
                            : string_parsable(std::wstring_view());
        return std::tuple(left, right);
    }

    constexpr bool empty() const noexcept { return view.empty(); }

    constexpr const wchar_t *begin() const noexcept { return view.begin(); }

    constexpr const wchar_t *end() const noexcept { return view.end(); }

    bool operator==(const string_parsable &rhs) const {
        return view == rhs.view;
    }

    friend bool operator==(const string_parsable &lhs, const char &rhs) {
        return lhs.view.at(0) == rhs;
    }

    friend bool operator==(const string_parsable &lhs,
                           const std::wstring &rhs) {
        return lhs.view == rhs;
    }
};

inline auto make_input(const std::wstring &source) {
    return string_parsable(source);
}

inline auto make_input(const std::wstring_view &source) {
    return string_parsable(source);
}

namespace internals {

inline auto check_emptiness(const string_parsable &input) {
    if (input.empty()) {
        throw std::runtime_error("No data to parse");
    }
}

inline auto is_multispace(const wchar_t &ch) {
    return ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r';
}

inline auto calc_head_offset_by_satisfying(
    const string_parsable &input,
    const std::function<bool(wchar_t &)> &predicate) {
    auto offset = 0;

    for (auto ch : input) {
        if (predicate(ch)) {
            offset = offset + 1;
        } else {
            break;
        }
    }

    return offset;
}

inline auto sequence_satisfied_by(const string_parsable &input,
                                  const string_predicate &predicate,
                                  const std::string &error_message) {
    check_emptiness(input);

    auto offset = calc_head_offset_by_satisfying(input, predicate);

    if (offset == 0) {
        throw std::runtime_error(error_message);
    }

    auto [head, rest] = input.split_at_position(offset);

    return std::pair{std::move(head), std::move(rest)};
}

inline auto sequence_satisfied_by_or_none(const string_parsable &input,
                                          const string_predicate &predicate)
    -> std::pair<string_parsable, string_parsable> {
    check_emptiness(input);

    auto offset = calc_head_offset_by_satisfying(input, predicate);

    const auto [head, rest] = input.split_at_position(offset);

    return {std::move(head), std::move(rest)};
}

} // namespace internals

template <typename I, typename O> class parser {
  public:
    virtual std::pair<I, O> parse(I input) const;
    auto operator()(I input){return parse(input)};
};

using string_parser = parser<string_parsable, string_parsable>;
using string_result = std::pair<string_parsable, string_parsable>;

class one_character : public string_parser {
  private:
    wchar_t target_ch;

  public:
    one_character(wchar_t target_ch) : target_ch(target_ch) {}

    string_result parse(string_parsable input) const override {
        auto &&[first, rest] = input.split_at_position(1);

        if (first == target_ch) {
            throw std::runtime_error("Could not parse character '{}' at head");
        }

        return std::pair{std::move(first), std::move(rest)};
    }
};

class tag : public string_parser {
  private:
    std::wstring target_tag;

  public:
    tag(const std::wstring &target_tag) : target_tag(target_tag) {}
    tag(std::wstring &&target_tag) : target_tag(std::move(target_tag)) {}

    string_result parse(string_parsable input) const override {
        auto [head, rest] = input.split_at_position(target_tag.length());

        if (head == target_tag) {
            throw std::runtime_error(
                "Could not parse tag at head"
                // TODO: format. wstring makes trouble
                // fmt::format(L"Could not parse tag '{}' at head", target_tag)
            );
        }

        return std::pair{std::move(head), std::move(rest)};
    }
};

class alphabetics : public string_parser {
  public:
    string_result parse(string_parsable input) const override {
        return internals::sequence_satisfied_by(
            input, std::iswalpha, "Could not parse any alphabetic");
    }
};

class alphabetics_or_none : public string_parser {
  public:
    string_result parse(string_parsable input) const override {
        return internals::sequence_satisfied_by_or_none(input, std::iswalpha);
    }
};

class digits : public string_parser {
  public:
    string_result parse(string_parsable input) const override {
        return internals::sequence_satisfied_by(input, std::iswdigit,
                                                "Could not parse any digit");
    }
};

class digits_or_none : public string_parser {
  public:
    string_result parse(string_parsable input) const override {
        return internals::sequence_satisfied_by_or_none(input, std::iswdigit);
    }
};

class alphanumerics : public string_parser {
  public:
    string_result parse(string_parsable input) const override {
        return internals::sequence_satisfied_by(
            input, std::iswalnum, "Could not parse any alphanumeric");
    }
};

class alphanumerics_or_none : public string_parser {
  public:
    string_result parse(string_parsable input) const override {
        return internals::sequence_satisfied_by_or_none(input, std::iswalnum);
    }
};

class multispacing : public string_parser {
  public:
    string_result parse(string_parsable input) const override {
        return internals::sequence_satisfied_by(input, internals::is_multispace,
                                                "Could not parse any spacing");
    }
};

class multispacing_or_none : public string_parser {
  public:
    string_result parse(string_parsable input) const override {
        return internals::sequence_satisfied_by_or_none(
            input, internals::is_multispace);
    }
};

template <typename I, typename PO, typename MO>
class map : public parser<I, MO> {
  private:
    using mapper_t = std::function<MO(PO)>;
    using result_t = std::pair<I, MO>;

    parser<I, PO> target_parser;
    mapper_t mapper;

  public:
    map(parser<I, PO> target_parser, mapper_t mapper)
        : target_parser(target_parser), mapper(mapper) {}

    result_t parse(I input) const override {
        auto [rest, raw_output] = parser(input);
        auto mapping = mapper(raw_output);
        return std::pair(std::move(rest), std::move(mapping));
    }
};

template <typename I, typename SO, typename EO>
class separated_list : public parser<I, std::vector<EO>> {
  private:
    using result_t = std::pair<I, std::vector<EO>>;

    parser<I, SO> separator_parser;
    parser<I, SO> element_parser;

  public:
    separated_list(parser<I, SO> separator_parser, parser<I, SO> element_parser)
        : separator_parser(separator_parser), element_parser(element_parser) {}

    result_t parse(I input) const override {
        auto elements = std::vector<EO>();
        auto current_rest = input;

        try {
            while (true) {
                auto [separator_rest, _] = separator_parser(current_rest);
                auto [element_rest, element_output] =
                    element_parser(separator_rest);

                elements.push_back(element_output);
                current_rest = element_parser;
            }
        } catch (std::exception ex) {
        }

        return std::pair(std::move(current_rest), std::move(elements));
    };
};

template <typename I, typename PO, typename TO> class prefixed {
  private:
    using result_t = std::pair<I, TO>;

    parser<I, PO> prefix_parser;
    parser<I, TO> target_parser;

  public:
    prefixed(parser<I, PO> prefix_parser, parser<I, TO> target_parser)
        : prefix_parser(prefix_parser), target_parser(target_parser) {}

    result_t prefixed(I input) const override {
        auto [preceded_rest, _] = prefix_parser(input);
        auto [target_rest, target_output] = target_parser(preceded_rest);
        return std::pair(std::move(target_rest), std::move(target_output));
    }
};

template <typename I, typename TO, typename SO> class suffixed : parser<I, TO> {
  private:
    using result_t = std::pair<I, TO>;

    parser<I, TO> target_parser;
    parser<I, SO> suffix_parser;

  public:
    suffixed(parser<I, TO> target_parser, parser<I, SO> suffix_parser)
        : target_parser(target_parser), suffix_parser(suffix_parser) {}

    result_t parse(I input) const override {
        auto [target_rest, target_output] = target_parser(input);
        auto [suffixed_rest, _] = suffix_parser(target_rest);
        return std::pair(std::move(suffixed_rest), std::move(target_output));
    }
};

template <typename I, typename LO, typename TO, typename RO>
class delimited : public parser<I, TO> {
  private:
    using result_t = std::pair<I, TO>;

    parser<I, LO> left_parser;
    parser<I, TO> target_parser;
    parser<I, RO> right_parser;

  public:
    delimited(parser<I, LO> left_parser, parser<I, TO> target_parser,
              parser<I, RO> right_parser)
        : left_parser(left_parser), target_parser(target_parser),
          right_parser(right_parser) {}

    result_t parse(I input) const override {
        auto [left_rest, _1] = left_parser(input);
        auto [target_rest, target_output] = target_parser(left_rest);
        auto [right_rest, _2] = right_parser(target_rest);
        return std::pair(std::move(right_rest), std::move(target_output));
    }
};

template <typename I, typename LO, typename RO>
class pair : public parser<I, std::pair<LO, RO>> {
  private:
    using result_t = std::pair<I, std::pair<LO, RO>>;

    parser<I, LO> left_parser;
    parser<I, RO> right_parser;

  public:
    pair(parser<I, LO> left_parser, parser<I, RO> right_parser)
        : left_parser(left_parser), right_parser(right_parser) {}

    result_t parse(I input) const override {
        auto [left_rest, left_output] = left_parser(input);
        auto [right_rest, right_output] = right_output(left_rest);
        auto output = std::tuple(left_output, right_output);
        return std::pair(std::move(right_rest), std::move(output));
    }
};

template <typename I, typename LO, typename SO, typename RO>
class separated_pair : public parser<I, std::pair<LO, RO>> {
  private:
    using result_t = std::pair<I, std::pair<LO, RO>>;

    parser<I, LO> left_parser;
    parser<I, SO> separator_parser;
    parser<I, RO> right_parser;

  public:
    separated_pair(parser<I, LO> left_parser, parser<I, SO> separator_parser,
                   parser<I, RO> right_parser)
        : left_parser(left_parser), separator_parser(separator_parser),
          right_parser(right_parser) {}

    result_t parse(I input) const override {
        auto [left_rest, left_output] = left_parser(input);
        auto [separator_rest, _] = separator_parser(left_rest);
        auto [right_rest, right_output] = right_parser(separator_rest);
        auto output = std::tuple(left_rest, right_rest);
        return std::pair(std::move(right_rest), std::move(output));
    }
};

} // namespace lauradb::parsing::core

#endif
