$(document).ready(()=>{
	$('.player-select').click(e=>{
		let t = $(e.target);
		$('.player-select').removeClass('selected');
		t.addClass('selected');
		let name = $('#worms-name').val();
		if(name.length > 0 || name.length < 4) sendNamePlayer();
	});
	$('.wbutton').click(e=>{
		highlight(e.target, "", 'lightgreen', 200);
	});
	$('.weapon-radio').click(e=>{
		$('.weapon-radio').removeClass('selected');
		$(e.target).addClass('selected');
		sendWormsC({'weapon': $(e.target).attr('data')});
	});
	$('#worms-shoot').click(e=>{
		highlight(e.target, '', 'red', 500);
		sendWormsC({'shoot': 1});
	});
	$('#confirm-select').click(e=>{
		highlight(e.target, '', 'red', 500);
		sendWormsC({'worm': 'c' + $(e.target).attr('data')});
	});
	$('#map-confirm').click(e=>{
		sendWormsC({'mapconfirm': 1}, e=>{
			if(e.mapconfirm) return;
			decentAlert('select a map and make sure to have set your name and player');
		});
	});
	$('#worms-name').keyup(e=>{
		if(e.key != "Enter") return;
		sendNamePlayer();
	});
	$('#worms-left').click(e=>move("l"));
	$('#worms-right').click(e=>move("r"));
	$.ajax({
			"url": "listfiles",
			"type": "GET",
			"dataType": "json"
		}).done((e)=>{
			$('#worms-maps').html('');
			for(let n of e){
				if(!n.startsWith('/paints/')) continue;
				let dispn = n.replace('/paints/', '').replace('.paint', '');
				$('#worms-maps').append('<div data="' + n + '" class="open-file">' + dispn + '</div>');
			}
			$('.open-file').click(e=>{
				$('.open-file').removeClass('selected');
				$(e.target).addClass('selected');
				sendWormsC({'setmap': $(e.target).html()});
			});
		}).fail((e)=> console.log(e));

	$('#slider-knob').on('touchmove', e=>{
		e.preventDefault();
		sliding(e.changedTouches[0].clientY);
	});
	$('#slider-knob').bind('touchend', function(){
	    $('#slider-val').css({'display': 'none'});
	    move('');
	});
	let down = false;
	$(document).mousedown(function(e){if(e.which === 1) down = true;});
    $(document).mouseup(function(e){
    	if(e.which === 1) down = false; 
    	$('#slider-val').css({'display': 'none'});
    	move('');
    });
	$('#slider-knob').mousemove(e=>{
		if(!down) return;
		sliding(e.clientY);
	});

	getState();
});

function sliding(y){
	let sliderheight = parseFloat($('#slider-knob').css('height'));
	let starty = $('#elevation').position().top;
	let maxtop = parseFloat($('#elevation').css('height')) - sliderheight;
	let slidertop = y - starty - sliderheight/2;
	if(slidertop < 0) slidertop = 0;
	if(slidertop > maxtop) slidertop = maxtop;
	$('#slider-knob').css('top', '' + slidertop + 'px');

	let startx = $('#elevation').position().left - 50;

	$('#slider-val').css({'top': (starty + slidertop + 1) + 'px', 'left': startx + 'px', 'display': 'block'});
	let value = sliderScale(slidertop/maxtop);
	if(isNaN(value)) 
		console.log('slider value isnan!')
	$('#slider-val').html('' + value);
}

function setSlider(val){
	let norm = (val+2)/4;
	let sliderheight = parseFloat($('#slider-knob').css('height'));
	let maxtop = parseFloat($('#elevation').css('height')) - sliderheight;
	$('#slider-knob').css('top', '' + (maxtop * norm).toFixed(0) + 'px');
}

function sliderScale(val){
	return -((val*4)-2).toFixed(1);
}

function sendNamePlayer(){
	let msg = "";
	if($('#player1').hasClass('selected')) msg = "0";
	else if($('#player2').hasClass('selected')) msg = "1";
	else{
		decentAlert('select player first!');
		return;
	}
	if(state !== undefined && state.state != 0){
		sendWorms({'verify': getCookie('token' + msg)}, e=>{
			if(e.verify) decentAlert('welcome back ' + playername());
			else setCookie('token' + msg, ';Expires=Wed, 21 Oct 2015 07:28:00 GMT');
		});
		return;
	}
	let name = $('#worms-name').val();
	if(name.length < 1 || name.length > 3) {
		decentAlert('name must have 1-3 letters');
		return;
	}
	sendWorms({'player': msg + name}, e=>{
		console.log(e);
		if(e.player === undefined || e.player == 0){
			if(parseInt(getToken()) > 0){
				decentAlert('player already set. if you can\'t play try to reload the page.');
				return;
			}
			decentAlert('something went wrong. please reload the page.');
			return;
		}
		setCookie('token' + msg, e.player)
	});
}

function playername(str){
	if(str == '0') return state.p1_name;
	if(str == '1') return state.p2_name;
}

function move(dir){
	if(state.state == 2 || state.state == 4 || state.state == 6 || state.state == 8){
		let dy = parseFloat($('#slider-val').html()) * -1;
		if(isNaN(dy)) {
			dy = 0;
			console.log('caution elevation slider produced NaN');
		}
		sendWormsC({'move': dir+dy}, e => {
			let res = e.move;
			if(res.startsWith('r') || res.startsWith('l')) res = res.substring(1);
			$('#slider-knob').css('top', );
		});
	}
	if(state.state == 1){
		selectnextworm(dir, "0");
	}
	if(state.state == 5){
		selectnextworm(dir, "1");
	}
}

function selectnextworm(dir, player){
	let inc = 0;
	if(dir == 'l') inc = -1;
	if(dir == 'r') inc = 1;
	for(let k in state){
		if(k.startsWith('worm' + player + '.') && state[k].selected){
			last = parseInt(k[6]);
		}
	}
	let errcounter = 0;
	while(errcounter < 5){
		last += inc;
		if(last < 0) last = 3;
		errcounter++;
		last %= 4;
		if(state['worm' + player + '.'+last].health > 0) break;
	}
	$('#confirm-select').attr('data', last);
	sendWormsC({'worm': ''+last});
}

function decentAlert(msg){
	let t = $('#overlay');
	if(t.css('display') == 'block'){
		setTimeout(()=>decentAlert(msg), 1000);
		return;
	}
	t.html(msg);
	t.css('display', 'block');
	setTimeout(()=>{
		t.css('display', 'none');
		t.html('');
	}, 900);
}

let debug = null;

let state = {};
function getState(){
	let data = {'state': 1}
	sendWorms(data, e=>{
		state = e;
		switch(state.state){
			case 0:
				setTitle('setting up');
				setContainers('11000');
				break;
			case 1:
				if($('#player1').hasClass('selected')){
					setTitle('select worm');
					setContainers('00110');
				}
				else{
					setTitle('wait for ' + state.p1_name);
					setContainers('00000');
				}
				break;
			case 2:
				if($('#player1').hasClass('selected')){
					setTitle('move/shoot ' + (state.t_rest/1000));
					setContainers('00011');
					setSelectors('0');
				}
				else{
					setTitle('wait for ' + state.p1_name);
					setContainers('00000');
				}
				break;
			case 3:
				setContainers('00000');
				if($('#player1').hasClass('selected')){
					setTitle('wait for hit');
				}
				else{
					setTitle('wait for ' + state.p1_name);
				}
				break;
			case 4:
				if($('#player1').hasClass('selected')){
					setTitle('move again ' + (state.t_rest/1000));
					setContainers('00010');
				}
				else{
					setTitle('wait for ' + state.p1_name);
					setContainers('00000');
				}
				break;
			case 5:
				if($('#player2').hasClass('selected')){
					setTitle('select worm');
					setContainers('00110');
				}
				else{
					setTitle('wait for ' + state.p2_name);
					setContainers('00000');
				}
				break;
			case 6:
				if($('#player2').hasClass('selected')){
					setTitle('move/shoot ' + (state.t_rest/1000));
					setContainers('00011');
					setSelectors('1');
				}
				else{
					setTitle('wait for ' + state.p2_name);
					setContainers('00000');
				}
				break;
			case 7:
				setContainers('00000');
				if($('#player2').hasClass('selected')){
					setTitle('wait for hit');
				}
				else{
					setTitle('wait for ' + state.p2_name);
				}
				break;
			case 8:
				if($('#player2').hasClass('selected')){
					setTitle('move again ' + (state.t_rest/1000));
					setContainers('00010');
				}
				else{
					setTitle('wait for ' + state.p2_name);
					setContainers('00000');
				}
				break;
			case 9:
				setTitle('setting up');
				setContainers('11000');
				setCookie('token0', ';Expires=Wed, 21 Oct 2015 07:28:00 GMT');
				setCookie('token1', ';Expires=Wed, 21 Oct 2015 07:28:00 GMT');
				setCookie('token', ';Expires=Wed, 21 Oct 2015 07:28:00 GMT');
				break;
		}
		if(!$('#player1').hasClass('selected') && !$('#player2').hasClass('selected')){
			$('#player-select').css('display', 'block');
		}
		if(state.p1_name !== undefined && 
			state.p1_name != null && 
			state.p1_name.length > 0 && 
			state.state != 0 &&
			state.state != 9)
			$('#player1').html(state.p1_name);
		else $('#player1').html('Player1');
		if(state.p2_name !== undefined && 
			state.p2_name != null && 
			state.p2_name.length > 0 && 
			state.state != 0 &&
			state.state != 9)
			$('#player2').html(state.p2_name);
		else $('#player2').html('Player2');
		if(debug !== null) setContainers('11111');
	});
	setTimeout(()=>getState(), 300);
}

function setContainers(str){
	if(str[0] == "1") $('#player-select').css({'display': 'block'});
	else $('#player-select').css({'display': 'none'});
	if(str[1] == "1") $('#map-select').css({'display': 'block'});
	else $('#map-select').css({'display': 'none'});
	if(str[2] == "1") $('#confirm-select').css({'display': 'block'});
	else $('#confirm-select').css({'display': 'none'});
	if(str[3] == "1") $('#move-container').css({'display': 'block'});
	else $('#move-container').css({'display': 'none'});
	if(str[4] == "1") $('#weapons-container').css({'display': 'block'});
	else $('#weapons-container').css({'display': 'none'});
}

function setSelectors(player, state){
	for(k in state){
		if(!k.startsWith('worm' + player + '.')) continue;
		if(!state[k].selected) continue;
		setSlider(state[k].dy);
		$('.weapon-radio').removeClass('selected');
		$('.weapon-radio[data=' + state[k].weapon + ']').addClass('selected');
	}
}

function setTitle(msg){
	$('#worms-state').html('worms: ' + msg);
}

function sendWorms(data, after){
	//console.log("sending:");
	//console.log(data);
	$.ajax({
		"url": "worms",
		"type": "POST",
		"dataType": "json",
		"data": data
	}).done((e)=>{
		//console.log(e);
		if(after !== undefined) after(e);
	}).fail((e)=> console.log(e));
}

function sendWormsC(data, after){
	let token = getToken();
	if(!(parseInt(token) > 0)){
		decentAlert('set player and name first!');
	}
	let dc = {'token': token};
	for(let k in data) dc[k] = data[k];
	sendWorms(dc);
}

function setCookie(key, value){
	document.cookie = "" + key + "=" + value;
}

function getToken(){
	if($('#player1').hasClass('selected'))
	return getCookie('token0');
	if($('#player2').hasClass('selected'))
	return getCookie('token1');;
}

function getCookie(key){
	let all = document.cookie;
	for(let c of all.split('; ')){
		let kv = c.split('=');
		if(kv[0] == key) return kv[1];
	}
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