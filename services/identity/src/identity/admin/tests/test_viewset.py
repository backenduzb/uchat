from django.urls import reverse
from rest_framework.test import APITestCase
from rest_framework import status
from django.contrib.auth import get_user_model

User = get_user_model()

class UserViewsetTestCase(APITestCase):
    def setUp(self):
        self.admin = User.objects.create_superuser("admin", "admin@m.ru", "1234")
        self.user1 = User.objects.create_user("user1", "mina@m.ru", "daxxuya")

        self.list_url = reverse('userviewset-list')
        self.detail_url = reverse('userviewset-detail', kwargs={'pk': self.user1.id})

    def test_list_users(self):
        self.client.force_authenticate(user=self.admin)
        response = self.client.get(self.list_url)

        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data), 2)

    def test_create_user(self):
        self.client.force_authenticate(user=self.admin)
        data = {"username": "new_user", "email": "new@m.ru", "password": "pass123"}

        response = self.client.post(self.list_url, data)

        self.assertEqual(response.status_code, status.HTTP_201_CREATED)
        self.assertTrue(User.objects.filter(username="new_user").exists())

    def test_update_user(self):
        self.client.force_authenticate(user=self.admin)
        data = {"first_name": "NewName"}

        response = self.client.patch(self.detail_url, data)

        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.user1.refresh_from_db()
        self.assertEqual(self.user1.first_name, "NewName")

    def test_delete_user(self):
        self.client.force_authenticate(user=self.admin)
        response = self.client.delete(self.detail_url)

        self.assertEqual(response.status_code, status.HTTP_204_NO_CONTENT)
        self.assertFalse(User.objects.filter(id=self.user1.id).exists())
