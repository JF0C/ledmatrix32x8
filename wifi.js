$(document).ready(()=>{
	$('#send').click(()=>{
		$.ajax({
			'url': 'set',
			'method': 'POST',
			'type': 'text',
			'data': {'ssid': $('#ssid').val()}
		}).done(e=>{
			console.log(e);
			$.ajax({
				'url': 'set',
				'method': 'POST',
				'type': 'text',
				'data': {'pw': $('#pw').val()}
			}).done(e=>console.log(e)).fail(e=>console.log(e));
		}).fail(e=>console.log(e));
	});
});