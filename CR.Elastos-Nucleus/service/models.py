# Create your models here.
from django.db import models


class files(models.Model):
    name = models.CharField(max_length=500)
    uploaded_file = models.FileField(upload_to='user_files')

