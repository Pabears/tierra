/* beagleGUI_TPingSWindow.c -- -*- C -*-
 * 
 * This work has been done at ATR HIP
 * 
 * Author          : YOSHIKAWA Tooru
 * Created On      : Tue May 19 11:41:16 1998
 * Last Modified By: YOSHIKAWA Tooru
 * Last Modified On: Tue Oct 12 15:57:29 1999
 * Update Count    : 31
 * Status          : 
 */

#include "clnt.h"
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/AsciiText.h>
#include <time.h>

#include "beagleMessageMgr.h"
#include "beagleUTIL_Convert.h"
#include "beagleGUI_Global.h"
#include "beagleGUI_TopWindow.h"
#include "beagleGUI_TPingSWindow.h"


/*
 *- SPECIAL VALIABLE NAME
 *- =====================
 *- This window show only three variables for another file using the file
 *- 'beagleGUI_Global.h' and following tow. another ones  name and type a
 *- 'Widget tPingSWindow'.
 *- So you can open this window from every GUI file if need it.
 */


/*
 *- HERE ARE EXTERN FUNCTION DEFINEs.
 *- ================================
 *- This window (tPingSWindow) use here external function only.
 *- So this file need the file include here function to linking.
 *- I tried to collect there functions in '../Bglclnt/beagleRoutines.c'.
 *- Another GUI functions are define in the 'beagleGUI_TPingSWindow.h'.
 */

/*** NONE ***/

/*
 *- HERE ARE GLOBAL VARIABLE (EFFECTIVE IN THIS FILE ONLY) DEFINEs.
 *- ===============================================================
 *- So another files functions cannot search these.
 *- I want to avoid from linking trouble.
 */
#define	TPING_S_LIST_MAX	100
#define TPING_S_LIST_COLUMN	115
#define TPING_S_DATA_W_FORMAT "%-20s %6d %6d %8d %5d %8d %8d %5u %2s/%2s %2s:%2s %4d"
#define TPING_S_DATA_R_FORMAT "%20s %6d %6d %8d %5d %8d %8d %5u %2s/%2s %2s:%2s %4d"

static Widget	tPingSOnOffButton;
static Widget	pageUpButton, pageDownButton;
static I32s	tPingSStats = 0;/* 0: off, 1: on */
static Widget	tPingSInfoArea;
static int	numOfListItem = 0;
static char*	listIndex[TPING_S_LIST_MAX];
static char	liststr[TPING_S_LIST_MAX * TPING_S_LIST_COLUMN];
static int	topOfPage = 0;
static int	maxId = 0;


/*
 *- HERE ARE STATIC FUNCTIONs PROTOTYPE DEFINEs.
 *- ============================================
 *- These functions call from in this file only.
 *- So another files functions cannot search these.
 *- I want to avoid from linking trouble.
 */

/*- Create windows.*/
static void	CreateMenuBar		P_((Widget));
static void	PutFileButton		P_((Widget));
static void	PutModeButton		P_((Widget));
static void	PutPageUpButton		P_((Widget));
static void	PutPageDownButton	P_((Widget));
static void	CreateTPingSInfoArea	P_((Widget));
/*- Action definition.*/
static void	CloseButtonPress	P_((Widget));
static void	TPingSToggleReq		P_((Widget));
static void	PageUpButtonPress	P_((Widget));
static void	PageDownButtonPress	P_((Widget));

/*- This windows process routines -*/

/* NONE */

/*
 *- HERE ARE THIS WINDOW's SERVICE FUNCTIONs FOR CALL FROM ANOTHER FILEs.
 *- =====================================================================
 *- I wrote prototype define in 'beagleGUI_TPingSWindow.h'.
 */
void	CreateTPingSWindow(topWidget)
     Widget topWidget;
{
  Widget	form;

  tPingSWindow
    = XtVaCreatePopupShell("tPingSWindow",
			   topLevelShellWidgetClass, topWidget,
			   NULL);
  form
    = XtVaCreateManagedWidget("tPingSForm",
			      panedWidgetClass,	tPingSWindow,
			      XtNwidth,		850,
			      XtNheight,	350,
			      NULL);
  CreateMenuBar(form);
  CreateTPingSInfoArea(form);
}

void	UpdateTPingSInfo()
{
  XawTextPosition	start, end;
  int			pos;
  Arg			args[3];
  int		i, readIndex, length, lineNo;
  char*	writePointer;
  int		times;
  BglTPingSt*		t;
  ip_t			node;
  char			hostNameStr[20];
  struct tm *           tmptr;
  int                   maxnamlen;
  char                  tm_mon[3];
  char                  tm_mday[3];
  char                  tm_hour[3];
  char                  tm_min[3];


  XawTextGetSelectionPos(tPingSInfoArea, &start,  &end);
  XtSetArg(args[0], XtNdisplayPosition, &pos);
  XtGetValues(tPingSInfoArea, args, 1);

  times = 10;
  while( BglMM_HowManyTPingS() && times-- ){
    t = BglMM_PickTPingS();
    if (t->id < topOfPage){
      XtSetArg(args[0], XtNsensitive, True);
      XtSetValues(pageUpButton, args, 1);
      continue;
    }
    maxId = (maxId < t->id)? t->id : maxId;
    if (t->id >= (topOfPage + TPING_S_LIST_MAX)){
      XtSetArg(args[0], XtNsensitive, True);
      XtSetValues(pageDownButton, args, 1);
      continue;
    }

    if ( t->mode != 'd' ){

      node.s_addr = t->node;

      maxnamlen=sizeof(hostNameStr)-1;
      strncpy(&(hostNameStr[0]), UTIL_Convert_IpToName(&node), maxnamlen);
      hostNameStr[maxnamlen]='\0';

      if(t->Time)
      {   tmptr=localtime((time_t*)&(t->Time));
          sprintf(&(tm_mon[0]),"%02d",(tmptr->tm_mon)+1);
          sprintf(&(tm_mday[0]),"%02d",tmptr->tm_mday);
          sprintf(&(tm_hour[0]),"%02d",tmptr->tm_hour);
          sprintf(&(tm_min[0]),"%02d",tmptr->tm_min);
      }
      else
      {   sprintf(&(tm_mon[0]),"%2s","--");
          sprintf(&(tm_mday[0]),"%2s","--");
          sprintf(&(tm_hour[0]),"%2s","--");
          sprintf(&(tm_min[0]),"%2s","--");
      }

      tsprintf(bglStrBuf,
	       TPING_S_DATA_W_FORMAT,
	       hostNameStr, (I32s)ntohs(t->portnb),
	       t->FecundityAvg,
	       t->Speed,
	       (int)t->NumCells,
	       t->AgeAvg,
	       t->SoupSize,
	       t->TransitTime,
	       tm_mon,
	       tm_mday,
	       tm_hour,
	       tm_min,
	       t->InstExec);

      lineNo = t->id - topOfPage;
      if ( ! listIndex[lineNo] ){
	listIndex[lineNo] = liststr + lineNo * TPING_S_LIST_COLUMN;
      }
      writePointer = listIndex[lineNo];
      for ( readIndex = 0; readIndex < TPING_S_LIST_COLUMN-1; readIndex++ ){
	if ( ! (*(bglStrBuf+readIndex) - '\0') ) break;
	*(writePointer++) = *(bglStrBuf+readIndex);
      }
      for ( /* NON */ ; readIndex < TPING_S_LIST_COLUMN-1; readIndex++ ){
	*(writePointer++) = ' ';
      }
      *(writePointer) = '\n';

      numOfListItem = (numOfListItem < lineNo+1 )? lineNo+1 : numOfListItem ;
    }
    else{
      /* numOfListItem--; */
    }
  }

  for ( lineNo=0; lineNo<numOfListItem; lineNo++ ){
    if ( listIndex[lineNo] ){
      /* over write the NL mark. Because sometimes there is a NULL char. */
      *(listIndex[lineNo]+TPING_S_LIST_COLUMN-1) = '\n';
    }
    else{
      writePointer = listIndex[lineNo] = liststr + lineNo * TPING_S_LIST_COLUMN;
      for ( readIndex = 0; readIndex < TPING_S_LIST_COLUMN-1; readIndex++ ){
	*(writePointer++) = ' ';
      }
      *(writePointer) = '\n';
    }
  }

  length = numOfListItem * TPING_S_LIST_COLUMN;
  *(liststr + length - 1) = '\0'; /* The end of text string */
  
  i = 0;
  XtSetArg(args[i], XtNstring,		liststr);	i++;
  XtSetArg(args[i], XtNdisplayPosition,	pos);		i++;
  XtSetValues(tPingSInfoArea, args, i);
  XawTextSetSelection(tPingSInfoArea, start, end);
}

void	TPingSAreaClean()
{
  Arg	args[2];
  int	i;

  for ( i=0; i<numOfListItem; i++ )    listIndex[i] = 0;
  numOfListItem = 0;
  liststr[0] = '\0';
  maxId = 0;

  i = 0;
  XtSetArg(args[i], XtNstring, liststr);i++;
  XtSetValues(tPingSInfoArea, args, i);
}

void	TPingSInfoTextDecode(w, e, s, c)
     Widget w;
     XExposeEvent* e;
     String* s;
     Cardinal* c;
{
  XawTextPosition	start, end;
  char	oneLineString[TPING_S_LIST_COLUMN+1];
  int	id;		/* for ID field			*/
  int	portnb;		/* for Port number field	*/
  int	fecundityAvg;	/* for Fecundity average field	*/
  int	speed;		/* for Speed field		*/
  int	numCells;	/* for Num of Cell field	*/
  int	ageAvg;		/* for Age average field	*/
  int	soupSize;	/* for Soup Size field		*/
  int	transitTime;	/* for Transit Time field	*/
  char  tm_mon[3];      /* for month                    */
  char  tm_mday[3];     /* for day of month             */
  char  tm_hour[3];     /* for hour                     */
  char  tm_min[3];      /* for minute                   */
  int	instExec;	/* for Instruct Exec field	*/

  XawTextGetSelectionPos(w, &start,  &end);

  if ( end <= start )  return;

  strncpy(oneLineString, liststr+start, TPING_S_LIST_COLUMN);
  oneLineString[TPING_S_LIST_COLUMN] = '\0';

  sscanf(oneLineString,
	 TPING_S_DATA_R_FORMAT,
	 &id,
	 selectedHostName,  /* <- This is defined in beagleGUI_Global.h */
	 &portnb,
	 &fecundityAvg,
	 &speed,
	 &numCells,
	 &ageAvg,
	 &soupSize,
	 &transitTime,
         &(tm_mon[0]),
         &(tm_mday[0]),
         &(tm_hour[0]),
         &(tm_min[0]),
	 &instExec);
}


/*
 *- HERE ARE STATIC FUNCTIONs DEFINITION FOR CALL FROM IN THIS FILE ONLY.
 *- =====================================================================
 *- So another files functions cannot search these.
 *- I want to avoid from linking trouble.
 *- -- First part are creating window style.
 *- -- Second part are window actions mechanism.
 */

/*
 *- Creating window style.
 *- -- This window has a 'menu bar' and 'information area'.
 */
static void	CreateMenuBar(w)
     Widget w;
{
  Widget	menuBar;

  menuBar
    = XtVaCreateManagedWidget("menuBar",
			      boxWidgetClass,	w,
			      XtNorientation,	"horizontal",
			      NULL);
  PutFileButton(menuBar);
  PutModeButton(menuBar);
  PutPageUpButton(menuBar);
  PutPageDownButton(menuBar);
}

static void	PutFileButton(w)
     Widget w;
{
  Widget openButton;
  Widget menu, button;

  openButton
    = XtVaCreateManagedWidget("fileButton",
			      menuButtonWidgetClass, w,
			      XtNmenuName,	"filePullDownMenu",
			      NULL);
  menu
    = XtVaCreatePopupShell("filePullDownMenu",
			   simpleMenuWidgetClass, openButton,
			   NULL);
  button
    = XtVaCreateManagedWidget("fileCloseButton",
			      smeBSBObjectClass, menu,
			      NULL);
  XtAddCallback(button,
		XtNcallback, (XtCallbackProc)CloseButtonPress,
		NULL);
}

static void	PutModeButton(w)
     Widget w;
{
  Widget openButton;
  Widget menu;

  openButton
    = XtVaCreateManagedWidget("modeButton",
			      menuButtonWidgetClass, w,
			      XtNmenuName,	"modePullDownMenu",
			      NULL);
  menu
    = XtVaCreatePopupShell("modePullDownMenu",
			   simpleMenuWidgetClass, openButton,
			   NULL);
  tPingSOnOffButton
    = XtVaCreateManagedWidget("tPingSToggleButton",
			      smeBSBObjectClass,menu,
			      XtNlabel,		"TPing information On ",
			      NULL);
  XtAddCallback(tPingSOnOffButton,
		XtNcallback, (XtCallbackProc)TPingSToggleReq,
		NULL);
}

static void	PutPageUpButton(w)
     Widget w;
{
  pageUpButton
    = XtVaCreateManagedWidget("pageUpButton",
			      commandWidgetClass, w,
			      XtNlabel,		"Page UP",
			      XtNsensitive,	False,
			      NULL);

  XtAddCallback(pageUpButton,
		XtNcallback, (XtCallbackProc)PageUpButtonPress,
		NULL);
}

static void	PutPageDownButton(w)
     Widget w;
{
  pageDownButton
    = XtVaCreateManagedWidget("pageDownButton",
			      commandWidgetClass, w,
			      XtNlabel,		"Page DOWN",
			      XtNsensitive,	False,
			      NULL);

  XtAddCallback(pageDownButton,
		XtNcallback, (XtCallbackProc)PageDownButtonPress,
		NULL);
}

static void	CreateTPingSInfoArea(w)
     Widget w;
{
  Widget nameLabel, itemLabel;
  static  XawTextSelectType sarray[] =
    {XawselectLine, XawselectAll, XawselectNull};
  static XtTranslations	addActionTransTable;
  static String	addActionTrans ="#override\n\
<Btn1Up>: extend-end(PRIMARY,CUT_BUFFER0) tPingSInfoTextDecode(tPingSInfoArea)\n\
<Btn2Down>: no-op(RingBell)\n\
<Btn3Up>: extend-end(PRIMARY,CUT_BUFFER0) tPingSInfoTextDecode(tPingSInfoArea)";

  nameLabel
    = XtVaCreateManagedWidget("nameLabel",
			      labelWidgetClass, w,
			      XtNshowGrip,	False,
			      XtNresize,		False,
			      XtNheight,		10,
			      XtNdefaultDistance,	0,
			      XtNhorizDistance,		0,
			      XtNvertDistance,		0,
			      XtNlabel,	"  Cluster Server Map",
			      NULL);

  itemLabel
    = XtVaCreateManagedWidget("itemLabel",
			      labelWidgetClass, w,
			      XtNshowGrip,	False,
			      XtNresize,		False,
			      XtNheight,		10,
			      XtNdefaultDistance,	0,
			      XtNhorizDistance,		0,
			      XtNvertDistance,		0,
			      XtNlabel,
"  Host                   Port Fecund    Speed   Pop    AgeAv SoupSize  Echo MM/DD HH:MM Inst                            ",
			      NULL);

  liststr[0] = '\0';

  addActionTransTable = XtParseTranslationTable(addActionTrans);

  tPingSInfoArea
    = XtVaCreateManagedWidget("tPingSInfoArea",
			      asciiTextWidgetClass,	w,
			      XtNshowGrip,		False,
			      XtNtranslations,		addActionTransTable,
			      XtNeditType,		XawtextRead,
			      XtNselectTypes,		sarray,
			      XtNwidth,			850,
			      XtNscrollVertical,	XawtextScrollAlways,
			      XtNdisplayCaret,		False,
			      XtNuseStringInPlace,	True,
			      XtNstring,		liststr,
			      XtNlength,
			      (TPING_S_LIST_MAX * TPING_S_LIST_COLUMN),
			      NULL);
}

/*
 *- Action definition.
 */
static void	CloseButtonPress(w)
     Widget w;
{
  Arg args[1];
  XtPopdown(tPingSWindow);
  XtSetArg(args[0], XtNsensitive, True);
  XtSetValues(tPingSOpenButton, args, 1);
}

static void	TPingSToggleReq(w)
     Widget w;
{
  Arg args[1];

  if ( tPingSStats == 0 ){
    (void)BglMM_Misc_tSOn_need();
    XtSetArg(args[0], XtNlabel, "TPing information Off");
    XtSetValues(tPingSOnOffButton, args, 1);
    tPingSStats = 1;
  }
  else if ( tPingSStats == 1 ){
    (void)BglMM_Misc_tSOff_need();
    XtSetArg(args[0], XtNlabel, "TPing information On ");
    XtSetValues(tPingSOnOffButton, args, 1);
    XtSetArg(args[0], XtNsensitive, False);
    XtSetValues(pageUpButton, args, 1);
    XtSetValues(pageDownButton, args, 1);
    tPingSStats = 0;
  }
}

static void	PageUpButtonPress(w)
     Widget w;
{
  Arg args[1];

  topOfPage -= TPING_S_LIST_MAX;
  if (topOfPage == 0){
    XtSetArg(args[0], XtNsensitive, False);
    XtSetValues(pageUpButton, args, 1);
  }
  XtSetArg(args[0], XtNsensitive, True);
  XtSetValues(pageDownButton, args, 1);

  TPingSAreaClean();
  return;
}

static void	PageDownButtonPress(w)
     Widget w;
{
  Arg args[1];

  topOfPage += TPING_S_LIST_MAX;
  if (maxId < (topOfPage + TPING_S_LIST_MAX)){
    XtSetArg(args[0], XtNsensitive, False);
    XtSetValues(pageDownButton, args, 1);
  }
  XtSetArg(args[0], XtNsensitive, True);
  XtSetValues(pageUpButton, args, 1);

  TPingSAreaClean();
  return;
}

/*
 *- This windows process routines 
 */
