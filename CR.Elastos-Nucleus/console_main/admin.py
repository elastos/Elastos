from django.contrib import admin

from .models import TrackUserPageVisits


class TrackUserPageVisitsAdmin(admin.ModelAdmin):
    model = TrackUserPageVisits
    list_display = ('did', 'name', 'view', 'last_visited', 'number_visits', 'is_service')


admin.site.register(TrackUserPageVisits, TrackUserPageVisitsAdmin)