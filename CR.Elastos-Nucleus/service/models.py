# Create your models here.
import os

from django.db import models
from django.utils import timezone

from console_main import settings
from login import models as loginModels


class UploadFile(models.Model):
    did = models.CharField(max_length=64)
    uploaded_file = models.FileField(upload_to='user_files')

    def delete(self, *args, **kwargs):
        os.remove(os.path.join(settings.MEDIA_ROOT, self.uploaded_file.name))
        super(UploadFile, self).delete(*args, **kwargs)


class UserAPIKeys(models.Model):
    did = models.CharField(max_length=64)
    api_key = models.CharField(max_length=64)
