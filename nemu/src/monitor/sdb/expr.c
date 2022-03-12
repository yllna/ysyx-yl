#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum
{
	NOTYPE = 256,
	EQ,

	/* TODO: Add more token types */

	NEQ,
	AND,
	OR,
	NOT,

	DECNUM,
	HEXNUM,
	REG,
	SYMB,
	NEG,
	DEREF
};

static struct rule
{
	const char *regex;
	sword_t token_type;
} rules[] = {

	/* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

	{" +", NOTYPE}, // spaces
	{"\\+", '+'},	// plus
	{"==", EQ},		// equal
	{"0[xX][0-9a-fA-f]+", HEXNUM},
	{"[0-9]{1,10}", DECNUM},
	{"-", '-'},
	{"\\*", '*'},
	{"/", '/'},
	{"\\(", '('},
	{"\\)", ')'},
	{"\\$e(ax|cx|dx|bx|sp|bp|si|di|ip)", REG},
	{"[a-zA-Z_][a-zA-z0-9_]+", SYMB},
	{"!=", NEQ},
	{"&&", AND},
	{"\\|\\|", OR},
	{"!", NOT},
};

bool isoperand(sword_t type)
{
	return type == DECNUM || type == HEXNUM || type == REG || type == SYMB;
}

bool isoperator(sword_t type)
{
	return type == '+' || type == '-' || type == '*' || type == '/' || type == '(' || type == ')' || type == NEG || type == DEREF || type == EQ || type == NEQ || type == AND || type == OR || type == NOT;
}

sword_t priority_of_operator(sword_t operator)
{
	switch (operator)
	{
	case NEG:
		return 2;
	case DEREF:
		return 2;
	case NOT:
		return 2;
	case '*':
		return 4;
	case '/':
		return 4;
	case '+':
		return 5;
	case '-':
		return 5;
	case EQ:
		return 8;
	case NEQ:
		return 8;
	case AND:
		return 12;
	case OR:
		return 13;
	default:
		return 14;
	}
	return 0;
}

bool priority_cmp(sword_t operator1, sword_t operator2)
{
	return priority_of_operator(operator2) >= priority_of_operator(operator1);
}
#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	sword_t i;
	char error_msg[128];
	sword_t ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token
{
	sword_t type;
	char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static sword_t nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
	sword_t position = 0;
	sword_t i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				sword_t substr_len = pmatch.rm_eo;

				// Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
				//     i, rules[i].regex, position, substr_len, substr_len, substr_start);

				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

				if (rules[i].token_type == NOTYPE)
					break;
				assert(substr_len < 31);
				strncpy(tokens[nr_token].str, substr_start, substr_len);
				tokens[nr_token].str[substr_len] = '\0';
				switch (rules[i].token_type)
				{
				case '*':
					if (nr_token == 0)
					{
						tokens[nr_token].type = DEREF;
					}
					else if (tokens[nr_token - 1].type != DECNUM &&
							 tokens[nr_token - 1].type != HEXNUM &&
							 tokens[nr_token - 1].type != REG &&
							 tokens[nr_token - 1].type != SYMB &&
							 tokens[nr_token - 1].type != ')')
						tokens[nr_token].type = DEREF;
					else
						tokens[nr_token].type = '*';
					nr_token++;
					break;
				case '-':
					if (nr_token == 0)
						tokens[nr_token].type = NEG;
					else if (tokens[nr_token - 1].type != DECNUM &&
							 tokens[nr_token - 1].type != HEXNUM &&
							 tokens[nr_token - 1].type != REG &&
							 tokens[nr_token - 1].type != SYMB &&
							 tokens[nr_token - 1].type != ')')
						tokens[nr_token].type = NEG;
					else
						tokens[nr_token].type = '-';
					nr_token++;
					break;

				default:
					tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %ld\n%s\n%*.s^\n", position, e, (int)position, "");
			return false;
		}
	}

	return true;
}

bool check_parentheses(uint32_t p, uint32_t q)
{
	if(tokens[p].type == '(' && tokens[q].type == ')'){
		int count_of_parentheses = 0;
		for (int i = p + 1; i <= q - 1 && count_of_parentheses >= 0; i++)
		{

			if (tokens[i].type == '(')
				count_of_parentheses += 1;
			else if (tokens[i].type == ')')
				count_of_parentheses -= 1;
		}
		if(count_of_parentheses == 0){
			return true;
		}
	}
	return false;
}

static bool eval_func_success;

word_t eval(word_t p, word_t q)
{
	if (!eval_func_success)
	{
		return 1;
	}
	if (p > q)
	{
		printf("Bad expression, you are sb.\n");
		fflush(stdout);
		eval_func_success = false;
		return 0;
	}
	else if (p == q)
	{
		word_t val = 0;
		bool success = 0;
		switch (tokens[p].type)
		{
		case DECNUM:
			if (sscanf(tokens[p].str, "%ld", &val) == 1)
			{

				return val;
			}
			else
			{
				printf("decimals input implemented uncorrectly.\n");
				fflush(stdout);
				eval_func_success = false;
				return 1;
			}
		case HEXNUM:
			if (sscanf(tokens[p].str, "%lx", &val) == 1)
				return val;
			else
			{
				printf("hexadecimal input implemented  uncorrectly.\n");
				fflush(stdout);
				eval_func_success = false;
				return 1;
			}
		case REG:
			val = isa_reg_str2val(tokens[p].str + 1, &success);
			if (success)
				return val;
			printf("registers input implemented uncorrectly.\n");
			fflush(stdout);
			eval_func_success = false;
			return false;
		// case SYMB:
		// 	val = look_up_symtab(tokens[p].str, &success);
		// 	if (success)
		// 		return val;
		// 	printf("symble input implemented uncorrectly.\n");
		// 	fflush(stdout);
		// 	eval_func_success = false;
		// 	return 1;
		default:
			printf("your problem, you are sb.\n");
			fflush(stdout);
			eval_func_success = false;
			return 1;
		}
	}
	else if (check_parentheses(p, q))
	{
		sword_t i = 0;
		sword_t count_of_parentheses = 0;
		for (i = p; i <= q && count_of_parentheses >= 0; i++)
		{

			if (tokens[i].type == '(')
				count_of_parentheses += 1;
			else if (tokens[i].type == ')')
				count_of_parentheses -= 1;
		}
		if (count_of_parentheses == 0)
			return eval(p + 1, q - 1);
		printf("parentheses input implemented uncorrectly1.\n");
		fflush(stdout);
		eval_func_success = false;
		return 1;
	}
	else
	{
		sword_t op = -1;
		sword_t count_of_parentheses = 0;
		for (sword_t i = p; i <= q; i++)
		{
			if (isoperator(tokens[i].type))
			{
				if (tokens[i].type == '(')
				{
					count_of_parentheses += 1;
				}
				else if (tokens[i].type == ')')
				{
					count_of_parentheses -= 1;
				}
				else if (op == -1 && count_of_parentheses == 0)
				{
					op = i;
				}
				else if (count_of_parentheses == 0 && priority_cmp(tokens[op].type, tokens[i].type))
				{
					op = i;
				}
			}
			//	else{
			//		printf("type = %d\n", tokens[i].type);
			//		fflush(stdout);
			//		}
		}
		if (count_of_parentheses < 0)
		{
			printf("parentheses input implemented uncorrectly2.\n");
			fflush(stdout);
			eval_func_success = false;
			return 1;
		}
		sword_t val1 = 0, val2 = 0;
		//	printf("op == %d, tokens[op].type = %d \n", op, tokens[op].type);
		fflush(stdout);
		switch (tokens[op].type)
		{
		case '+':
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			return val1 + val2;
		case '-':
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			return val1 - val2;
		case '*':
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			return val1 * val2;
		case '/':
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			if(val2 == 0){
				printf("that's impossible to divide a zero.\n");
				fflush(stdout);
				eval_func_success = false;
				return 0;
			}
			return val1 / val2;
		case EQ:
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			return val1 == val2;
		case NEQ:
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			return val1 != val2;
		case AND:
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			return val1 && val2;
		case OR:
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			return val1 || val2;
		case NOT:
			val1 = eval(p, op - 1);
			val2 = eval(op + 1, q);
			return !val2;
		case NEG:
			val2 = eval(op + 1, q);
			return 0 - val2;
		// case DEREF:
		// 	val2 = eval(op + 1, q);
		// 	//	printf("val2 = %x\n", val2);
		// 	val2 = vaddr_read(val2, SREG_CS, 4);
		// 	//	printf("value2 = %x\n", val2);
		// 	return val2;
		default:
			printf("many code implemented uncorrectly3.\n");
			fflush(stdout);
			eval_func_success = false;
			return 1;
		}
	}
	return 0;
}

word_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}

	eval_func_success = 1;
	uint32_t val = eval(0, nr_token - 1);
	if (eval_func_success)
	{
		*success = true;
		return val;
	}
	*success = false;
	//   /* TODO: Insert codes to evaluate the expression. */
	//   TODO();

	return 0;
}
