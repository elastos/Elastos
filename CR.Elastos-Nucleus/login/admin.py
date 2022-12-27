from django.contrib import admin
from django.contrib.auth.admin import UserAdmin

from .forms import DIDUserCreationForm, DIDUserChangeForm
from .models import DIDUser, DIDRequest


class DIDUserAdmin(UserAdmin):
    add_form = DIDUserCreationForm
    form = DIDUserChangeForm
    model = DIDUser
    list_display = ('email', 'name', 'did', 'date_joined',
                    'is_superuser', 'is_staff', 'is_active',)
    list_filter = ('email', 'name', 'did', 'date_joined',
                   'is_superuser', 'is_staff', 'is_active',)
    fieldsets = (
        (None, {'fields': ('email', 'password')}),
        ('Permissions', {'fields': ('is_superuser', 'is_staff', 'is_active')}),
    )
    add_fieldsets = (
        (None, {
            'classes': ('wide',),
            'fields': (
                'email', 'name', 'did', 'date_joined', 'password1', 'password2', 'is_superuser', 'is_staff',
                'is_active')}
         ),
    )
    search_fields = ('email', 'name', 'did',)
    ordering = ('email', 'name', 'did',)


admin.site.register(DIDUser, DIDUserAdmin)


class DIDRequestAdmin(admin.ModelAdmin):
    model = DIDRequest
    list_display = ('id', 'state', 'data', 'created_at', 'last_updated')


admin.site.register(DIDRequest, DIDRequestAdmin)
