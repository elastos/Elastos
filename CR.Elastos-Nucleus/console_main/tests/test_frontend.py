from django.test import LiveServerTestCase
from selenium import webdriver
from selenium.webdriver.common.by import By
from webdriver_manager.chrome import ChromeDriverManager
from selenium.common.exceptions import NoSuchElementException
from django_simple_cookie_consent.models import CookieConsentSettings
from decouple import config
import os
import time

class LandingPageCase(LiveServerTestCase):
    def setUp(self):
        CookieConsentSettings.objects.create(cookie_policy_link="google.com")
        options = webdriver.ChromeOptions()
        options.add_argument('--incognito')
        options.add_argument("--headless")
        options.add_argument("--no-sandbox")
        self.selenium = webdriver.Chrome(ChromeDriverManager().install(), chrome_options=options)
        super(LandingPageCase, self).setUp()

    def tearDown(self):
        self.selenium.quit()
        super(LandingPageCase, self).tearDown()

    def test_landing(self):
        selenium = self.selenium
        selenium.get(self.live_server_url)
        landing_title = selenium.title
        self.assertEqual(landing_title, "Landing - Elastos Nucleus")

    def test_login_button_exist(self):
        selenium = self.selenium
        selenium.get(self.live_server_url)
        button = selenium.find_elements_by_id("login_button")
        assert len(button) > 0

    def test_login_modal_exist(self):
        selenium = self.selenium
        selenium.get(self.live_server_url)
        modal = selenium.find_elements_by_id('myModal')
        assert len(modal) > 0

    def test_login_modal_toggle(self):
        selenium = self.selenium
        selenium.get(self.live_server_url)
        modal = selenium.find_element_by_id('myModal')
        if modal is None:
            assert False, "login modal doesn't exist"

        if modal.is_displayed():
            assert False, "login modal is not hidden at the start"

        button = selenium.find_element_by_id("login_button")
        button.click()
        time.sleep(1)
        if modal.is_displayed():
            assert True
        else:
            assert False

    def test_login_modal_toggle_switch(self):
        selenium = self.selenium
        selenium.get(self.live_server_url)
        try:
            modal = selenium.find_element_by_id('myModal')
        except NoSuchElementException:
            assert False, "login modal doesn't exist"
        if modal.is_displayed():
            assert False, "login modal is not hidden at the start"
        button = selenium.find_element_by_id("login_button")
        button.click()
        time.sleep(1)
        if modal.is_displayed():
            assert True
        else:
            assert False , "login button does not toggle modal"

        try:
            elephant_wallet = selenium.find_element_by_id("elephant_qr_code_tab")
            elephant_wallet.click()
            assert selenium.find_element_by_id("elephantWallet").is_displayed()
        except NoSuchElementException:
            assert False
    

    def test_cookie_consent(self):
        selenium = self.selenium
        selenium.get(self.live_server_url)
        time.sleep(3)
        consent = selenium.find_elements_by_id("cookieconsent:desc")
        assert len(consent) > 0
