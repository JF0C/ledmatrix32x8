$(document).ready(()=>{
	$('#p1').click(()=>{
		$('#p1').addClass("selected");
		$('#p2').removeClass("selected");
		setName();
	});
	$('#p2').click(()=>{
		$('#p2').addClass("selected");
		$('#p1').removeClass("selected");
		setName();
	});
	$('.fscontainer').css('line-height', $($('.fscontainer')[0]).css('height'));
	$('.fsbutton').click((e)=>{
		if(!$('#p1').hasClass("selected") && !$('#p2').hasClass("selected")){
			highlight($(e.target), "select player first", 'darkred', 1000);
			return;
		}
		if($('#p1').hasClass("selected")){
			highlight($(e.target), "", 'lightgreen', 200);
			sendpong("p1", $(e.target).attr('data'));
		}
		if($('#p2').hasClass("selected")){
			highlight($(e.target), "", 'lightgreen', 200);
			sendpong("p2", $(e.target).attr('data'));
		}
	});
	$('#pointscontainer .radio3').click(e=>{
		let limit = parseInt($(e.target).html());
		$('#pointscontainer .radio3').removeClass('selected');
		$(e.target).addClass('selected');
		sendpong("limit", limit);
	});
	$('#header').click(e=>{
		if($('#p1').hasClass("selected")){
			changeName("name_p1");
		}
		else if($('#p2').hasClass("selected")){
			changeName("name_p2");
		}
	});
	init();
});

function changeName(whichname){
	let name = prompt("Enter new name (max 3 letters)");
	if(name == null) return;
	name = name.substring(0,3);
	sendpong(whichname, name, ()=>init());
}

let conf = {};

function setName(){
	if($('#p1').hasClass("selected") && conf.name_p1 !== undefined && conf.name_p1 != ""){
		$('#header').html("Pong: " + conf.name_p1);
	}
	else if($('#p2').hasClass("selected") && conf.name_p2 !== undefined && conf.name_p2 != ""){
		$('#header').html("Pong: " + conf.name_p2);
	}
	else{
		$('#header').html("Pong");
	}
	if(conf.name_p1 !== undefined && conf.name_p1 != ""){
		$('#p1').html("Player 1: " + conf.name_p1);
	}
	else{
		$('#p1').html("Player 1");
	}
	if(conf.name_p2 !== undefined && conf.name_p2 != ""){
		$('#p2').html("Player 2: " + conf.name_p2);
	}
	else{
		$('#p2').html("Player 2");
	}
}

function init(){
	$.ajax({
		"url": "/pconfig.json",
		"type": "GET",
		"dataType": "json"
	}).done(e=>{
		$('#' + e.limit + "point").addClass('selected');
		conf = e;
		setName();
	}).fail(e => console.log(e));
}

function highlight(el, text, color, timeout){
	if($(el).attr('highlighted') == 'true') return;
	let temp_text = $(el).html();
	let temp_col = $(el).css('background');
	$(el).attr('highlighted', 'true');
	if(text != "")
		$(el).html(text);
	$(el).css('background', color);
	setTimeout(()=>{
		$(el).html(temp_text);
		$(el).css('background', temp_col);
		$(el).attr('highlighted', 'false');
	}, timeout);
}

function sendpong(key, val, sfun){
	console.log("setting " + key + " to " + val);
	let data = {};
	data[key] = val;
	$.ajax({
			"url": "pong",
			"type": "POST",
			"dataType": "text",
			"data": data
		}).done((e)=>{
			console.log(e); 
			if(sfun !== undefined)
				sfun();
		}).fail((e)=> console.log(e));
}