use anyhow::{bail, Result};
use uuid::Uuid;

use crate::domain::message::Message;
use crate::infrastructure::postgres::{
    message_repository::MessageRepository,
    participant_repository::ParticipantRepository,
};

#[derive(Clone)]
pub struct ChatService {
    messages: MessageRepository,
    participants: ParticipantRepository,
}

impl ChatService {
    pub fn new(
        messages: MessageRepository,
        participants: ParticipantRepository,
    ) -> Self {
        Self {
            messages,
            participants,
        }
    }

    pub async fn send_message(
        &self,
        sender_id: Uuid,
        conversation_id: Uuid,
        text: String,
    ) -> Result<Message> {
        let text = text.trim();

        if text.is_empty() {
            bail!("message cannot be empty");
        }

        if text.len() > 4000 {
            bail!("message is too long");
        }

        let is_member = self
            .participants
            .is_member(
                conversation_id,
                sender_id,
            )
            .await?;

        if !is_member {
            bail!("user is not a conversation participant");
        }

        let message = self
            .messages
            .create(
                conversation_id,
                sender_id,
                text,
            )
            .await?;

        Ok(message)
    }
}