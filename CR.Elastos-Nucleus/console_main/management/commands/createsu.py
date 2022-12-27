from django.core.management.base import BaseCommand, CommandError
from login.models import DIDUser
from decouple import config


class Command(BaseCommand):

    def handle(self, *args, **options):
        if not DIDUser.objects.filter(email=config('SUPERUSER_USER')).exists():
            DIDUser.objects.create_superuser(config('SUPERUSER_USER'), config('SUPERUSER_PASSWORD'))
            self.stdout.write(self.style.SUCCESS('Successfully created new super user'))
