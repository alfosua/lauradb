use crate::parsing::syntax_tree;

use nom::branch::alt;
use nom::bytes::complete::tag_no_case;
use nom::character::complete::{alphanumeric1, char, multispace0, satisfy};
use nom::combinator::{eof, map, opt, value};
use nom::multi::{many0, separated_list1};
use nom::sequence::{delimited, pair, terminated, tuple};

pub fn syntax_tree(input: &str) -> nom::IResult<&str, syntax_tree::SyntaxTree> {
    let body = terminated(many_statements, opt(eof));
    let mut tree = map(body, |statements| syntax_tree::SyntaxTree { statements });
    tree(input)
}

pub fn many_statements(input: &str) -> nom::IResult<&str, Vec<syntax_tree::Statement>> {
    let statement = alt((select_query_statement,));
    let mut many_of_them = many0(statement);
    many_of_them(input)
}

pub fn select_query_statement(input: &str) -> nom::IResult<&str, syntax_tree::Statement> {
    let mut statement = map(select_query_data, |data| {
        syntax_tree::Statement::SelectQuery(data)
    });
    statement(input)
}

pub fn select_query_data(input: &str) -> nom::IResult<&str, syntax_tree::SelectQueryData> {
    let select_tag = tag_no_case("select");
    let select_prefix = pair(select_tag, multispace0);

    let from_tag = tag_no_case("from");
    let from_prefix = pair(from_tag, multispace0);

    let column_separator = delimited(multispace0, char(','), multispace0);
    let column_list = separated_list1(column_separator, name_identifier);
    let columns = delimited(select_prefix, column_list, multispace0);
    let source = delimited(from_prefix, alphanumeric1, multispace0);

    let select_compilation = terminated(pair(columns, source), statement_termination);
    let mut select_query_map = map(select_compilation, |(columns, source)| {
        syntax_tree::SelectQueryData {
            source: String::from(source),
            columns: columns,
        }
    });

    select_query_map(input)
}

pub fn name_identifier(input: &str) -> nom::IResult<&str, String> {
    let first = satisfy(|c| c == '_' || c.is_alphabetic());
    let rest = many0(satisfy(|c| c == '_' || c.is_alphanumeric()));
    let mut join = map(pair(first, rest), |(f, r)| {
        [vec![f], r].concat().into_iter().collect()
    });
    join(input)
}

pub fn statement_termination(input: &str) -> nom::IResult<&str, ()> {
    let semicolon = opt(char(';'));
    let termination = tuple((multispace0, semicolon, multispace0));
    let mut headless = value((), termination);
    headless(input)
}
