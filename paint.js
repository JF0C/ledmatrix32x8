$(document).ready(()=>{
	resizeGrid();
	$(window).resize(()=>resizeGrid());
	$('.paint-control').change(()=>{drawbrush();});
	for(let y = 0; y < 8; y++){
		pixels[y] = [];
		for(let x = 0; x < 32; x++){
			pixels[y][x] = new Pixel(x, y);
		}
	}
	drawbrush();
	$('#canvas').click(e=>brush(e.clientX-10, e.clientY-10));
	$('.checkbox').click(e=>{
		let t = $(e.target);
		if(t.html() == 'x'){
			t.html('');
			sendpaint('erase', 'false');
		}
		else{
			t.html('x');
			sendpaint('erase', 'true');
		}
	});

	$('#canvas').on('touchmove', e=>{
		e.preventDefault();
		let x = e.changedTouches[0].clientX-10;
		let y = e.changedTouches[0].clientY-10;
		checkDraw(x, y);
	});
	let down = false;
	$(document).mousedown(function(e){if(e.which === 1) down = true;});
    $(document).mouseup(function(e){if(e.which === 1) down = false;});
	$('#canvas').mousemove(e=>{
		if(!down) return;
		let x = e.clientX-10;
		let y = e.clientY-10;
		checkDraw(x, y);
	});
	checkSend();

	$('#menu-title').click(()=>{
		$('.option-container').css('display', 'block');
	});
	$('#abort').click(()=>{
		$('.option-container').css('display', 'none');
		$('#files-view').css('display', 'none');
		$('#files-view').html('');
	});
	$('#back').click(()=>{
		if(unsaved)
			sendpaint("leaving", "", e=>{location = "/index.html";});
		else
			location = "/index.html";
	});
	$('#clear-paint').click(()=>{
		if(confirm("sure?")){
			sendpaint("clear", "true");
			forAllPixels(p=>p.clear());
		}
	});
	$('#save-paint').click(()=>{
		let name = prompt("enter name");
		if(name == null || name == "") return;
		sendpaint("save", name, e=>unsaved=false);
	});
	$('#load-paint').click(()=>{
		$.ajax({
			"url": "listfiles",
			"type": "GET",
			"dataType": "json"
		}).done((e)=>{
			$('#files-view').html('');
			for(let n of e){
				if(!n.startsWith('/paints/')) continue;
				let dispn = n.replace('/paints/', '').replace('.paint', '');
				$('#files-view').append('<div data="' + n + '" class="open-file">' + dispn + '</div>');
			}
			$('#files-view').append('<div id="close-file-view">abort</div>');
			$('#close-file-view').click(e=>{
				$('#files-view').html('');
				$('#files-view').css('display', 'none');
			})
			$('#files-view').css('display', 'block');
			$('.open-file').click(e=>{
				loadpaint($(e.target).attr('data'));
				$('#files-view').html('');
				$('#files-view').css('display', 'none');
			});
		}).fail((e)=> console.log(e));
	});
	$('#red').change(e=>sendpaint('red', e.target.value));
	$('#blue').change(e=>sendpaint('blue', e.target.value));
	$('#green').change(e=>sendpaint('green', e.target.value));
	$('#hardness').change(e=>sendpaint('hard', e.target.value));
	$('#size').change(e=>sendpaint('size', e.target.value));
	init();
});

function init(){
	$.ajax({
		'url': 'config.json',
		'method': 'GET',
		'type': 'json'
	}).done(e=>{
		e = JSON.parse(e);
		if(e.paintr !== undefined) $('#red').val(e.paintr);
		if(e.paintg !== undefined) $('#green').val(e.paintg);
		if(e.paintb !== undefined) $('#blue').val(e.paintb);
		if(e.painthard !== undefined) $('#hardness').val(e.painthard);
		if(e.paintsize !== undefined) $('#size').val(e.paintsize);
		if(e.painterase !== undefined) {
			if(e.painterase == "1")
				$('#sub').html('x');
			else
				$('#sub').html('');
		}
		drawbrush();
	}).fail(e=>console.log(e));
	$.ajax({
		'url': 'paints/bu/temp.paint',
		'type': 'GET',
		'dataType': 'text'
	}).done(e=>paintFromString(e)).fail(e=>console.log(e));
}

let unsaved = true;

function loadpaint(file){
	sendpaint('load', file.replace("/paints/", "").replace(".paint", ""));
	$.ajax({
		"url": file,
		"type": "GET",
		"dataType": "text"
	}).done(e=>{paintFromString(e); unsaved=false;}).fail(e=>console.log(e));
}

function paintFromString(str, scaleup){
	forAllPixels(p => p.clear());
	let idx = 0;
	let c = 0;
	let rgb = [0, 0, 0];
	let list = str.substring(0, str.length-1).split(',');
	for(let v of list){
		if(scaleup !== undefined){
			rgb[c] = (parseFloat(v)/255)**0.3*255;
		}
		else
			rgb[c] = parseInt(v);
		if(c==2){
			paintPixel(idx, rgb);
			c = -1;
			idx++;
		}
		if(idx >= 256){
			break;
		}
		c++;
	}
}

function paintPixel(idx, rgb){
	let grb = [rgb[1], rgb[0], rgb[2]];
	y = parseInt(idx/32);
	x = idx%32;
	if(y%2 == 1)
		x = 31-x;
	pixels[y][x].paintrgb(grb, true);
}

let lastpos = [0,0];
function checkDraw(x, y){
	let d = Math.sqrt((x-lastpos[0])**2, (y-lastpos[1])**2);
	if(d/gridlen < parseFloat($('#size').val())/100) return;
	console.log("drawing");
	lastpos = [x, y];
	brush(x, y);
	unsaved = true;
}

let pixels = [];

function brush(bx, by){
	let r = parseFloat($('#size').val())/100*4;
	let r2 = parseFloat($('#hardness').val())/100*r-1;
	let subtract = $('#sub').html() == 'x';
	let rgb = getRgb();
	forAllPixels(p=>{
		let d = p.dist(bx, by) / gridlen;
		if(d > r) return;
		let f = hrelu(r, r2, d);
		if(subtract) p.erasergb(rgb);
		else p.paintrgb(rgb);
	});
}

function forAllPixels(fun){
	for(let y = 0; y < 8; y ++){
		for(let x = 0; x < 32; x++){
			fun(pixels[y][x]);
		}
	}
}

function hrelu(r1, r2, d){
	if(d < r2) return 1.0;
	return (r1-d)/(r1-r2);
}

function resizeGrid(){
	let l = $('body').width()/32 - 0.3;
	gridlen = l;
	$('.pixel').css({'width': ''+l+'px', 'height': ''+l+'px'});
}

let gridlen = 1.0;

function drawbrush(){
	$('#brushsvg').remove();
	let r = parseFloat($('#size').val())/100*50;
	let h = parseInt($('#hardness').val());
	let o = getOpacity();
	svgbrush(r, o, h, 'brush', '#' + getColor());
}

class Pixel{
	constructor(x, y){
		this.p = $("[name='"+y+","+x+"']");
		this.x = x;
		this.y = y;
		this.r = 0;
		this.g = 0;
		this.b = 0;
		this.modified = false;
	}
	center(){
		return [(this.x + 0.5) * this.p.width(), (this.y + 0.5) * this.p.height()];
	}
	dist(x, y){
		let pos = this.center();
		return Math.sqrt((x-pos[0])**2 + (y - pos[1])**2);
	}
	paintrgb(rgb, dontsend){
		this.paint(rgb[0], rgb[1], rgb[2], dontsend);
	}
	paint(r, g, b, dontsend){
		this.r += ((1-this.r/255) * r);
		this.g += ((1-this.g/255) * g);
		this.b += ((1-this.b/255) * b);
		
		if(dontsend===undefined)
			this.modified = true;
		this.setpaint();
	}
	erasergb(rgb, dontsend){
		this.erase(rgb[0], rgb[1], rgb[2], dontsend);
	}
	erase(r, g, b, dontsend){
		this.r -= (this.r/255) * r;
		this.g -= (this.g/255) * g;
		this.b -= (this.b/255) * b;
		if(dontsend === undefined)
			this.modified = true;
		this.setpaint();
	}
	clear(){
		this.erase(255, 255, 255, true);
	}
	setpaint(){
		let r = this.r;
		let g = this.g;
		let b = this.b;
		let m = Math.max(r, g, b);
		let o = (m/255)**0.2;
		this.p.css('background', 'rgb(' + r + "," + g + "," + b + ")");
		this.p.css('opacity', o);
	}
	payload(){
		this.modified = false;
		return {'r': Math.round(this.r), 'g': Math.round(this.g), 'b': Math.round(this.b), 'x': this.x, 'y': this.y};
	}
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

let sendx = 0;
function checkSend(){
	setTimeout(()=>{
		let data = [];
		for(let y = 0; y < 8; y++){
			if(pixels[y][sendx].modified){
				data.push(pixels[y][sendx].payload());
			}
		}
		if(data.length > 0){
			console.log(data);
			sendpaint('pixels', JSON.stringify(data));
		}
		sendx++;
		sendx%=32;
		checkSend();
	}, 100);
}

function sendpaint(key, val, after){
	//console.log("setting " + key + " to " + val);
	let data = {};
	data[key] = val;
	$.ajax({
		"url": "paint",
		"type": "POST",
		"dataType": "text",
		"data": data
	}).done((e)=>{console.log(e); if(after !== undefined) after(e);}).fail((e)=> console.log(e));
}

function getOpacity(){
	let r = parseInt($('#red').val());
	let g = parseInt($('#green').val());
	let b = parseInt($('#blue').val());
	return Math.max(r, g, b)/255.0;
}

