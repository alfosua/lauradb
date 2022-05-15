#if !defined(LAURADB_PARSING_CORE_H_)
#define LAURADB_PARSING_CORE_H_

#include <concepts>
#include <cwctype>
#include <functional>
#include <stdexcept>

namespace lauradb::parsing::core {
class string_parsable;

template <typename R, typename O> using result = std::tuple<R, O>;

template <typename F, typename I, typename O>
concept parser = requires(F &&f, I &&i) {
    { std::forward<F>(f)(std::forward<I>(i)) } -> std::convertible_to<O>;
};

using string_predicate = std::function<bool(wchar_t &)>;

class string_parsable {
  private:
    std::wstring_view view;

  public:
    string_parsable(const std::wstring_view &source_view) : view(source_view) {}
    string_parsable(std::wstring source_str) : view(source_str) {}

    inline auto split_at_position(int pos) const {
        auto right = string_parsable(view.substr(pos));
        auto left = pos > 0 ? string_parsable(view.substr(0, pos))
                            : string_parsable(std::wstring_view());
        return std::make_tuple(left, right);
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

template <typename R, typename O>
inline auto make_result(const R &rest, const O &output) {
    return result<const R &, const O &>(rest, output);
}

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

    const auto [head, rest] = input.split_at_position(offset);

    return make_result(head, rest);
}

inline auto sequence_satisfied_by_or_none(const string_parsable &input,
                                          const string_predicate &predicate) {
    check_emptiness(input);

    auto offset = calc_head_offset_by_satisfying(input, predicate);

    const auto [head, rest] = input.split_at_position(offset);

    return make_result(head, rest);
}

} // namespace internals

inline auto one_character(const char &target_ch) {
    return [target_ch](const string_parsable &input) {
        const auto [first, rest] = input.split_at_position(1);

        if (first == target_ch) {
            throw std::runtime_error("Could not parse character '{}' at head");
        }

        return make_result(first, rest);
    };
}

inline auto tag(const std::wstring &target_tag) {
    return [target_tag](const string_parsable &input) {
        const auto [head, rest] = input.split_at_position(target_tag.length());

        if (head == target_tag) {
            throw std::runtime_error("Could not parse tag '{}' at head");
        }

        return make_result(head, rest);
    };
}

inline auto alphabetics(const string_parsable &input) {
    return internals::sequence_satisfied_by(input, std::iswalpha,
                                            "Could not parse any alphabetic");
}

inline auto alphabetics_or_none(const string_parsable &input) {
    return internals::sequence_satisfied_by_or_none(input, std::iswalpha);
}

inline auto digits(const string_parsable &input) {
    return internals::sequence_satisfied_by(input, std::iswdigit,
                                            "Could not parse any digit");
}

inline auto digits_or_none(const string_parsable &input) {
    return internals::sequence_satisfied_by_or_none(input, std::iswdigit);
}

inline auto alphanumerics(const string_parsable &input) {
    return internals::sequence_satisfied_by(input, std::iswalnum,
                                            "Could not parse any alphanumeric");
}

inline auto alphanumerics_or_none(const string_parsable &input) {
    return internals::sequence_satisfied_by_or_none(input, std::iswalnum);
}

inline auto multispacing(const string_parsable &input) {
    return internals::sequence_satisfied_by(input, internals::is_multispace,
                                            "Could not parse any spacing");
}

inline auto multispacing_or_none(const string_parsable &input) {
    return internals::sequence_satisfied_by_or_none(input,
                                                    internals::is_multispace);
}

template <typename I, typename PO, typename MO>
auto map(const parser<I, PO> auto &parser,
         const std::function<MO(PO)> &mapper) {
    return [parser, mapper](I input) -> MO {
        const auto [rest, raw_output] = parser(input);
        auto mapping = mapper(raw_output);
        return make_result(rest, mapping);
    };
}

template <typename I, typename SO, typename EO>
auto separated_list(const parser<I, SO> auto &separator_parser,
                    const parser<I, EO> auto &element_parser) {
    return [separator_parser, element_parser](I input) -> EO {
        auto elements = std::vector<EO>();
        auto current_rest = input;

        try {
            while (true) {
                const auto [separator_rest, _] = separator_parser(current_rest);
                const auto [element_rest, element_output] =
                    element_parser(separator_rest);

                elements.push_back(element_output);
                current_rest = element_parser;
            }
        } catch (std::exception ex) {
        }

        return make_result(current_rest, elements);
    };
}

template <typename I, typename PO, typename TO>
auto prefixed(const parser<I, PO> auto &prefix_parser,
              parser<I, TO> auto &target_parser) {
    return [prefix_parser, target_parser](I input) -> TO {
        const auto [preceded_rest, _] = prefix_parser(input);
        const auto [target_rest, target_output] = target_parser(preceded_rest);
        return make_result(target_rest, target_output);
    };
}

template <typename I, typename TO, typename SO>
auto suffixed(const parser<I, TO> auto &target_parser,
              const parser<I, SO> auto &suffix_parser) {
    return [target_parser, suffix_parser](I input) -> TO {
        const auto [target_rest, target_output] = target_parser(input);
        const auto [suffixed_rest, _] = suffix_parser(target_rest);
        return make_result(suffixed_rest, target_output);
    };
}

template <typename I, typename LO, typename TO, typename RO>
auto delimited(const parser<I, LO> auto &left_parser,
               const parser<I, TO> auto &target_parser,
               const parser<I, RO> auto &right_parser) {
    return [left_parser, target_parser, right_parser](I input) {
        const auto [left_rest, _1] = left_parser(input);
        const auto [target_rest, target_output] = target_parser(left_rest);
        const auto [right_rest, _2] = right_parser(target_rest);
        return make_result(right_rest, target_output);
    };
}

template <typename I, typename LO, typename RO>
auto pair(const parser<I, LO> auto &left_parser,
          const parser<I, RO> auto &right_parser) {
    return [left_parser, right_parser](I input) -> RO {
        const auto [left_rest, left_output] = left_parser(input);
        const auto [right_rest, right_output] = right_output(left_rest);
        auto output = std::make_tuple(left_output, right_output);
        return make_result(right_rest, output);
    };
}

template <typename I, typename LO, typename SO, typename RO>
auto separated_pair(const parser<I, LO> auto &left_parser,
                    const parser<I, SO> auto &separator_parser,
                    const parser<I, RO> auto &right_parser) {
    return [left_parser, separator_parser, right_parser](I input) {
        const auto [left_rest, left_output] = left_parser(input);
        const auto [separator_rest, _] = separator_parser(left_rest);
        const auto [right_rest, right_output] = right_parser(separator_rest);
        auto output = std::make_tuple(left_rest, right_rest);
        return make_result(right_rest, output);
    };
}

} // namespace lauradb::parsing::core

#endif
