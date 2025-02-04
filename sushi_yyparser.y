%{
#include "Sushi.hh"
  int yylex();
  void yyerror(const char* s);
%}

%union {
  std::string *s;
  int i;
}

%token YY_SUSHI_AMP
%token<i> YY_SUSHI_BANG
%token YY_SUSHI_BAR
%token YY_SUSHI_CD
%token YY_SUSHI_EXIT
%token YY_SUSHI_HISTORY
%token YY_SUSHI_JOBS
%token YY_SUSHI_LESS
%token YY_SUSHI_MORE
%token YY_SUSHI_MOREMORE
%token YY_SUSHI_PWD
%token YY_SUSHI_SET
%token YY_SUSHI_UNKNOWN
%token<s> YY_SUSHI_TOK

%type<s> arg
%type<i> bg_mode

%start cmdline

%%

cmdline: 
  %empty /* an empty line is valid, too! Do nothing */ 
| redir_exe bg_mode    
| in_exe pipe bg_mode  
| arg YY_SUSHI_SET arg 
| YY_SUSHI_PWD         
| YY_SUSHI_CD arg      
| YY_SUSHI_HISTORY { my_shell.show_history(); }
| YY_SUSHI_BANG    { my_shell.re_parse($1); }
| YY_SUSHI_EXIT    { my_shell.set_exit_flag(); }

pipe: 
  YY_SUSHI_BAR out_exe  
| pipe YY_SUSHI_BAR out_exe 

redir_exe: 
  exe          
| exe any_redir 

in_exe:   
  exe          
| exe in_redir  

out_exe:   
  exe           
| exe out_redir 

inout_redir:    
  in_redir out_redir 
| out_redir in_redir 

out_redir:
  out1_redir 
| out2_redir 

any_redir:
  in_redir    
| out_redir   
| inout_redir 

in_redir:   YY_SUSHI_LESS arg     
out1_redir: YY_SUSHI_MORE arg     
out2_redir: YY_SUSHI_MOREMORE arg 

bg_mode: 
 %empty        { $$ = 0; }
| YY_SUSHI_AMP { $$ = 1; }

exe: 
  args 

args:  
  arg      
| args arg 

arg: 
  YY_SUSHI_TOK { $$ = $1; }

%%

/* This section is empty */
