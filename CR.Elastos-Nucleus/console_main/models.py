# Create your models here.
from django.db import models

from django.utils import timezone


class TrackUserPageVisits(models.Model):
    did = models.CharField(max_length=64)
    name = models.CharField(max_length=300)  # use to display name or description to user
    view = models.CharField(max_length=400)  # give the reverse url that would normally be in template(helps reduce
    # code)
    last_visited = models.DateTimeField(default=timezone.now)
    number_visits = models.PositiveIntegerField(default=0)
    is_service = models.BooleanField(default=False)
    activity_completed = models.BooleanField(default=False)
    additional_field = models.CharField(max_length=200, default='')

    @staticmethod
    def user_name():
        return 'User Visits'
