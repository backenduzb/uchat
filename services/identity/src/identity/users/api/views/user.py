from rest_framework import generics

from identity.users.models import User
from ..serializers.user import UserSerializer


class UserListView(generics.ListAPIView):
    queryset = User.objects.all()
    serializer_class = UserSerializer
