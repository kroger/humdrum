/************************************
  midi.h : contains definitions
  for internals used only by the
  record program
************************************/

#define 	TRUE	1
#define	FALSE	0
#define 	NUMARGS	2
#define 	NUM_SPINES 60
#define	BUFSIZE	56000
#define	QUARTER	72
#define	REST	200
#define	TIE		221	
#define	TEMPO_TOL	3

/* Spine path indicators */

#define TERMINATE 	-1
#define NOTHING 	0
#define ADD			1
#define EXCHANGE	2	
#define SPLIT		3
#define JOIN		4

/* Kern signifiers */

#define	KERN		 1
#define	NOTKERN	-1

/* Structures */

typedef	struct geek {
	int	note;
	int	on;
	int	velocity;
	int	delta;
	int	channel;
	int number;
	struct geek *next;
	}	MIDI_EVENT;

typedef	MIDI_EVENT	*MIDI_PTR;
