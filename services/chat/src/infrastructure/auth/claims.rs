use serde::{Deserialize};

#[derive(Debug, Deserialize)]
pub struct Claims {
    pub user_id: String,
    pub exp: usize,
}