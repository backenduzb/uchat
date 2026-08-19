mod config;
mod error;
mod state;

mod domain;
mod application;
mod infrastructure;
mod transport;

use std::sync::Arc;

use sqlx::postgres::PgPoolOptions;
use tracing_subscriber::EnvFilter;

use crate::application::manager::SessionManager;
use crate::config::Config;
use crate::infrastructure::auth::JwtVerifier;
use crate::state::AppState;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(
            EnvFilter::from_default_env()
        )
        .init();

    let config =
        Config::from_env();

    let db =
        PgPoolOptions::new()
            .max_connections(20)
            .connect(
                &config.database_url
            )
            .await?;

    
    let public_key =
        tokio::fs::read(
            &config.jwt_public_key_path
        )
        .await?;

    let auth =
        JwtVerifier::from_pem(
            &public_key
        )?;

    let state =
        Arc::new(AppState {
            db,
            sessions:
                SessionManager::new(),
            auth,
        });

    transport::tcp::server::run(
        config,
        state,
    )
    .await?;

    Ok(())
}