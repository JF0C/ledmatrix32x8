$(document).ready(()=>{
	$('.paint-control').change(()=>{
		$('#brushsvg').remove();
		svgbrush(50, 1, 50, 'brush', '#' + getColor());
	});
	svgbrush(50, 1, 50, 'brush', '#' + getColor());
	$('.setcolor').click(e=>{
		if($(e.target).hasClass('clicked')) return;
		highlight(e.target, 'green');
		let data = {};

		let rgb = getRgb();
		let m = Math.max(rgb[0], rgb[1], rgb[2]);
		if(m != 0){
			m /= 255;
			rgb[0] = Math.round(rgb[0]/m);
			rgb[1] = Math.round(rgb[1]/m);
			rgb[2] = Math.round(rgb[2]/m);
		}

		data['col' + $(e.target).attr('data')] = rgb[0] + ',' + rgb[1] + ',' + rgb[2];
		sendFourier(data, e=>console.log(e));
		
		$(e.target).parent().css('background', 'rgb(' + rgb[0] + ',' + rgb[1] + ',' + rgb[2] + ')');
	});
	$('.clearcolor').click(e=>{
		if($(e.target).hasClass('clicked')) return;
		let data = {};
		data['col' + $(e.target).attr('data')] = '0,0,0';
		sendFourier(data, e=>console.log(e));
		highlight(e.target, 'red');
		$(e.target).parent().css('background', 'black');
	});
	$.ajax({
		'url': '/config.json',
		'method': 'GET',
		'dataType': 'json'
	}).done(e=>{
		let cols = e.audioCols.split(';');
		for(let k = 0; k < 6; k++){
			let rgb = cols[k].split(',');
			if(rgb.length != 3) continue;
			$('#color' + k).css('background', '#' + getColorRgb(rgb[0], rgb[1], rgb[2]));
		}
	}).fail(e=>console.log(e));
	$('#mirror-switch').click(()=>{
		$('#mirror-switch-thumb').toggleClass('active');
		if($('#mirror-switch-thumb').hasClass('active'))
			sendFourier({'mirror': 'true'});
		else
			sendFourier({'mirror': 'false'});
	});
	$('#min-freq').change(e=>sendFourier({'minfreq': $(e.target).val()}));
	$('#max-freq').change(e=>sendFourier({'maxfreq': $(e.target).val()}));
});

function sendFourier(obj, after){
	$.ajax({
		'url': '/fourier',
		'method': 'POST',
		'dataType': 'text',
		'data': obj
	}).done(e=>{
		if(after !== undefined)after(e);
	}).fail(e=>console.log(e));
}

function getColor(){
	let rgb = getRgb();
	return getColorRgb(rgb[0], rgb[1], rgb[2]);
}

function getRgb(){
	let r = parseInt($('#red').val());
	let g = parseInt($('#green').val());
	let b = parseInt($('#blue').val());
	return [r, g, b];
}

function getColorRgb(r, g, b){
	let m = Math.max(r,g,b);
	if(m == 0) return 0;
	let f = Math.max(1, m);
	f /= 255;
	r = Math.round(r/f);
	g = Math.round(g/f);
	b = Math.round(b/f);
	if(r < 16) r = "0" + r.toString(16);
	else if(r == 0) r = "00";
	else r = r.toString(16)
	if(g < 16) g = "0" + g.toString(16);
	else if(g == 0) g = "00"
	else g = g.toString(16);
	if(b < 16) b = "0" + b.toString(16);
	else if (b == 0) b = "00";
	else b = b.toString(16);
	return r + g + b;
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