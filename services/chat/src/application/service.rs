use chrono::Utc;
use sqlx::PgPool;
use uuid::Uuid;

use crate::domain::message::Message;

pub async fn send_message(
    db: &PgPool,
    sender_id: Uuid,
    conversation_id: Uuid,
    text: String,
) -> anyhow::Result<Message> {
    let message = Message {
        id: Uuid::new_v4(),
        conversation_id,
        sender_id,
        text,
        created_at: Utc::now(),
    };

    sqlx::query!(
        r#"
        INSERT INTO messages (
            id,
            conversation_id,
            sender_id,
            text,
            created_at
        )
        VALUES ($1, $2, $3, $4, $5)
        "#,
        message.id,
        message.conversation_id,
        message.sender_id,
        message.text,
        message.created_at,
    )
    .execute(db)
    .await?;

    Ok(message)
}