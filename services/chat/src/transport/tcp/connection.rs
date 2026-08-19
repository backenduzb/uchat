use std::sync::Arc;

use anyhow::Result;
use tokio::io::split;
use tokio::net::TcpStream;
use tokio::sync::mpsc;
use uuid::Uuid;

use crate::state::AppState;

use super::frame::{
    read_frame,
    write_frame,
};

use super::protocol::{
    ClientMessage,
    ServerMessage,
};

pub async fn handle_connection(
    stream: TcpStream,
    state: Arc<AppState>,
) -> Result<()> {
    let (mut reader, mut writer) =
        split(stream);

    let (outgoing_tx, mut outgoing_rx) =
        mpsc::unbounded_channel::<ServerMessage>();

    /*
     * Writer task.
     *
     * Boshqa logiclar:
     *
     * Session
     * Chat
     * Notification
     *
     * shu channel orqali response yuboradi.
     */
    let writer_task = tokio::spawn(async move {
        while let Some(message) =
            outgoing_rx.recv().await
        {
            let payload =
                serde_json::to_vec(&message)?;

            write_frame(
                &mut writer,
                &payload,
            ).await?;
        }

        Ok::<(), anyhow::Error>(())
    });

    /*
     * Hozircha connection
     * authentication qilinmagan.
     */
    let mut authenticated_user: Option<Uuid> =
        None;

    loop {
        let payload =
            match read_frame(&mut reader).await {
                Ok(payload) => payload,

                Err(_) => {
                    break;
                }
            };

        let message: ClientMessage =
            serde_json::from_slice(&payload)?;

        match message {

            /*
             * =========================
             * AUTH
             * =========================
             */
            ClientMessage::Auth { token } => {

                /*
                 * JWT verify.
                 */
                let user_id =
                    match state.auth.verify(&token) {
                        Ok(user_id) => user_id,

                        Err(error) => {
                            tracing::warn!(
                                ?error,
                                "JWT verification failed"
                            );

                            let _ =
                                outgoing_tx.send(
                                    ServerMessage::Error {
                                        code:
                                            "INVALID_TOKEN"
                                                .to_string(),

                                        message:
                                            "Invalid or expired token"
                                                .to_string(),
                                    }
                                );

                            continue;
                        }
                    };

                /*
                 * Connection ichida
                 * authenticated user saqlanadi.
                 */
                authenticated_user =
                    Some(user_id);

                /*
                 * Session registry.
                 */
                state
                    .sessions
                    .register(
                        user_id,
                        outgoing_tx.clone(),
                    )
                    .await;

                /*
                 * Clientga confirmation.
                 */
                let _ =
                    outgoing_tx.send(
                        ServerMessage::Authenticated {
                            user_id,
                        }
                    );

                tracing::info!(
                    %user_id,
                    "user authenticated"
                );
            }

            /*
             * =========================
             * PING
             * =========================
             */
            ClientMessage::Ping => {
                let _ =
                    outgoing_tx.send(
                        ServerMessage::Pong
                    );
            }

            /*
             * =========================
             * SEND MESSAGE
             * =========================
             */
            ClientMessage::SendMessage {
                conversation_id,
                text,
            } => {
                let user_id =
                    match authenticated_user {
                        Some(user_id) => user_id,

                        None => {
                            let _ =
                                outgoing_tx.send(
                                    ServerMessage::Error {
                                        code:
                                            "UNAUTHENTICATED"
                                                .to_string(),

                                        message:
                                            "Authentication required"
                                                .to_string(),
                                    }
                                );

                            continue;
                        }
                    };

                /*
                 * Hozircha faqat test.
                 *
                 * Keyingi stage'da:
                 *
                 * ChatService
                 *      ↓
                 * PostgreSQL
                 */
                tracing::info!(
                    %user_id,
                    %conversation_id,
                    %text,
                    "message received"
                );
            }

            /*
             * =========================
             * READ
             * =========================
             */
            ClientMessage::ReadMessage {
                message_id,
            } => {
                let user_id =
                    match authenticated_user {
                        Some(user_id) => user_id,

                        None => {
                            let _ =
                                outgoing_tx.send(
                                    ServerMessage::Error {
                                        code:
                                            "UNAUTHENTICATED"
                                                .to_string(),

                                        message:
                                            "Authentication required"
                                                .to_string(),
                                    }
                                );

                            continue;
                        }
                    };

                tracing::info!(
                    %user_id,
                    %message_id,
                    "message read"
                );
            }

            /*
             * =========================
             * TYPING
             * =========================
             */
            ClientMessage::Typing {
                conversation_id,
            } => {
                let user_id =
                    match authenticated_user {
                        Some(user_id) => user_id,

                        None => {
                            let _ =
                                outgoing_tx.send(
                                    ServerMessage::Error {
                                        code:
                                            "UNAUTHENTICATED"
                                                .to_string(),

                                        message:
                                            "Authentication required"
                                                .to_string(),
                                    }
                                );

                            continue;
                        }
                    };

                tracing::debug!(
                    %user_id,
                    %conversation_id,
                    "typing"
                );
            }
        }
    }

    /*
     * Connection closed.
     *
     * Session registry'dan olib tashlaymiz.
     */
    if let Some(user_id) =
        authenticated_user
    {
        state
            .sessions
            .unregister(user_id)
            .await;

        tracing::info!(
            %user_id,
            "user disconnected"
        );
    }

    writer_task.abort();

    Ok(())
}