from django.conf.urls import url
from ToDos import views
from ToDos import classviews

urlpatterns=[
    url(r'todoslist/listitem/(?P<id>[0-9]+)$',views.listitem,name='ToDositems'),
    url(r'todoslist/$',classviews.Todolist_view.as_view(),name='views'),
    url(r'todoslist/(?P<pk>[0-9]+)$', classviews.Todolist_detailview.as_view(), name='views_detail'),
    url(r'todoslist/create/$', classviews.Todolist_create.as_view(), name='views_create'),
    url(r'todoslist/update/(?P<pk>[0-9]+)', classviews.Todolist_update.as_view(), name='views_update'),
    url(r'todoslist/delete/(?P<pk>[0-9]+)', classviews.Todolist_delete.as_view(), name='views_delete'),
    url(r'todoslist/(?P<pk>[0-9]+)/createitem', classviews.Todoitem_create.as_view(), name='viewitems')
]
