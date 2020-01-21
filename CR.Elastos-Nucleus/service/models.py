# Create your models here.
import os

from django.db import models

from console_main import settings




class UploadFile(models.Model):
    did = models.CharField(max_length=64)
    uploaded_file = models.FileField(upload_to='user_files', blank=True)

    def delete(self, *args, **kwargs):
        os.remove(os.path.join(settings.MEDIA_ROOT, self.uploaded_file.name))
        super(UploadFile, self).delete(*args, **kwargs)

    def filename(self):
        name_list = self.uploaded_file.name.split('/')
        return name_list[-1]




class UserServiceSessionVars(models.Model):
    did = models.CharField(max_length=64)
    api_key = models.CharField(max_length=64)

    mnemonic_mainchain = models.CharField(max_length=300)
    private_key_mainchain = models.CharField(max_length=300)
    public_key_mainchain = models.CharField(max_length=300)
    address_mainchain = models.CharField(max_length=64)

    private_key_did = models.CharField(max_length=300)
    public_key_did = models.CharField(max_length=300)
    address_did = models.CharField(max_length=64)
    did_did = models.CharField(max_length=64)

    address_eth = models.CharField(max_length=64)
    private_key_eth = models.CharField(max_length=300)

    @staticmethod
    def user_name():
        return 'User BlockChain Wallet Information '


class SavedFileInformation(models.Model):
    did = models.CharField(max_length= 64 ,  null=False)
    file_name = models.CharField(max_length=300 , null=True)
    message_hash = models.CharField(max_length=300, null=False)
    signature = models.CharField(max_length=300 , null=False)
    file_hash = models.CharField(max_length=300 , null=False)

    def __str__(self):
        return self.file_name

    @staticmethod
    def user_name():
        return 'Uploaded File Information'


