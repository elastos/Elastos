from django.contrib import admin

from .models import UploadFile, UserServiceSessionVars, SavedFileInformation, SavedETHContractInformation


class UploadFileAdmin(admin.ModelAdmin):
    model = UploadFile
    list_display = ('did', 'uploaded_file')


admin.site.register(UploadFile, UploadFileAdmin)


class UserServiceSessionVarsAdmin(admin.ModelAdmin):
    model = UserServiceSessionVars
    list_display = (
    'did', 'api_key', 'mnemonic_mainchain', 'private_key_mainchain', 'public_key_mainchain', 'address_mainchain',
    'private_key_did', 'public_key_did', 'address_did', 'did_did', 'address_eth', 'private_key_eth')


admin.site.register(UserServiceSessionVars, UserServiceSessionVarsAdmin)


class SavedFileInformationAdmin(admin.ModelAdmin):
    model = SavedFileInformation
    list_display = ('did', 'file_name', 'message_hash', 'signature', 'file_hash')


admin.site.register(SavedFileInformation, SavedFileInformationAdmin)


class SavedETHContractInformationAdmin(admin.ModelAdmin):
    model = SavedETHContractInformation
    list_display = ('did', 'file_name','contract_name', 'contract_address', 'contract_code_hash')


admin.site.register(SavedETHContractInformation, SavedETHContractInformationAdmin)
