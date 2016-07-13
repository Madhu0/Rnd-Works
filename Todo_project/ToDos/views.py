from django.shortcuts import render
from django.template import loader
from ToDos.models import *
from django.http.response import *

# Create your views here.


def todolist(request):
    list = Todolist.objects.all()
    template = loader.get_template('lists.html')
    result = template.render(context={'lists':list})
    return HttpResponse(result)


def listitem(request,id):
    items = Todoitem.objects.all().filter(list_id=id)
    template=loader.get_template("items.html")
    result=template.render(context={'items':items})
    return HttpResponse(result)