from rest_framework.viewsets import ModelViewSet
from rest_framework.response import Response
from rest_framework.filters import SearchFilter
from rest_framework.permissions import IsAdminUser, IsAuthenticated, AllowAny
from ..serializers.user import UserAdminSerializer
from identity.users.models.user import User


class UserViewset(ModelViewSet):
    queryset = (
        User.objects.all().order_by('date_joined')
    )
    serializer_class = UserAdminSerializer
    permission_classes = [IsAdminUser]
    filter_backends = [SearchFilter]
    search_fields = [
        "username",
        "first_name",
        "last_name",
        "email",
    ]
