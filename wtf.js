function ready(fn) {
  if (document.readyState != 'loading'){
    fn();
  } else {
    document.addEventListener('DOMContentLoaded', fn);
  }
}

ready(function() {
//  alert("ready");
});

var play = function() {
//  alert("play");
  try{
  var input_type = document.getElementById('type_input');
  alert(input_type.value);
}
catch(e) {
alert(e.message);
}
};
