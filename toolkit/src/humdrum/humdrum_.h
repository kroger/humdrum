/****************************************************************************/
/*						HUMDRUM_.H											*/	
/*	This is the header file for humdrum_.c and contains function prototypes	*/
/* and global variables for the humdrum_ program.								*/
/****************************************************************************/

file_list *file_header;
interp_list *interp_header;
interp_inventory *ex_inven_header;
interp_inventory *tand_inven_header;

int verbose;
int help;
int current_no_of_spines;
int first_interpretation_yet;
int fnr;
char current_filename[FILE_LENGTH];

int new_path;
char signifiers[127];
int global_comments;
int local_comments;
int maximum_no_of_spines;
int minimum_no_of_spines;
int number_of_data_records;
int null_tokens;
int added_spines;
int terminated_spines;
int split_spines;
int joined_spines;
int exchanged_spines;

/* Functions in humdrum.c */

void init_data(void);
int parse_command(int, char**);
void clean_up(void);
void check_files(void);
void process_global();
void process_local(tokens);
void process_interpretation(tokens);
void process_data(tokens);
void check_first_interp(tokens);
void store_new_interps(tokens);
void take_inventory(char *,interp_inventory *);
void check_new_path(tokens);
void print_syntax_error(char *);
void print_verbose(void);
void print_help(void);

void store_indicators(tokens);
void process_indicators(tokens);
interp_list *do_split(interp_list *);
interp_list *do_terminate(interp_list *,interp_list *);
interp_list *do_exchange(interp_list *);
interp_list *do_join(interp_list *);
interp_list *do_add(interp_list *);
interp_list *insert_new(interp_list *);
interp_list *delete_old(interp_list *,interp_list *);
int open_file(char *, char*);
void add_file(char *);
void split_string(char *, char *, tokens*);
void free_tokens(tokens);
void print_error(char *,char *);
