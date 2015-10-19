#include <stdio.h>
#include <stdlib.h>

#define TOKLEN 100
#define MAXTOK 100

//dest is where it's copied to. make sure allocated enough room in dest!
//start is the 0-based start index
//returns number of characters copied, -1 means error
//len < 0 means copy the whole string
//TODO moved to my str lib
int strlen(char* str) {
    int len=0;
    while(str[len]) len++;

    return len;
}

int comparestr(char* a, char* b) {
    int i = 0;
    while(a[i] != 0 && b[i] != 0 && a[i] == b[i]) {
        i++;
    }
    return (int)a[i] - (int)b[i];
}

int mystrncpy(char* source, char* dest, int start, int len) {
    if(start < 0) {
        return -1;
    }
    
    int index = 0;
    while((index < len || len < 0) && source[start + index] != NULL) {
        dest[index] = source[start+index];
        index ++;
    }

    dest[index] = '\0';

    return index;
}

/*
test mystrncpy
index < len    len < 0    source[s+i] != NULL    start=0, len=0, len > length, start < 0, len 

*/

int mystrcpy(char* source, char* dest) {
    return mystrncpy(source, dest, 0, -1);
}



struct token {
    char string[TOKLEN];
};

struct token toks[MAXTOK] = {};

int is_id_start_char(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

int is_num_char(char c) {
    return c >= '0' && c <= '9';
}

int is_id_char(char c) {
    return is_id_start_char(c) ||
           is_num_char(c);
}


//badly named token_index also the token list length
int token_index = 0;

void tokenize(char* input_str) {
    int index = 0;

    token_index = 0;
    
    //while not end of string
    while(input_str[index] != NULL) {
        //skip spaces (make sure no new lines, tabs, etc?)
        while(input_str[index] == ' ') ++index;
        
        if(input_str[index] == NULL) return;
        
        
        //if start of id
        if(is_id_start_char(input_str[index])) {
            //read that word
            int start = index;
            while(is_id_char(input_str[index])) ++index;
            
            //create new token
            mystrncpy(input_str, toks[token_index++].string,
                      start, index-start);
            
            continue;
        }
        //if starts with a num - for array indices
                                //don't want many single
                                //num char tokens in a row
        if(is_num_char(input_str[index])) {
            //read the rest of that num
            int start = index;
            while(is_num_char(input_str[index])) ++index;
            mystrncpy(input_str, toks[token_index++].string,
                      start, index-start);
            
            continue;
        }
        //else is a char like '('
          //just store that char as its own token
        
        mystrncpy(input_str, toks[token_index++].string,
                  index++, 1);
    }
}

int is_num_str(char* input_str) {
//is_num_char
    int is_a_num = 1;
    if(input_str == 0) is_a_num = 0;
    while(*input_str) {
        if(is_num_char(*input_str++)) {
            continue;
        }
        
        is_a_num = 0;
        break;
    }
    
    return is_a_num;
}

int is_id(char* input_str) {
//what about numbers?
  if(comparestr(input_str, "char")  == 0 ||
     comparestr(input_str, "const") == 0 ||
     comparestr(input_str, "int")   == 0 ||
     comparestr(input_str, "*") == 0 ||
     comparestr(input_str, "(") == 0 ||
     comparestr(input_str, ")") == 0 ||
     comparestr(input_str, "[") == 0 ||
     comparestr(input_str, "]") == 0 ||
     comparestr(input_str, ";") == 0 ||
     is_num_str(input_str)) {

      return 0;      
  }
  
  return 1;
}

enum state {
    START_FIND_ID,
    ARRAY,
    FUNCTION,
    TYPE,
    END
};

int s = START_FIND_ID;

int current_tok = 0;
int prev_tok;
int did_not_find_id = 1;

void find_id() {
    while(current_tok < token_index) {
        if(is_id(toks[current_tok].string)) {
            printf("\n%s is a ", toks[current_tok].string);
            did_not_find_id = 0;
            prev_tok = current_tok-1;
            current_tok++;
            //look for array, function. if none start going back
            
            s = ARRAY;
            return;
        }
        ++current_tok;
    }
    s = END;
}

void array() {
    //if [ array then read array and go to next state
    //if not array then check for func
    if(current_tok < token_index && 
       comparestr(toks[current_tok].string, "[") == 0)  {
        printf(" array ");
        current_tok++;
        //if a number say array of that many, go to next
        if(is_num_str(toks[current_tok].string)) {
            printf(" %s ", toks[current_tok++].string);
        }
        
        printf(" of ");
        
        //if ] go to next state
        if(comparestr(toks[current_tok].string, "]") == 0) {
            current_tok++;
            //if now '(' like start of function, error
            if(current_tok < token_index &&
               comparestr(toks[current_tok].string, "(") == 0) {
               printf("\nerror, can't have []()\n");
               s = END;
               return;         
            }
            s = ARRAY;//go to array again because can have [][]
        }
        else {            
            //if neither, then error
            printf("\nerror, expecting ']' or number after '['\n");
            s = END;
        }
    }
    else {
        //not array, try function
        //and maybe at end of tokens, see if that might be a bug
        s = FUNCTION;
    }
}

void func() {
    if(current_tok < token_index && 
       comparestr(toks[current_tok].string, "(") == 0) {
        current_tok++;
        printf(" function (");
        while(current_tok < token_index && 
              comparestr(toks[current_tok].string, ")") != 0) {
            printf(" %s ", toks[current_tok++].string);
            //read types in here?
        }
        //should be pointing at a ) or error end of tokens
        if(current_tok >= token_index) {
            printf(" error didn't find matching ')' for a function \n");
            s = END;
            return;
        }
        
        if(comparestr(toks[current_tok].string, ")") != 0) {
            printf(" error something wrong parsing function ");
            s = END;
            return;
        }
        
        printf(") returning ");
        current_tok++;
        
        //test
        s = TYPE;
    }
    else {
        //go to next thing to try
        s = TYPE;
    }
}

void type() {
    if(prev_tok < 0) {
        s = END;
        return;
    }
    
    if(comparestr(toks[prev_tok].string, "(") == 0) {
        prev_tok--;
        //going the other way again, current token better be a closing ')'
        //if current over total or current not a ')' then error
        if(current_tok >= token_index ||
           comparestr(toks[current_tok].string, ")") != 0) {
            printf("\nerror, expecting closing ')'\n");
            s = END;
            return;
        }
        
        //it is a ')', advance current token, go to array state
        current_tok++;
        s = ARRAY;
        return;
    }
    
    //consume this type, move to next prev, state still TYPE
    if(comparestr(toks[prev_tok].string, "const") == 0) {
        printf(" read-only ");
    }
    else if(comparestr(toks[prev_tok].string, "*") == 0) {
        printf(" pointer to ");
    }
    else {
        printf(" %s ", toks[prev_tok].string);
    }
    
    prev_tok--;
    
    //state still TYPE
}

//rule - caller/prev state should have tok indexes set before 
//       calling the next state, called state assumes
//       the indexes are set
void cdecl(char* input_str) {
    current_tok = 0;
    s = START_FIND_ID;
    did_not_find_id = 1;
    tokenize(input_str);
    //find id
    while(s != END) {
        switch(s) {
            case START_FIND_ID: {
                find_id();
            } break;
            case ARRAY: {
                array();
            } break;
            case FUNCTION: {
                func();                
            } break;
            case TYPE: {
                type();
            } break;
            default:
                printf("\nState not implemented\n");
                s = END;
                break;
        }
    }
    
    
    if(did_not_find_id) {
        printf("\nerror, id not found\n");
    }
}

void test(char* this) {
    printf("\n*****************\nTesting %s\n", this);
    tokenize(this);
    int print_tok_index;
    for(print_tok_index = 0;
         print_tok_index < token_index;
         ++print_tok_index) {
        printf("%s\n", toks[print_tok_index].string);
    }
    cdecl(this);
}

main() {
    printf("hello world\n");

    char *src = "test";
    char dest[10];
    mystrncpy(src, dest, 0, 4);
    printf("%s", dest);
    char d = '0';
    printf("\n%d\n", d);
    d = '9';
    printf("\n%d\n", d);

    test("int thing[]");
    test("char thing()");
    test("int (*next)()");
    test("char* const *(*next)(char more)");
}
