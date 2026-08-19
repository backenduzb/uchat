use anyhow::Result;
use sqlx::PgPool;
use uuid::Uuid;

#[derive(Clone)]
pub struct ParticipantRepository {
    pool: PgPool,
}

impl ParticipantRepository {
    pub fn new(pool: PgPool) -> Self {
        Self { pool }
    }

    pub async fn is_member(
        &self,
        conversation_id: Uuid,
        user_id: Uuid,
    ) -> Result<bool> {
        let exists = sqlx::query_scalar!(
            r#"
            SELECT EXISTS(
                SELECT 1
                FROM conversation_participants
                WHERE conversation_id = $1
                  AND user_id = $2
            )
            "#,
            conversation_id,
            user_id,
        )
        .fetch_one(&self.pool)
        .await?;

        Ok(exists.unwrap_or(false))
    }

    pub async fn get_members(
        &self,
        conversation_id: Uuid,
    ) -> Result<Vec<Uuid>> {
        let members = sqlx::query_scalar!(
            r#"
            SELECT user_id
            FROM conversation_participants
            WHERE conversation_id = $1
            "#,
            conversation_id,
        )
        .fetch_all(&self.pool)
        .await?;

        Ok(members)
    }
}