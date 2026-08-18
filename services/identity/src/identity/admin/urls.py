from django.urls import path, include
from rest_framework.routers import DefaultRouter

from .viewsets.user import UserViewset

router = DefaultRouter()
router.register(f"users", UserViewset, basename="userviewset")

urlpatterns = [
    path("", include(router.urls))
]