use core::fmt::Debug;

pub struct SyntaxTree {
    pub statements: Vec<Statement>,
}

pub enum Statement {
    SelectQuery(SelectQueryData),
}

#[derive(Debug)]
pub struct SelectQueryData {
    pub source: String,
    pub columns: Vec<String>,
}
