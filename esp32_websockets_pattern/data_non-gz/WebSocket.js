// global variables here

//server functions
var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
connection.onopen = function () {
    connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {  
    console.log('Server: ', e.data);
};
connection.onclose = function(){
    console.log('WebSocket connection closed');
};


// project specific functions as reactions to button presses and so on 
/* function xyz(){
}

function sendOn () {
  console.log('turning on');
  connection.send('on');
}

*/
