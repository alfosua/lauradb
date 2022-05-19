pub mod parsing;

use parsing::sql::syntax_tree;
use std::{
    io::Read,
    net::{TcpListener, TcpStream},
};

use crate::parsing::syntax_tree::Statement;

fn main() -> std::io::Result<()> {
    let listener = TcpListener::bind("127.0.0.1:1234")?;
    for stream in listener.incoming() {
        handle_connection(stream?)?;
    }
    Ok(())
}

fn handle_connection(mut stream: TcpStream) -> std::io::Result<()> {
    println!("Connection stablished");
    let mut buffer = [0; 1024];
    let _ = stream.read(&mut buffer)?;
    let request = String::from_utf8_lossy(&buffer[..]);
    let (_, syntax_tree) = syntax_tree(&request).unwrap();
    for statement in syntax_tree.statements {
        match statement {
            Statement::SelectQuery(data) => println!("{:?}", data),
        }
    }
    Ok(())
}
