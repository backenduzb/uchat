from django.urls import include, path
from django.conf import settings
from django.conf.urls.static import static

urlpatterns = [
    path(
        "api/admin/",
        include("identity.admin.urls"),
    ),
    path(
        "api/auth/",
        include("identity.users.api.urls"),
    ),
    path(
        "api/users/",
        include("identity.details.api.urls"),
    ),
]
urlpatterns += static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)