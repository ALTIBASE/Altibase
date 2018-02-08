%pure_parser
%{
%}
%union { 
   int dummy;
}
%token FINAL

%%
root:
  FINAL
;  
%%
