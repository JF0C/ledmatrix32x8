$(document).ready(()=>{
	$.ajax({
		"url": "/listfiles",
		"type": "GET",
		"dataType": "json"
	}).done((e)=> showfiles(e)).fail((e)=> console.log(e));
});

function showfiles(list){
	for(let name of list){
		$('body').append('<div class="delete" data="' + name + '">delete</div>' +
			'<div class="rename" data="' + name + '">rename</div>' +
			'<div class="filename">' + name + '</div><br>');
	}
	$('.delete').click(e=>{
		let file = $(e.target).attr('data');
		if(!confirm("sure?")) return;
		sendfiles("remove", file);
	});
	$('.filename').click(e=>{
		location = $(e.target).html();
	});
}

function sendfiles(key, val){
	console.log("setting " + key + " to " + val);
	let data = {};
	data[key] = val;
	$.ajax({
		"url": "changefile",
		"type": "POST",
		"dataType": "text",
		"data": data
	}).done((e)=> console.log(e)).fail((e)=> console.log(e));
}