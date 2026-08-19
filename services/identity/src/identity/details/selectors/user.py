from identity.users.models import User
from django.db.models import Q
from django.shortcuts import get_object_or_404

def search_users(search_term: str = ""):
    if not search_term or not search_term.strip():
        return list(User.objects.all()[:50])  
        
    users = User.objects.filter(
            Q(first_name__icontains=search_term) | 
            Q(last_name__icontains=search_term) | 
            Q(email__icontains=search_term) | 
            Q(username__icontains=search_term)
    )
    
    return list(users)

def get_user_by_id(id: str):
    user = get_object_or_404(User, id)
    
    return user

