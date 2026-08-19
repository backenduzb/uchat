use std::sync::Arc;

use tokio::net::TcpListener;
use tracing::{error, info};

use crate::config::Config;
use crate::state::AppState;
use super::connection::handle_connection;

pub async fn run(
    config: Config,
    state: Arc<AppState>,
)  -> anyhow::Result<()> {
    let address = format!(
        "{}:{}",
        config.host,
        config.port,
    );

    let listener = TcpListener::bind(&address).await?;
    info!("ishlayapti....");

    loop {
        let (stream, address) = listener.accept().await?;

        let state = state.clone();

        tokio::spawn(async move {
            if let Err(error) = handle_connection(stream, state).await {
                tracing::error!(
                    ?error,
                    %address,
                    "Bog'lanish uzuldi"
                );
            }
        });
    }
}