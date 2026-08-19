use anyhow::Result;
use sqlx::PgPool;
use uuid::Uuid;

use crate::domain::message::{self, Message};

#[derive(Clone)]
pub struct MessageRepository {
    pool: PgPool,
}

impl MessageRepository {
    pub fn new(pool: PgPool) -> Self {
        Self { pool }
    }

    pub async fn create(
        &self,
        conversation_id: Uuid,
        sender_id: Uuid,
        text: &str,
    ) -> Result<Message> {
        let message_id = Uuid::new_v4();
        
        let message = sqlx::query_as!(
            Message,
            r#"
            INSERT INTO messages (
                id,
                conversation_id,
                sender_id,
                text
            )
            VALUES ($1, $2, $3, $4)
            RETURNING
                id,
                conversation_id,
                sender_id,
                text,
                created_at
            "#,
            message_id,
            conversation_id,
            sender_id,
            text,
        ).fetch_one(&self.pool).await?;
        Ok(message)
    }
}