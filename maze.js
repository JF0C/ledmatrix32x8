$(document).ready(()=>{
	$('.maze-control').click(e=>{
		highlight(e.target, 'blue');
		if(!$('#single-player').hasClass('selected') && 
			!$('#player1').hasClass('selected') && 
			!$('#player2').hasClass('selected')){
			alert('select player first');
			return;
		}
		let val = 0;
		let player = getPlayer();
		let id = $(e.target).attr('id');
		if(player == "player1"){
			switch(id){
				case "maze-up": val = 4; break;
				case "maze-down": val = 3; break;
				case "maze-left": val = 1; break;
				case "maze-right": val = 2; break;
			}
		}
		else{
			switch(id){
				case "maze-up": val = 3; break;
				case "maze-down": val = 4; break;
				case "maze-left": val = 2; break;
				case "maze-right": val = 1; break;
			}
		}
		let data = {};
		data[player] = val;
		send(data);
	});
	$('.maze-player').click(e=>{
		$('.maze-player').removeClass('selected');
		$(e.target).addClass('selected');
		let data = {};
		let player = getPlayer();
		data[player] = 0;
		if($('#single-player').hasClass('selected')) data[player] = 5;
		send(data);
	});
});

function getPlayer(){
	let player = "";
	if($('#single-player').hasClass('selected') || $('#player1').hasClass('selected'))
		player = "player1";
	if($('#player2').hasClass('selected')) 
		player = "player2";
	return player;
}

function send(data){
	$.ajax({
		'url': 'maze',
		'method': 'POST',
		'dataType': 'text',
		'data': data
	}).done(e=>console.log(e)).fail(e=>console.log(e));
}

function highlight(e, col){
	let t = $(e);
	if(t.hasClass('clicked')) return;
	t.addClass('clicked');
	let temp = t.css('background');
	t.css('background', col);
	setTimeout(()=>{
		t.css('background', temp);
		t.removeClass('clicked');
	}, 300);
}