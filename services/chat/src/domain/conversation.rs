use uuid::Uuid;

#[derive(Debug, Clone, Copy)]
pub enum ConversationType {
    Direct,
    Group,
} 

#[derive(Debug, Clone)]
pub struct Conversation {
    pub id: Uuid,
    pub consation_type: ConversationType,
}