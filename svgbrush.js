function svgbrush(r, o, h, id, hexcolor){
	let ns = 'http://www.w3.org/2000/svg';
	let div = document.getElementById(id);
	if(div == null) return;
	let svg = document.createElementNS(ns, 'svg');
	$('#brushsvg').remove();
	let r2 = h/100*r-1;
	svg.setAttributeNS(null, 'width', 100);
	svg.setAttributeNS(null, 'height', 100);
	svg.setAttributeNS(null, 'id', 'brushsvg');
	div.appendChild(svg);
	svg.appendChild(getRect(50, 50, '#ababab', 0, 0));
	svg.appendChild(getRect(50, 50, '#ffffff', 50, 0));
	svg.appendChild(getRect(50, 50, '#ababab', 50, 50));
	svg.appendChild(getRect(50, 50, '#ffffff', 0, 50));
	svg.appendChild(getFrame(100, 100, '#000000', 0, 0));
	svg.appendChild(getDot(50, 50, hexcolor, o));
	for(let k = 0; k < r; k++){
		let o2=1.0;
		if(k >= r2) o2 = (r-k)/(r-r2);
		svg.appendChild(getCircleFill(50, 50, hexcolor, k, o2*o));
	}
	svg.appendChild(getCircle(50, 50, '#000000', r));
}

function getRect(w, h, col, x, y){
	var ns = 'http://www.w3.org/2000/svg';
	var rect = document.createElementNS(ns, 'rect');
	rect.setAttributeNS(null, 'width', w);
	rect.setAttributeNS(null, 'height', h);
	rect.setAttributeNS(null, 'fill', col);
	rect.setAttributeNS(null, 'x', x);
	rect.setAttributeNS(null, 'y', y);
	return rect;
}

function getFrame(w, h, col, x, y){
	var ns = 'http://www.w3.org/2000/svg';
	var rect = document.createElementNS(ns, 'rect');
	rect.setAttributeNS(null, 'width', w);
	rect.setAttributeNS(null, 'height', h);
	rect.setAttributeNS(null, 'stroke', col);
	rect.setAttributeNS(null, 'stroke-width', 1);
	rect.setAttributeNS(null, 'fill-opacity', 0);
	rect.setAttributeNS(null, 'x', x);
	rect.setAttributeNS(null, 'y', y);
	return rect;
}

function getCircle(cx, cy, col, r){
	var ns = 'http://www.w3.org/2000/svg';
	var circle = document.createElementNS(ns, 'circle');
	circle.setAttributeNS(null, 'cx', cx);
	circle.setAttributeNS(null, 'cy', cy);
	circle.setAttributeNS(null, 'r', r+0.5);
	circle.setAttributeNS(null, 'stroke', col);
	circle.setAttributeNS(null, 'stroke-width', 1);
	circle.setAttributeNS(null, 'fill-opacity', 0);
	return circle;
}

function getCircleFill(cx, cy, col, r, o){
	var ns = 'http://www.w3.org/2000/svg';
	var circle = document.createElementNS(ns, 'circle');
	circle.setAttributeNS(null, 'cx', cx);
	circle.setAttributeNS(null, 'cy', cy);
	circle.setAttributeNS(null, 'r', r);
	circle.setAttributeNS(null, 'stroke', col);
	circle.setAttributeNS(null, 'fill-opacity', 0);
	circle.setAttributeNS(null, 'stroke-opacity', o);
	circle.setAttributeNS(null, 'stroke-width', 1);
	return circle;
}

function getDot(cx, cy, col, o){
	var ns = 'http://www.w3.org/2000/svg';
	var circle = document.createElementNS(ns, 'circle');
	circle.setAttributeNS(null, 'cx', cx);
	circle.setAttributeNS(null, 'cy', cy);
	circle.setAttributeNS(null, 'r', .5);
	circle.setAttributeNS(null, 'stroke', col);
	circle.setAttributeNS(null, 'fill-opacity', o);
	return circle;
}