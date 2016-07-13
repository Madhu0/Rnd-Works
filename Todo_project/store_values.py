import sys
import os
import django
import click


sys.path.append("../Todo_project")
os.environ["DJANGO_SETTINGS_MODULE"] = "Todo_project.settings"
django.setup()

from ToDos.models import *

@click.group()
def store_values():
    pass

@store_values.command()
def populate():
    list_names=['daily','weekly','monthly','yearly','mandatory','optional','suggested_by_friends','suggested_to_friends','post_poned','high_priority']
    list_date=['2016-05-16','2016-04-10','2016-05-12','2016-05-11','2016-04-10','2016-01-10','2016-02-16','2016-04-04','2016-03-26','2016-04-12']
    for i in range(10):
        t=Todolist(name=list_names[i],created=list_date[i])
        t.save()

    item_list=["gardening",'nursery','python_assignments','c_assignments','walking']

    for i in range(10):
        for j in range(5):
            t=Todoitem(description="This is "+item_list[j]+str(j),completed=list_date[(i+5)%10],list=Todolist.objects.get(name=list_names[i]))
            t.save()


@store_values.command()
def clear_data():
    Todolist.objects.all().delete()
    Todoitem.objects.all().delete()


if __name__ == "__main__":
    store_values()