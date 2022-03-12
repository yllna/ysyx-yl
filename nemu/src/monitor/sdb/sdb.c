#include <isa.h>
#include <cpu/cpu.h>
#include <memory/vaddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
	static char *line_read = NULL;

	if (line_read)
	{
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read)
	{
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args)
{
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args)
{
	nemu_state.state = NEMU_QUIT;
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
	if (args == NULL)
	{
		cpu_exec(1);
		return 0;
	}
	char *steps = strtok(NULL, " ");
	if (steps == NULL)
	{
		cpu_exec(1);
		return 0;
	}
	else
	{
		sword_t n = 1;
		if (sscanf(steps, "%ld", &n) == 1 && n > 0)
		{
			cpu_exec(n);
		}
		else
		{
			printf("Bad number: \e[0;31m%s\e[0m\n", steps);
		}
	}
	return 0;
}

static int cmd_info(char *args)
{
	char *arg = strtok(NULL, " ");
	if (arg == NULL)
	{
		printf("undefined info args\n");
		return 0;
	}
	if (strcmp(arg, "r") == 0)
	{
		isa_reg_display();
	}
	else if (strcmp(arg, "w") == 0)
	{
		display_watch_point();
	}
	// else{
	// 	bool success = false;
	// 	int val = get_reg_val(arg, &success);
	// 	if(success){
	// 		printf("%s\t0x%08x\t%d\t%u\n", arg, val, val, val);
	// 	}
	// 	else{
	// 		printf("undefined info args\n");
	// 	}
	// }
	return 0;
}

static int switch_u(char u){
	switch (u)
	{
	case 'b':
		return 1;
	case 'h':
		return 2;
	case 'w':
		return 4;
	case 'g':
		return 8;
	default:
		return 4;
	}
}

static int cmd_x(char *args)
{
	if (args == NULL)
	{
		goto x_error1;
	}
	args += strspn(args, " ");
	int i = 0;
	while(args[i] != ' ')
		i++;

	word_t n = 0;
	char f = 'x', u = 'w';	
	char * e = args + i;
	if(*args == '/'){
		args += 1;
		sscanf(args, "%lu", &n);
		while(isdigit(*args))
			args++;
		f = *(args++);
		u = *args;
	}
	else{
		sscanf(args, "%lu", &n);
	}
	bool success = false;
	word_t addr = expr(e, &success);
	if(!success){
		goto x_error2;
	}
	else{
		int len = switch_u(u);
		for(int j = 0; j < n; j++){
			switch(f){
				case 'x':
					printf("0x%016lx: %016lx\n", addr, vaddr_read(addr, len));
					break;
				case 'd':
					printf("0x%016lx: %016ld\n", addr, vaddr_read(addr, len));
					break;
				case 'c':
					printf("0x%016lx: %c\n", addr, (char )vaddr_read(addr, len));
					break;
				default:
					printf("0x%016lx: %016lx\n", addr, vaddr_read(addr, len));
					break;
			}
			addr += len;	
		}
	}
	return 0;
x_error1:
	puts("x needs args");
x_error2:
	puts("x needs an address");
	return 0;
}

static int cmd_p(char *args)
{
	if (args == NULL)
	{
		goto p_error;
	}
	//if(args + strspn(args, " ") >= cmd_end) { goto p_error; }

	bool success;
	sword_t val = expr(args, &success);
	if (!success)
	{
		printf("invalid expression: '%s'\n", args);
	}
	else
	{
		printf("%ld\n", val);
	}
	return 0;

p_error:
	puts("Command format: \"p EXPR\"");
	return 0;
}

static int cmd_w(char *args)
{
	if (args == NULL)
	{
		goto w_error;
	}
	args += strspn(args, " ");
	int return_val = new_watch_point(args);
	if(return_val >= 0){
		printf("Set watch point-- NO.%d.\nexpression:%s\n", return_val, args);
		return 0;
	}
	puts("Expression bad.");
	return 0;
w_error:
	puts("w command needs an expression.");
	return 0;
}

static int cmd_d(char *args)
{
	if (args == NULL)
	{
		goto d_error;
	}
	args += strspn(args, " ");
	bool success = false;
	word_t val = expr(args, &success);
	if(!success){
		puts("d Expression bad.");
		return 0;
	}
	success = delete_watch_point(val);
	if(!success){
		puts("no this watch point.");
		return 0;
	}
	printf("Delete watch point no.%ld", val);
	return 0;
d_error:
	puts("d command needs an argument.");
	return 0;
}

static struct
{
	const char *name;
	const char *description;
	int (*handler)(char *);
} cmd_table[] = {
	{"help", "Display informations about all supported commands", cmd_help},
	{"c", "Continue the execution of the program", cmd_c},
	{"q", "Exit NEMU", cmd_q},
	{"si", "Execute n commands then pause", cmd_si},
	{"info", "Display some imformation", cmd_info},
	{"x", "Evaluate expression to value and display n bytes from the address which qual to the value ", cmd_x},
	{"p", "Evaluate expression to value", cmd_p},
	{"w", "Set a watch point, when this expression's value changed, pause  nemu", cmd_w},
	{"d", "delete all the watch points", cmd_d},
	/* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if (arg == NULL)
	{
		/* no argument given */
		for (i = 0; i < NR_CMD; i++)
		{
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else
	{
		for (i = 0; i < NR_CMD; i++)
		{
			if (strcmp(arg, cmd_table[i].name) == 0)
			{
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void sdb_set_batch_mode()
{
	is_batch_mode = true;
}

void sdb_mainloop()
{
	if (is_batch_mode)
	{
		cmd_c(NULL);
		return;
	}

	for (char *str; (str = rl_gets()) != NULL;)
	{
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if (cmd == NULL)
		{
			continue;
		}

		/* treat the remaining string as the arguments,
     * which may need further parsing
     */
		char *args = cmd + strlen(cmd) + 1;
		if (args >= str_end)
		{
			args = NULL;
		}

#ifdef CONFIG_DEVICE
		extern void sdl_clear_event_queue();
		sdl_clear_event_queue();
#endif

		int i;
		for (i = 0; i < NR_CMD; i++)
		{
			if (strcmp(cmd, cmd_table[i].name) == 0)
			{
				if (cmd_table[i].handler(args) < 0)
				{
					return;
				}
				break;
			}
		}

		if (i == NR_CMD)
		{
			printf("Unknown command '%s'\n", cmd);
		}
	}
}

void init_sdb()
{
	/* Compile the regular expressions. */
	init_regex();

	/* Initialize the watchpoint pool. */
	init_wp_pool();
}
