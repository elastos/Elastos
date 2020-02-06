from django.test import LiveServerTestCase
from selenium import webdriver
from decouple import config


class LandingPageCase(LiveServerTestCase):
    def setUp(self):
        options = webdriver.ChromeOptions()
        options.add_argument('--incognito')
        path = config('WEB_DRIVER_PATH')
        self.selenium = webdriver.Chrome(executable_path=path, chrome_options=options)
        super(LandingPageCase , self).setUp()

    def tearDown(self):
        self.selenium.quit()
        super(LandingPageCase , self).tearDown()

    def test_feed(self):
        selenium = self.selenium
        selenium.get("http://127.0.0.1:8000/")
        landing_title = selenium.find_elements_by_tag_name('h1')
        assert len(landing_title) > 0

