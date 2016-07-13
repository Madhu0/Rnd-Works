from django.views.generic.list import ListView
from django.views.generic.detail import DetailView
from django.views.generic.edit import CreateView
from django.core.urlresolvers import reverse
from django.views.generic.edit import UpdateView
from django.views.generic.edit import DeleteView

from ToDos.models import *

class Todolist_view(ListView):
    model=Todolist

class Todolist_detailview(DetailView):
    model=Todolist
    context_object_name = "list"

class Todolist_create(CreateView):
    model=Todolist
    fields = ['name','created']

    def get_success_url(self):
        return reverse('views')

class Todolist_update(UpdateView):
    model=Todolist
    fields = ['name','created']
    template_name = "ToDos/update_form.html"

    def get_success_url(self):
        return reverse('views')


class Todolist_delete(DeleteView):
    model=Todolist
    template_name = "ToDos/delete_form.html"

    def get_success_url(self):
        return reverse('views')

class Todoitem_create(CreateView):

    model = Todoitem

    fields = ['description','due_date','completed']

    def form_valid(self, form):
        form.fields['list_id']=self.kwargs.get('pk')
        if form.is_valid():
            form.save(commit=False)
            form.save()
        return super(Todoitem_create,self).form_valid(form)

    def get_success_url(self):
        return reverse('view_detail')
