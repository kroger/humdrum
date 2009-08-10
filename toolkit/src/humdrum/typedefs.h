/****************************************************************************/
/*						TYPEDEFS.H											*/	
/* Structures used in all humdrum toolkit programs 							*/
/****************************************************************************/

typedef struct file_list file_list;
typedef struct tokens tokens;
typedef struct interp_list interp_list;
typedef struct interp_inventory interp_inventory;

struct file_list {
	FILE 		*stream;
	char 		file_name[FILE_LENGTH];
	file_list 	*next_file;
};

struct tokens {
	int 	number;
	char 	**token;
};

struct interp_list {
	char			name[NAME_LENGTH];
	char			indicator;
	interp_list		*next_interp;
};

struct interp_inventory {
	char				name[NAME_LENGTH];
	int					count;
	interp_inventory	*next_interp;
};
