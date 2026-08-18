import uuid
from django.contrib.auth.models import AbstractBaseUser, PermissionsMixin
from typing_extensions import override
from ..managers.user import UserManager
from django.db import models
from django.utils import timezone
from ..services.pfpimg import optimize_pfp

class User(AbstractBaseUser, PermissionsMixin):
    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    
    username = models.CharField(max_length=150, unique=True, db_index=True)
    email = models.EmailField(unique=True, db_index=True)
    password = models.CharField(max_length=128)  
    
    first_name = models.CharField(max_length=150, blank=True, default="")
    last_name = models.CharField(max_length=150, blank=True, default="")
    
    is_active = models.BooleanField(default=True)
    is_staff = models.BooleanField(default=False)
    is_superuser = models.BooleanField(default=False)

    profile_pic = models.ImageField(null=True, blank=True, upload_to="pfps/")
    bio = models.CharField(max_length=256, null=True, blank=True)

    date_joined = models.DateTimeField(default=timezone.now)
    date_of_birth = models.DateTimeField(null=True, blank=True)
    updated_at = models.DateTimeField(auto_now=True)

    USERNAME_FIELD = "username"  

    objects = UserManager()
    
    def save(self, *args, **kwargs):
        if self.pk:
            try:
                old_instance = User.objects.get(pk=self.pk)

                if (old_instance.profile_pic and
                        old_instance.profile_pic != self.profile_pic):
                    if old_instance.profile_pic.name and \
                            old_instance.profile_pic.storage.exists(old_instance.profile_pic.name):
                        old_instance.profile_pic.storage.delete(old_instance.profile_pic.name)
            except User.DoesNotExist:
                pass

        if self.profile_pic and not self.profile_pic.name.endswith(".webp"):
            optimized = optimize_pfp(self.profile_pic)
            if optimized:
                self.profile_pic.save(optimized.name, optimized, save=False)

        super().save(*args, **kwargs)

    class Meta:
        db_table = "users"
        verbose_name = "User"
        verbose_name_plural = "Users"
        ordering = ["-date_joined"]

