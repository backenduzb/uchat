use std::sync::Arc;
use sqlx::PgPool;
use crate::application::manager::SessionManager;
use crate::infrastructure::auth::JwtVerifier;

pub struct AppState{
    pub db: PgPool,
    pub sessions: SessionManager,
    pub auth: JwtVerifier,
}

pub type ShareState = Arc<AppState>;