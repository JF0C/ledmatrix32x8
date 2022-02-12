$(document).ready(()=>{
	$('#new').click((e)=>{
		if($(e.target).hasClass('clicked')) return;
		let data = {};
		data[$('#ssid').val()] = $('#pw').val();
		$.ajax({
			'url': '/addwifi',
			'method': 'POST',
			'type': 'text',
			'data': data
		}).done(e=>{
			console.log(e);
			loadWifis();
		}).fail(e=>console.log(e));
		highlight(e.target);
	});
	loadWifis();
});

function loadWifis(){
	$('#knownwifis').html('');
	$.ajax({
		'url': '/wificonfig.json',
		'method': 'GET',
		'type': 'json'
	}).done(e=>{
		e = JSON.parse(e);
		for(let k in e){
			$('#knownwifis').append('<div class="wifi-entry">' +
				'<div class="wifi-name" data="' + e[k] + '">' + k + '</div>' +
				'<div class="wifi-remove" data="' + k + '">x</div>' + 
				'</div>');
		}
		$('.wifi-name').click(e=>{
			$('.wifi-entry').removeClass('selected');
			$(e.target).parent().addClass('selected');
		});
		$('.wifi-remove').click(e=>{
			removeWifi($(e.target).attr('data'));
		});
	});
}

function setWifi(ssid, pw){
	$.ajax({
		'url': '/set',
		'method': 'POST',
		'type': 'text',
		'data': {'ssid': ssid}
	}).done(e=>{
		console.log(e);
		$.ajax({
			'url': '/set',
			'method': 'POST',
			'type': 'text',
			'data': {'pw': pw}
		}).done(e=>{
			console.log(e);
		}).fail(e=>console.log(e));
	}).fail(e=>console.log(e));
}

function removeWifi(ssid){
	$.ajax({
		'url': '/addwifi',
		'method': 'POST',
		'type': 'text',
		'data': {'remove': ssid}
	}).done(e=>{
		console.log(e);
		loadWifis();
	}).fail(e=>console.log(e));
}

function highlight(e){
	let t = $(e);
	if(t.hasClass('clicked')) return;
	t.addClass('clicked');
	let temp = t.css('background');
	t.css('background', 'green');
	setTimeout(()=>{
		t.css('background', temp);
		t.removeClass('clicked');
	}, 300);
}