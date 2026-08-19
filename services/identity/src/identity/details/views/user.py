from rest_framework import generics, views

from identity.users.models import User
from ..serializers.user import UserDetailSerializer
from rest_framework.permissions import IsAuthenticated
from ..selectors.user import get_user_by_id, search_users
from rest_framework.response import Response

class UserSearchView(views.APIView):
    model = User
    serializer_class = UserDetailSerializer
    permission_classes = [IsAuthenticated]

    def get(self, request):
        user_id = request.query_params.get("id")

        user = get_user_by_id(user_id)
        serializer = self.serializer_class(user)
        return Response(
            serializer.data,
            status=200,
        )

    def post(self, request):
        search_term = request.data.get("search_term", "")

        users = search_users(search_term)
        serializer = self.serializer_class(users, many=True)
        
        return Response(
            serializer.data,
            status=200
        )