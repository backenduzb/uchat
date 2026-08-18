from rest_framework.viewsets import ModelViewSet
from rest_framework.response import Response
from rest_framework.filters import SearchFilter
from rest_framework.permissions import IsAdminUser, IsAuthenticated, AllowAny
from rest_framework.parsers import MultiPartParser, FormParser, JSONParser 
from ..serializers.user import UserAdminSerializer
from identity.users.models.user import User


class UserViewset(ModelViewSet):
    queryset = (
        User.objects.all().order_by('date_joined')
    )
    serializer_class = UserAdminSerializer
    permission_classes = [IsAdminUser]
    
    parser_classes = [MultiPartParser, FormParser, JSONParser]

    filter_backends = [SearchFilter]
    search_fields = [
        "username",
        "first_name",
        "last_name",
        "email",
    ]

    def create(self, request, *args, **kwargs):
            print("FILES:", request.FILES)  
            print("DATA:", request.data)    
            return super().create(request, *args, **kwargs)