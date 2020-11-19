/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>

#define MEMORY_SIZE 30000
#define MAX_PROGRAM_SIZE 4096
#define STACK_SIZE 512

#define STACK_PUSH(x)   (stack[sp++] = x)
#define STACK_POP()     (sp--)
#define STACK_EMPTY()   (sp == 0)
#define STACK_FULL()    (sp == STACK_SIZE)
#define STACK_PEEK()	(stack[sp-1])

void die(const char *msg) { perror(msg); exit(EXIT_FAILURE); }

void usage (char *argv[]) { printf("usage: %s script\n", argv[0]); exit(EXIT_SUCCESS); }

uint8_t memory[MEMORY_SIZE] = {0};
uint8_t program[MAX_PROGRAM_SIZE] = {0};
uint32_t stack[STACK_SIZE] = {0};
uint32_t sp = 0; // stack pointer

char *bf_file;
int bf_fd;

void
load(void)
{
	size_t i = 0;
	char c;
	while ((read(bf_fd, &c, 1)) > 0)
	{
		switch (c)
		{
			case '>':
			case '<':
			case '+':
			case '-':
			case '.':
			case ',':
			case '[':
			case ']':
				program[i] = c;
				i++;
				break;

			default: break;
		}
	}
}

ssize_t
find_closing_bracket(size_t pos)
{
	/*
	 * find closing ] for opening [ at location pos in program, return its position
	 * -1 on not found
	 *
	 *  count each opening [ and closing ] until the numbers match, that will be our closing bracket
	 */
	ssize_t close_pos = -1;
	size_t open_count = 0;
	size_t close_count = 0;

	if (pos > MAX_PROGRAM_SIZE)
	{
		return -1;
	}
	if (program[pos] != '[')
	{
		fprintf(stderr, "instruction not [ at %zu\n", pos);
		exit(EXIT_FAILURE);
	}

	uint8_t c;
	while ((c = program[pos]) != 0)
	{
		switch (c)
		{
			case '[':

				open_count++;
				break;
			case ']':
				close_count++;
				break;
			default: break;
		}
		if (open_count == close_count)
		{
			close_pos = pos;
			break;
		}
		pos++;
		if (pos > MAX_PROGRAM_SIZE)
			break;
	}

	assert(close_pos != -1);
	return close_pos;
}

void
run(void)
{
	int pc = 0; // program counter
	size_t datap = 0; // data pointer into memory
	uint8_t i; // current instruction
	while ((i = program[pc]) != 0)
	{
		/*
		size_t in;
		int x = 0;
		printf("PC = %d\n", pc);
		while ((in = program[x]) != 0)
		{
			if (x == pc)
				putchar('A');
			putchar(in);
			if (x == pc)
				putchar('A');
			x++;
		}
		getchar();
		*/
		switch (i)
		{
		case '>':
			datap++;
			pc++;
			break;
		case '<':
			datap--;
			pc++;
			break;
		case '+':
			memory[datap]++;
			pc++;
			break;
		case '-':memory[datap]--;
			pc++;
			break;

		case '.':
			putchar(memory[datap]);
			fflush(stdout);
			pc++;
			break;
		case ',':
			memory[datap] = (char)getchar();
			pc++;
			break;
		case '[':
		{
			if (memory[datap] == 0)
			{
				uint32_t end = find_closing_bracket(pc);
				pc = end+1;
			}
			else
			{
				STACK_PUSH(pc+1);
				pc++;
			}
			break;
		}
		case ']':
			if (memory[datap] != 0)
			{
				pc = STACK_PEEK();
			}
			else
			{
				STACK_POP();
				pc++;
			}
			break;

		default: die("unknown instruction"); break;
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2)
		usage(argv);

	bf_file = argv[1];
	if ((bf_fd = open(bf_file, O_RDONLY)) < 0)
		die("open");

	load();
	
	run();

	close(bf_fd);
	return EXIT_SUCCESS;
}
