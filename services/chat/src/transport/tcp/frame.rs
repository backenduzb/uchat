use tokio::io::{self, AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

const MAX_FRAME_SIZE: usize = 1024 * 1024;

pub async fn read_frame<R>(
    reader: &mut R,
) -> io::Result<Vec<u8>>
where
    R: AsyncRead + Unpin,
{
    let length = reader.read_u32().await? as usize;

    if length > MAX_FRAME_SIZE {
        return Err(io::Error::new(
            io::ErrorKind::InvalidData,
            "frame too large",
        ));
    }

    let mut buffer = vec![0u8; length];

    reader.read_exact(&mut buffer).await?;

    Ok(buffer)
}

pub async fn write_frame<W>(
    writer: &mut W,
    payload: &[u8],
) -> io::Result<()> 
where
    W: AsyncWrite + Unpin,
{
    let length = payload.len();

    if length > MAX_FRAME_SIZE {
        return Err(io::Error::new(
            io::ErrorKind::InvalidData,
            "frame too large",
        ));
    }

    writer.write_u32(length as u32).await?;
    writer.write_all(payload).await?;
    writer.flush().await?;
    
    Ok(())
}