from django.urls import include, path


urlpatterns = [
    path(
        "api/admin/",
        include("identity.admin.urls"),
    ),
    path(
        "api/auth/",
        include("identity.users.api.urls"),
    ),
]