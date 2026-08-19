use std::collections::HashMap;
use std::sync::Arc;

use tokio::sync::{
    mpsc,
    RwLock,
};

use uuid::Uuid;

use crate::transport::tcp::protocol::ServerMessage;

pub type ClientSender =
    mpsc::UnboundedSender<ServerMessage>;

#[derive(Clone)]
pub struct SessionManager {
    sessions: Arc<
        RwLock<
            HashMap<Uuid, ClientSender>
        >
    >,
}

impl SessionManager {
    pub fn new() -> Self {
        Self {
            sessions: Arc::new(
                RwLock::new(HashMap::new())
            ),
        }
    }

    pub async fn register(
        &self,
        user_id: Uuid,
        sender: ClientSender,
    ) {
        let mut sessions =
            self.sessions.write().await;

        sessions.insert(user_id, sender);
    }

    pub async fn unregister(
        &self,
        user_id: Uuid,
    ) {
        let mut sessions =
            self.sessions.write().await;

        sessions.remove(&user_id);
    }

    pub async fn send_to(
        &self,
        user_id: Uuid,
        message: ServerMessage,
    ) {
        let sessions =
            self.sessions.read().await;

        if let Some(sender) =
            sessions.get(&user_id)
        {
            let _ = sender.send(message);
        }
    }
}