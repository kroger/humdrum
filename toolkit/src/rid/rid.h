/****************************************************************************
*								RID.H
*
* This file is used by the file rid_.c and contains important definitions
* for that program.
****************************************************************************/

/****************************************************************************/
/* Some useful definitions													*/
/****************************************************************************/

#define TRUE 1
#define FALSE 0
#define FILE_NAME_LENGTH 100
#define LINE_LENGTH 1024
#define NAME_LENGTH 80
#define TAB "\t"
#define TAB_CHAR '\t'

/****************************************************************************/
/* Some error messages														*/
/****************************************************************************/

#define OPEN_ERROR "rid: ERROR: Cannot open file %s\n"
#define MEMORY "rid: ERROR: Out of Memory\n"
#define USAGE "\nUSAGE: rid -h|-?              (Help Screen)\n       ri" \
			  "d [-dgiluDGILUT] [file ...]\n"
#define INVOKE "rid: ERROR: Invalid number of command line arguments\n" USAGE
#define ORDER "rid: ERROR: Record cannot occur after all spines terminate\n"

/****************************************************************************/
/* Some New type definitions												*/
/****************************************************************************/

typedef unsigned char boolean;
typedef struct tokens tokens;
typedef struct interp_list interp_list;
typedef struct buffer buffer;
typedef struct global_options global_options;

struct tokens {
	int 	number;
	char 	**token;
};

struct interp_list {
	char			name[NAME_LENGTH];
	char			indicator;
	interp_list		*next_interp;
};

struct buffer {
	char	buf_line[LINE_LENGTH];
	buffer	*next_line;
};

struct global_options {
	boolean D;
	boolean G;
	boolean I;
	boolean L;
	boolean U;
	boolean T;
	boolean d;
	boolean g;
	boolean i;
	boolean l;
	boolean u;
};

/****************************************************************************/
/* Some global variables													*/
/****************************************************************************/

global_options options;
interp_list *current_interps, *old_interps;
buffer *comment_buffer;
boolean storing, first_interpretation_yet;
FILE *stream;

/****************************************************************************/
/* Function prototypes														*/
/****************************************************************************/

void init_data(void);
void parse_command(int, char**);
void process_file(void);
void clean_up(void);

void process_global(char *);
void process_local(tokens);
void process_interpretation(tokens);
void process_data(tokens);
void process_exclusive(tokens);
void check_past(tokens);
void remove_redundant(tokens);
void process_spine_path(tokens);
void copy_new_to_old(void);
void store_terminator(tokens);
void delete_old_interps(void);

void store_new_interps(tokens);
void store_indicators(tokens);
void process_indicators(void);
interp_list *do_split(interp_list *);
interp_list *do_terminate(interp_list *,interp_list *);
interp_list *do_exchange(interp_list *);
interp_list *do_join(interp_list *);
interp_list *do_add(interp_list *);
interp_list *insert_new(interp_list *);
interp_list *delete_old(interp_list *,interp_list *);

void print_line(tokens);
void flush_buffer(tokens);
void print_error(char *,char *);
void split_string(char *, char, tokens*);
void free_tokens(tokens);
boolean null_record(tokens, char *);
boolean exclusive_record(tokens);
boolean tandem_record(tokens);
boolean spine_path_record(tokens);
boolean spine_terminate(tokens);
boolean all_found(tokens);
