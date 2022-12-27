from django.test import TestCase
from console_main.models import TrackUserPageVisits
from console_main.views import track_page_visit
from django.db.models import F


class BasicModelTest(TestCase):
    def setUp(self) -> None:
        self.did = 'modeltestDID'
        TrackUserPageVisits.objects.create(did=self.did, name='model test description', view='test_view_name',
                                           is_service=False, activity_completed=False, number_visits=1)

    def test_update(self):
        track_page_visit(did=self.did, name='model test description', view='test_view_name', is_service=False, activity=False)
        obj = TrackUserPageVisits.objects.get(did=self.did)
        assert obj.number_visits > 1

    def test_activity_completed(self):
        track_page_visit(did=self.did, name='model test description', view='test_view_name', is_service=False,
                         activity=True)
        obj_list = TrackUserPageVisits.objects.filter(did=self.did , view='test_view_name')
        assert(len(obj_list) == 2)

    def test_entry_new(self):
        track_page_visit(did=self.did , name='second model test' , view='second_test_view_name' , is_service=True,
                         activity=False)

        obj_list = TrackUserPageVisits.objects.filter(did=self.did , view='second_test_view_name')
        assert (len(obj_list) > 0)

    def test_delete(self):
        entry = TrackUserPageVisits.objects.get(did=self.did)
        entry.delete()
        try:
            TrackUserPageVisits.objects.get(did=self.did)
            assert False, 'object was not deleted'
        except Exception as e:
            assert True

    @staticmethod
    def test_user_name():
        assert TrackUserPageVisits.user_name() is not None
