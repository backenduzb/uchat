use serde::{Deserialize, Serialize};
use uuid::Uuid;

#[derive(Debug, Serialize, Deserialize)]
#[serde(tag = "type", content = "data")]
pub enum ClientMessage {
    Auth {
        token: String,
    },

    SendMessage {
        conversation_id: Uuid,
        text: String,
    },

    ReadMessage {
        message_id: Uuid,
    },

    Typing {
        conversation_id: Uuid,
    },

    Ping,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(tag = "type", content = "data")]
pub enum ServerMessage {
    Authenticated {
        user_id: Uuid,
    },

    Message {
        id: Uuid,
        conversation_id: Uuid,
        sender_id: Uuid,
        text: String,
    },

    MessageSent {
        id: Uuid,
    },

    MessageRead {
        message_id: Uuid,
        user_id: Uuid,
    },

    Pong,

    Error {
        code: String,
        message: String,
    },
}