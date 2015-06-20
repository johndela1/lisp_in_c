#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

typedef struct {
	int type;
	long num;
	char *err;
	char *sym;
	int count;
	struct lval **cell;
} lval;

enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};
enum {ERR_DIV_ZERO, ERR_UNKNOWN, ERR_TOO_BIG};

#ifdef _WIN32


char *readline(char *prompt)
{
	char *buf;
	fputs(prompt, stdout);
	buf = malloc(4096);
	if (buf == NULL)
		exit(1);
	fgets(buf, 4096, stdin);
	returr buf;
}

void add_history(char *_) {
	return;
}
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif


lval *eval(mpc_ast_t* t);

int number_of_nodes(mpc_ast_t* t) {
	int i;
	int num = 1;
	for (i = 0; i < t->children_num; i++)
		num += number_of_nodes(t->children[i]);
	return num;
}

lval *lval_num(long x) {
	lval *v;
	v = malloc(sizeof(*v));
	if (v == NULL)
		exit(-1);
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}
void* strdup(char *s)
{
	char *ret = malloc(strlen(s) + 1);
	if (ret)
		strcpy(ret, s);
	return ret;
}

lval *lval_err(char *type) {
	lval *v;
	v = malloc(sizeof(*v));
	if (v == NULL)
		exit(-1);
	v->type = LVAL_ERR;
	v->err = strdup(type);
	return v;
}

lval *lval_sym(char *s)
{
	lval *v = malloc(sizeof(*v));
	if (v == NULL)
		exit(-1);
	v->type = LVAL_SYM;
	v->sym = strdup(s);
	return v;
}

lval *lval_sexpr(void)
{
	lval *v = malloc(sizeof(*v));
	if (v == NULL)
		exit(-1);
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval *apply_op(char *op, long x, long y) {
	//printf("apply operator: -%s- %ld, %ld\n", op, x, y);
	if (!strcmp(op, "+"))
		return lval_num(x + y);
	if (!strcmp(op, "-"))
		return lval_num(x - y);
	if (!strcmp(op, "*"))
		return lval_num(x * y);
	if (!strcmp(op, "/")) {
		return y == 0
			? lval_err(ERR_DIV_ZERO)
			: lval_num(x / y);
	}
	return lval_err("unknown op");
}

lval *eval(mpc_ast_t* t) {
	int i;
	lval *x;
	char *op;
printf("tag: %s, cont: %s\n", t->tag, t->contents);
printf("child tag: %s, cont: %s\n", t->children[2]->tag, t->children[2]->contents);
	if (strstr(t->tag, "number"))
		return lval_num(strtol(t->contents, NULL, 10));

	op = t->children[1]->contents;
	printf("op %s \n", op);
	x = eval(t->children[2]);
	for (i = 3; strstr(t->children[i]->tag, "expr"); i++)
		x = apply_op(op, x->num, eval(t->children[i])->num);
	return x;
}

void lval_print(lval *l)
{
	fprintf(stderr, l->err);
}

int main(int argc, char** argv)
{
	char *input;

	puts("Welcome to jlisp ver: 0.0.1");
	puts("^c to exit\n");

	mpc_parser_t *Number = mpc_new("number");
	mpc_parser_t *Symbol = mpc_new("symbol");
	mpc_parser_t *Sexpr = mpc_new("sexpr");
	mpc_parser_t *Expr = mpc_new("expr");
	mpc_parser_t *Jlisp = mpc_new("jlisp");
	mpc_result_t r;

	lval *l;

	/* Define them with the following Language */
	mpca_lang(MPCA_LANG_DEFAULT,
	  "                                                    \
	    number   : /-?[0-9]+/ ;                            \
	    symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;      \
	    sexpr    : '(' <expr>* ')' ;  \
	    expr     : <number> | <symbol> | <sexpr> ;  \
	    jlisp    : /^/ <expr>* /$/ ;             \
	  ",
	  Number, Symbol, Sexpr, Expr, Jlisp);


	while(1) {
		input = "(/ 10 0)";
		//input = readline("jlisp> ");
		add_history(input);
		if (mpc_parse("<stdin>", input, Jlisp, &r)) {
			mpc_ast_print(r.output);
			//printf("node count: %d\n", number_of_nodes(r.output));
			l = eval(r.output);
			if (l->type == LVAL_ERR) {
				lval_print(l);
			} else {
				printf("eval: %ld\n", l->num);
			}
			mpc_ast_delete(r.output);
		} else{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		//break;
	}
	mpc_cleanup(4, Number, Symbol, Sexpr, Expr, Jlisp);
	return 0;
}
