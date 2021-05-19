from django.contrib.auth.base_user import AbstractBaseUser
from django.contrib.auth.models import PermissionsMixin
from django.db import models
from django.contrib.postgres.fields import JSONField
from django.utils import timezone

from .managers import DIDUserManager


class DIDUser(AbstractBaseUser, PermissionsMixin):
    email = models.EmailField(max_length=255, unique=True)
    is_staff = models.BooleanField(default=False)
    is_active = models.BooleanField(default=True)
    name = models.CharField(max_length=255)
    did = models.CharField(max_length=64)
    date_joined = models.DateTimeField(default=timezone.now)
    last_updated = models.DateTimeField(auto_now=True)
    USERNAME_FIELD = 'email'
    REQUIRED_FIELDS = []
    objects = DIDUserManager()

    def __str__(self):
        return self.email

    @staticmethod
    def user_name():
        return 'User Identification Info'


class DIDRequest(models.Model):
    state = models.CharField(max_length=20)
    data = JSONField()
    created_at = models.DateTimeField(default=timezone.now)
    last_updated = models.DateTimeField(auto_now=True)

