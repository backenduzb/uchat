use anyhow::{anyhow, Result};
use jsonwebtoken::{
    decode,
    Algorithm,
    DecodingKey,
    Validation,
};
use uuid::Uuid;

use super::claims::Claims;

pub struct JwtVerifier {
    decoding_key: DecodingKey,
    validation: Validation,
}

impl JwtVerifier {
    pub fn from_pem(public_key: &[u8]) -> Result<Self> {
        let decoding_key =
            DecodingKey::from_rsa_pem(public_key)?;

        let mut validation =
            Validation::new(Algorithm::RS256);

        validation.validate_exp = true;

        Ok(Self {
            decoding_key,
            validation,
        })
    }

    pub fn verify(&self, token: &str) -> Result<Uuid> {
        let token_data = decode::<Claims>(
            token,
            &self.decoding_key,
            &self.validation,
        )?;

        let user_id =
            Uuid::parse_str(
                &token_data.claims.user_id
            )
            .map_err(|_| {
                anyhow!("invalid user_id in JWT")
            })?;

        Ok(user_id)
    }
}