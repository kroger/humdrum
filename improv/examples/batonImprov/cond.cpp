//cond.cpp--conductor program--float tempo--100ms retrigger time
// Converted to linux Wed Dec 20 19:10:35 PST 2000, by Craig Sapp
// $Smake: g++ -O6 -DLINUX -DALSA -I/user/c/craig/hciimprov/include -o %b %f %
//     -lpthread -L/home/mvm/cp1/hci/condtest/lib -limprov -lasound

#include <stdio.h>

#ifndef OLDCPP
   #include <iostream>
   using namespace std;
#else
   #include <iostream.h>
#endif

// #include <conio.h>   /* doesn't exist in linux */

#include "batonImprov.h"

void pitch_wheel(int a, int b, int c);
void command(int a, int b);

//////////////////////////////////////////////////////////////////////////
//#include "cond.h"

// cond.h for dos newbat
#define NOCONTROL 16/* max number of patchchords for control changes */
#define CHORDSIZE 15                   /* max no of notes in a chord */
#define VOICES 25                            /* maximum no of voices */
#define SCORERECSIZE 30            /* max no bytes in a score record */
#define NOTE 1
#define TRIG 2              /* / */
#define REST 4              /* r */
#define PROGRAMCHANGE 5     /* t */
#define NOOP 6
#define SETCHANNEL 7        /* h */
#define TEMPO 8             /* v or V */
#define UPPEDAL 9           //P
#define PEDAL 10            // p
#define CONTTEMPO 11        /* w */
#define MARK 12             /* m */
#define KEYVELOCITY 13      /* k */
#define BATON2 14           /* T */
#define DOTEQUALS 15        /* j */
#define INIT 19             /* I */
#define FTEMP 20            /* W */
#define CONTCONTROL 21      /* q */
#define MNOTEON 22
#define MNOTEOFF 23
#define NOACTION 24
#define STICKLEVELS 25
#define SYSEX 26
#define PITCHBEND 27
#define MEASURE 36

// nc.c and ns constants
#define SMOOTHING 0.5// 0.0==infinite smoothing, 1.0==no smoothing
#define X 1
#define Y 2
#define Z 3
#define P 4
#define V 5
#define S 6
short a[28],abeatset,arg1,arg2;
long beatsum;
long beat_time;
short b[28],baton2,bufchan;
unsigned char bufp;
short b1,b2;
unsigned char calf[2048];
short chan;
short channel[VOICES];
short chord[VOICES*CHORDSIZE];
short chordpi[VOICES],chordp[VOICES];
short controler;
short cont4,contp,ctempo,contpatch;
short contch[NOCONTROL],contno[NOCONTROL];/* arrays defining patchcords */
short controlerfn[NOCONTROL],controlerfnv[NOCONTROL];
int xform[NOCONTROL];
float xforma[NOCONTROL],xformb[NOCONTROL];
float tx0,tx1,ty0,ty1;
//short contbalance[NOCONTROL];
short sustped[16];
long  contime;
short dataa,dist;
short dotchange,dotro,dotrn;
short diff[11];
short dx,dr0,drumset1,drumset2;
short dynamic[VOICES],dyntype[VOICES];
long dur;
short endscoreflag;
// extern short hit1,hit2;short hungnote,hungnoteno;
long ibeat,itim;
short interval;
short kk;
short keych,keyno,keyvel;
long keytime;
short mark,midich,midichar,minus14,minus15,mm;
unsigned char midibuf[256],midirbuf[256];
unsigned char midipi,midipo,midirpi,midirpo;
short num,nextkeyno,noteon,nextbeat,nobat,needscore;
short notenumb1,notenumb2,noteon1,noteon2;
long nexttime,nextnotetime;
short zpause,plus14,plus15;
short potvalue,pot[5];
long pt_time,pt_beat;
short prog;
short recordmark,retoff,retseg,restpoint,recordcnt,recordlength;
long samptime;
short samp,scanmark,scancount,sustain;
short scorestate,scoreadp,scorkd[SCORERECSIZE],scorkdend;
// extern short set1,set2;short stick,st0,st1,st2,set12p,sticklevelflag;
short& set1 = baton.calib[1]; 
short sum,suminc,range;
short t[28],temposet,thru;
long threshtime;
short tranftype;
extern long t_time;long t_times,t_tdata,t_tkey;
short volume[16],vtemp,volumeconst,volx;
int value;
short whack;
short x1,x2,x[3];
short x1s,x1cont[16],x1type[16],x2s,x2cont[16],x2type[16];
short y11,y2,y[3];
short y1s,y1cont[16],y1type[16],y2s,y2cont[16],y2type[16];
short zp,z1,z2,z[3];
short zero1x,zero1y,zero2x,zero2y,zcalib;
char midif[100],scorepf[100];

// these variables were removed from "extern"
char scoref[1000];
int argcglobal;

short newdotp,scoresok;

//
// end of cond.h
//
//////////////////////////////////////////////////////////////////////////

int temp1,temp2,playmode;
short sysexmessage,midich1,maxscore,minscore,sendingscore,timecount;
unsigned char midi_port;
unsigned char midififo[256];
unsigned int midiip,midiop,midiipsav;
short midimessages,sysexcase,scorechars,charcount,endplayscoreflag;
unsigned int scoreip,scoreop;
long controlchangetime,lastwhacktime;
int startmeasure;
int scan,discard;
int measure;
int readscore;
int ii;
FILE *fp2;
short buf[17];
int b14minusdown;
int b14minusup;
int b15minusdown;
int b15minusup;
float tempo,temposave;


//functions
int keyvelfun(int);
int getch(void);
void xformandsend(void);
void pcc(void);
void loadplay(void);
void playrecord(void);
void readscorez(void);
void stkcontrol(void);

/*--------------------beginning improvization algorithms------------------*/


/*---------------------initialization algorithms--------------------------*/
void description(void){
  printf
  ("CONDUCTOR COMMANDS s--select score   z--start playing\n");
  printf
  ("                   S--silence synth  n--toggle no baton mode on/off\n");
}//end description()


void initialization(void){
  printf("CONDUCTOR COMMANDS s--select score   z--play   n--no baton mode on/off\n");

  argcglobal = options.getArgCount() + 1;
  const char* scorefx;
  if (argcglobal >= 2) {
     scorefx = options.getArg(1);
  } else {
     scorefx = "\0";
  }
  strcpy(scoref, scorefx);

  if(argcglobal==2){
	  printf("score file is %s \n", scoref);
      ii=-1;while(scoref[++ii]!='\0')scorepf[ii]=scoref[ii];
        scorepf[ii++]='.';scorepf[ii++]='p'; scorepf[ii]='\0';
      ii=-1;while(scoref[++ii]!='\0')midif[ii]=scoref[ii];
        midif[ii++]='.';midif[ii++]='m';midif[ii++]='i';
        midif[ii++]='d';midif[ii]='\0';
      if((fp2=fopen(scorepf,"r+b")) != NULL){
         newdotp=0;
         printf("type COMMAND (type z to start playing)\n");
	  }
      else{printf("SCORE NOT FOUND\n");}
  }
  startmeasure=1;
}


void initializecond(void){
int i;
//whackint1=0;
sysexcase=midiip=midiop=midimessages=0;
for(i=0;i<VOICES;i++)chordpi[i]=chordp[i]=CHORDSIZE*i;
tempo=temposave=temposet=1;scan=ctempo=vtemp=zpause=0;
whack1=whack2=b14minusdown=b14minusup=b15minusdown=b15minusup=0;
scan=0;measure=1;/*if(startmeasure!=1)*/scan=1;
endscoreflag=0;
scoreop=scoreip=scoreadp=needscore=scorestate=scorechars=0;
endplayscoreflag=0;
threshtime=nexttime=pt_time=pt_beat=controlchangetime=t_time;
nexttime=0;
readscorez();
playmode=1;


}//end initializecond()

/*--------------------main loop algorithms -------------------------------*/
void mainloopalgorithms(void){
if(playmode){
  while(scan){                                        //jump to next measure
       if(scorkd[0]==NOTE)goto readsk;
       if(scorkd[0]==MNOTEON)goto readsk;           //skip midi note on's
       if(scorkd[0]==MNOTEOFF)goto readsk;           //skip midi note on's
       if(scorkd[0]==REST)goto readsk;           //skip midi note on's
       playrecord();                                   //play score record
       readsk:if(playmode)readscorez();           //read next score record
  }
  if(t_time>controlchangetime){         // if midibuf[] is empty send control changes
    controlchangetime=t_time+2;
    if(contno[contp])stkcontrol();         //send data from next patchchord
               /*contno[j] is zero for patchchord which is not in use */
    if(++contp==NOCONTROL)contp=0;
  }
  if(zpause)goto endmainloop;


  if(nobat){  /* in nobaton mode, play score at constant tempo (=tempo)*/
    if(t_time>nexttime){
        playrecord();if(playmode)readscorez();nexttime=t_time+(nextbeat*(int)tempo);
      }
  }
  else{
    if(ctempo){                     /* continuous tempo control by y1 */
      if(t_time>nexttime){
          playrecord();if(playmode)readscorez();
             if(zt1>set1 && vtemp){
                 if(!yt1)yt1=1;
                 tempo=(float)(((int)temposave*64)/yt1);
             }
          nexttime=t_time+(nextbeat*(int)tempo);
      }
    }
    else{
      if(whack1){                           /*baton 1 control of tempo*/
          whack1=0;
          if((t_time-lastwhacktime)<100)goto falsewhack;
          lastwhacktime=t_time;
          beat_time=trigtime1;itim=trigtime1-pt_time;
          while(scorkd[0]!=TRIG){
             if(scorkd[0]==NOTE)scorkd[0]=REST;  /*skip unplayed notes*/
             playrecord();if(playmode)readscorez();
          }
          /* compute new tempo from time between last two baton beats */
          ibeat=beatsum-pt_beat;pt_beat=beatsum;pt_time=beat_time;
          if(dotchange){ibeat=(ibeat*dotrn)/dotro;dotchange=0;}
          if(!temposet){tempo=temposave=(short)(itim/ibeat);}
             else{temposet=0;}
          if(playmode)readscorez();nexttime=t_time+(nextbeat*(int)tempo);
          falsewhack:;
      }
      else{
        if(scorkd[0]!=TRIG){
           if(t_time>nexttime){
             playrecord();if(playmode)readscorez();nexttime=t_time+(nextbeat*(int)tempo);
           }
        }
      }//end if(!whack1)
    }
  }
endmainloop:;
} //end if(playmode
}

/*--------------------triggered algorithms--------------------------------*/
void stick1trig(void){}
void stick2trig(void){}
void b14plustrig(void){
    // clmidi();offmidi();t_sched_off();t_stop();exit(0);
}
void b15plustrig(void){printf("b15+ trigger\n");}
void b14minusuptrig(void)  {b14minusdown=1;
          scan=1;startmeasure=measure;
}
void b14minusdowntrig(void){b14minusup=1;}
void b15minusuptrig(void)  {b15minusdown=1;}
void b15minusdowntrig(void){b15minusup=1;}
void keyboardchar(int testch){
   switch(testch){
      case 's':                                      //select score
      echoKeysOn();
      printf("type name of score file \n"); getch();scanf("%s",scoref);
      echoKeysOff();
      ii=-1;while(scoref[++ii]!='\0')scorepf[ii]=scoref[ii];
        scorepf[ii++]='.';scorepf[ii++]='p'; scorepf[ii]='\0';
      ii=-1;while(scoref[++ii]!='\0')midif[ii]=scoref[ii];
        midif[ii++]='.';midif[ii++]='m';midif[ii++]='i';
        midif[ii++]='d';midif[ii]='\0';
      if((fp2=fopen(scorepf,"r+b")) != NULL){
         newdotp=0;
         printf("type COMMAND (type z to start playing)\n");
      }
      else{printf("SCORE NOT FOUND\n");}
      break;
      case 'z':
         command(17,127);       //turn on position reporting
         loadplay();pcc();
      break;
      case 'n':                                          //toggle no baton mode
         if(nobat){nobat=0;printf("NOBAT MODE OFF\n");}
         else{nobat=1;printf("NOBAT MODE ON\n");}
         break;
      case 'm':                                         //set startmeasure
         echoKeysOn();
         printf("startmeasure =");getch();scanf("%d",&startmeasure);
         echoKeysOff();
         break;
      default: printf("UNKNOWN COMMAND \n");pcc();break;
   }//end switch(testch)
}//end keyboardchar(testch)

void finishup(void) { }

void loadplay(void){
int ii;
//long nexttime;
      for(ii=0;ii<16;ii++)control_change(ii,64,0);
      nexttime=t_time;
      while(t_time<nexttime);
           if((fp2=fopen(scorepf,"r+b")) != NULL){
			   printf("PLAY SCORE %s \n", scoref);
               initializecond();
           }
           else{printf("SCORE.P FILE NOT FOUND\ntype COMMAND\n"); scoresok=0; }
}//end loadplay()


void stkcontrol(void)    // coupling batons to control changes
{
  int i,j;
  value=-1;
  i=controlerfn[contp];
  j=controlerfnv[contp];
  switch(i){
      case X:if(j==1 ){value=xt1;xformandsend();}
             if(j==2 ){value=xt2;xformandsend();}
             break;
      case Y:if(j==1 ){value=yt1;xformandsend();}
             if(j==2 ){value=yt2;xformandsend();}
             break;
      case Z:if(j==1){value=zt1;xformandsend();}
		     if(j==2){value=zt2;xformandsend();}            
             break;
      case P:switch(j){ 
		       case 1:control_change(contch[contp]+1,contno[contp],pt1);break;
               case 2:control_change(contch[contp]+1,contno[contp],pt2);break;
			   case 3:control_change(contch[contp]+1,contno[contp],pt3);break;
			   case 4:control_change(contch[contp]+1,contno[contp],pt4);break;
             }            
             break;
      case S:
       if(j==15){                                     //buf15- trigger
        if(!sustped[contch[contp]] && b15minusdown){       //down trig
           b15minusdown=0;
           sustped[contch[contp]]=1;control_change(contch[contp],contno[contp],127);
        }
        if(sustped[contch[contp]] && b15minusup){          //up trig
           b15minusup=0;
           sustped[contch[contp]]=0;control_change(contch[contp],contno[contp],0);
        }
       }
       break;

  }
}

void xformandsend(void){
	float xformvalue;
	if(xform[contp]==0){control_change(contch[contp]+1,contno[contp],value);}
    else{
      xformvalue=xformb[contp]+(xforma[contp]*(float)value);
	  value=(int)xformvalue;if(value<0)value=0;if(value>127)value=127;
	  control_change(contch[contp]+1,contno[contp],value);
	}
}



// play--------------------execute a score record------------------------
/* play executes a score record--for example starting a note.  it
is called and is executed at the real-time moment that the note is
suppose to be played.  it looks at the record stored in scorkd[]. it
looks at the op code of the record and does a switch depending on
the op code to different blocks of code which handle the different
op codes.  */
void playrecord(void){
int i,c,v,keyno,keyvel,beg,end,accent,keyvela,tp;
c=scorkd[0];
switch(c){
case NOTE:                             /* turns on a note or a chord */
  //printf("note= %d %d %d %d %d",scorkd[0],scorkd[1],scorkd[2],scorkd[3],scorkd[4]);
  v=scorkd[2];c=channel[v];beg=chordpi[v];end=chordp[v];
  keyvel=keyvelfun(v);
  for(i=beg;i<end;i++)note_on(c+1,chord[i],0);/*turns off notes that are
                                                             playing */
  i=3;
  while(i<scorkdend){                  /* turns on new note or notes */
    keyno=scorkd[i++];
    keyvela=scorkd[i++]+keyvel;
    if(keyvela<1)keyvela=1;if(keyvela>127)keyvela=127;
    //printf("%d %d %d\n",c,keyno,keyvela);
    note_on(c+1,keyno,keyvela);chord[beg++]=keyno;
   }
  chordp[v]=beg;
  break;
case TRIG:                        /* record produced by a "/" in score */
  break;
case REST:              /*turns off all notes playing in a given voice */
  v=scorkd[2];c=channel[v];beg=chordpi[v];end=chordp[v];
  for(i=beg;i<end;i++)note_on(c+1,chord[i],0);
  chordp[v]=beg;
  break;
  keyvel=keyvelfun(v);
case MNOTEON:      /*turns on a single note. note must be ended with a
                                              subsequent MNOTEOFF record*/
  v=scorkd[2];c=channel[v];keyno=scorkd[3];accent=scorkd[4];
  keyvel=keyvelfun(v);
  keyvela=accent+keyvel;
  if(keyvela<1)keyvela=1;if(keyvela>127)keyvela=127;
  note_on(c+1,keyno,keyvela);
  break;
case MNOTEOFF:                              /* turns off a single note */
  v=scorkd[2];c=channel[v];keyno=scorkd[3];accent=scorkd[4];
  keyvel=keyvelfun(v);
  keyvela=accent+keyvel;
  if(keyvela<1)keyvela=1;if(keyvela>127)keyvela=127;
  note_on(c+1,keyno,0);
  break;
case MEASURE:                         //print measure number, stop scan
  cout << "measure= " << measure << "\r" << flush;
  measure++;
  if(measure>startmeasure)scan=0;
  break;
case PITCHBEND:
  v=scorkd[2];c=channel[v];
  pitch_wheel(c,scorkd[3],scorkd[4]);break;
case PEDAL:              // "p" in score turns sustain pedal on
  v=scorkd[2];c=channel[v];control_change(c,64,127);break;
case UPPEDAL:            // "P" in score turns sustain pedal off
  v=scorkd[2];c=channel[v];control_change(c,64,0);break;
case CONTTEMPO:  /* "w"--continuous tempo control with y1, ignor beats */
  if(ctempo){ctempo=0;vtemp=0;}else{ctempo=1;vtemp=1;}
  break;
case FTEMP:                   /* "W"--fixed tempo control, ignor beats */
  if(ctempo){ctempo=0;vtemp=0;}else{ctempo=1;vtemp=0;}
  break;
case PROGRAMCHANGE:    /* "t" in score will cause a midi program change*/
  v=scorkd[2];c=channel[v];program_change(c+1,scorkd[3]);
  break;
case SETCHANNEL:/* the midi channel for a given voice is kept in
                         channel[]. it is set with an "h" in the score */
  channel[scorkd[2]]=scorkd[3];
  break;
case TEMPO:         /* "v"--set a fixed tempo starting at next event */
  tempo=temposave=(float)((scorkd[2]+(scorkd[3]<<6))>>2);temposet=1;
  break;
case KEYVELOCITY:                  /* "k"--setup keyvelocity control */
  dynamic[scorkd[2]]=scorkd[3];dyntype[scorkd[2]]=scorkd[4];
  break;
case BATON2:    // T in score turna baton 2 on or off as a source of beats
  if(baton2){baton2=0;}else{baton2=1;}
  break;
case MARK:                   //rehersal mark--displayed during performance
  mark=scorkd[3];break;
case DOTEQUALS:                        /* change ratio of dot to comma */
  tempo=(float)(((int)tempo*scorkd[3])/scorkd[4]);
  dotchange=1;dotro=scorkd[3];dotrn=scorkd[4];
  break;
case CONTCONTROL:                 // setup patchcord for continuous control
  if(scorkd[6]==V)       // if control change is a constant, send only once
   {tp=scorkd[7]-scorkd[8];if(tp<0)tp=0;control_change(scorkd[5]+1,scorkd[4],tp);}
  else{           // if control change is baton position, setup patchchord
    contpatch=scorkd[3];contno[contpatch]=scorkd[4];
    contch[contpatch]=scorkd[5];controlerfn[contpatch]=scorkd[6];
    controlerfnv[contpatch]=scorkd[7];
	if(scorkd[8]==scorkd[9]){xform[contpatch]=0;}
	else{
		tx0=(float)scorkd[8];tx1=(float)scorkd[9];ty0=(float)scorkd[10];ty1=(float)scorkd[11];
		xform[contpatch]=1;
		xforma[contpatch]= (ty1-ty0)/(tx1-tx0);
		xformb[contpatch]= ((tx1*ty0)-(tx0*ty1))/(tx1-tx0);
		//printf("xform,contpatch= %d %d \n",xform[contpatch],contpatch);
	}
  }
  break;
case INIT:                                                //initialization
  for(i=1;i<17;i++)channel[i]=i-1;               //default  channel#=voice#
  tempo=temposave=temposet=1;ctempo=vtemp=0;
  break;
case NOACTION:
  break;
}  // end switch(c)
}  // playrecord()

keyvelfun(int v)       // keyvelocity control, v is voice
{
int type,keyvel;
type=dyntype[v];
  switch(type){
    case X:if(dynamic[v]==1){keyvel=xt1;}
           else             {keyvel=xt2;}
           break;
    case Y:if(dynamic[v]==1){keyvel=yt1;}
           else             {keyvel=yt2;}
           break;
    case Z:if(dynamic[v]==1){keyvel=zt1;}
           else             {keyvel=zt2;}
           break;
    case P:
       if(dynamic[v]<4){keyvel=buf[10+dynamic[v]]>>5;} //pots 1-3
       else{keyvel=buf[5];}                            //pot 4
       break;
    case V:keyvel=dynamic[v];break;
  }
return(keyvel);
}

getnextchar(){
   int nextchar;
   nextchar=getc(fp2);
   if(nextchar==EOF){playmode=0;printf("END OF SCORE\n");pcc();}
   return(nextchar);
}

void readscorez(void){
int iii,opcode;
  if(fp2==NULL || !playmode) return; // return(0);
  nextbeat=0;
  nextrecord:
  charcount=getnextchar();if(!playmode)return; //return(0);
  opcode=getnextchar();if(!playmode)return; //return(0);
  nextbeat+=getnextchar();if(!playmode)return; //return(0);
  if(opcode==NOOP)goto nextrecord;
  scorkd[0]=opcode;
  scorkd[1]=nextbeat;
  scorkdend=charcount-1;
  if(charcount>3){
     for(iii=2;iii<scorkdend;iii++){
          scorkd[iii]=getnextchar();if(!playmode)return; //return(0);
     }
}
  scorkd[scorkdend]=0;
  //printf("scorkd= %d %d %d %d %d %d %d %d \n",scorkd[0],scorkd[1],scorkd[2],scorkd[3],scorkd[4],scorkd[5],scorkd[6],scorkd[7]);
  beatsum+=nextbeat;
//  return(0);
}//end readscore()




void pcc(void){
   printf (
   "type NEXT COMMAND  (type Q to quit, type ? to print list of commands)\n");
}


/////////////////////////////////////////////////////////////////////////////////////
// 
// added functions:
//


void command(int a, int b) {
   synth.send(0xa0, a, b);
}

void pitch_wheel(int a, int b, int c) {
   synth.send(0xe0 | a, b, c);
}

#ifndef VISUAL
int getch(void) { }
#endif
// md5sum: 67cc02b5b82cf04dbe79d6708ee2976b cond.cpp [20050403]
