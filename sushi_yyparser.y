%{
#include "Sushi.hh"
  int yylex();
  void yyerror(const char* s);
%}

%union {
  bool b;
  int i;
  std::string *s;
  std::vector <std::string*> *s_vec;
  Program *p;
  Redirection redir;
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
%type<redir> in_redir out1_redir out2_redir inout_redir out_redir  any_redir
%type<s_vec> args
%type<b> bg_mode
%type<p> exe redir_exe in_exe out_exe pipe

%start cmdline

%%

cmdline: 
%empty /* an empty line is valid, too! Do nothing */ 
| redir_exe bg_mode    { my_shell.spawn($1, $2); }
| in_exe pipe bg_mode  { $1->set_pipe($2); my_shell.spawn($1, $3); }
| arg YY_SUSHI_SET arg // Not implemented yet
| YY_SUSHI_PWD         // Not implemented yet
| YY_SUSHI_CD arg      // Not implemented yet
| YY_SUSHI_HISTORY { my_shell.show_history(); }
| YY_SUSHI_BANG    { my_shell.re_parse($1); }
| YY_SUSHI_EXIT    { my_shell.set_exit_flag(); }

pipe: 
YY_SUSHI_BAR out_exe  { $$ = $2; }
| pipe YY_SUSHI_BAR out_exe { $3->set_pipe($1); $$ = $1; }

redir_exe: 
  exe { $$ = $1; }          
| exe any_redir { $1->set_redir($2); $$ = $1; }

in_exe:   
  exe { $$ = $1; }          
| exe in_redir { $1->set_redir($2); $$ = $1; }

out_exe:   
  exe { $$ = $1; }          
| exe out_redir { $1->set_redir($2); $$ = $1; }

inout_redir:    
  in_redir out_redir { $2.set_in($1); $$ = $2; }
| out_redir in_redir { $1.set_in($2); $$ = $1; } 

out_redir:
  out1_redir { $$ = $1; }
| out2_redir  { $$ = $1; }

any_redir:
  in_redir  { $$ = $1; }   
| out_redir  { $$ = $1; }  
| inout_redir  { $$ = $1; }

in_redir:   YY_SUSHI_LESS arg      { $$.set_in($2); }
out1_redir: YY_SUSHI_MORE arg      { $$.set_out1($2); }
out2_redir: YY_SUSHI_MOREMORE arg  { $$.set_out2($2); }

bg_mode: 
 %empty        { $$ = false; }
| YY_SUSHI_AMP { $$ = true; }

exe: 
  args { $$ = new Program($1); }

args:  
  arg { $$ = new std::vector<std::string*>(); $$->push_back($1); }
| args arg { $1->push_back($2); $$ = $1; }

arg: 
  YY_SUSHI_TOK { $$ = $1; }

%%

/* This section is empty */
