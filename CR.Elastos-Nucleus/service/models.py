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

    def your_activity(self):
        return {
            'generate_key': {
                'display_string': 'Generated a new API Key',
                'did': None,
                'api_key': None,
            },
            'create_wallet': {
                'display_string': 'Created a new wallet',
                'did': None,
                'api_key': None,
                'mnemonic_mainchain': None,
                'private_key_mainchain': None,
                'public_key_mainchain': self.public_key_mainchain,
                'address_mainchain': self.address_mainchain,
                'private_key_did': None,
                'public_key_did': self.public_key_did,
                'address_did': self.address_did,
                'did_did': self.did_did,
                'address_eth': self.address_eth,
                'private_key_eth': None,
            },
            'view_wallet': {
                'display_string': 'Viewed a wallet',
                'did': None,
                'api_key': None,
                'mnemonic_mainchain': None,
                'private_key_mainchain': None,
                'public_key_mainchain': self.public_key_mainchain,
                'address_mainchain': self.address_mainchain,
                'private_key_did': None,
                'public_key_did': self.public_key_did,
                'address_did': self.address_did,
                'did_did': self.did_did,
                'address_eth': self.address_eth,
                'private_key_eth': None,
            }
        }


class SavedFileInformation(models.Model):
    did = models.CharField(max_length=64, null=False)
    file_name = models.CharField(max_length=300, null=True)
    message_hash = models.CharField(max_length=300, null=False)
    signature = models.CharField(max_length=300, null=False)
    file_hash = models.CharField(max_length=300, null=False)

    def __str__(self):
        return self.file_name

    @staticmethod
    def user_name():
        return 'Uploaded File Information'

    def your_activity(self):
        return {
            'upload_and_sign': {
                'display_string': 'Signed a new file using a private key and uploaded the '
                                  'encrypted version of the file to Elastos Hive',
                'did': None,
                'file_name': self.file_name,
                'message_hash': self.message_hash,
                'signature': self.signature,
                'file_hash': self.file_hash
            },
            'verify_and_show': {
                'display_string': 'Verified the file using a private key and retrieved it '
                                  'from Elastos Hive',
                'did': None,
                'file_name': self.file_name,
                'message_hash': self.message_hash,
                'signature': self.signature,
                'file_hash': self.file_hash,
            }
        }
