// swad_layout.h: page layout

#ifndef _SWAD_LAY
#define _SWAD_LAY
/*
    SWAD (Shared Workspace At a Distance in Spanish),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2015 Antonio Ca�as Vargas

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*****************************************************************************/
/********************************* Headers ***********************************/
/*****************************************************************************/

#include "swad_action.h"

/*****************************************************************************/
/****************************** Public constants *****************************/
/*****************************************************************************/

#define Lay_MAX_BYTES_ALERT (16*1024)		// Max. size for alert message
// Important: the size of alert message must be enough large to store the longest message.

#define Lay_MAX_BYTES_TITLE 1024	// Max. size for window status message

#define Lay_HIDE_BOTH_COLUMNS	0						// 00
#define Lay_SHOW_RIGHT_COLUMN	1						// 01
#define Lay_SHOW_LEFT_COLUMN	2						// 10
#define Lay_SHOW_BOTH_COLUMNS (Lay_SHOW_LEFT_COLUMN | Lay_SHOW_RIGHT_COLUMN)	// 11

/*****************************************************************************/
/********************************* Public types ******************************/
/*****************************************************************************/

#define Lay_NUM_ALERT_TYPES 4
typedef enum
  {
   Lay_INFO    = 0,
   Lay_SUCCESS = 1,
   Lay_WARNING = 2,
   Lay_ERROR   = 3,
  } Lay_AlertType_t;

#define Lay_NUM_LAYOUTS 2
typedef enum
  {
   Lay_LAYOUT_DESKTOP = 0,
   Lay_LAYOUT_MOBILE  = 1,
   Lay_LAYOUT_UNKNOWN = 2,
  } Lay_Layout_t;	// Stored in database. Don't change numbers!

#define Lay_LAYOUT_DEFAULT Lay_LAYOUT_DESKTOP

typedef enum
  {
   Lay_NO_BUTTON,
   Lay_CREATE_BUTTON,
   Lay_CONFIRM_BUTTON,
   Lay_REMOVE_BUTTON,
  } Lay_Button_t;

/*****************************************************************************/
/****************************** Public prototypes ****************************/
/*****************************************************************************/

void Lay_WriteStartOfPage (void);
void Lay_WriteTitle (const char *Title);

void Lay_PutFormToView (Act_Action_t Action);
void Lay_PutFormToEdit (Act_Action_t Action);
void Lay_PutIconWithText (const char *Icon,const char *Alt,const char *Text);
void Lay_PutCalculateIconWithText (const char *Alt,const char *Text);

void Lay_PutIconRemovalNotAllowed (void);
void Lay_PutIconBRemovalNotAllowed (void);
void Lay_PutIconRemove (void);
void Lay_PutIconBRemove (void);

void Lay_PutCreateButton (const char *Text);
void Lay_PutCreateButtonInline (const char *Text);
void Lay_PutConfirmButton (const char *Text);
void Lay_PutConfirmButtonInline (const char *Text);
void Lay_PutRemoveButton (const char *Text);
void Lay_PutRemoveButtonInline (const char *Text);

void Lay_StartRoundFrameTable (const char *Width,unsigned CellPadding,const char *Title);
void Lay_StartRoundFrame (const char *Width,const char *Title);
void Lay_StartRoundFrameTableShadow (const char *Width,unsigned CellPadding);
void Lay_EndRoundFrameTable (void);
void Lay_EndRoundFrame (void);
void Lay_EndRoundFrameTableWithButton (Lay_Button_t Button,const char *TxtButton);
void Lay_EndRoundFrameWithButton (Lay_Button_t Button,const char *TxtButton);

void Lay_ShowErrorAndExit (const char *Message);
void Lay_ShowAlert (Lay_AlertType_t MsgType,const char *Message);
void Lay_RefreshNotifsAndConnected (void);
void Lay_RefreshLastClicks (void);
void Lay_WritePageFooter (void);

void Lay_WriteHeaderClassPhoto (unsigned NumColumns,bool PrintView,bool DrawingClassPhoto,
                                long InsCod,long DegCod,long CrsCod);

void Lay_PutIconsToSelectLayout (void);
void Lay_ChangeLayout (void);
Lay_Layout_t Lay_GetParamLayout (void);
Lay_Layout_t Lay_GetLayoutFromStr (const char *Str);

void Lay_AdvertisementMobile (void);

void Lay_IndentDependingOnLevel (unsigned Level,bool IsLastItemInLevel[]);

void Lay_HelpPlainEditor (void);
void Lay_HelpRichEditor (void);

#endif
