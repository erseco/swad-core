// swad_place.c: places

/*
    SWAD (Shared Workspace At a Distance),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2017 Antonio Ca�as Vargas

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
/********************************** Headers **********************************/
/*****************************************************************************/

#include <linux/stddef.h>	// For NULL
#include <stdlib.h>		// For calloc
#include <string.h>		// For string functions

#include "swad_constant.h"
#include "swad_database.h"
#include "swad_global.h"
#include "swad_parameter.h"
#include "swad_place.h"
#include "swad_text.h"

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/******************************* Private types *******************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private variables *****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

static void Plc_GetParamPlcOrder (void);
static void Plc_PutIconToEditPlaces (void);
static void Plc_PutIconToViewPlacesWhenEditing (void);
static void Plc_ListPlacesForEdition (void);
static void Plc_PutParamPlcCod (long PlcCod);

static void Plc_RenamePlace (Cns_ShrtOrFullName_t ShrtOrFullName);
static bool Plc_CheckIfPlaceNameExists (const char *FieldName,const char *Name,long PlcCod);
static void Plc_UpdatePlcNameDB (long PlcCod,const char *FieldName,const char *NewPlcName);

static void Plc_PutFormToCreatePlace (void);
static void Plc_PutHeadPlaces (void);
static void Plc_CreatePlace (struct Place *Plc);

/*****************************************************************************/
/********************** Put link (form) to view places ***********************/
/*****************************************************************************/

void Plc_PutIconToViewPlaces (void)
  {
   extern const char *Txt_Places;

   Lay_PutContextualLink (ActSeePlc,NULL,NULL,
                          "mapmarker64x64.png",
                          Txt_Places,NULL,
                          NULL);
  }

/*****************************************************************************/
/*************************** List all the places *****************************/
/*****************************************************************************/

void Plc_SeePlaces (void)
  {
   extern const char *Hlp_INSTITUTION_Places;
   extern const char *Txt_Places;
   extern const char *Txt_PLACES_HELP_ORDER[2];
   extern const char *Txt_PLACES_ORDER[2];
   extern const char *Txt_Other_places;
   extern const char *Txt_Place_unspecified;
   extern const char *Txt_New_place;
   Plc_Order_t Order;
   unsigned NumPlc;
   unsigned NumCtrsWithPlc = 0;
   unsigned NumCtrsInOtherPlcs;
   bool ICanEdit;

   if (Gbl.CurrentIns.Ins.InsCod > 0)
     {
      ICanEdit = (Gbl.Usrs.Me.Role.Logged >= Rol_INS_ADM);

      /***** Get parameter with the type of order in the list of places *****/
      Plc_GetParamPlcOrder ();

      /***** Get list of places *****/
      Plc_GetListPlaces ();

      /***** Table head *****/
      Lay_StartRoundFrame (NULL,Txt_Places,
                           ICanEdit ? Plc_PutIconToEditPlaces :
                        	      NULL,
                           Hlp_INSTITUTION_Places);
      Lay_StartTableWideMargin (2);
      fprintf (Gbl.F.Out,"<tr>");
      for (Order = Plc_ORDER_BY_PLACE;
	   Order <= Plc_ORDER_BY_NUM_CTRS;
	   Order++)
	{
	 fprintf (Gbl.F.Out,"<th class=\"LEFT_MIDDLE\">");
	 Act_FormStart (ActSeePlc);
	 Par_PutHiddenParamUnsigned ("Order",(unsigned) Order);
	 Act_LinkFormSubmit (Txt_PLACES_HELP_ORDER[Order],"TIT_TBL",NULL);
	 if (Order == Gbl.Plcs.SelectedOrder)
	    fprintf (Gbl.F.Out,"<u>");
	 fprintf (Gbl.F.Out,"%s",Txt_PLACES_ORDER[Order]);
	 if (Order == Gbl.Plcs.SelectedOrder)
	    fprintf (Gbl.F.Out,"</u>");
	 fprintf (Gbl.F.Out,"</a>");
	 Act_FormEnd ();
	 fprintf (Gbl.F.Out,"</th>");
	}
      fprintf (Gbl.F.Out,"</tr>");

      /***** Write all places and their nuber of centres *****/
      for (NumPlc = 0;
	   NumPlc < Gbl.Plcs.Num;
	   NumPlc++)
	{
	 /* Write data of this place */
	 fprintf (Gbl.F.Out,"<tr>"
			    "<td class=\"DAT LEFT_MIDDLE\">"
			    "%s"
			    "</td>"
	                    "<td class=\"DAT RIGHT_MIDDLE\">"
	                    "%u"
	                    "</td>"
			    "</tr>",
		  Gbl.Plcs.Lst[NumPlc].FullName,
		  Gbl.Plcs.Lst[NumPlc].NumCtrs);
	 NumCtrsWithPlc += Gbl.Plcs.Lst[NumPlc].NumCtrs;
	}

      /***** Separation row *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td colspan=\"2\" class=\"DAT\">"
			 "&nbsp;"
			 "</td>"
			 "</tr>");

      /***** Write centres (of the current institution) with other place *****/
      NumCtrsInOtherPlcs = Ctr_GetNumCtrsInPlc (0);
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"DAT LEFT_MIDDLE\">"
			 "%s"
			 "</td>"
			 "<td class=\"DAT RIGHT_MIDDLE\">"
			 "%u"
			 "</td>"
			 "</tr>",
	       Txt_Other_places,NumCtrsInOtherPlcs);
      NumCtrsWithPlc += NumCtrsInOtherPlcs;

      /***** Write centres (of the current institution) with no place *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"DAT LEFT_MIDDLE\">"
			 "%s"
			 "</td>"
			 "<td class=\"DAT RIGHT_MIDDLE\">"
			 "%u"
			 "</td>"
			 "</tr>",
	       Txt_Place_unspecified,
	       Ctr_GetNumCtrsInIns (Gbl.CurrentIns.Ins.InsCod) -
	       NumCtrsWithPlc);

      /***** End table *****/
      Lay_EndTable ();

      /***** Button to create place *****/
      if (ICanEdit)
	{
	 Act_FormStart (ActEdiPlc);
	 Lay_PutConfirmButton (Txt_New_place);
	 Act_FormEnd ();
	}

      /***** End frame *****/
      Lay_EndRoundFrame ();

      /***** Free list of places *****/
      Plc_FreeListPlaces ();
     }
  }

/*****************************************************************************/
/********** Get parameter with the type or order in list of places ***********/
/*****************************************************************************/

static void Plc_GetParamPlcOrder (void)
  {
   Gbl.Plcs.SelectedOrder = (Plc_Order_t)
	                    Par_GetParToUnsignedLong ("Order",
                                                      0,
                                                      Plc_NUM_ORDERS - 1,
                                                      (unsigned long) Plc_ORDER_DEFAULT);
  }

/*****************************************************************************/
/********************** Put a link (form) to edit places *********************/
/*****************************************************************************/

static void Plc_PutIconToEditPlaces (void)
  {
   Lay_PutContextualIconToEdit (ActEdiPlc,NULL);
  }

/*****************************************************************************/
/*************************** Put forms to edit places ************************/
/*****************************************************************************/

void Plc_EditPlaces (void)
  {
   extern const char *Hlp_INSTITUTION_Places_edit;
   extern const char *Txt_Places;

   /***** Get list of places *****/
   Plc_GetListPlaces ();

   /***** Start frame *****/
   Lay_StartRoundFrame (NULL,Txt_Places,Plc_PutIconToViewPlacesWhenEditing,
                        Hlp_INSTITUTION_Places_edit);

   /***** Put a form to create a new place *****/
   Plc_PutFormToCreatePlace ();

   /***** Forms to edit current places *****/
   if (Gbl.Plcs.Num)
      Plc_ListPlacesForEdition ();

   /***** End frame *****/
   Lay_EndRoundFrame ();

   /***** Free list of places *****/
   Plc_FreeListPlaces ();
  }

/*****************************************************************************/
/**************** Put contextual icons in edition of places *****************/
/*****************************************************************************/

static void Plc_PutIconToViewPlacesWhenEditing (void)
  {
   Lay_PutContextualIconToView (ActSeePlc,NULL);
  }

/*****************************************************************************/
/**************************** List all the places ****************************/
/*****************************************************************************/

void Plc_GetListPlaces (void)
  {
   char OrderBySubQuery[256];
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;
   unsigned NumPlc;
   struct Place *Plc;

   /***** Get places from database *****/
   switch (Gbl.Plcs.SelectedOrder)
     {
      case Plc_ORDER_BY_PLACE:
         sprintf (OrderBySubQuery,"FullName");
         break;
      case Plc_ORDER_BY_NUM_CTRS:
         sprintf (OrderBySubQuery,"NumCtrs DESC,FullName");
         break;
     }
   sprintf (Query,"(SELECT places.PlcCod,places.ShortName,places.FullName,COUNT(*) AS NumCtrs"
                  " FROM places,centres"
                  " WHERE places.InsCod=%ld"
                  " AND places.PlcCod=centres.PlcCod"
                  " AND centres.InsCod=%ld"
                  " GROUP BY places.PlcCod)"
                  " UNION "
                  "(SELECT PlcCod,ShortName,FullName,0 AS NumCtrs"
                  " FROM places"
                  " WHERE InsCod=%ld"
                  " AND PlcCod NOT IN"
                  " (SELECT DISTINCT PlcCod FROM centres WHERE InsCod=%ld))"
                  " ORDER BY %s",
            Gbl.CurrentIns.Ins.InsCod,
            Gbl.CurrentIns.Ins.InsCod,
            Gbl.CurrentIns.Ins.InsCod,
            Gbl.CurrentIns.Ins.InsCod,
            OrderBySubQuery);
   NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get places");

   /***** Count number of rows in result *****/
   if (NumRows) // Places found...
     {
      Gbl.Plcs.Num = (unsigned) NumRows;

      /***** Create list with courses in centre *****/
      if ((Gbl.Plcs.Lst = (struct Place *) calloc (NumRows,sizeof (struct Place))) == NULL)
          Lay_ShowErrorAndExit ("Not enough memory to store places.");

      /***** Get the places *****/
      for (NumPlc = 0;
	   NumPlc < Gbl.Plcs.Num;
	   NumPlc++)
        {
         Plc = &(Gbl.Plcs.Lst[NumPlc]);

         /* Get next place */
         row = mysql_fetch_row (mysql_res);

         /* Get place code (row[0]) */
         if ((Plc->PlcCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
            Lay_ShowErrorAndExit ("Wrong code of place.");

         /* Get the short name of the place (row[1]) */
         Str_Copy (Plc->ShrtName,row[1],
                   Plc_MAX_BYTES_PLACE_SHRT_NAME);

         /* Get the full name of the place (row[2]) */
         Str_Copy (Plc->FullName,row[2],
                   Plc_MAX_BYTES_PLACE_FULL_NAME);

         /* Get number of centres in this place (row[3]) */
         if (sscanf (row[3],"%u",&Plc->NumCtrs) != 1)
            Plc->NumCtrs = 0;
        }
     }
   else
      Gbl.Plcs.Num = 0;

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/**************************** Get place full name ****************************/
/*****************************************************************************/

void Plc_GetDataOfPlaceByCod (struct Place *Plc)
  {
   extern const char *Txt_Place_unspecified;
   extern const char *Txt_Another_place;
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;

   /***** Clear data *****/
   Plc->ShrtName[0] = '\0';
   Plc->FullName[0] = '\0';
   Plc->NumCtrs = 0;

   /***** Check if place code is correct *****/
   if (Plc->PlcCod < 0)
     {
      Str_Copy (Plc->ShrtName,Txt_Place_unspecified,
                Plc_MAX_BYTES_PLACE_SHRT_NAME);
      Str_Copy (Plc->FullName,Txt_Place_unspecified,
                Plc_MAX_BYTES_PLACE_FULL_NAME);
     }
   else if (Plc->PlcCod == 0)
     {
      Str_Copy (Plc->ShrtName,Txt_Another_place,
                Plc_MAX_BYTES_PLACE_SHRT_NAME);
      Str_Copy (Plc->FullName,Txt_Another_place,
                Plc_MAX_BYTES_PLACE_FULL_NAME);
     }
   else if (Plc->PlcCod > 0)
     {
      /***** Get data of a place from database *****/
      sprintf (Query,"(SELECT places.ShortName,places.FullName,COUNT(*)"
                     " FROM places,centres"
                     " WHERE places.PlcCod=%ld"
                     " AND places.PlcCod=centres.PlcCod"
                     " AND centres.PlcCod=%ld"
                     " GROUP BY places.PlcCod)"
                     " UNION "
                     "(SELECT ShortName,FullName,0"
                     " FROM places"
                     " WHERE PlcCod=%ld"
                     " AND PlcCod NOT IN"
                     " (SELECT DISTINCT PlcCod FROM centres))",
               Plc->PlcCod,
               Plc->PlcCod,
               Plc->PlcCod);
      NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get data of a place");

      /***** Count number of rows in result *****/
      if (NumRows) // Place found...
        {
         /* Get row */
         row = mysql_fetch_row (mysql_res);

         /* Get the short name of the place (row[0]) */
         Str_Copy (Plc->ShrtName,row[0],
                   Plc_MAX_BYTES_PLACE_SHRT_NAME);

         /* Get the full name of the place (row[1]) */
         Str_Copy (Plc->FullName,row[1],
                   Plc_MAX_BYTES_PLACE_FULL_NAME);

         /* Get number of centres in this place (row[2]) */
         if (sscanf (row[2],"%u",&Plc->NumCtrs) != 1)
            Plc->NumCtrs = 0;
        }

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);
     }
  }

/*****************************************************************************/
/**************************** Free list of places ****************************/
/*****************************************************************************/

void Plc_FreeListPlaces (void)
  {
   if (Gbl.Plcs.Lst)
     {
      /***** Free memory used by the list of places in institution *****/
      free ((void *) Gbl.Plcs.Lst);
      Gbl.Plcs.Lst = NULL;
      Gbl.Plcs.Num = 0;
     }
  }

/*****************************************************************************/
/*************************** List all the places *****************************/
/*****************************************************************************/

static void Plc_ListPlacesForEdition (void)
  {
   unsigned NumPlc;
   struct Place *Plc;

   /***** Write heading *****/
   Lay_StartTableWide (2);
   Plc_PutHeadPlaces ();

   /***** Write all the places *****/
   for (NumPlc = 0;
	NumPlc < Gbl.Plcs.Num;
	NumPlc++)
     {
      Plc = &Gbl.Plcs.Lst[NumPlc];

      /* Put icon to remove place */
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td class=\"BM\">");
      if (Plc->NumCtrs)	// Place has centres ==> deletion forbidden
         Lay_PutIconRemovalNotAllowed ();
      else
        {
         Act_FormStart (ActRemPlc);
         Plc_PutParamPlcCod (Plc->PlcCod);
         Lay_PutIconRemove ();
         Act_FormEnd ();
        }
      fprintf (Gbl.F.Out,"</td>");

      /* Place code */
      fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE\">"
	                 "%ld"
	                 "</td>",
               Plc->PlcCod);

      /* Place short name */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActRenPlcSho);
      Plc_PutParamPlcCod (Plc->PlcCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"ShortName\""
	                 " maxlength=\"%u\" value=\"%s\""
                         " class=\"INPUT_SHORT_NAME\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Plc_MAX_CHARS_PLACE_SHRT_NAME,Plc->ShrtName,Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Place full name */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActRenPlcFul);
      Plc_PutParamPlcCod (Plc->PlcCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"FullName\""
	                 " maxlength=\"%u\" value=\"%s\""
                         " class=\"INPUT_FULL_NAME\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Plc_MAX_CHARS_PLACE_FULL_NAME,Plc->FullName,Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Number of centres */
      fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE\">"
	                 "%u"
	                 "</td>"
	                 "</tr>",
               Plc->NumCtrs);
     }

   /***** End table *****/
   Lay_EndTable ();
  }

/*****************************************************************************/
/******************** Write parameter with code of place *********************/
/*****************************************************************************/

static void Plc_PutParamPlcCod (long PlcCod)
  {
   Par_PutHiddenParamLong ("PlcCod",PlcCod);
  }

/*****************************************************************************/
/********************* Get parameter with code of place **********************/
/*****************************************************************************/

long Plc_GetParamPlcCod (void)
  {
   /***** Get code of place *****/
   return Par_GetParToLong ("PlcCod");
  }

/*****************************************************************************/
/******************************* Remove a place ******************************/
/*****************************************************************************/

void Plc_RemovePlace (void)
  {
   extern const char *Txt_To_remove_a_place_you_must_first_remove_all_centres_of_that_place;
   extern const char *Txt_Place_X_removed;
   char Query[128];
   struct Place Plc;

   /***** Get place code *****/
   if ((Plc.PlcCod = Plc_GetParamPlcCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of place is missing.");

   /***** Get data of the place from database *****/
   Plc_GetDataOfPlaceByCod (&Plc);

   /***** Check if this place has centres *****/
   if (Plc.NumCtrs)	// Place has centres ==> don't remove
      Ale_ShowAlert (Ale_WARNING,Txt_To_remove_a_place_you_must_first_remove_all_centres_of_that_place);
   else			// Place has no centres ==> remove it
     {
      /***** Remove place *****/
      sprintf (Query,"DELETE FROM places WHERE PlcCod=%ld",
	       Plc.PlcCod);
      DB_QueryDELETE (Query,"can not remove a place");

      /***** Write message to show the change made *****/
      sprintf (Gbl.Alert.Txt,Txt_Place_X_removed,
               Plc.FullName);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }

   /***** Show the form again *****/
   Plc_EditPlaces ();
  }

/*****************************************************************************/
/********************* Change the short name of a place **********************/
/*****************************************************************************/

void Plc_RenamePlaceShort (void)
  {
   Plc_RenamePlace (Cns_SHRT_NAME);
  }

/*****************************************************************************/
/********************* Change the full name of a place ***********************/
/*****************************************************************************/

void Plc_RenamePlaceFull (void)
  {
   Plc_RenamePlace (Cns_FULL_NAME);
  }

/*****************************************************************************/
/************************ Change the name of a place *************************/
/*****************************************************************************/

static void Plc_RenamePlace (Cns_ShrtOrFullName_t ShrtOrFullName)
  {
   extern const char *Txt_You_can_not_leave_the_name_of_the_place_X_empty;
   extern const char *Txt_The_place_X_already_exists;
   extern const char *Txt_The_place_X_has_been_renamed_as_Y;
   extern const char *Txt_The_name_of_the_place_X_has_not_changed;
   struct Place *Plc;
   const char *ParamName = NULL;	// Initialized to avoid warning
   const char *FieldName = NULL;	// Initialized to avoid warning
   unsigned MaxBytes = 0;		// Initialized to avoid warning
   char *CurrentPlcName = NULL;		// Initialized to avoid warning
   char NewPlcName[Plc_MAX_BYTES_PLACE_FULL_NAME + 1];

   Plc = &Gbl.Plcs.EditingPlc;
   switch (ShrtOrFullName)
     {
      case Cns_SHRT_NAME:
         ParamName = "ShortName";
         FieldName = "ShortName";
         MaxBytes = Plc_MAX_BYTES_PLACE_SHRT_NAME;
         CurrentPlcName = Plc->ShrtName;
         break;
      case Cns_FULL_NAME:
         ParamName = "FullName";
         FieldName = "FullName";
         MaxBytes = Plc_MAX_BYTES_PLACE_FULL_NAME;
         CurrentPlcName = Plc->FullName;
         break;
     }

   /***** Get parameters from form *****/
   /* Get the code of the place */
   if ((Plc->PlcCod = Plc_GetParamPlcCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of place is missing.");

   /* Get the new name for the place */
   Par_GetParToText (ParamName,NewPlcName,MaxBytes);

   /***** Get from the database the old names of the place *****/
   Plc_GetDataOfPlaceByCod (Plc);

   /***** Check if new name is empty *****/
   if (!NewPlcName[0])
     {
      sprintf (Gbl.Alert.Txt,Txt_You_can_not_leave_the_name_of_the_place_X_empty,
               CurrentPlcName);
      Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);
     }
   else
     {
      /***** Check if old and new names are the same (this happens when user press enter with no changes in the form) *****/
      if (strcmp (CurrentPlcName,NewPlcName))	// Different names
        {
         /***** If place was in database... *****/
         if (Plc_CheckIfPlaceNameExists (ParamName,NewPlcName,Plc->PlcCod))
           {
            sprintf (Gbl.Alert.Txt,Txt_The_place_X_already_exists,
                     NewPlcName);
            Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);
           }
         else
           {
            /* Update the table changing old name by new name */
            Plc_UpdatePlcNameDB (Plc->PlcCod,FieldName,NewPlcName);

            /* Write message to show the change made */
            sprintf (Gbl.Alert.Txt,Txt_The_place_X_has_been_renamed_as_Y,
                     CurrentPlcName,NewPlcName);
            Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
           }
        }
      else	// The same name
        {
         sprintf (Gbl.Alert.Txt,Txt_The_name_of_the_place_X_has_not_changed,
                  CurrentPlcName);
         Ale_ShowAlert (Ale_INFO,Gbl.Alert.Txt);
        }
     }

   /***** Show the form again *****/
   Str_Copy (CurrentPlcName,NewPlcName,
             MaxBytes);
   Plc_EditPlaces ();
  }

/*****************************************************************************/
/********************** Check if the name of place exists ********************/
/*****************************************************************************/

static bool Plc_CheckIfPlaceNameExists (const char *FieldName,const char *Name,long PlcCod)
  {
   char Query[256 + Plc_MAX_BYTES_PLACE_FULL_NAME];

   /***** Get number of places with a name from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM places"
	          " WHERE InsCod=%ld AND %s='%s' AND PlcCod<>%ld",
            Gbl.CurrentIns.Ins.InsCod,FieldName,Name,PlcCod);
   return (DB_QueryCOUNT (Query,"can not check if the name of a place already existed") != 0);
  }

/*****************************************************************************/
/****************** Update place name in table of places *********************/
/*****************************************************************************/

static void Plc_UpdatePlcNameDB (long PlcCod,const char *FieldName,const char *NewPlcName)
  {
   char Query[128 + Plc_MAX_BYTES_PLACE_FULL_NAME];

   /***** Update place changing old name by new name */
   sprintf (Query,"UPDATE places SET %s='%s' WHERE PlcCod=%ld",
	    FieldName,NewPlcName,PlcCod);
   DB_QueryUPDATE (Query,"can not update the name of a place");
  }

/*****************************************************************************/
/********************* Put a form to create a new place **********************/
/*****************************************************************************/

static void Plc_PutFormToCreatePlace (void)
  {
   extern const char *Txt_New_place;
   extern const char *Txt_Create_place;
   struct Place *Plc;

   /***** Place data *****/
   Plc = &Gbl.Plcs.EditingPlc;

   /***** Start form *****/
   Act_FormStart (ActNewPlc);

   /***** Start frame *****/
   Lay_StartRoundFrameTable (NULL,Txt_New_place,NULL,NULL,2);

   /***** Write heading *****/
   Plc_PutHeadPlaces ();

   /***** Column to remove place, disabled here *****/
   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"BM\"></td>");

   /***** Place code *****/
   fprintf (Gbl.F.Out,"<td class=\"CODE\"></td>");

   /***** Place short name *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"text\" name=\"ShortName\""
                      " maxlength=\"%u\" value=\"%s\""
                      " class=\"INPUT_SHORT_NAME\""
                      " required=\"required\" />"
                      "</td>",
            Plc_MAX_CHARS_PLACE_SHRT_NAME,Plc->ShrtName);

   /***** Place full name *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
                      "<input type=\"text\" name=\"FullName\""
                      " maxlength=\"%u\" value=\"%s\""
                      " class=\"INPUT_FULL_NAME\""
                      " required=\"required\" />"
                      "</td>",
            Plc_MAX_CHARS_PLACE_FULL_NAME,Plc->FullName);

   /***** Number of centres *****/
   fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE\">"
		      "0"
		      "</td>"
		      "</tr>");

   /***** Send button and end frame *****/
   Lay_EndRoundFrameTableWithButton (Lay_CREATE_BUTTON,Txt_Create_place);

   /***** End formn *****/
   Act_FormEnd ();
  }

/*****************************************************************************/
/******************** Write header with fields of a place ********************/
/*****************************************************************************/

static void Plc_PutHeadPlaces (void)
  {
   extern const char *Txt_Code;
   extern const char *Txt_Short_name;
   extern const char *Txt_Full_name;
   extern const char *Txt_Centres;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"BM\"></th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Code,
            Txt_Short_name,
            Txt_Full_name,
            Txt_Centres);
  }

/*****************************************************************************/
/******************* Receive form to create a new place **********************/
/*****************************************************************************/

void Plc_RecFormNewPlace (void)
  {
   extern const char *Txt_The_place_X_already_exists;
   extern const char *Txt_You_must_specify_the_short_name_and_the_full_name_of_the_new_place;
   struct Place *Plc;

   Plc = &Gbl.Plcs.EditingPlc;

   /***** Get parameters from form *****/
   /* Get place short name */
   Par_GetParToText ("ShortName",Plc->ShrtName,Plc_MAX_BYTES_PLACE_SHRT_NAME);

   /* Get place full name */
   Par_GetParToText ("FullName",Plc->FullName,Plc_MAX_BYTES_PLACE_FULL_NAME);

   if (Plc->ShrtName[0] && Plc->FullName[0])	// If there's a place name
     {
      /***** If name of place was in database... *****/
      if (Plc_CheckIfPlaceNameExists ("ShortName",Plc->ShrtName,-1L))
        {
         sprintf (Gbl.Alert.Txt,Txt_The_place_X_already_exists,
                  Plc->ShrtName);
         Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);
        }
      else if (Plc_CheckIfPlaceNameExists ("FullName",Plc->FullName,-1L))
        {
         sprintf (Gbl.Alert.Txt,Txt_The_place_X_already_exists,
                  Plc->FullName);
         Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);
        }
      else	// Add new place to database
         Plc_CreatePlace (Plc);
     }
   else	// If there is not a place name
      Ale_ShowAlert (Ale_WARNING,Txt_You_must_specify_the_short_name_and_the_full_name_of_the_new_place);

   /***** Show the form again *****/
   Plc_EditPlaces ();
  }

/*****************************************************************************/
/**************************** Create a new place *****************************/
/*****************************************************************************/

static void Plc_CreatePlace (struct Place *Plc)
  {
   extern const char *Txt_Created_new_place_X;
   char Query[256 +
              Plc_MAX_BYTES_PLACE_SHRT_NAME +
              Plc_MAX_BYTES_PLACE_FULL_NAME];

   /***** Create a new place *****/
   sprintf (Query,"INSERT INTO places"
	          " (InsCod,ShortName,FullName)"
	          " VALUES"
	          " (%ld,'%s','%s')",
            Gbl.CurrentIns.Ins.InsCod,Plc->ShrtName,Plc->FullName);
   DB_QueryINSERT (Query,"can not create place");

   /***** Write success message *****/
   sprintf (Gbl.Alert.Txt,Txt_Created_new_place_X,
            Plc->FullName);
   Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
  }
