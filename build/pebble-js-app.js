Pebble.addEventListener('ready', function(e) {
	//initialise here
});

Pebble.addEventListener('showConfiguration', function(e) {
	Pebble.openURL('https://rawgit.com/danVnest/Flux/master/config/index.html');
	// TODO: for production use https://cdn.rawgit.com/danVnest/Flux/[ current hash ]/config/index.html
});

Pebble.addEventListener("webviewclosed", function(e){
	var settings = JSON.parse(decodeURIComponent(e.response));
	Pebble.sendAppMessage(settings);
});
