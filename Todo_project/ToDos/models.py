from __future__ import unicode_literals

from django.db import models


class Todolist(models.Model):
    name = models.CharField(max_length=30)
    created = models.DateField()

    def __unicode__(self):
        return self.name


class Todoitem(models.Model):
    description = models.TextField(max_length=100)
    due_date = models.DateField(blank=False,null=True)
    completed = models.BooleanField(default=False)
    list = models.ForeignKey(Todolist)

    def __unicode__(self):
        return self.description


# Create your models here.
