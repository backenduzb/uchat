from ..views.user import UserSearchView
from django.urls import path

urlpatterns = [
    path("search/", UserSearchView.as_view(), name="user-search")
]
