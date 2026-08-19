use std::env;

#[derive(Clone)]
pub struct Config {
    pub host: String,
    pub port: u16,
    pub database_url: String,
    pub jwt_public_key_path: String,
}

impl Config {
    pub fn from_env() -> Self {
        dotenvy::dotenv().ok();

        let host = env::var("CHAT_HOST")
            .unwrap_or_else(|_| "0.0.0.0".to_string());

        let port = env::var("CHAT_PORT")
            .unwrap_or_else(|_| "9000".to_string())
            .parse()
            .expect("CHAT_PORT must be a number");

        let database_url = env::var("DATABASE_URL")
            .expect("DATABASE_URL is required");

        let jwt_public_key_path = env::var("JWT_PUBLIC_KEY_PATH")
            .expect("JWT_PUBLIC_KEY_PATH is required");

        Self {
            host,
            port,
            database_url,
            jwt_public_key_path,
        }
    }
}