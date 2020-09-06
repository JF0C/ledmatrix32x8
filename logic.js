$(document).ready(()=>{
	$('#send-text').click(()=>sendtext());
	$('#text-input').keyup((e)=>{if(e.key == "Enter") sendtext();});
	$('#brightness-slider').change((e)=>sendset("bright", parseFloat(e.target.value)/100));
	$('#velocity-slider').change((e)=>sendset("velocity", parseFloat(e.target.value)/10));
	
	$('.paint-control').change(()=>{drawbrush();});
	$('#sel-paint').click(e=>showFiles());
	$('#set-bg-col').click(e=>setBgCol());
	$('#bg-off').click(e=>{sendset('background', '<off>')});
	$('#bgbright').change(e=>sendset('background', 'bright:' + (parseFloat(e.target.value)/100)));
	drawbrush();
	init();
});

let lasttexts = [];
function sendset(key, val){
	console.log("setting " + key + " to " + val);
	let data = {};
	data[key] = val;
	$.ajax({
		"url": "set",
		"type": "POST",
		"dataType": "text",
		"data": data
	}).done((e)=> console.log(e)).fail((e)=> console.log(e));
}

function setBgCol(){
	let r = parseInt($('#red').val());
	let g = parseInt($('#green').val());
	let b = parseInt($('#blue').val());
	sendset('background', 'color:' + r + ',' + g + ',' + b);
}

function showFiles(e){
	if($('#file-list').html() != ""){
		$('#file-list').html('');
		return;
	}
	$.ajax({
		'url': 'listfiles',
		'method': 'GET',
		'type': 'json'
	}).done(f=>{
		for(let n of f){
			if(!n.startsWith('/paints/')) continue;
			let dispn = n.replace('/paints/', '').replace('.paint', '');
			$('#file-list').append('<div class="open-file" data="' + n + '">' + dispn + '</div>');
		}
		$('.open-file').click(e=>{
			$('#file-list').html('');
			sendset('background', $(e.target).html());
		});
	}).fail(f=>console.log(f));
}

function sendtext(){
	let text = $('#text-input').val();
	text = text.replace('ä', '<ae>').replace('ö', '<oe>').replace('ü', '<ue>');
	text = text.replace('Ä', '<AE>').replace('Ö', '<OE>').replace('Ü', '<UE>');
	if(lasttexts === null || lasttexts === undefined) lasttexts = [];
	let exists = false;
	for(let k of lasttexts)
		if(k == text)
			exists = true;
	if(!exists && text != ""){
		lasttexts.unshift(text);
		while(lasttexts.length > 30) lasttexts.pop();
		sendset("lasttexts", JSON.stringify(lasttexts));
	}
	sendset("text", text);
	loadLastWords();
}

function init(){
	$.ajax({
		"url": "config.json",
		"type": "GET",
		"dataType": "json"
	}).done(e=>{
		$('#text-input').val(e.text);
		$('#brightness-slider').val(parseFloat(e.bright)*100);
		$('#velocity-slider').val(parseFloat(e.velocity)*10);
		lasttexts = JSON.parse(e.lasttexts);
		$('#red').val(parseFloat(e.bgr));
		$('#green').val(parseFloat(e.bgg));
		$('#blue').val(parseFloat(e.bgb));
		$('#bgbright').val(parseFloat(e.bgbright)*100);
		drawbrush();
		loadLastWords();
	}).fail(e=>console.log(e));
}

function loadLastWords(){
	$('#last-words').html('');
	for(let k in lasttexts){
		if(k > 20) break;
		let t = lasttexts[k].split('<').join('<span><</span>');
		$('#last-words').append("<div class='sel' id=\"last" + k + "\">" + t + "</div>");
		$('#last' + k).click(()=>{
			$('#text-input').val(t.split('<span><</span>').join('<'));
			sendtext();
		});
	}
}

function getBgCol(){
	let r = parseInt($('#red').val());
	let g = parseInt($('#green').val());
	let b = parseInt($('#blue').val());
	if(r == 0) r = "00";
	else if(r < 16) r = "0" + r.toString(16);
	else r = r.toString(16);
	if(g == 0) g = "00";
	else if(g < 16) g = "0" + g.toString(16);
	else g = g.toString(16);
	if(b == 0) b = "00";
	else if(b < 16) b = "0" + b.toString(16);
	else b = b.toString(16);
	return '#' + r + g + b;
}

function drawbrush(){
	svgbrush(50, 0.5, 100, 'brush', getBgCol());
}