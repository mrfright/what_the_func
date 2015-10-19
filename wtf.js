function ready(fn) {
  if (document.readyState != 'loading'){
    fn();
  } else {
    document.addEventListener('DOMContentLoaded', fn);
  }
}

ready(function() {
  wtf("int x");
  wtf("int thing[]");
  wtf("char thing()");
  wtf("int (*next)()");
  wtf("char* const *(*next)(char more)");
});

var State = {
  START_FIND_ID : "start_find_id",
  ARRAY         : "array",
  FUNCTION      : "function",
  TYPE          : "type",
  END           : "end",
}

var current_tok;
var prev_tok;
var wtf_state;
var did_not_find_id = true;
var full;

function isNumeric(num){
    return !isNaN(num)
}

function is_id_start_char(c) {
    var reg = /^[a-zA-Z_]$/;
    var ok = reg.exec(c);
    if(ok) {
        return true;
    }
    else {
        return false;
    }
}

function is_num_char(c) {
    var reg = /^[0-9]$/;
    var ok = reg.exec(c);
    if(ok) {
        return true;
    }
    else {
        return false;
    }
}

function is_id_char(c) {
    return is_id_start_char(c) || is_num_char(c);
}

var toks = [];

function tokenize(input_str) {
    var index = 0;
    toks = [];
    while(index < input_str.length) {
        while(input_str[index] == " ") ++index;
        
        if(index >= input_str.length) return;
        
        if(is_id_start_char(input_str[index])) {
            var start = index;
            while(index < input_str.length && 
                  is_id_char(input_str[index])) ++index;
            
            toks.push(input_str.substring(start, index));
            
            continue;
        }
        
        if(is_num_char(input_str[index])) {
            var start = index;
            while(is_num_char(input_str[index])) ++index;
            
            toks.push(input_str.substring(start, index));
            
            continue;
        }
        
        toks.push(input_str[index]);
        
        ++index;
    }
    
    return toks;
};

function is_id(str) {
  return isNaN(str)      &&
         str !== "char"  &&
         str !== "const" &&
         str !== "int"   &&
         str !== "*"     &&
         str !== "("     &&
         str !== ")"     &&
         str !== "["     &&
         str !== "]"     &&
         str !== ";";
  
  //TODO javascript probably has something like "not in this list" this can use
};

function find_id(){
  while(current_tok < toks.length) {
    if(is_id(toks[current_tok])) {
      full = toks[current_tok] + " is a ";
      did_not_find_id = false;
      prev_tok = current_tok-1;
      current_tok++;
      wtf_state = State.ARRAY;
      return;
    }
    
    current_tok++;
  }
  wtf_state = State.END;
};

function is_num_str(input_str) {
    var is_a_num = true;
    var index = 0;
    if(input_str == "") is_a_num = false;
    while(index < input_str.length) {
        if(is_num_char(input_str[index])) {
            index++;
            continue;
        }
        
        is_a_num = false;
        break;
    }
    
    return is_a_num;
};

var array  = function() {
    if(current_tok < toks.length && toks[current_tok] == "[") {
        full += " array ";
        current_tok++;
        
        if(is_num_str(toks[current_tok])) {
            full += toks[current_tok];
            current_tok++;
        }
        
        full += " of ";
        
        if(toks[current_tok] == "]") {
            current_tok++;
            if(current_tok < toks.length &&
               toks[current_tok] == "(") {
                full += "\nerror, can't have []()";
                wtf_state = State.END;
                return;
            }
            wtf_state = State.ARRAY;
        }
        else {
            full += " error, expecting ']' or number after '['";
            wtf_state = State.END;
            return;
        }                
    }  
    else {//may also be at end of functions, see if that's a bug
        wtf_state = State.FUNCTION;
    }
};

function func() {
    if(current_tok < toks.length && toks[current_tok] == "(") {
        current_tok++;
        full += " function (";
        while(current_tok < toks.length &&
              toks[current_tok] !== ")") {
            full += " "+toks[current_tok]+" ";
            current_tok++;
        }
        
        if(current_tok >= toks.length) {
            full += " error, didn't find matching ')' for a function before end of string";
            wtf_state = State.END;
            return;
        }
        
        if(toks[current_tok] !== ")") {
            full += "error, found '" + toks[current_tok] + "' when expecting a ')' for a function";
            wtf_state = State.END;
            return;
        }
        
        full += ") returning ";
        current_tok++;
        wtf_state = State.TYPE;
    }
    else {
        wtf_state = State.TYPE;
    }
}

function type() {
    if(prev_tok < 0) {
        wtf_state = State.END;
        return;
    }
    
    if(toks[prev_tok] == "(") {
        prev_tok--;
        
        if(current_tok >= toks.length || toks[current_tok] !== ")") {
            full += " error, expecting closing ')'";
            wtf_state = State.END;
            return;
        }
        
        current_tok++;
        wtf_state = State.ARRAY;
        return;
    }
    
    if(toks[prev_tok] == "const") {
        full += " read-only ";
    }
    else if(toks[prev_tok] == "*") {
        full += " pointer to ";
    }
    else {
        full += " " + toks[prev_tok] + " ";
    }
    
    prev_tok--;
}

function wtf(text) {
  tokenize(text);
  current_tok = 0;
  wtf_state = State.START_FIND_ID;
  did_not_find_id = true;
  while(wtf_state != State.END) {
    switch(wtf_state) {
      case State.START_FIND_ID: 
        find_id();
        break;
      case State.ARRAY:
        array();
        break;
      case State.FUNCTION:
        func();
        break;
      case State.TYPE:
        type();
        break;
      default:
        alert("state not implemented");
        break;
    }
  }

  if(did_not_find_id) {
    alert("error, id not found")
  }
  else {
    alert(full);
  }
};

var play = function() {
  try{
    var input_type = document.getElementById('type_input');
    wtf(input_type.value);    
  }
  catch(e) {
    alert(e.message);
  }
};

