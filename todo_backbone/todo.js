var TodoList = Backbone.Model.extend({
	defaults:{
		name: "",
		id: null,
		created: Date(),
		owner:null
	}
});

var TodoListCollection = Backbone.Collection.extend({
	Model: TodoList,
	localStorage: new Backbone.LocalStorage('backbone-storage-lists')
});

var TodoItem = Backbone.Model.extend({
	defaults:{
		description:"",
		id: null,
		list: null,
		completed: false,
		due_date: Date()
	}
});

var TodoItemCollection = Backbone.Collection.extend({
	Model: TodoItem,
	localStorage: new Backbone.LocalStorage('backbone-storage-items')
});

var ListView = Backbone.View.extend({
	tagName: 'tr',
	template: _.template($('#list-template').html()),
	initialize: function(){},

	render: function(){
		console.log(this.template);
		this.$el.html(this.template(this.model.attributes));
		return this;
	}
});

var lists = new TodoListCollection();

var ListCollectionView = Backbone.View.extend({
	model: TodoList,
	el: '.container',
	initialize: function(){
		lists.fetch();
		$("#div2").hide();
	},

	events:
	{
		'click .add-name': 'add',
		'click .update':'update'
	},

	add: function(){
		name = $('.name-input').val(),
 		id = lists.length + 1,
 		list = new TodoList({name: name, id: id});
		lists.create(list);
		console.log(lists);
		// this.render();
		this.$el.append(new ListView({model: list}).render().el);
		$('.name-input').val('');
	},

	update:function(e){
		console.log($(e.currentTarget).val())
	},

	render: function(){
		console.log(this.$el);
		lists.each(function(list){
			this.$el.append(new ListView({model: list}).render().el);
		}, this);
		return this;
	}

});


var ItemView=Backbone.View.extend({
	tagName:'li',
	template:_.template($("#item-template").html()),
	initialize:function(){
	},

	render:function(){
		this.$el.html(this.template(this.model.attributes));
		return this;
	}
});

var items=new TodoItemCollection();

var ItemCollctionView=Backbone.View.extend({
	tagName:'li',
	el:'.items',
	initialize:function(options){
		this.options=options;
		items.fetch();
	},

	events:
	{
		'click .add-item': 'additem'
	},

	additem: function(){
		console.log("In additem");
		name = $('.item-name').val(),
 		id = items.length + 1,
 		item = new TodoItem({description: name, id: id,due_date:Date(),list:this.options.parentid});
		items.create(item);
		console.log(items);
		// this.render();
		this.$el.append(new ItemView({model: item}).render().el);
		$('.item-name').val('');
	},

	render: function(){
		//var todos=items.where({parent:this.options.parentid});
		items.each(function(item){
			console.log(item.get("list"));
			if (item.get("list")==this.options.parentid)
				this.$el.append(new ItemView({model:item}).render().el);
		},this);
		return this;
	}
})


var router=Backbone.Router.extend({
	routes:{
		'show/:id':'showitems'
	},

	showitems:function(id){
		console.log("Hello");
		console.log("IN router");
		var itemview=new ItemCollctionView({parentid:id});
		itemview.render();
		$('#div1').hide();
		$('#div2').show();
	}
});

new router;

Backbone.history.start();
var view = new ListCollectionView();
view.render();