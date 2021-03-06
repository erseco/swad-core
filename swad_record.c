// swad_record.c: users' record cards

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
/********************************* Headers ***********************************/
/*****************************************************************************/

#include <linux/limits.h>	// For PATH_MAX
#include <linux/stddef.h>	// For NULL
#include <stdlib.h>		// For calloc
#include <string.h>

#include "swad_action.h"
#include "swad_config.h"
#include "swad_database.h"
#include "swad_enrolment.h"
#include "swad_follow.h"
#include "swad_global.h"
#include "swad_ID.h"
#include "swad_logo.h"
#include "swad_network.h"
#include "swad_parameter.h"
#include "swad_photo.h"
#include "swad_preference.h"
#include "swad_privacy.h"
#include "swad_QR.h"
#include "swad_record.h"
#include "swad_role.h"
#include "swad_user.h"

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

#define Rec_INSTITUTION_LOGO_SIZE	64
#define Rec_DEGREE_LOGO_SIZE		64

#define Rec_USR_MIN_AGE  12	// years old
#define Rec_USR_MAX_AGE 120	// years old

#define Rec_SHOW_OFFICE_HOURS_DEFAULT	true

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

static void Rec_WriteHeadingRecordFields (void);

static void Rec_PutParamFielCod (void);
static void Rec_GetFieldByCod (long FieldCod,char Name[Rec_MAX_BYTES_NAME_FIELD + 1],
                               unsigned *NumLines,Rec_VisibilityRecordFields_t *Visibility);

static void Rec_ListRecordsGsts (Rec_SharedRecordViewType_t TypeOfView);

static void Rec_ShowRecordOneStdCrs (void);
static void Rec_ListRecordsStds (Rec_SharedRecordViewType_t ShaTypeOfView,
                                 Rec_CourseRecordViewType_t CrsTypeOfView);

static void Rec_ShowRecordOneTchCrs (void);
static void Rec_ListRecordsTchs (Rec_SharedRecordViewType_t TypeOfView);

static void Rec_ShowLinkToPrintPreviewOfRecords (void);
static void Rec_GetParamRecordsPerPage (void);
static void Rec_WriteFormShowOfficeHoursOneTch (bool ShowOfficeHours);
static void Rec_WriteFormShowOfficeHoursSeveralTchs (bool ShowOfficeHours);
static void Rec_PutParamsShowOfficeHoursOneTch (void);
static void Rec_PutParamsShowOfficeHoursSeveralTchs (void);
static bool Rec_GetParamShowOfficeHours (void);
static void Rec_ShowCrsRecord (Rec_CourseRecordViewType_t TypeOfView,
                               struct UsrData *UsrDat,const char *Anchor);
static void Rec_ShowMyCrsRecordUpdated (void);
static bool Rec_CheckIfICanEditField (Rec_VisibilityRecordFields_t Visibility);

static void Rec_PutIconsCommands (void);
static void Rec_PutParamUsrCodEncrypted (void);
static void Rec_PutParamsWorks (void);
static void Rec_PutParamsStudent (void);
static void Rec_PutParamsMsgUsr (void);
static void Rec_ShowInstitutionInHead (struct Instit *Ins,bool PutFormLinks);
static void Rec_ShowPhoto (struct UsrData *UsrDat);
static void Rec_ShowFullName (struct UsrData *UsrDat);
static void Rec_ShowNickname (struct UsrData *UsrDat,bool PutFormLinks);
static void Rec_ShowCountryInHead (struct UsrData *UsrDat,bool ShowData);
static void Rec_ShowWebsAndSocialNets (struct UsrData *UsrDat,
                                       Rec_SharedRecordViewType_t TypeOfView);
static void Rec_ShowEmail (struct UsrData *UsrDat,const char *ClassForm);
static void Rec_ShowUsrIDs (struct UsrData *UsrDat,const char *ClassForm,
                            const char *Anchor);
static void Rec_ShowRole (struct UsrData *UsrDat,
                          Rec_SharedRecordViewType_t TypeOfView,
                          const char *ClassForm);
static void Rec_ShowSurname1 (struct UsrData *UsrDat,
                              Rec_SharedRecordViewType_t TypeOfView,
                              bool ICanEdit,
                              const char *ClassForm);
static void Rec_ShowSurname2 (struct UsrData *UsrDat,
                              bool ICanEdit,
                              const char *ClassForm);
static void Rec_ShowFirstName (struct UsrData *UsrDat,
                               Rec_SharedRecordViewType_t TypeOfView,
                               bool ICanEdit,
                               const char *ClassForm);
static void Rec_ShowCountry (struct UsrData *UsrDat,
                             Rec_SharedRecordViewType_t TypeOfView,
                             const char *ClassForm);
static void Rec_ShowOriginPlace (struct UsrData *UsrDat,
                                 bool ShowData,bool ICanEdit,
                                 const char *ClassForm);
static void Rec_ShowDateOfBirth (struct UsrData *UsrDat,
                                 bool ShowData,bool ICanEdit,
                                 const char *ClassForm);
static void Rec_ShowLocalAddress (struct UsrData *UsrDat,
                                  bool ShowData,bool ICanEdit,
                                  const char *ClassForm);
static void Rec_ShowLocalPhone (struct UsrData *UsrDat,
                                bool ShowData,bool ICanEdit,
                                const char *ClassForm);
static void Rec_ShowFamilyAddress (struct UsrData *UsrDat,
                                   bool ShowData,bool ICanEdit,
                                   const char *ClassForm);
static void Rec_ShowFamilyPhone (struct UsrData *UsrDat,
                                 bool ShowData,bool ICanEdit,
                                 const char *ClassForm);
static void Rec_ShowComments (struct UsrData *UsrDat,
                              bool ShowData,bool ICanEdit,
                              const char *ClassForm);
static void Rec_ShowInstitution (struct Instit *Ins,
                                 bool ShowData,const char *ClassForm);
static void Rec_ShowCentre (struct UsrData *UsrDat,
                            bool ShowData,const char *ClassForm);
static void Rec_ShowDepartment (struct UsrData *UsrDat,
                                bool ShowData,const char *ClassForm);
static void Rec_ShowOffice (struct UsrData *UsrDat,
                            bool ShowData,const char *ClassForm);
static void Rec_ShowOfficePhone (struct UsrData *UsrDat,
                                 bool ShowData,const char *ClassForm);

static void Rec_WriteLinkToDataProtectionClause (void);

static void Rec_GetUsrExtraDataFromRecordForm (struct UsrData *UsrDat);
static void Rec_GetUsrCommentsFromForm (struct UsrData *UsrDat);

/*****************************************************************************/
/*************** Create, edit and remove fields of records *******************/
/*****************************************************************************/

void Rec_ReqEditRecordFields (void)
  {
   extern const char *Hlp_USERS_Students_course_record_card;
   extern const char *Txt_There_are_no_record_fields_in_the_course_X;
   extern const char *Txt_Record_fields;

   /***** Get list of fields of records in current course *****/
   Rec_GetListRecordFieldsInCurrentCrs ();

   /***** List the current fields of records for edit them *****/
   if (Gbl.CurrentCrs.Records.LstFields.Num)	// Fields found...
     {
      Lay_StartRoundFrameTable (NULL,Txt_Record_fields,NULL,
                                Hlp_USERS_Students_course_record_card,2);
      Rec_ListFieldsRecordsForEdition ();
      Lay_EndRoundFrameTable ();
     }
   else	// No fields of records found for current course in the database
     {
      sprintf (Gbl.Alert.Txt,Txt_There_are_no_record_fields_in_the_course_X,
               Gbl.CurrentCrs.Crs.FullName);
      Ale_ShowAlert (Ale_INFO,Gbl.Alert.Txt);
     }

   /***** Put a form to create a new record field *****/
   Rec_ShowFormCreateRecordField ();

   /* Free list of fields of records */
   Rec_FreeListFields ();
  }

/*****************************************************************************/
/****** Create a list with the fields of records from current course *********/
/*****************************************************************************/

void Rec_GetListRecordFieldsInCurrentCrs (void)
  {
   char Query[256];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRow;
   unsigned Vis;

   if (++Gbl.CurrentCrs.Records.LstFields.NestedCalls > 1) // If the list is already created, don't do anything
      return;

   /***** Get fields of cards of a course from database *****/
   sprintf (Query,"SELECT FieldCod,FieldName,NumLines,Visibility"
	          " FROM crs_record_fields"
                  " WHERE CrsCod=%ld ORDER BY FieldName",
            Gbl.CurrentCrs.Crs.CrsCod);
   Gbl.CurrentCrs.Records.LstFields.Num = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get fields of cards of a course");

   /***** Get the fields of records *****/
   if (Gbl.CurrentCrs.Records.LstFields.Num)
     {
      /***** Create a list of fields *****/
      if ((Gbl.CurrentCrs.Records.LstFields.Lst = (struct RecordField *) calloc (Gbl.CurrentCrs.Records.LstFields.Num,sizeof (struct RecordField))) == NULL)
         Lay_ShowErrorAndExit ("Not enough memory to store fields of records in current course.");

      /***** Get the fields *****/
      for (NumRow = 0;
	   NumRow < Gbl.CurrentCrs.Records.LstFields.Num;
	   NumRow++)
        {
         /* Get next field */
         row = mysql_fetch_row (mysql_res);

         /* Get the code of field (row[0]) */
         if ((Gbl.CurrentCrs.Records.LstFields.Lst[NumRow].FieldCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
            Lay_ShowErrorAndExit ("Wrong code of field.");

         /* Name of the field (row[1]) */
         Str_Copy (Gbl.CurrentCrs.Records.LstFields.Lst[NumRow].Name,row[1],
                   Rec_MAX_BYTES_NAME_FIELD);

         /* Number of lines (row[2]) */
         Gbl.CurrentCrs.Records.LstFields.Lst[NumRow].NumLines = Rec_ConvertToNumLinesField (row[2]);

         /* Visible or editable by students? (row[3]) */
         if (sscanf (row[3],"%u",&Vis) != 1)
	    Lay_ShowErrorAndExit ("Error when getting field of record in current course.");
         if (Vis < Rec_NUM_TYPES_VISIBILITY)
            Gbl.CurrentCrs.Records.LstFields.Lst[NumRow].Visibility = (Rec_VisibilityRecordFields_t) Vis;
         else
            Gbl.CurrentCrs.Records.LstFields.Lst[NumRow].Visibility = Rec_VISIBILITY_DEFAULT;
        }
     }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/********* List the fields of records already present in database ************/
/*****************************************************************************/

void Rec_ListFieldsRecordsForEdition (void)
  {
   extern const char *Txt_RECORD_FIELD_VISIBILITY_MENU[Rec_NUM_TYPES_VISIBILITY];
   unsigned NumField;
   Rec_VisibilityRecordFields_t Vis;

   /***** Write heading *****/
   Rec_WriteHeadingRecordFields ();

   /***** List the fields *****/
   for (NumField = 0;
	NumField < Gbl.CurrentCrs.Records.LstFields.Num;
	NumField++)
     {
      fprintf (Gbl.F.Out,"<tr>");

      /* Write icon to remove the field */
      fprintf (Gbl.F.Out,"<td class=\"BM\">");
      Act_FormStart (ActReqRemFie);
      Par_PutHiddenParamLong ("FieldCod",Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod);
      Lay_PutIconRemove ();
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Name of the field */
      fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">");
      Act_FormStart (ActRenFie);
      Par_PutHiddenParamLong ("FieldCod",Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"FieldName\""
	                 " style=\"width:500px;\" maxlength=\"%u\" value=\"%s\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Rec_MAX_CHARS_NAME_FIELD,
               Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Name,
               Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Number of lines in the form */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActChgRowFie);
      Par_PutHiddenParamLong ("FieldCod",Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod);
      fprintf (Gbl.F.Out,"<input type=\"text\" name=\"NumLines\""
	                 " size=\"2\" maxlength=\"2\" value=\"%u\""
                         " onchange=\"document.getElementById('%s').submit();\" />",
               Gbl.CurrentCrs.Records.LstFields.Lst[NumField].NumLines,
               Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>");

      /* Visibility of a field */
      fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">");
      Act_FormStart (ActChgVisFie);
      Par_PutHiddenParamLong ("FieldCod",Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod);
      fprintf (Gbl.F.Out,"<select name=\"Visibility\""
                         " onchange=\"document.getElementById('%s').submit();\">",
               Gbl.Form.Id);
      for (Vis = (Rec_VisibilityRecordFields_t) 0;
	   Vis < (Rec_VisibilityRecordFields_t) Rec_NUM_TYPES_VISIBILITY;
	   Vis++)
        {
         fprintf (Gbl.F.Out,"<option value=\"%u\"",(unsigned) Vis);
         if (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility == Vis)
	    fprintf (Gbl.F.Out," selected=\"selected\"");
         fprintf (Gbl.F.Out,">%s</option>",
                  Txt_RECORD_FIELD_VISIBILITY_MENU[Vis]);
        }
      fprintf (Gbl.F.Out,"</select>");
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");
     }
  }

/*****************************************************************************/
/******************* Show form to create a new record field ******************/
/*****************************************************************************/

void Rec_ShowFormCreateRecordField (void)
  {
   extern const char *Hlp_USERS_Students_course_record_card;
   extern const char *Txt_New_record_field;
   extern const char *Txt_RECORD_FIELD_VISIBILITY_MENU[Rec_NUM_TYPES_VISIBILITY];
   extern const char *Txt_Create_record_field;
   Rec_VisibilityRecordFields_t Vis;

   /***** Start form *****/
   Act_FormStart (ActNewFie);

   /***** Start of frame *****/
   Lay_StartRoundFrameTable (NULL,Txt_New_record_field,NULL,
                             Hlp_USERS_Students_course_record_card,2);

   /***** Write heading *****/
   Rec_WriteHeadingRecordFields ();

   /***** Write disabled icon to remove the field *****/
   fprintf (Gbl.F.Out,"<tr>"
                      "<td class=\"BM\">");
   Lay_PutIconRemovalNotAllowed ();
   fprintf (Gbl.F.Out,"</td>");

   /***** Field name *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">"
                      "<input type=\"text\" name=\"FieldName\""
                      " style=\"width:500px;\" maxlength=\"%u\" value=\"%s\""
                      " required=\"required\" />"
                      "</td>",
            Rec_MAX_CHARS_NAME_FIELD,Gbl.CurrentCrs.Records.Field.Name);

   /***** Number of lines in form ******/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
	              "<input type=\"text\" name=\"NumLines\""
	              " size=\"2\" maxlength=\"2\" value=\"%u\""
	              " required=\"required\" />"
	              "</td>",
            Gbl.CurrentCrs.Records.Field.NumLines);

   /***** Visibility to students *****/
   fprintf (Gbl.F.Out,"<td class=\"CENTER_MIDDLE\">"
	              "<select name=\"Visibility\">");
   for (Vis = (Rec_VisibilityRecordFields_t) 0;
	Vis < (Rec_VisibilityRecordFields_t) Rec_NUM_TYPES_VISIBILITY;
	Vis++)
     {
      fprintf (Gbl.F.Out,"<option value=\"%u\"",(unsigned) Vis);
      if (Gbl.CurrentCrs.Records.Field.Visibility == Vis)
         fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",
	       Txt_RECORD_FIELD_VISIBILITY_MENU[Vis]);
     }

   fprintf (Gbl.F.Out,"</select>"
	              "</td>"
	              "</tr>");

   /***** Send button and end frame *****/
   Lay_EndRoundFrameTableWithButton (Lay_CREATE_BUTTON,Txt_Create_record_field);

   /***** End form *****/
   Act_FormEnd ();
  }

/*****************************************************************************/
/************************** Write heading of groups **************************/
/*****************************************************************************/

static void Rec_WriteHeadingRecordFields (void)
  {
   extern const char *Txt_Field_BR_name;
   extern const char *Txt_No_of_BR_lines;
   extern const char *Txt_Visible_by_BR_the_student;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th></th>"
                      "<th class=\"CENTER_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"CENTER_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"CENTER_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Field_BR_name,
            Txt_No_of_BR_lines,
            Txt_Visible_by_BR_the_student);
  }

/*****************************************************************************/
/*************** Receive data from a form of record fields *******************/
/*****************************************************************************/

void Rec_ReceiveFormField (void)
  {
   extern const char *Txt_The_record_field_X_already_exists;
   extern const char *Txt_You_must_specify_the_name_of_the_new_record_field;

   /***** Get parameters from the form *****/
   /* Get the name of the field */
   Par_GetParToText ("FieldName",Gbl.CurrentCrs.Records.Field.Name,
                     Rec_MAX_BYTES_NAME_FIELD);

   /* Get the number of lines */
   Gbl.CurrentCrs.Records.Field.NumLines = (unsigned)
	                                   Par_GetParToUnsignedLong ("NumLines",
                                                                     Rec_MIN_LINES_IN_EDITION_FIELD,
                                                                     Rec_MAX_LINES_IN_EDITION_FIELD,
                                                                     Rec_DEF_LINES_IN_EDITION_FIELD);

   /* Get the field visibility by students */
   Gbl.CurrentCrs.Records.Field.Visibility = (Rec_VisibilityRecordFields_t)
	                                     Par_GetParToUnsignedLong ("Visibility",
                                                                       0,
                                                                       Rec_NUM_TYPES_VISIBILITY - 1,
                                                                       (unsigned long) Rec_VISIBILITY_DEFAULT);

   if (Gbl.CurrentCrs.Records.Field.Name[0])	// If there's a name
     {
      /***** If the field already was in the database... *****/
      if (Rec_CheckIfRecordFieldIsRepeated (Gbl.CurrentCrs.Records.Field.Name))
        {
         sprintf (Gbl.Alert.Txt,Txt_The_record_field_X_already_exists,
                  Gbl.CurrentCrs.Records.Field.Name);
         Ale_ShowAlert (Ale_ERROR,Gbl.Alert.Txt);
        }
      else	// Add the new field to the database
         Rec_CreateRecordField ();
     }
   else		// If there is not name
      Ale_ShowAlert (Ale_ERROR,Txt_You_must_specify_the_name_of_the_new_record_field);

   /***** Show the form again *****/
   Rec_ReqEditRecordFields ();
  }

/*****************************************************************************/
/********* Get number of lines of the form to edit a record field ************/
/*****************************************************************************/

unsigned Rec_ConvertToNumLinesField (const char *StrNumLines)
  {
   int NumLines;

   if (sscanf (StrNumLines,"%d",&NumLines) != 1)
      return Rec_DEF_LINES_IN_EDITION_FIELD;
   else if (NumLines < Rec_MIN_LINES_IN_EDITION_FIELD)
      return Rec_MIN_LINES_IN_EDITION_FIELD;
   else if (NumLines > Rec_MAX_LINES_IN_EDITION_FIELD)
      return Rec_MAX_LINES_IN_EDITION_FIELD;
   return (unsigned) NumLines;
  }

/*****************************************************************************/
/* Check if the name of the field of record equals any of the existing ones **/
/*****************************************************************************/

bool Rec_CheckIfRecordFieldIsRepeated (const char *FieldName)
  {
   bool FieldIsRepeated = false;
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRow,NumRows;

   /* Query database */
   if ((NumRows = Rec_GetAllFieldsInCurrCrs (&mysql_res)) > 0)	// If se han encontrado groups...
      /* Compare with all the tipos of group from the database */
      for (NumRow = 0;
	   NumRow < NumRows;
	   NumRow++)
        {
	 /* Get next type of group */
	 row = mysql_fetch_row (mysql_res);

         /* The name of the field is in row[1] */
         if (!strcasecmp (FieldName,row[1]))
           {
            FieldIsRepeated = true;
            break;
           }
	}

   /* Free structure that stores the query result */
   DB_FreeMySQLResult (&mysql_res);

   return FieldIsRepeated;
  }

/*****************************************************************************/
/******* Get the fields of records already present in current course *********/
/*****************************************************************************/

unsigned long Rec_GetAllFieldsInCurrCrs (MYSQL_RES **mysql_res)
  {
   char Query[256];

   /***** Get fields of cards of current course from database *****/
   sprintf (Query,"SELECT FieldCod,FieldName,Visibility"
	          " FROM crs_record_fields"
                  " WHERE CrsCod=%ld ORDER BY FieldName",
            Gbl.CurrentCrs.Crs.CrsCod);
   return DB_QuerySELECT (Query,mysql_res,
                          "can not get fields of cards of a course");
  }

/*****************************************************************************/
/************************* Create a field of record **************************/
/*****************************************************************************/

void Rec_CreateRecordField (void)
  {
   extern const char *Txt_Created_new_record_field_X;
   char Query[256 + Rec_MAX_BYTES_NAME_FIELD];

   /***** Create the new field *****/
   sprintf (Query,"INSERT INTO crs_record_fields"
	          " (CrsCod,FieldName,NumLines,Visibility)"
	          " VALUES"
	          " (%ld,'%s',%u,%u)",
            Gbl.CurrentCrs.Crs.CrsCod,
            Gbl.CurrentCrs.Records.Field.Name,
            Gbl.CurrentCrs.Records.Field.NumLines,
            (unsigned) Gbl.CurrentCrs.Records.Field.Visibility);
   DB_QueryINSERT (Query,"can not create field of record");

   /***** Write message of success *****/
   sprintf (Gbl.Alert.Txt,Txt_Created_new_record_field_X,
            Gbl.CurrentCrs.Records.Field.Name);
   Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
  }

/*****************************************************************************/
/**************** Request the removing of a field of records *****************/
/*****************************************************************************/

void Rec_ReqRemField (void)
  {
   unsigned NumRecords;

   /***** Get the code of field *****/
   if ((Gbl.CurrentCrs.Records.Field.FieldCod = Rec_GetFieldCod ()) == -1)
      Lay_ShowErrorAndExit ("Code of field is missing.");

   /***** Check if exists any record with that field filled *****/
   if ((NumRecords = Rec_CountNumRecordsInCurrCrsWithField (Gbl.CurrentCrs.Records.Field.FieldCod)))	// There are records with that field filled
      Rec_AskConfirmRemFieldWithRecords (NumRecords);
   else			// There are no records with that field filled
      Rec_RemoveFieldFromDB ();
  }

/*****************************************************************************/
/************ Get a parameter with a code of field of records ****************/
/*****************************************************************************/

long Rec_GetFieldCod (void)
  {
   /***** Get the code of the field *****/
   return Par_GetParToLong ("FieldCod");
  }

/*****************************************************************************/
/*************** Get the number of records with a field filled ***************/
/*****************************************************************************/

unsigned Rec_CountNumRecordsInCurrCrsWithField (long FieldCod)
  {
   char Query[128];

   /***** Get number of cards with a given field in a course from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM crs_records WHERE FieldCod=%ld",
            FieldCod);
   return (unsigned) DB_QueryCOUNT (Query,"can not get number of cards"
	                                  " with a given field not empty"
	                                  " in a course");
  }

/*****************************************************************************/
/******* Request confirmation for the removing of a field with records *******/
/*****************************************************************************/

void Rec_AskConfirmRemFieldWithRecords (unsigned NumRecords)
  {
   extern const char *Txt_Do_you_really_want_to_remove_the_field_X_from_the_records_of_X;
   extern const char *Txt_this_field_is_filled_in_the_record_of_one_student;
   extern const char *Txt_this_field_is_filled_in_the_records_of_X_students;
   extern const char *Txt_Remove_record_field;
   char Message_part2[512];

   /***** Get from the database the name of the field *****/
   Rec_GetFieldByCod (Gbl.CurrentCrs.Records.Field.FieldCod,
                      Gbl.CurrentCrs.Records.Field.Name,
                      &Gbl.CurrentCrs.Records.Field.NumLines,
                      &Gbl.CurrentCrs.Records.Field.Visibility);

   /***** Show question and button to remove my photo *****/
   sprintf (Gbl.Alert.Txt,Txt_Do_you_really_want_to_remove_the_field_X_from_the_records_of_X,
            Gbl.CurrentCrs.Records.Field.Name,Gbl.CurrentCrs.Crs.FullName);
   if (NumRecords == 1)
      Str_Concat (Gbl.Alert.Txt,Txt_this_field_is_filled_in_the_record_of_one_student,
                  Ale_MAX_BYTES_ALERT);
   else
     {
      sprintf (Message_part2,Txt_this_field_is_filled_in_the_records_of_X_students,
               NumRecords);
      Str_Concat (Gbl.Alert.Txt,Message_part2,
                  Ale_MAX_BYTES_ALERT);
     }
   Ale_ShowAlertAndButton (Ale_QUESTION,Gbl.Alert.Txt,
                           ActRemFie,NULL,NULL,Rec_PutParamFielCod,
			   Lay_REMOVE_BUTTON,Txt_Remove_record_field);

   /***** List record fields again *****/
   Rec_ReqEditRecordFields ();
  }

/*****************************************************************************/
/************** Remove from the database a field of records ******************/
/*****************************************************************************/

void Rec_RemoveFieldFromDB (void)
  {
   extern const char *Txt_Record_field_X_removed;
   char Query[128];

   /***** Get from the database the name of the field *****/
   Rec_GetFieldByCod (Gbl.CurrentCrs.Records.Field.FieldCod,
                      Gbl.CurrentCrs.Records.Field.Name,
                      &Gbl.CurrentCrs.Records.Field.NumLines,
                      &Gbl.CurrentCrs.Records.Field.Visibility);

   /***** Remove field from all records *****/
   sprintf (Query,"DELETE FROM crs_records WHERE FieldCod=%ld",
            Gbl.CurrentCrs.Records.Field.FieldCod);
   DB_QueryDELETE (Query,"can not remove field from all students' records");

   /***** Remove the field *****/
   sprintf (Query,"DELETE FROM crs_record_fields WHERE FieldCod=%ld",
            Gbl.CurrentCrs.Records.Field.FieldCod);
   DB_QueryDELETE (Query,"can not remove field of record");

   /***** Write message to show the change made *****/
   sprintf (Gbl.Alert.Txt,Txt_Record_field_X_removed,
            Gbl.CurrentCrs.Records.Field.Name);
   Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);

   /***** Show the form again *****/
   Rec_ReqEditRecordFields ();
  }

/*****************************************************************************/
/********************** Put parameter with field code ************************/
/*****************************************************************************/

static void Rec_PutParamFielCod (void)
  {
   Par_PutHiddenParamLong ("FieldCod",Gbl.CurrentCrs.Records.Field.FieldCod);
  }

/*****************************************************************************/
/************** Get the data of a field of records from its code *************/
/*****************************************************************************/

static void Rec_GetFieldByCod (long FieldCod,char Name[Rec_MAX_BYTES_NAME_FIELD + 1],
                               unsigned *NumLines,Rec_VisibilityRecordFields_t *Visibility)
  {
   char Query[256];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;
   unsigned Vis;

   /***** Get a field of a record in a course from database *****/
   sprintf (Query,"SELECT FieldName,NumLines,Visibility FROM crs_record_fields"
                  " WHERE CrsCod=%ld AND FieldCod=%ld",
            Gbl.CurrentCrs.Crs.CrsCod,FieldCod);
   NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get a field of a record in a course");

   /***** Count number of rows in result *****/
   if (NumRows != 1)
      Lay_ShowErrorAndExit ("Error when getting a field of a record in a course.");

   /***** Get the field *****/
   row = mysql_fetch_row (mysql_res);

   /* Name of the field */
   Str_Copy (Name,row[0],
             Rec_MAX_BYTES_NAME_FIELD);

   /* Number of lines of the field (row[1]) */
   *NumLines = Rec_ConvertToNumLinesField (row[1]);

   /* Visible or editable by students? (row[2]) */
   if (sscanf (row[2],"%u",&Vis) != 1)
      Lay_ShowErrorAndExit ("Error when getting a field of a record in a course.");
   if (Vis < Rec_NUM_TYPES_VISIBILITY)
      *Visibility = (Rec_VisibilityRecordFields_t) Vis;
   else
      *Visibility = Rec_VISIBILITY_DEFAULT;

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/************************* Remove a field of records *************************/
/*****************************************************************************/

void Rec_RemoveField (void)
  {
   /***** Get the code of the field *****/
   if ((Gbl.CurrentCrs.Records.Field.FieldCod = Rec_GetFieldCod ()) == -1)
      Lay_ShowErrorAndExit ("Code of field is missing.");

   /***** Borrarlo from the database *****/
   Rec_RemoveFieldFromDB ();
  }

/*****************************************************************************/
/************************** Rename a field of records ************************/
/*****************************************************************************/

void Rec_RenameField (void)
  {
   extern const char *Txt_You_can_not_leave_the_name_of_the_field_X_empty;
   extern const char *Txt_The_record_field_X_already_exists;
   extern const char *Txt_The_record_field_X_has_been_renamed_as_Y;
   extern const char *Txt_The_name_of_the_field_X_has_not_changed;
   char Query[256 + Rec_MAX_BYTES_NAME_FIELD];
   char NewFieldName[Rec_MAX_BYTES_NAME_FIELD + 1];

   /***** Get parameters of the form *****/
   /* Get the code of the field */
   if ((Gbl.CurrentCrs.Records.Field.FieldCod = Rec_GetFieldCod ()) == -1)
      Lay_ShowErrorAndExit ("Code of field is missing.");

   /* Get the new group name */
   Par_GetParToText ("FieldName",NewFieldName,Rec_MAX_BYTES_NAME_FIELD);

   /***** Get from the database the antiguo group name *****/
   Rec_GetFieldByCod (Gbl.CurrentCrs.Records.Field.FieldCod,
                      Gbl.CurrentCrs.Records.Field.Name,
                      &Gbl.CurrentCrs.Records.Field.NumLines,
                      &Gbl.CurrentCrs.Records.Field.Visibility);

   /***** Check if new name is empty *****/
   if (!NewFieldName[0])
     {
      sprintf (Gbl.Alert.Txt,Txt_You_can_not_leave_the_name_of_the_field_X_empty,
               Gbl.CurrentCrs.Records.Field.Name);
      Ale_ShowAlert (Ale_ERROR,Gbl.Alert.Txt);
     }
   else
     {
      /***** Check if the name of the olde field match the new one
             (this happens when return is pressed without changes) *****/
      if (strcmp (Gbl.CurrentCrs.Records.Field.Name,NewFieldName))	// Different names
        {
         /***** If the group ya estaba in the database... *****/
         if (Rec_CheckIfRecordFieldIsRepeated (NewFieldName))
           {
            sprintf (Gbl.Alert.Txt,Txt_The_record_field_X_already_exists,
                     NewFieldName);
            Ale_ShowAlert (Ale_ERROR,Gbl.Alert.Txt);
           }
         else
           {
            /* Update the table of fields changing then old name by the new one */
            sprintf (Query,"UPDATE crs_record_fields SET FieldName='%s'"
        	           " WHERE FieldCod=%ld",
                     NewFieldName,Gbl.CurrentCrs.Records.Field.FieldCod);
            DB_QueryUPDATE (Query,"can not update name of field of record");

            /***** Write message to show the change made *****/
            sprintf (Gbl.Alert.Txt,Txt_The_record_field_X_has_been_renamed_as_Y,
                     Gbl.CurrentCrs.Records.Field.Name,NewFieldName);
            Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
           }
        }
      else	// The same name
        {
         sprintf (Gbl.Alert.Txt,Txt_The_name_of_the_field_X_has_not_changed,
                  NewFieldName);
         Ale_ShowAlert (Ale_INFO,Gbl.Alert.Txt);
        }
     }

   /***** Show the form again *****/
   Str_Copy (Gbl.CurrentCrs.Records.Field.Name,NewFieldName,
             Rec_MAX_BYTES_NAME_FIELD);
   Rec_ReqEditRecordFields ();
  }

/*****************************************************************************/
/********* Change number of lines of the form of a field of records **********/
/*****************************************************************************/

void Rec_ChangeLinesField (void)
  {
   extern const char *Txt_The_number_of_editing_lines_in_the_record_field_X_has_not_changed;
   extern const char *Txt_From_now_on_the_number_of_editing_lines_of_the_field_X_is_Y;
   char Query[256];
   unsigned NewNumLines;

   /***** Get parameters of the form *****/
   /* Get the code of field */
   if ((Gbl.CurrentCrs.Records.Field.FieldCod = Rec_GetFieldCod ()) == -1)
      Lay_ShowErrorAndExit ("Code of field is missing.");

   /* Get the new number of lines */
   NewNumLines = (unsigned)
	         Par_GetParToUnsignedLong ("NumLines",
                                           Rec_MIN_LINES_IN_EDITION_FIELD,
                                           Rec_MAX_LINES_IN_EDITION_FIELD,
                                           Rec_DEF_LINES_IN_EDITION_FIELD);

   /* Get from the database the number of lines of the field */
   Rec_GetFieldByCod (Gbl.CurrentCrs.Records.Field.FieldCod,Gbl.CurrentCrs.Records.Field.Name,&Gbl.CurrentCrs.Records.Field.NumLines,&Gbl.CurrentCrs.Records.Field.Visibility);

   /***** Check if the old number of rows antiguo match the new one
          (this happens when return is pressed without changes) *****/
   if (Gbl.CurrentCrs.Records.Field.NumLines == NewNumLines)
     {
      sprintf (Gbl.Alert.Txt,Txt_The_number_of_editing_lines_in_the_record_field_X_has_not_changed,
               Gbl.CurrentCrs.Records.Field.Name);
      Ale_ShowAlert (Ale_INFO,Gbl.Alert.Txt);
     }
   else
     {
      /***** Update of the table of fields changing the old maximum of students by the new one *****/
      sprintf (Query,"UPDATE crs_record_fields SET NumLines=%u"
	             " WHERE FieldCod=%ld",
               NewNumLines,Gbl.CurrentCrs.Records.Field.FieldCod);
      DB_QueryUPDATE (Query,"can not update the number of lines of a field of record");

      /***** Write message to show the change made *****/
      sprintf (Gbl.Alert.Txt,Txt_From_now_on_the_number_of_editing_lines_of_the_field_X_is_Y,
	       Gbl.CurrentCrs.Records.Field.Name,NewNumLines);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }

   /***** Show the form again *****/
   Gbl.CurrentCrs.Records.Field.NumLines = NewNumLines;
   Rec_ReqEditRecordFields ();
  }

/*****************************************************************************/
/************ Change wisibility by students of a field of records ************/
/*****************************************************************************/

void Rec_ChangeVisibilityField (void)
  {
   extern const char *Txt_The_visibility_of_the_record_field_X_has_not_changed;
   extern const char *Txt_RECORD_FIELD_VISIBILITY_MSG[Rec_NUM_TYPES_VISIBILITY];
   char Query[256];
   Rec_VisibilityRecordFields_t NewVisibility;

   /***** Get parameters of the form *****/
   /* Get the code of field */
   if ((Gbl.CurrentCrs.Records.Field.FieldCod = Rec_GetFieldCod ()) == -1)
      Lay_ShowErrorAndExit ("Code of field is missing.");

   /* Get the new visibility of the field */
   NewVisibility = (Rec_VisibilityRecordFields_t)
	           Par_GetParToUnsignedLong ("Visibility",
                                             0,
                                             Rec_NUM_TYPES_VISIBILITY - 1,
                                             (unsigned long) Rec_VISIBILITY_DEFAULT);

   /* Get from the database the visibility of the field */
   Rec_GetFieldByCod (Gbl.CurrentCrs.Records.Field.FieldCod,Gbl.CurrentCrs.Records.Field.Name,&Gbl.CurrentCrs.Records.Field.NumLines,&Gbl.CurrentCrs.Records.Field.Visibility);

   /***** Check if the old visibility matches the new one
          (this happens whe return is pressed without changes in the form) *****/
   if (Gbl.CurrentCrs.Records.Field.Visibility == NewVisibility)
     {
      sprintf (Gbl.Alert.Txt,Txt_The_visibility_of_the_record_field_X_has_not_changed,
               Gbl.CurrentCrs.Records.Field.Name);
      Ale_ShowAlert (Ale_INFO,Gbl.Alert.Txt);
     }
   else
     {
      /***** Update of the table of fields changing the old visibility by the new *****/
      sprintf (Query,"UPDATE crs_record_fields SET Visibility=%u"
	             " WHERE FieldCod=%ld",
               (unsigned) NewVisibility,Gbl.CurrentCrs.Records.Field.FieldCod);
      DB_QueryUPDATE (Query,"can not update the visibility of a field of record");

      /***** Write message to show the change made *****/
      sprintf (Gbl.Alert.Txt,Txt_RECORD_FIELD_VISIBILITY_MSG[NewVisibility],
	       Gbl.CurrentCrs.Records.Field.Name);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }

   /***** Show the form again *****/
   Gbl.CurrentCrs.Records.Field.Visibility = NewVisibility;
   Rec_ReqEditRecordFields ();
  }

/*****************************************************************************/
/********************** Liberar list of fields of records ********************/
/*****************************************************************************/

void Rec_FreeListFields (void)
  {
   if (Gbl.CurrentCrs.Records.LstFields.NestedCalls > 0)
      if (--Gbl.CurrentCrs.Records.LstFields.NestedCalls == 0)
         if (Gbl.CurrentCrs.Records.LstFields.Lst)
           {
            free ((void *) Gbl.CurrentCrs.Records.LstFields.Lst);
            Gbl.CurrentCrs.Records.LstFields.Lst = NULL;
            Gbl.CurrentCrs.Records.LstFields.Num = 0;
           }
  }

/*****************************************************************************/
/******************* Put a link to list official students ********************/
/*****************************************************************************/

void Rec_PutLinkToEditRecordFields (void)
  {
   extern const char *Txt_Edit_record_fields;

   /***** Link to edit record fields *****/
   Lay_PutContextualLink (ActEdiRecFie,NULL,NULL,
                          "edit64x64.png",
                          Txt_Edit_record_fields,Txt_Edit_record_fields,
		          NULL);
  }

/*****************************************************************************/
/*********************** Draw records of several guests **********************/
/*****************************************************************************/

void Rec_ListRecordsGstsShow (void)
  {
   Gbl.Action.Original = ActSeeRecSevGst;	// Used to know where to go when confirming ID
   Rec_ListRecordsGsts (Rec_SHA_RECORD_LIST);
  }

void Rec_ListRecordsGstsPrint (void)
  {
   Rec_ListRecordsGsts (Rec_SHA_RECORD_PRINT);
  }

static void Rec_ListRecordsGsts (Rec_SharedRecordViewType_t TypeOfView)
  {
   extern const char *Txt_You_must_select_one_ore_more_users;
   unsigned NumUsr = 0;
   const char *Ptr;
   struct UsrData UsrDat;
   char RecordSectionId[32];

   /***** Assign users listing type depending on current action *****/
   Gbl.Usrs.Listing.RecsUsrs = Rec_RECORD_USERS_GUESTS;

   /***** Get parameter with number of user records per page (only for printing) *****/
   if (TypeOfView == Rec_SHA_RECORD_PRINT)
      Rec_GetParamRecordsPerPage ();

   /***** Get list of selected users *****/
   Usr_GetListsSelectedUsrsCods ();

   /* Check the number of students to show */
   if (!Usr_CountNumUsrsInListOfSelectedUsrs ())	// If no students selected...
     {						// ...write warning notice
      Ale_ShowAlert (Ale_WARNING,Txt_You_must_select_one_ore_more_users);
      Usr_SeeGuests ();			// ...show again the form
      return;
     }

   if (TypeOfView == Rec_SHA_RECORD_LIST)	// Listing several records
     {
      fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");

      /* Link to print view */
      Act_FormStart (ActPrnRecSevGst);
      Usr_PutHiddenParUsrCodAll (ActPrnRecSevGst,Gbl.Usrs.Select[Rol_UNK]);
      Rec_ShowLinkToPrintPreviewOfRecords ();
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</div>");
     }

   /***** Initialize structure with user's data *****/
   Usr_UsrDataConstructor (&UsrDat);
   UsrDat.Accepted = false;	// Guests have no courses,...
				// ...so they have not accepted...
				// ...inscription in any course

   /***** List the records *****/
   Ptr = Gbl.Usrs.Select[Rol_UNK];
   while (*Ptr)
     {
      Par_GetNextStrUntilSeparParamMult (&Ptr,UsrDat.EncryptedUsrCod,
                                         Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64);
      Usr_GetUsrCodFromEncryptedUsrCod (&UsrDat);
      if (Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (&UsrDat))                // Get from the database the data of the student
	{
         /* Start container for this user */
	 sprintf (RecordSectionId,"record_%u",NumUsr);
	 Lay_StartSection (RecordSectionId);
	 fprintf (Gbl.F.Out,"<div class=\"REC_USR\"");
	 if (Gbl.Action.Act == ActPrnRecSevGst &&
	     NumUsr != 0 &&
	     (NumUsr % Gbl.Usrs.Listing.RecsPerPag) == 0)
	    fprintf (Gbl.F.Out," style=\"page-break-before:always;\"");
	 fprintf (Gbl.F.Out,">");

	 /* Show optional alert */
	 if (UsrDat.UsrCod == Gbl.Usrs.Other.UsrDat.UsrCod)	// Selected user
	    Ale_ShowPendingAlert ();

	 /* Shared record */
	 fprintf (Gbl.F.Out,"<div class=\"REC_SHA\">");
	 Rec_ShowSharedUsrRecord (TypeOfView,&UsrDat,RecordSectionId);
	 fprintf (Gbl.F.Out,"</div>");

         /* End container for this user */
	 fprintf (Gbl.F.Out,"</div>");
	 Lay_EndSection ();

	 NumUsr++;
	}
     }
   /***** Free memory used for user's data *****/
   Usr_UsrDataDestructor (&UsrDat);

   /***** Free memory used by list of selected users' codes *****/
   Usr_FreeListsSelectedUsrsCods ();
  }

/*****************************************************************************/
/********** Get user's data and draw record of one unique student ************/
/*****************************************************************************/

void Rec_GetUsrAndShowRecordOneStdCrs (void)
  {
   /***** Get the selected student *****/
   Usr_GetParamOtherUsrCodEncryptedAndGetListIDs ();

   if (Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (&Gbl.Usrs.Other.UsrDat))	// Get from the database the data of the student
      if (Usr_CheckIfICanViewRecordStd (&Gbl.Usrs.Other.UsrDat))
	 Rec_ShowRecordOneStdCrs ();
  }

/*****************************************************************************/
/******************** Draw record of one unique student **********************/
/*****************************************************************************/

static void Rec_ShowRecordOneStdCrs (void)
  {
   /***** Get if student has accepted enrolment in current course *****/
   Gbl.Usrs.Other.UsrDat.Accepted = Usr_CheckIfUsrBelongsToCrs (Gbl.Usrs.Other.UsrDat.UsrCod,
                                                                Gbl.CurrentCrs.Crs.CrsCod,
                                                                true);

   /***** Assign users listing type depending on current action *****/
   Gbl.Usrs.Listing.RecsUsrs = Rec_RECORD_USERS_STUDENTS;

   /***** Get list of fields of records in current course *****/
   Rec_GetListRecordFieldsInCurrentCrs ();

   /***** Put contextual links *****/
   fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");

   /* Link to edit record fields */
   if (Gbl.Usrs.Me.Role.Logged == Rol_TCH)
      Rec_PutLinkToEditRecordFields ();

   /* Link to print view */
   Act_FormStart (ActPrnRecSevStd);
   Usr_PutHiddenParUsrCodAll (ActPrnRecSevStd,Gbl.Usrs.Other.UsrDat.EncryptedUsrCod);
   Rec_ShowLinkToPrintPreviewOfRecords ();
   Act_FormEnd ();

   fprintf (Gbl.F.Out,"</div>");

   /***** Show optional alert (result of editing data in course record) *****/
   Ale_ShowPendingAlert ();

   /***** Start container for this user *****/
   fprintf (Gbl.F.Out,"<div class=\"REC_USR\">");

   /***** Shared record *****/
   fprintf (Gbl.F.Out,"<div class=\"REC_SHA\">");
   Rec_ShowSharedUsrRecord (Rec_SHA_RECORD_LIST,&Gbl.Usrs.Other.UsrDat,NULL);
   fprintf (Gbl.F.Out,"</div>");

   /***** Record of the student in the course *****/
   if (Gbl.CurrentCrs.Records.LstFields.Num)	// There are fields in the record
     {
      if (Gbl.Usrs.Me.Role.Logged == Rol_NET ||
	  Gbl.Usrs.Me.Role.Logged == Rol_TCH ||
	  Gbl.Usrs.Me.Role.Logged == Rol_SYS_ADM)
	{
         fprintf (Gbl.F.Out,"<div class=\"REC_CRS\">");
	 Rec_ShowCrsRecord (Rec_CRS_LIST_ONE_RECORD,&Gbl.Usrs.Other.UsrDat,NULL);
         fprintf (Gbl.F.Out,"</div>");
	}
      else if (Gbl.Usrs.Me.Role.Logged == Rol_STD &&
	       Gbl.Usrs.Me.UsrDat.UsrCod == Gbl.Usrs.Other.UsrDat.UsrCod)	// It's me
	{
         fprintf (Gbl.F.Out,"<div class=\"REC_CRS\">");
	 Rec_ShowCrsRecord (Rec_CRS_MY_RECORD_AS_STUDENT_FORM,&Gbl.Usrs.Other.UsrDat,NULL);
         fprintf (Gbl.F.Out,"</div>");
	}
     }

   /***** End container for this user *****/
   fprintf (Gbl.F.Out,"</div>");

   /***** Free list of fields of records *****/
   Rec_FreeListFields ();
  }

/*****************************************************************************/
/******************** Draw records of several students ***********************/
/*****************************************************************************/

void Rec_ListRecordsStdsShow (void)
  {
   Gbl.Action.Original = ActSeeRecSevStd;	// Used to know where to go when confirming ID...
						// ...or changing course record
   Rec_ListRecordsStds (Rec_SHA_RECORD_LIST,
                        Rec_CRS_LIST_SEVERAL_RECORDS);
  }

void Rec_ListRecordsStdsPrint (void)
  {
   Rec_ListRecordsStds (Rec_SHA_RECORD_PRINT,
                        Rec_CRS_PRINT_SEVERAL_RECORDS);
  }

static void Rec_ListRecordsStds (Rec_SharedRecordViewType_t ShaTypeOfView,
                                 Rec_CourseRecordViewType_t CrsTypeOfView)
  {
   extern const char *Txt_You_must_select_one_ore_more_students;
   unsigned NumUsr = 0;
   const char *Ptr;
   struct UsrData UsrDat;
   char RecordSectionId[32];

   /***** Assign users listing type depending on current action *****/
   Gbl.Usrs.Listing.RecsUsrs = Rec_RECORD_USERS_STUDENTS;

   /***** Get parameter with number of user records per page (only for printing) *****/
   if (ShaTypeOfView == Rec_SHA_RECORD_PRINT)
      Rec_GetParamRecordsPerPage ();

   /***** Get list of selected students *****/
   Usr_GetListsSelectedUsrsCods ();

   /* Check the number of students to show */
   if (!Usr_CountNumUsrsInListOfSelectedUsrs ())	// If no students selected...
     {						// ...write warning notice
      Ale_ShowAlert (Ale_WARNING,Txt_You_must_select_one_ore_more_students);
      Usr_SeeStudents ();			// ...show again the form
      return;
     }

   /***** Get list of fields of records in current course *****/
   Rec_GetListRecordFieldsInCurrentCrs ();

   if (ShaTypeOfView == Rec_SHA_RECORD_LIST)
     {
      fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");

      /* Link to edit record fields */
      if (Gbl.Usrs.Me.Role.Logged == Rol_TCH)
         Rec_PutLinkToEditRecordFields ();

      /* Link to print view */
      Act_FormStart (ActPrnRecSevStd);
      Usr_PutHiddenParUsrCodAll (ActPrnRecSevStd,Gbl.Usrs.Select[Rol_UNK]);
      Rec_ShowLinkToPrintPreviewOfRecords ();
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</div>");
     }

   /***** Initialize structure with user's data *****/
   Usr_UsrDataConstructor (&UsrDat);

   /***** List the records *****/
   Ptr = Gbl.Usrs.Select[Rol_UNK];
   while (*Ptr)
     {
      Par_GetNextStrUntilSeparParamMult (&Ptr,UsrDat.EncryptedUsrCod,
                                         Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64);
      Usr_GetUsrCodFromEncryptedUsrCod (&UsrDat);
      if (Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (&UsrDat))                // Get from the database the data of the student
         if (Usr_CheckIfUsrBelongsToCrs (UsrDat.UsrCod,
                                         Gbl.CurrentCrs.Crs.CrsCod,
                                         false))
           {
            /* Check if this user has accepted
               his/her inscription in the current course */
            UsrDat.Accepted = Usr_CheckIfUsrBelongsToCrs (UsrDat.UsrCod,
                                                          Gbl.CurrentCrs.Crs.CrsCod,
                                                          true);

            /* Start container for this user */
	    sprintf (RecordSectionId,"record_%u",NumUsr);
	    Lay_StartSection (RecordSectionId);
	    fprintf (Gbl.F.Out,"<div class=\"REC_USR\"");
            if (Gbl.Action.Act == ActPrnRecSevStd &&
                NumUsr != 0 &&
                (NumUsr % Gbl.Usrs.Listing.RecsPerPag) == 0)
               fprintf (Gbl.F.Out," style=\"page-break-before:always;\"");
            fprintf (Gbl.F.Out,">");

	    /* Show optional alert */
	    if (UsrDat.UsrCod == Gbl.Usrs.Other.UsrDat.UsrCod)	// Selected user
               Ale_ShowPendingAlert ();

            /* Shared record */
            fprintf (Gbl.F.Out,"<div class=\"REC_SHA\">");
            Rec_ShowSharedUsrRecord (ShaTypeOfView,&UsrDat,RecordSectionId);
            fprintf (Gbl.F.Out,"</div>");

            /* Record of the student in the course */
            if (Gbl.CurrentCrs.Records.LstFields.Num)	// There are fields in the record
	       if ( Gbl.Usrs.Me.Role.Logged == Rol_NET     ||
		    Gbl.Usrs.Me.Role.Logged == Rol_TCH     ||
		    Gbl.Usrs.Me.Role.Logged == Rol_SYS_ADM ||
		   (Gbl.Usrs.Me.Role.Logged == Rol_STD &&		// I am student in this course...
		    UsrDat.UsrCod == Gbl.Usrs.Me.UsrDat.UsrCod))	// ...and it's me
		 {
		  fprintf (Gbl.F.Out,"<div class=\"REC_CRS\">");
		  Rec_ShowCrsRecord (CrsTypeOfView,&UsrDat,RecordSectionId);
                  fprintf (Gbl.F.Out,"</div>");
		 }

            /* End container for this user */
            fprintf (Gbl.F.Out,"</div>");
            Lay_EndSection ();

            NumUsr++;
           }
     }

   /***** Free memory used for user's data *****/
   Usr_UsrDataDestructor (&UsrDat);

   /***** Free list of fields of records *****/
   Rec_FreeListFields ();

   /***** Free memory used by list of selected users' codes *****/
   Usr_FreeListsSelectedUsrsCods ();
  }

/*****************************************************************************/
/********** Get user's data and draw record of one unique teacher ************/
/*****************************************************************************/

void Rec_GetUsrAndShowRecordOneTchCrs (void)
  {
   /***** Get the selected teacher *****/
   Usr_GetParamOtherUsrCodEncryptedAndGetListIDs ();

   /***** Show the record *****/
   if (Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (&Gbl.Usrs.Other.UsrDat))	// Get from the database the data of the teacher
      if (Usr_CheckIfICanViewRecordTch (&Gbl.Usrs.Other.UsrDat))
	 Rec_ShowRecordOneTchCrs ();
  }

/*****************************************************************************/
/******************** Draw record of one unique teacher **********************/
/*****************************************************************************/

static void Rec_ShowRecordOneTchCrs (void)
  {
   extern const char *Hlp_USERS_Teachers_timetable;
   extern const char *Txt_TIMETABLE_TYPES[TT_NUM_TIMETABLE_TYPES];
   char Width[10 + 2 + 1];
   bool ShowOfficeHours;

   /***** Width for office hours *****/
   sprintf (Width,"%upx",Rec_RECORD_WIDTH);

   /***** Get if teacher has accepted enrolment in current course *****/
   Gbl.Usrs.Other.UsrDat.Accepted = Usr_CheckIfUsrBelongsToCrs (Gbl.Usrs.Other.UsrDat.UsrCod,
                                                                Gbl.CurrentCrs.Crs.CrsCod,
                                                                true);

   /***** Assign users listing type depending on current action *****/
   Gbl.Usrs.Listing.RecsUsrs = Rec_RECORD_USERS_TEACHERS;

   /***** Get if I want to see teachers' office hours in teachers' records *****/
   ShowOfficeHours = Rec_GetParamShowOfficeHours ();

   /***** Show contextual menu *****/
   fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");

   /* Show office hours? */
   Rec_WriteFormShowOfficeHoursOneTch (ShowOfficeHours);

   /* Link to print view */
   Act_FormStart (ActPrnRecSevTch);
   Usr_PutHiddenParUsrCodAll (ActPrnRecSevTch,Gbl.Usrs.Other.UsrDat.EncryptedUsrCod);
   Par_PutHiddenParamChar ("ParamOfficeHours",'Y');
   Par_PutHiddenParamChar ("ShowOfficeHours",'Y');
   Rec_ShowLinkToPrintPreviewOfRecords ();
   Act_FormEnd ();

   fprintf (Gbl.F.Out,"</div>");

   /***** Start container for this user *****/
   fprintf (Gbl.F.Out,"<div class=\"REC_USR\">");

   /***** Shared record *****/
   fprintf (Gbl.F.Out,"<div class=\"REC_SHA\">");
   Rec_ShowSharedUsrRecord (Rec_SHA_RECORD_LIST,&Gbl.Usrs.Other.UsrDat,NULL);
   fprintf (Gbl.F.Out,"</div>");

   /***** Office hours *****/
   if (ShowOfficeHours)
     {
      fprintf (Gbl.F.Out,"<div class=\"REC_TT\">");
      Gbl.TimeTable.Type = TT_TUTORING_TIMETABLE;
      Lay_StartRoundFrame (Width,Txt_TIMETABLE_TYPES[Gbl.TimeTable.Type],
			   NULL,Hlp_USERS_Teachers_timetable);
      TT_ShowTimeTable (Gbl.Usrs.Other.UsrDat.UsrCod);
      Lay_EndRoundFrame ();
      fprintf (Gbl.F.Out,"</div>");
     }

   /***** Start container for this user *****/
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/******************** Draw records of several teachers ***********************/
/*****************************************************************************/

void Rec_ListRecordsTchsShow (void)
  {
   Gbl.Action.Original = ActSeeRecSevTch;	// Used to know where to go when confirming ID
   Rec_ListRecordsTchs (Rec_SHA_RECORD_LIST);
  }

void Rec_ListRecordsTchsPrint (void)
  {
   Rec_ListRecordsTchs (Rec_SHA_RECORD_PRINT);
  }

static void Rec_ListRecordsTchs (Rec_SharedRecordViewType_t TypeOfView)
  {
   extern const char *Hlp_USERS_Teachers_timetable;
   extern const char *Txt_You_must_select_one_ore_more_teachers;
   extern const char *Txt_TIMETABLE_TYPES[TT_NUM_TIMETABLE_TYPES];
   unsigned NumUsr = 0;
   const char *Ptr;
   struct UsrData UsrDat;
   char RecordSectionId[32];
   bool ShowOfficeHours;
   char Width[10 + 2 + 1];

   /***** Width for office hours *****/
   sprintf (Width,"%upx",Rec_RECORD_WIDTH);

   /***** Assign users listing type depending on current action *****/
   Gbl.Usrs.Listing.RecsUsrs = Rec_RECORD_USERS_TEACHERS;

   /***** Get if I want to see teachers' office hours in teachers' records *****/
   ShowOfficeHours = Rec_GetParamShowOfficeHours ();

   /***** Get parameter with number of user records per page (only for printing) *****/
   if (Gbl.Action.Act == ActPrnRecSevTch)
      Rec_GetParamRecordsPerPage ();

   /***** Get list of selected teachers *****/
   Usr_GetListsSelectedUsrsCods ();

   /* Check the number of teachers to show */
   if (!Usr_CountNumUsrsInListOfSelectedUsrs ())	// If no teachers selected...
     {						// ...write warning notice
      Ale_ShowAlert (Ale_WARNING,Txt_You_must_select_one_ore_more_teachers);
      Usr_SeeTeachers ();			// ...show again the form
      return;
     }

   if (Gbl.Action.Act == ActSeeRecSevTch)
     {
      /***** Show contextual menu *****/
      fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");

      /* Show office hours? */
      Rec_WriteFormShowOfficeHoursSeveralTchs (ShowOfficeHours);

      /* Link to print view */
      Act_FormStart (ActPrnRecSevTch);
      Usr_PutHiddenParUsrCodAll (ActPrnRecSevTch,Gbl.Usrs.Select[Rol_UNK]);
      Par_PutHiddenParamChar ("ParamOfficeHours",'Y');
      Par_PutHiddenParamChar ("ShowOfficeHours",
                              ShowOfficeHours ? 'Y' :
                        	                'N');
      Rec_ShowLinkToPrintPreviewOfRecords ();
      Act_FormEnd ();

      fprintf (Gbl.F.Out,"</div>");
     }

   /***** Initialize structure with user's data *****/
   Usr_UsrDataConstructor (&UsrDat);

   /***** List the records *****/
   Ptr = Gbl.Usrs.Select[Rol_UNK];
   while (*Ptr)
     {
      Par_GetNextStrUntilSeparParamMult (&Ptr,UsrDat.EncryptedUsrCod,
                                         Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64);
      Usr_GetUsrCodFromEncryptedUsrCod (&UsrDat);
      if (Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (&UsrDat))                // Get from the database the data of the student
         if (Usr_CheckIfUsrBelongsToCrs (UsrDat.UsrCod,
                                         Gbl.CurrentCrs.Crs.CrsCod,
                                         false))
           {
            /* Check if this user has accepted
               his/her inscription in the current course */
            UsrDat.Accepted = Usr_CheckIfUsrBelongsToCrs (UsrDat.UsrCod,
                                                          Gbl.CurrentCrs.Crs.CrsCod,
                                                          true);

            /* Start container for this user */
	    sprintf (RecordSectionId,"record_%u",NumUsr);
	    Lay_StartSection (RecordSectionId);
	    fprintf (Gbl.F.Out,"<div class=\"REC_USR\"");
            if (Gbl.Action.Act == ActPrnRecSevTch &&
                NumUsr != 0 &&
                (NumUsr % Gbl.Usrs.Listing.RecsPerPag) == 0)
               fprintf (Gbl.F.Out," style=\"page-break-before:always;\"");
            fprintf (Gbl.F.Out,">");

	    /* Show optional alert */
	    if (UsrDat.UsrCod == Gbl.Usrs.Other.UsrDat.UsrCod)	// Selected user
	       Ale_ShowPendingAlert ();

	    /* Shared record */
            fprintf (Gbl.F.Out,"<div class=\"REC_SHA\">");
            Rec_ShowSharedUsrRecord (TypeOfView,&UsrDat,RecordSectionId);
            fprintf (Gbl.F.Out,"</div>");

            /* Office hours */
            if (ShowOfficeHours)
              {
	       fprintf (Gbl.F.Out,"<div class=\"REC_TT\">");
               Gbl.TimeTable.Type = TT_TUTORING_TIMETABLE;
	       Lay_StartRoundFrame (Width,Txt_TIMETABLE_TYPES[Gbl.TimeTable.Type],
	                            NULL,Hlp_USERS_Teachers_timetable);
	       TT_ShowTimeTable (UsrDat.UsrCod);
	       Lay_EndRoundFrame ();
               fprintf (Gbl.F.Out,"</div>");
              }

            /* End container for this user */
            fprintf (Gbl.F.Out,"</div>");
            Lay_EndSection ();

            NumUsr++;
           }
     }
   /***** Free memory used for user's data *****/
   Usr_UsrDataDestructor (&UsrDat);

   /***** Free memory used by list of selected users' codes *****/
   Usr_FreeListsSelectedUsrsCods ();
  }

/*****************************************************************************/
/*************** Show a link to print preview of users' records **************/
/*****************************************************************************/

static void Rec_ShowLinkToPrintPreviewOfRecords (void)
  {
   extern const char *The_ClassFormBold[The_NUM_THEMES];
   extern const char *The_ClassForm[The_NUM_THEMES];
   extern const char *Txt_Print;
   extern const char *Txt_record_cards_per_page;
   unsigned i;

   Act_LinkFormSubmit (Txt_Print,The_ClassFormBold[Gbl.Prefs.Theme],NULL);
   Lay_PutIconWithText ("print64x64.png",Txt_Print,Txt_Print);
   fprintf (Gbl.F.Out,"</a>"
                      "<label class=\"%s\">"
                      "(<select name=\"RecsPerPag\">",
	    The_ClassForm[Gbl.Prefs.Theme]);
   for (i = Rec_MIN_RECORDS_PER_PAGE;
        i <= Rec_MAX_RECORDS_PER_PAGE;
        i++)
     {
      fprintf (Gbl.F.Out,"<option");
      if (i == Gbl.Usrs.Listing.RecsPerPag)
         fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%u</option>",i);
     }
   fprintf (Gbl.F.Out,"</select> %s)"
	              "</label>",
            Txt_record_cards_per_page);
  }

/*****************************************************************************/
/** Get parameter with number of user records per page (only for printing) ***/
/*****************************************************************************/

static void Rec_GetParamRecordsPerPage (void)
  {
   Gbl.Usrs.Listing.RecsPerPag = (unsigned)
	                         Par_GetParToUnsignedLong ("RecsPerPag",
                                                           Rec_MIN_RECORDS_PER_PAGE,
                                                           Rec_MAX_RECORDS_PER_PAGE,
                                                           Rec_DEF_RECORDS_PER_PAGE);
  }

/*****************************************************************************/
/*********** Write a form to select whether show all office hours ************/
/*****************************************************************************/

static void Rec_WriteFormShowOfficeHoursOneTch (bool ShowOfficeHours)
  {
   extern const char *Txt_Show_office_hours;

   Lay_PutContextualCheckbox (ActSeeRecOneTch,Rec_PutParamsShowOfficeHoursOneTch,
                              "ShowOfficeHours",
                              ShowOfficeHours,false,
                              Txt_Show_office_hours,
                              Txt_Show_office_hours);
  }

static void Rec_WriteFormShowOfficeHoursSeveralTchs (bool ShowOfficeHours)
  {
   extern const char *Txt_Show_office_hours;

   Lay_PutContextualCheckbox (ActSeeRecSevTch,Rec_PutParamsShowOfficeHoursSeveralTchs,
                              "ShowOfficeHours",
                              ShowOfficeHours,false,
                              Txt_Show_office_hours,
                              Txt_Show_office_hours);
  }

static void Rec_PutParamsShowOfficeHoursOneTch (void)
  {
   Usr_PutParamOtherUsrCodEncrypted ();
   Par_PutHiddenParamChar ("ParamOfficeHours",'Y');
  }

static void Rec_PutParamsShowOfficeHoursSeveralTchs (void)
  {
   Usr_PutHiddenParUsrCodAll (ActSeeRecSevTch,Gbl.Usrs.Select[Rol_UNK]);
   Par_PutHiddenParamChar ("ParamOfficeHours",'Y');
  }

/*****************************************************************************/
/********** Get parameter to show (or not) teachers' office hours ************/
/*****************************************************************************/
// Returns true if office hours must be shown

static bool Rec_GetParamShowOfficeHours (void)
  {
   if (Par_GetParToBool ("ParamOfficeHours"))
      return Par_GetParToBool ("ShowOfficeHours");

   return Rec_SHOW_OFFICE_HOURS_DEFAULT;
  }

/*****************************************************************************/
/*************** Update my record in the course and show it ******************/
/*****************************************************************************/

void Rec_UpdateAndShowMyCrsRecord (void)
  {
   /***** Get list of fields of records in current course *****/
   Rec_GetListRecordFieldsInCurrentCrs ();

   /***** Allocate memory for the texts of the fields *****/
   Rec_AllocMemFieldsRecordsCrs ();

   /***** Get data of the record from the form *****/
   Rec_GetFieldsCrsRecordFromForm ();

   /***** Update the record *****/
   Rec_UpdateCrsRecord (Gbl.Usrs.Me.UsrDat.UsrCod);

   /***** Show updated record *****/
   Rec_ShowMyCrsRecordUpdated ();

   /***** Free memory used for some fields *****/
   Rec_FreeMemFieldsRecordsCrs ();
  }

/*****************************************************************************/
/***** Update record in the course of one student and show records again *****/
/*****************************************************************************/

void Rec_UpdateAndShowOtherCrsRecord (void)
  {
   extern const char *Txt_Student_record_card_in_this_course_has_been_updated;
   long OriginalActCod;

   /***** Initialize alert type and message *****/
   Gbl.Alert.Type = Ale_NONE;	// Do not show alert

   /***** Get where we came from *****/
   OriginalActCod = Par_GetParToLong ("OriginalActCod");
   Gbl.Action.Original = Act_GetActionFromActCod (OriginalActCod);

   /***** Get the user whose record we want to modify *****/
   Usr_GetParamOtherUsrCodEncryptedAndGetListIDs ();
   Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (&Gbl.Usrs.Other.UsrDat);

   /***** Get list of fields of records in current course *****/
   Rec_GetListRecordFieldsInCurrentCrs ();

   /***** Allocate memory for the texts of the fields *****/
   Rec_AllocMemFieldsRecordsCrs ();

   /***** Get data of the record from the form *****/
   Rec_GetFieldsCrsRecordFromForm ();

   /***** Update the record *****/
   Rec_UpdateCrsRecord (Gbl.Usrs.Other.UsrDat.UsrCod);
   Gbl.Alert.Type = Ale_SUCCESS;
   sprintf (Gbl.Alert.Txt,"%s",
            Txt_Student_record_card_in_this_course_has_been_updated);

   /***** Show one or multiple records *****/
   switch (Gbl.Action.Original)
     {
      case ActSeeRecSevStd:
	 /* Show multiple records again (including the updated one) */
	 Rec_ListRecordsStdsShow ();
	 break;
      default:
	 /* Show only the updated record of one student */
	 Rec_ShowRecordOneStdCrs ();
	 break;
     }

   /***** Free memory used for some fields *****/
   Rec_FreeMemFieldsRecordsCrs ();
  }

/*****************************************************************************/
/************************* Show shared record card ***************************/
/*****************************************************************************/
// Show form or only data depending on TypeOfView

static void Rec_ShowCrsRecord (Rec_CourseRecordViewType_t TypeOfView,
                               struct UsrData *UsrDat,const char *Anchor)
  {
   extern struct Act_Actions Act_Actions[Act_NUM_ACTIONS];
   extern const char *Hlp_USERS_Students_course_record_card;
   extern const char *The_ClassForm[The_NUM_THEMES];
   extern const char *Txt_You_dont_have_permission_to_perform_this_action;
   extern const char *Txt_RECORD_FIELD_VISIBILITY_RECORD[Rec_NUM_TYPES_VISIBILITY];
   extern const char *Txt_Save;
   const char *Rec_RecordHelp[Rec_COURSE_NUM_VIEW_TYPES] =
     {
      Hlp_USERS_Students_course_record_card,	// Rec_CRS_MY_RECORD_AS_STUDENT_FORM
      Hlp_USERS_Students_course_record_card,	// Rec_CRS_MY_RECORD_AS_STUDENT_CHECK
      Hlp_USERS_Students_course_record_card,	// Rec_CRS_LIST_ONE_RECORD
      Hlp_USERS_Students_course_record_card,	// Rec_CRS_LIST_SEVERAL_RECORDS
      NULL,					// Rec_CRS_PRINT_ONE_RECORD
      NULL,					// Rec_CRS_PRINT_SEVERAL_RECORDS
		// Rec_CRS_RECORD_PRINT
     };
   char StrRecordWidth[10 + 1];
   bool ItsMe;
   bool ICanEdit = false;
   unsigned NumField;
   MYSQL_RES *mysql_res;
   MYSQL_ROW row = NULL; // Initialized to avoid warning
   bool ShowField;
   bool ThisFieldHasText;
   bool ICanEditThisField;
   char Text[Cns_MAX_BYTES_TEXT + 1];

   switch (Gbl.Usrs.Me.Role.Logged)
     {
      case Rol_STD:	// I am a student
	 ItsMe = (Gbl.Usrs.Me.UsrDat.UsrCod == UsrDat->UsrCod);	// It's me
	 if (ItsMe)	// It's me
	   {
	    switch (TypeOfView)
	      {
	       case Rec_CRS_LIST_ONE_RECORD:
	       case Rec_CRS_LIST_SEVERAL_RECORDS:
		  // When listing records, I can see only my record as student
		  TypeOfView = Rec_CRS_MY_RECORD_AS_STUDENT_FORM;
		  break;
	       case Rec_CRS_MY_RECORD_AS_STUDENT_FORM:
	       case Rec_CRS_MY_RECORD_AS_STUDENT_CHECK:
	       case Rec_CRS_PRINT_ONE_RECORD:
	       case Rec_CRS_PRINT_SEVERAL_RECORDS:
		  break;
	       default:
		  Lay_ShowErrorAndExit (Txt_You_dont_have_permission_to_perform_this_action);
		  break;
	      }

	    if (TypeOfView == Rec_CRS_MY_RECORD_AS_STUDENT_FORM)
	       /* Check if I can edit any of the record fields */
	       for (NumField = 0;
		    NumField < Gbl.CurrentCrs.Records.LstFields.Num;
		    NumField++)
		  if (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility == Rec_EDITABLE_FIELD)
		    {
		     ICanEdit = true;
		     Act_FormStart (ActRcvRecCrs);
		     break;
		    }
	   }
	 else	// It's not me ==> I am a student trying to do something forbidden
	    Lay_ShowErrorAndExit (Txt_You_dont_have_permission_to_perform_this_action);
	 break;
      case Rol_NET:
	 break;
      case Rol_TCH:
      case Rol_SYS_ADM:
	 if (TypeOfView == Rec_CRS_LIST_ONE_RECORD ||
	     TypeOfView == Rec_CRS_LIST_SEVERAL_RECORDS)
	   {
	    ICanEdit = true;
	    Act_FormStartAnchor (ActRcvRecOthUsr,Anchor);
	    Par_PutHiddenParamLong ("OriginalActCod",
				    Act_Actions[ActSeeRecSevStd].ActCod);	// Original action, used to know where we came from
	    Usr_PutParamUsrCodEncrypted (UsrDat->EncryptedUsrCod);
	    if (TypeOfView == Rec_CRS_LIST_SEVERAL_RECORDS)
	       Usr_PutHiddenParUsrCodAll (ActRcvRecOthUsr,Gbl.Usrs.Select[Rol_UNK]);
	   }
	 break;
      default:
	 Lay_ShowErrorAndExit ("Wrong role.");
     }

   /***** Start frame *****/
   sprintf (StrRecordWidth,"%upx",Rec_RECORD_WIDTH);
   Lay_StartRoundFrameTable (StrRecordWidth,NULL,
                             NULL,Rec_RecordHelp[TypeOfView],2);

   /***** Header *****/
   fprintf (Gbl.F.Out,"<tr>"
	              "<td colspan=\"2\" class=\"LEFT_TOP\">");
   Lay_StartTableWide (0);
   fprintf (Gbl.F.Out,"<tr>"
                      "<td class=\"LEFT_MIDDLE\" style=\"width:%upx;\">",
            Rec_DEGREE_LOGO_SIZE);
   Log_DrawLogo (Sco_SCOPE_DEG,Gbl.CurrentDeg.Deg.DegCod,
                 Gbl.CurrentDeg.Deg.ShrtName,Rec_DEGREE_LOGO_SIZE,NULL,true);
   fprintf (Gbl.F.Out,"</td>"
                      "<td class=\"REC_HEAD CENTER_MIDDLE\">"
                      "%s<br />%s<br />%s"
                      "</td>"
                      "</tr>",
            Gbl.CurrentDeg.Deg.FullName,Gbl.CurrentCrs.Crs.FullName,
            UsrDat->FullName);
   Lay_EndTable ();
   fprintf (Gbl.F.Out,"</td>"
                      "</tr>");

   /***** Fields of the record that depends on the course *****/
   for (NumField = 0, Gbl.RowEvenOdd = 0;
	NumField < Gbl.CurrentCrs.Records.LstFields.Num;
	NumField++, Gbl.RowEvenOdd = 1 - Gbl.RowEvenOdd)
     {
      ShowField = !(TypeOfView == Rec_CRS_MY_RECORD_AS_STUDENT_FORM ||
	            TypeOfView == Rec_CRS_MY_RECORD_AS_STUDENT_CHECK) ||
                  Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility != Rec_HIDDEN_FIELD;
      // If the field must be shown...
      if (ShowField)
        {
	 /* Can I edit this field? */
	 switch (Gbl.Usrs.Me.Role.Logged)
	   {
	    case Rol_STD:
	       ICanEditThisField = (TypeOfView == Rec_CRS_MY_RECORD_AS_STUDENT_FORM &&
				    Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility == Rec_EDITABLE_FIELD);
	       break;
	    case Rol_TCH:
	    case Rol_SYS_ADM:
	       ICanEditThisField = (TypeOfView == Rec_CRS_LIST_ONE_RECORD ||
				    TypeOfView == Rec_CRS_LIST_SEVERAL_RECORDS);
               break;
	    default:
	       ICanEditThisField = false;
	       break;
	   }

         /* Name of the field */
         fprintf (Gbl.F.Out,"<tr>"
                            "<td class=\"REC_C1_BOT %s RIGHT_TOP COLOR%u\">"
                            "%s:",
                  ICanEditThisField ? The_ClassForm[Gbl.Prefs.Theme] :
                	             "REC_DAT_SMALL",
                  Gbl.RowEvenOdd,
                  Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Name);
         if (TypeOfView == Rec_CRS_LIST_ONE_RECORD ||
             TypeOfView == Rec_CRS_LIST_SEVERAL_RECORDS)
            fprintf (Gbl.F.Out,"<span class=\"DAT_SMALL\"> (%s)</span>",
                     Txt_RECORD_FIELD_VISIBILITY_RECORD[Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility]);
         fprintf (Gbl.F.Out,"</td>");

         /* Get the text of the field */
         if (Rec_GetFieldFromCrsRecord (UsrDat->UsrCod,Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod,&mysql_res))
           {
            ThisFieldHasText = true;
            row = mysql_fetch_row (mysql_res);
           }
         else
            ThisFieldHasText = false;

         /* Write form, text, or nothing depending on
            the user's role and the visibility of the field */
         fprintf (Gbl.F.Out,"<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_TOP COLOR%u\">",
                  Gbl.RowEvenOdd);
         if (ICanEditThisField)	// Show with form
           {
            fprintf (Gbl.F.Out,"<textarea name=\"Field%ld\" rows=\"%u\""
        	               " class=\"REC_C2_BOT_INPUT\">",
                     Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod,
                     Gbl.CurrentCrs.Records.LstFields.Lst[NumField].NumLines);
            if (ThisFieldHasText)
               fprintf (Gbl.F.Out,"%s",row[0]);
            fprintf (Gbl.F.Out,"</textarea>");
           }
         else			// Show without form
           {
            if (ThisFieldHasText)
              {
               Str_Copy (Text,row[0],
                         Cns_MAX_BYTES_TEXT);
               Str_ChangeFormat (Str_FROM_HTML,Str_TO_RIGOROUS_HTML,
                                 Text,Cns_MAX_BYTES_TEXT,false);
               fprintf (Gbl.F.Out,"%s",Text);
              }
            else
               fprintf (Gbl.F.Out,"-");
           }
         fprintf (Gbl.F.Out,"</td>"
                            "</tr>");

         /* Free structure that stores the query result */
         DB_FreeMySQLResult (&mysql_res);
        }
     }

   /***** Button to save changes and end frame *****/
   if (ICanEdit)
     {
      Lay_EndRoundFrameTableWithButton (Lay_CONFIRM_BUTTON,Txt_Save);
      Act_FormEnd ();
     }
   else
      Lay_EndRoundFrameTable ();
  }

/*****************************************************************************/
/************** Get the text of a field of a record of course ****************/
/*****************************************************************************/

unsigned long Rec_GetFieldFromCrsRecord (long UsrCod,long FieldCod,MYSQL_RES **mysql_res)
  {
   char Query[256];

   /***** Get the text of a field of a record from database *****/
   sprintf (Query,"SELECT Txt FROM crs_records"
	          " WHERE FieldCod=%ld AND UsrCod=%ld",
            FieldCod,UsrCod);
   return DB_QuerySELECT (Query,mysql_res,"can not get the text of a field of a record.");
  }

/*****************************************************************************/
/****************** Get the fields of the record from form *******************/
/*****************************************************************************/

void Rec_GetFieldsCrsRecordFromForm (void)
  {
   unsigned NumField;
   char FieldParamName[5 + 10 + 1];

   for (NumField = 0;
	NumField < Gbl.CurrentCrs.Records.LstFields.Num;
	NumField++)
      if (Rec_CheckIfICanEditField (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility))
        {
         /* Get text of the form */
         sprintf (FieldParamName,"Field%ld",Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod);
         Par_GetParToHTML (FieldParamName,Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text,Cns_MAX_BYTES_TEXT);
        }
  }

/*****************************************************************************/
/*************************** Update record of a user *************************/
/*****************************************************************************/

void Rec_UpdateCrsRecord (long UsrCod)
  {
   unsigned NumField;
   char Query[256 + Cns_MAX_BYTES_TEXT];
   MYSQL_RES *mysql_res;
   bool FieldAlreadyExists;

   for (NumField = 0;
	NumField < Gbl.CurrentCrs.Records.LstFields.Num;
	NumField++)
      if (Rec_CheckIfICanEditField (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility))
        {
         /***** Check if already exists this field for this user in database *****/
         FieldAlreadyExists = (Rec_GetFieldFromCrsRecord (UsrCod,Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod,&mysql_res) != 0);
         DB_FreeMySQLResult (&mysql_res);
         if (FieldAlreadyExists)
           {
            if (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text[0])
              {
               /***** Update text of the field of record course *****/
               sprintf (Query,"UPDATE crs_records SET Txt='%s'"
        	              " WHERE UsrCod=%ld AND FieldCod=%ld",
                        Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text,
                        UsrCod,Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod);
               DB_QueryUPDATE (Query,"can not update field of record");
              }
            else
              {
               /***** Remove text of the field of record course *****/
               sprintf (Query,"DELETE FROM crs_records"
        	              " WHERE UsrCod=%ld AND FieldCod=%ld",
                        UsrCod,Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod);
               DB_QueryDELETE (Query,"can not remove field of record");
              }
           }
         else if (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text[0])
	   {
	    /***** Insert text field of record course *****/
	    sprintf (Query,"INSERT INTO crs_records"
		           " (FieldCod,UsrCod,Txt)"
		           " VALUES"
		           " (%ld,%ld,'%s')",
		     Gbl.CurrentCrs.Records.LstFields.Lst[NumField].FieldCod,
		     UsrCod,
		     Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text);
	    DB_QueryINSERT (Query,"can not create field of record");
	   }
       }
  }

/*****************************************************************************/
/************ Remove fields of record of a user in current course ************/
/*****************************************************************************/

void Rec_RemoveFieldsCrsRecordInCrs (long UsrCod,struct Course *Crs)
  {
   char Query[256];

   /***** Remove text of the field of record course *****/
   sprintf (Query,"DELETE FROM crs_records"
	          " WHERE UsrCod=%ld AND FieldCod IN"
                  " (SELECT FieldCod FROM crs_record_fields WHERE CrsCod=%ld)",
            UsrCod,Crs->CrsCod);
   DB_QueryDELETE (Query,"can not remove user's record in a course");
  }

/*****************************************************************************/
/************* Remove fields of record of a user in all courses **************/
/*****************************************************************************/

void Rec_RemoveFieldsCrsRecordAll (long UsrCod)
  {
   char Query[128];

   /***** Remove text of the field of record course *****/
   sprintf (Query,"DELETE FROM crs_records WHERE UsrCod=%ld",UsrCod);
   DB_QueryDELETE (Query,"can not remove user's records in all courses");
  }

/*****************************************************************************/
/*************** Show my record in the course already updated ****************/
/*****************************************************************************/

static void Rec_ShowMyCrsRecordUpdated (void)
  {
   extern const char *Txt_Your_record_card_in_this_course_has_been_updated;

   /***** Write mensaje of success *****/
   Ale_ShowAlert (Ale_SUCCESS,Txt_Your_record_card_in_this_course_has_been_updated);

   /***** Shared record *****/
   Rec_ShowSharedUsrRecord (Rec_SHA_RECORD_LIST,&Gbl.Usrs.Me.UsrDat,NULL);

   /***** Show updated user's record *****/
   Rec_ShowCrsRecord (Rec_CRS_MY_RECORD_AS_STUDENT_CHECK,&Gbl.Usrs.Me.UsrDat,NULL);
  }

/*****************************************************************************/
/***** Allocate memory for the text of the field of the record in course *****/
/*****************************************************************************/

void Rec_AllocMemFieldsRecordsCrs (void)
  {
   unsigned NumField;

   for (NumField = 0;
	NumField < Gbl.CurrentCrs.Records.LstFields.Num;
	NumField++)
      if (Rec_CheckIfICanEditField (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility))
         /* Allocate memory for the texts of the fields */
         if ((Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text = malloc (Cns_MAX_BYTES_TEXT + 1)) == NULL)
            Lay_ShowErrorAndExit ("Not enough memory to store records of the course.");
  }

/*****************************************************************************/
/**** Free memory used by the texts of the field of the record in course *****/
/*****************************************************************************/

void Rec_FreeMemFieldsRecordsCrs (void)
  {
   unsigned NumField;

   for (NumField = 0;
	NumField < Gbl.CurrentCrs.Records.LstFields.Num;
	NumField++)
      if (Rec_CheckIfICanEditField (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Visibility))
         /* Free memory of the text of the field */
         if (Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text)
           {
            free ((void *) Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text);
            Gbl.CurrentCrs.Records.LstFields.Lst[NumField].Text = NULL;
           }
  }

/*****************************************************************************/
/* Check if I can edit a field depending on my role and the field visibility */
/*****************************************************************************/

static bool Rec_CheckIfICanEditField (Rec_VisibilityRecordFields_t Visibility)
  {
   // Non-editing teachers can not edit fields
   return  Gbl.Usrs.Me.Role.Logged == Rol_TCH     ||
	   Gbl.Usrs.Me.Role.Logged == Rol_SYS_ADM ||
	  (Gbl.Usrs.Me.Role.Logged == Rol_STD &&
	   Visibility == Rec_EDITABLE_FIELD);
  }

/*****************************************************************************/
/*********** Show form to sign up and edit my shared record card *************/
/*****************************************************************************/

void Rec_ShowFormSignUpInCrsWithMySharedRecord (void)
  {
   /***** Show the form *****/
   Rec_ShowSharedUsrRecord (Rec_SHA_SIGN_UP_IN_CRS_FORM,&Gbl.Usrs.Me.UsrDat,NULL);
  }

/*****************************************************************************/
/***************** Show form to edit my shared record card *******************/
/*****************************************************************************/

void Rec_ShowFormMySharedRecord (void)
  {
   extern const char *Txt_Please_fill_in_your_record_card_including_your_country_nationality;
   extern const char *Txt_Please_fill_in_your_record_card_including_your_sex;
   extern const char *Txt_Please_fill_in_your_record_card_including_your_name;

   /***** If user has no sex, name and surname... *****/
   if (Gbl.Usrs.Me.UsrDat.CtyCod < 0)
      Ale_ShowAlert (Ale_WARNING,Txt_Please_fill_in_your_record_card_including_your_country_nationality);
   else if (Gbl.Usrs.Me.UsrDat.Sex == Usr_SEX_UNKNOWN)
      Ale_ShowAlert (Ale_WARNING,Txt_Please_fill_in_your_record_card_including_your_sex);
   else if (!Gbl.Usrs.Me.UsrDat.FirstName[0] ||
	    !Gbl.Usrs.Me.UsrDat.Surname1[0])
      Ale_ShowAlert (Ale_WARNING,Txt_Please_fill_in_your_record_card_including_your_name);

   /***** Contextual links *****/
   fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");
   Rec_PutLinkToChangeMyInsCtrDpt ();			// Put link (form) to change my institution, centre, department...
   Net_PutLinkToChangeMySocialNetworks ();		// Put link (form) to change my social networks
   Pho_PutLinkToChangeMyPhoto ();			// Put link (form) to change my photo
   Pri_PutLinkToChangeMyPrivacy ();			// Put link (form) to change my privacy
   fprintf (Gbl.F.Out,"</div>");

   /***** My record *****/
   Rec_ShowSharedUsrRecord (Rec_SHA_MY_RECORD_FORM,&Gbl.Usrs.Me.UsrDat,NULL);
   Rec_WriteLinkToDataProtectionClause ();
  }

/*****************************************************************************/
/*************** Show form to edit the record of a new user ******************/
/*****************************************************************************/

void Rec_ShowFormOtherNewSharedRecord (struct UsrData *UsrDat,Rol_Role_t DefaultRole)
  {
   /***** Show the form *****/
   /* In this case UsrDat->Roles.InCurrentCrsDB
      is not the current role in current course.
      Instead it is initialized with the preferred role. */
   UsrDat->Role.InCurrentCrs = (Gbl.CurrentCrs.Crs.CrsCod > 0) ? DefaultRole :	// Course selected
	                                                          Rol_GST;	// No course selected
   Rec_ShowSharedUsrRecord (Rec_SHA_OTHER_NEW_USR_FORM,UsrDat,NULL);
  }

/*****************************************************************************/
/*********************** Show my record after update *************************/
/*****************************************************************************/

void Rec_ShowMySharedRecordUpd (void)
  {
   extern const char *Txt_Your_personal_data_have_been_updated;

   /***** Write alert *****/
   Ale_ShowAlert (Ale_SUCCESS,Txt_Your_personal_data_have_been_updated);

   /***** Show my record for checking *****/
   Rec_ShowSharedUsrRecord (Rec_SHA_MY_RECORD_CHECK,&Gbl.Usrs.Me.UsrDat,NULL);
  }

/*****************************************************************************/
/********************** Show user's record for check *************************/
/*****************************************************************************/

void Rec_ShowSharedRecordUnmodifiable (struct UsrData *UsrDat)
  {
   /***** Get password, user type and user's data from database *****/
   Usr_GetAllUsrDataFromUsrCod (UsrDat);
   UsrDat->Accepted = Usr_CheckIfUsrBelongsToCrs (UsrDat->UsrCod,
                                                  Gbl.CurrentCrs.Crs.CrsCod,
                                                  true);

   /***** Show user's record *****/
   fprintf (Gbl.F.Out,"<div class=\"CENTER_MIDDLE\">");
   Rec_ShowSharedUsrRecord (Rec_SHA_OTHER_USR_CHECK,UsrDat,NULL);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/************************** Show shared record card **************************/
/*****************************************************************************/
// Show form or only data depending on TypeOfView

void Rec_ShowSharedUsrRecord (Rec_SharedRecordViewType_t TypeOfView,
                              struct UsrData *UsrDat,const char *Anchor)
  {
   extern struct Act_Actions Act_Actions[Act_NUM_ACTIONS];
   extern const char *Hlp_USERS_SignUp;
   extern const char *Hlp_PROFILE_Record;
   extern const char *Hlp_SOCIAL_Profiles_view_public_profile;
   extern const char *Hlp_USERS_Guests;
   extern const char *Hlp_USERS_Students_shared_record_card;
   extern const char *Hlp_USERS_Teachers_shared_record_card;
   extern const char *The_ClassForm[The_NUM_THEMES];
   extern const char *Txt_Sign_up;
   extern const char *Txt_Save_changes;
   extern const char *Txt_Register;
   extern const char *Txt_Confirm;
   const char *Rec_RecordHelp[Rec_SHARED_NUM_VIEW_TYPES] =
     {
      Hlp_USERS_SignUp,				// Rec_SHA_SIGN_UP_IN_CRS_FORM

      Hlp_PROFILE_Record,			// Rec_SHA_MY_RECORD_FORM
      Hlp_PROFILE_Record,			// Rec_SHA_MY_RECORD_CHECK

      NULL,					// Rec_SHA_OTHER_EXISTING_USR_FORM
      NULL,					// Rec_SHA_OTHER_NEW_USR_FORM
      NULL,					// Rec_SHA_OTHER_USR_CHECK

      NULL,					// Rec_SHA_RECORD_LIST
      NULL,					// Rec_SHA_RECORD_PRINT
      Hlp_SOCIAL_Profiles_view_public_profile,	// Rec_SHA_RECORD_PUBLIC
     };
   const char *Rec_RecordListHelp[Rol_NUM_ROLES] =
     {
      NULL,					// Rol_UNK
      Hlp_USERS_Guests,				// Rol_GST
      NULL,					// Rol_USR
      Hlp_USERS_Students_shared_record_card,	// Rol_STD
      Hlp_USERS_Teachers_shared_record_card,	// Rol_NET
      Hlp_USERS_Teachers_shared_record_card,	// Rol_TCH
      NULL,					// Rol_DEG_ADM
      NULL,					// Rol_CTR_ADM
      NULL,					// Rol_INS_ADM
      NULL,					// Rol_SYS_ADM
     };
   char StrRecordWidth[10 + 1];
   const char *ClassForm = "REC_DAT";
   bool ItsMe;
   bool IAmLoggedAsTeacherOrSysAdm;
   bool CountryForm;
   bool ICanEdit;
   bool PutFormLinks;	// Put links (forms) inside record card
   bool ShowData;
   bool ShowIDRows;
   bool ShowAddressRows;
   bool StudentInCurrentCrs;
   bool TeacherInCurrentCrs;
   bool TeacherInAnyCrs;
   bool ShowTeacherRows;
   struct Instit Ins;
   Act_Action_t NextAction;

   /***** Initializations *****/
   ItsMe = (Gbl.Usrs.Me.UsrDat.UsrCod == UsrDat->UsrCod);
   IAmLoggedAsTeacherOrSysAdm = (Gbl.Usrs.Me.Role.Logged == Rol_NET ||		// My current role is non-editing teacher
	                         Gbl.Usrs.Me.Role.Logged == Rol_TCH ||		// My current role is teacher
                                 Gbl.Usrs.Me.Role.Logged == Rol_SYS_ADM);	// My current role is system admin
   CountryForm = (TypeOfView == Rec_SHA_MY_RECORD_FORM);
   ShowData = (ItsMe ||
	       Gbl.Usrs.Me.Role.Logged >= Rol_DEG_ADM ||
	       UsrDat->Accepted);
   ShowIDRows = (TypeOfView != Rec_SHA_RECORD_PUBLIC);

   StudentInCurrentCrs = UsrDat->Role.InCurrentCrs == Rol_STD;
   TeacherInCurrentCrs = UsrDat->Role.InCurrentCrs == Rol_NET ||
	                 UsrDat->Role.InCurrentCrs == Rol_TCH;
   TeacherInAnyCrs = UsrDat->Role.InCrss & ((1 << Rol_NET) |
			              (1 << Rol_TCH));

   ShowAddressRows = (TypeOfView == Rec_SHA_MY_RECORD_FORM  ||
		      TypeOfView == Rec_SHA_MY_RECORD_CHECK ||
		      ((TypeOfView == Rec_SHA_RECORD_LIST   ||
		        TypeOfView == Rec_SHA_RECORD_PRINT) &&
		       IAmLoggedAsTeacherOrSysAdm &&
		       StudentInCurrentCrs));			// He/she is a student in the current course
   Rol_GetRolesInAllCrssIfNotYetGot (UsrDat);	// Get user's roles if not got
   ShowTeacherRows = (((TypeOfView == Rec_SHA_MY_RECORD_FORM  ||
		        TypeOfView == Rec_SHA_MY_RECORD_CHECK) &&
		       TeacherInAnyCrs) ||			// He/she (me, really) is a teacher in any course
		      ((TypeOfView == Rec_SHA_RECORD_LIST ||
		        TypeOfView == Rec_SHA_RECORD_PRINT) &&
		       TeacherInCurrentCrs));			// He/she is a teacher in the current course

   /* Data form = I can edit fields like surnames and name */
   switch (TypeOfView)
     {
      case Rec_SHA_MY_RECORD_FORM:
      case Rec_SHA_OTHER_NEW_USR_FORM:
	 ICanEdit = true;
	 break;
      case Rec_SHA_OTHER_EXISTING_USR_FORM:
	 ICanEdit = Usr_ICanChangeOtherUsrData (UsrDat);
	 break;
      default:	// In other options, I can not edit user's data
	 ICanEdit = false;
         break;
     }

   /* Class for labels */
   switch (TypeOfView)
     {
      case Rec_SHA_SIGN_UP_IN_CRS_FORM:
      case Rec_SHA_MY_RECORD_FORM:
      case Rec_SHA_OTHER_NEW_USR_FORM:
      case Rec_SHA_OTHER_EXISTING_USR_FORM:
         ClassForm = The_ClassForm[Gbl.Prefs.Theme];
	 break;
      case Rec_SHA_MY_RECORD_CHECK:
      case Rec_SHA_OTHER_USR_CHECK:
      case Rec_SHA_RECORD_LIST:
      case Rec_SHA_RECORD_PUBLIC:
      case Rec_SHA_RECORD_PRINT:
         ClassForm = "REC_DAT";
         break;
     }

   Rec_RecordHelp[Rec_SHA_RECORD_LIST] = Rec_RecordListHelp[UsrDat->Role.InCurrentCrs];

   PutFormLinks = !Gbl.Form.Inside &&						// Only if not inside another form
                  Act_Actions[Gbl.Action.Act].BrowserWindow == Act_THIS_WINDOW;	// Only in main window

   Ins.InsCod = UsrDat->InsCod;
   if (Ins.InsCod > 0)
      Ins_GetDataOfInstitutionByCod (&Ins,Ins_GET_BASIC_DATA);

   /***** Start frame *****/
   sprintf (StrRecordWidth,"%upx",Rec_RECORD_WIDTH);
   Gbl.Record.UsrDat = UsrDat;
   Gbl.Record.TypeOfView = TypeOfView;
   Lay_StartRoundFrameTable (StrRecordWidth,NULL,
                             TypeOfView == Rec_SHA_OTHER_NEW_USR_FORM ? NULL :	// New user ==> don't put icons
                        	                                        Rec_PutIconsCommands,
                             Rec_RecordHelp[TypeOfView],
                             2);

   /***** Institution and user's photo *****/
   fprintf (Gbl.F.Out,"<tr>");
   Rec_ShowInstitutionInHead (&Ins,PutFormLinks);
   Rec_ShowPhoto (UsrDat);
   fprintf (Gbl.F.Out,"</tr>");

   /***** Commands and full name *****/
   fprintf (Gbl.F.Out,"<tr>");
   Rec_ShowFullName (UsrDat);
   fprintf (Gbl.F.Out,"</tr>");

   /***** User's nickname *****/
   fprintf (Gbl.F.Out,"<tr>");
   Rec_ShowNickname (UsrDat,PutFormLinks);
   fprintf (Gbl.F.Out,"</tr>");

   /***** User's country, web and social networks *****/
   fprintf (Gbl.F.Out,"<tr>");
   Rec_ShowCountryInHead (UsrDat,ShowData);
   Rec_ShowWebsAndSocialNets (UsrDat,TypeOfView);
   fprintf (Gbl.F.Out,"</tr>");

   if (ShowIDRows ||
       ShowAddressRows ||
       ShowTeacherRows)
     {
      fprintf (Gbl.F.Out,"<tr>"
			 "<td colspan=\"3\">");

      /***** Show email and user's IDs *****/
      if (ShowIDRows)
	{
         Lay_StartTableWide (2);

         /* Show email */
	 Rec_ShowEmail (UsrDat,ClassForm);

	 /* Show user's IDs */
	 Rec_ShowUsrIDs (UsrDat,ClassForm,Anchor);

         Lay_EndTable ();
	}

      /***** Start form *****/
      switch (TypeOfView)
        {
	 case Rec_SHA_SIGN_UP_IN_CRS_FORM:
            Act_FormStart (ActSignUp);
            break;
	 case Rec_SHA_MY_RECORD_FORM:
	    Act_FormStart (ActChgMyData);
            break;
         case Rec_SHA_OTHER_EXISTING_USR_FORM:
            switch (Gbl.Action.Act)
	      {
               case ActReqMdfStd:
		  NextAction = ActUpdStd;
		  break;
	       case ActReqMdfNET:
		  NextAction = ActUpdNET;
		  break;
	       case ActReqMdfTch:
		  NextAction = ActUpdTch;
		  break;
	       default:
		  NextAction = ActUpdOth;
		  break;
	      }
	    Act_FormStart (NextAction);
	    Usr_PutParamUsrCodEncrypted (UsrDat->EncryptedUsrCod);	// Existing user
	    break;
	 case Rec_SHA_OTHER_NEW_USR_FORM:
	    switch (Gbl.Action.Act)
	      {
	       case ActReqMdfStd:
		  NextAction = ActCreStd;
		  break;
	       case ActReqMdfNET:
		  NextAction = ActCreNET;
		  break;
	       case ActReqMdfTch:
		  NextAction = ActCreTch;
		  break;
	       default:
		  NextAction = ActCreOth;
		  break;
	      }
	    Act_FormStart (NextAction);
	    ID_PutParamOtherUsrIDPlain ();				// New user
	    break;
         default:
            break;
        }

      Lay_StartTableWide (2);

      if (ShowIDRows)
	{
	 /***** Role or sex *****/
	 Rec_ShowRole (UsrDat,TypeOfView,ClassForm);

	 /***** Name *****/
	 Rec_ShowSurname1 (UsrDat,TypeOfView,ICanEdit,ClassForm);
	 Rec_ShowSurname2 (UsrDat,ICanEdit,ClassForm);
	 Rec_ShowFirstName (UsrDat,TypeOfView,ICanEdit,ClassForm);

	 /***** Country *****/
	 if (CountryForm)
            Rec_ShowCountry (UsrDat,TypeOfView,ClassForm);
	}

      if (ShowAddressRows)
	{
	 /***** Origin place *****/
         Rec_ShowOriginPlace (UsrDat,ShowData,ICanEdit,ClassForm);

	 /***** Date of birth *****/
         Rec_ShowDateOfBirth (UsrDat,ShowData,ICanEdit,ClassForm);

	 /***** Local address *****/
         Rec_ShowLocalAddress (UsrDat,ShowData,ICanEdit,ClassForm);

	 /***** Local phone *****/
         Rec_ShowLocalPhone (UsrDat,ShowData,ICanEdit,ClassForm);

	 /***** Family address *****/
         Rec_ShowFamilyAddress (UsrDat,ShowData,ICanEdit,ClassForm);

	 /***** Family phone *****/
         Rec_ShowFamilyPhone (UsrDat,ShowData,ICanEdit,ClassForm);

	 /***** User's comments *****/
         Rec_ShowComments (UsrDat,ShowData,ICanEdit,ClassForm);
	}

      if (ShowTeacherRows)
	{
	 /***** Institution *****/
         Rec_ShowInstitution (&Ins,ShowData,ClassForm);

	 /***** Centre *****/
         Rec_ShowCentre (UsrDat,ShowData,ClassForm);

	 /***** Department *****/
         Rec_ShowDepartment (UsrDat,ShowData,ClassForm);

	 /***** Office *****/
         Rec_ShowOffice (UsrDat,ShowData,ClassForm);

	 /***** Office phone *****/
         Rec_ShowOfficePhone (UsrDat,ShowData,ClassForm);
	}

      Lay_EndTable ();

      /***** Button and end form *****/
      switch (TypeOfView)
        {
	 case Rec_SHA_SIGN_UP_IN_CRS_FORM:
	    Lay_PutConfirmButton (Txt_Sign_up);
	    Act_FormEnd ();
	    break;
	 case Rec_SHA_MY_RECORD_FORM:
	    Lay_PutConfirmButton (Txt_Save_changes);
	    Act_FormEnd ();
	    break;
	 case Rec_SHA_OTHER_NEW_USR_FORM:
	    if (Gbl.CurrentCrs.Grps.NumGrps) // This course has groups?
	       Grp_ShowLstGrpsToChgOtherUsrsGrps (UsrDat->UsrCod);
	    Lay_PutConfirmButton (Txt_Register);
	    Act_FormEnd ();
	    break;
	 case Rec_SHA_OTHER_EXISTING_USR_FORM:
	    /***** Show list of groups to register/remove me/user *****/
	    if (Gbl.CurrentCrs.Grps.NumGrps) // This course has groups?
	      {
	       if (ItsMe)
		 {
		  // Don't show groups if I don't belong to course
		  if (Gbl.Usrs.Me.IBelongToCurrentCrs)
		     Grp_ShowLstGrpsToChgMyGrps ();
		 }
	       else
		  Grp_ShowLstGrpsToChgOtherUsrsGrps (UsrDat->UsrCod);
	      }

	    /***** Which action, register or removing? *****/
	    if (Enr_PutActionsRegRemOneUsr (ItsMe))
	       Lay_PutConfirmButton (Txt_Confirm);

	    Act_FormEnd ();
	    break;
	 default:
	    break;
        }

      fprintf (Gbl.F.Out,"</td>"
			 "</tr>");
     }

   /***** End frame *****/
   Lay_EndRoundFrameTable ();
  }

/*****************************************************************************/
/*********** Show commands (icon to make actions) in record card *************/
/*****************************************************************************/

static void Rec_PutIconsCommands (void)
  {
   extern struct Act_Actions Act_Actions[Act_NUM_ACTIONS];
   extern const char *Txt_Edit_my_personal_data;
   extern const char *Txt_My_public_profile;
   extern const char *Txt_Another_user_s_profile;
   extern const char *Txt_View_record_for_this_course;
   extern const char *Txt_View_record_and_office_hours;
   extern const char *Txt_Show_agenda;
   extern const char *Txt_Administer_user;
   extern const char *Txt_QR_code;
   extern const char *Txt_Write_a_message;
   extern const char *Txt_View_homework;
   extern const char *Txt_View_test_results;
   extern const char *Txt_View_attendance;
   extern const char *Txt_Following_unfollow;
   extern const char *Txt_Follow;
   bool ItsMe = (Gbl.Usrs.Me.UsrDat.UsrCod == Gbl.Record.UsrDat->UsrCod);
   bool IAmLoggedAsStudent = (Gbl.Usrs.Me.Role.Logged == Rol_STD);	// My current role is student
   bool IAmLoggedAsTeacher = (Gbl.Usrs.Me.Role.Logged == Rol_NET ||	// My current role is non-editing teacher
	                      Gbl.Usrs.Me.Role.Logged == Rol_TCH);	// My current role is teacher
   bool IAmLoggedAsSysAdm  = (Gbl.Usrs.Me.Role.Logged == Rol_SYS_ADM);	// My current role is superuser
   bool ICanViewUsrProfile;
   Act_Action_t NextAction;

   if (!Gbl.Form.Inside &&						// Only if not inside another form
       Act_Actions[Gbl.Action.Act].BrowserWindow == Act_THIS_WINDOW &&	// Only in main window
       Gbl.Usrs.Me.Logged)						// Only if I am logged
     {
      ICanViewUsrProfile = Pri_ShowingIsAllowed (Gbl.Record.UsrDat->ProfileVisibility,
				                 Gbl.Record.UsrDat);

      /***** Start container *****/
      fprintf (Gbl.F.Out,"<div class=\"FRAME_ICO\">");

      if (ItsMe)
         /***** Button to edit my record card *****/
	 Lay_PutContextualLink (ActReqEdiRecCom,NULL,NULL,
	                        "edit64x64.png",
			        Txt_Edit_my_personal_data,NULL,
		                NULL);
      if (ICanViewUsrProfile)
         /***** Button to view user's profile *****/
         Lay_PutContextualLink (ActSeeOthPubPrf,NULL,
			        Rec_PutParamUsrCodEncrypted,
				"usr64x64.gif",
				ItsMe ? Txt_My_public_profile :
			                Txt_Another_user_s_profile,NULL,
				NULL);

      /***** Button to view user's record card *****/
      if (Usr_CheckIfICanViewRecordStd (Gbl.Record.UsrDat))
	 /* View student's records: common record card and course record card */
         Lay_PutContextualLink (ActSeeRecOneStd,NULL,
			        Rec_PutParamUsrCodEncrypted,
				"card64x64.gif",
				Txt_View_record_for_this_course,NULL,
				NULL);
      else if (Usr_CheckIfICanViewRecordTch (Gbl.Record.UsrDat))
	 Lay_PutContextualLink (ActSeeRecOneTch,NULL,
				Rec_PutParamUsrCodEncrypted,
				"card64x64.gif",
				Txt_View_record_and_office_hours,NULL,
				NULL);

      /***** Button to view user's agenda *****/
      if (ItsMe)
	 Lay_PutContextualLink (ActSeeMyAgd,NULL,NULL,
				"calendar64x64.png",
				Txt_Show_agenda,NULL,
				NULL);
      else if (Usr_CheckIfICanViewUsrAgenda (Gbl.Record.UsrDat))
	 Lay_PutContextualLink (ActSeeUsrAgd,NULL,Rec_PutParamUsrCodEncrypted,
				"calendar64x64.png",
				Txt_Show_agenda,NULL,
				NULL);

      /***** Button to admin user *****/
      if (ItsMe ||
	  (Gbl.CurrentCrs.Crs.CrsCod > 0 && Gbl.Usrs.Me.Role.Logged == Rol_TCH) ||
	  (Gbl.CurrentDeg.Deg.DegCod > 0 && Gbl.Usrs.Me.Role.Logged == Rol_DEG_ADM) ||
	  (Gbl.CurrentCtr.Ctr.CtrCod > 0 && Gbl.Usrs.Me.Role.Logged == Rol_CTR_ADM) ||
	  (Gbl.CurrentIns.Ins.InsCod > 0 && Gbl.Usrs.Me.Role.Logged == Rol_INS_ADM) ||
	  Gbl.Usrs.Me.Role.Logged == Rol_SYS_ADM)
	{
	 switch (Gbl.Record.UsrDat->Role.InCurrentCrs)
	   {
	    case Rol_STD:
	       NextAction = ActReqMdfStd;
	       break;
	    case Rol_NET:
	       NextAction = ActReqMdfNET;
	       break;
	    case Rol_TCH:
	       NextAction = ActReqMdfTch;
	       break;
	    default:	// Guest, user or admin
	       NextAction = ActReqMdfOth;
	       break;
	   }
	 Lay_PutContextualLink (NextAction,NULL,
	                        Rec_PutParamUsrCodEncrypted,
				"config64x64.gif",
			        Txt_Administer_user,NULL,
		                NULL);
	}

      if (Gbl.CurrentCrs.Crs.CrsCod > 0 &&	// A course is selected
	  Gbl.Record.UsrDat->Role.InCurrentCrs == Rol_STD &&	// He/she is a student in the current course
	  (ItsMe || IAmLoggedAsTeacher || IAmLoggedAsSysAdm))	// I can view
	{
	 /***** Button to view user's assignments and works *****/
	 if (ItsMe)	// I am a student
	    Lay_PutContextualLink (ActAdmAsgWrkUsr,NULL,NULL,
			           "folder64x64.gif",
			           Txt_View_homework,NULL,
		                   NULL);
	 else		// I am a teacher or superuser
	    Lay_PutContextualLink (ActAdmAsgWrkCrs,NULL,Rec_PutParamsWorks,
			           "folder64x64.gif",
			           Txt_View_homework,NULL,
		                   NULL);

	 /***** Button to view user's test exams *****/
	 if (ItsMe)
	    Lay_PutContextualLink (ActSeeMyTstRes,NULL,NULL,
			           "exam64x64.png",
			           Txt_View_test_results,NULL,
		                   NULL);
	 else
	    Lay_PutContextualLink (ActSeeUsrTstRes,NULL,Rec_PutParamsStudent,
			           "exam64x64.png",
			           Txt_View_test_results,NULL,
		                   NULL);

	 /***** Button to view user's attendance *****/
	 if (ItsMe && IAmLoggedAsStudent)
	    // As student, I can see my attendance
	    Lay_PutContextualLink (ActSeeLstMyAtt,NULL,NULL,
				   "rollcall64x64.png",
				   Txt_View_attendance,NULL,
				   NULL);
	 else if (IAmLoggedAsTeacher || IAmLoggedAsSysAdm)
	    // As teacher, I can see attendance of the student
	    Lay_PutContextualLink (ActSeeLstStdAtt,NULL,Rec_PutParamsStudent,
				   "rollcall64x64.png",
				   Txt_View_attendance,NULL,
				   NULL);
	}

      /***** Button to print QR code *****/
      if (ItsMe || IAmLoggedAsSysAdm ||
	  (Gbl.CurrentCrs.Crs.CrsCod > 0 &&			// A course is selected
	   Gbl.Record.UsrDat->Role.InCurrentCrs == Rol_STD &&	// He/she is a student in the current course
	   IAmLoggedAsTeacher))					// I am a teacher in the current course
	 Lay_PutContextualLink (ActPrnUsrQR,NULL,Rec_PutParamUsrCodEncrypted,
				"qr64x64.gif",
				Txt_QR_code,NULL,
				NULL);

      /***** Button to send a message *****/
      Lay_PutContextualLink (ActReqMsgUsr,NULL,Rec_PutParamsMsgUsr,
			     "msg64x64.gif",
			     Txt_Write_a_message,NULL,
		             NULL);

      /***** Button to follow / unfollow *****/
      if (!ItsMe)
	{
	 if (Fol_CheckUsrIsFollowerOf (Gbl.Usrs.Me.UsrDat.UsrCod,
				       Gbl.Record.UsrDat->UsrCod))
	    // I follow user
	    Lay_PutContextualLink (ActUnfUsr,NULL,Rec_PutParamUsrCodEncrypted,
				   "following64x64.png",
				   Txt_Following_unfollow,NULL,
				   NULL);	// Put button to unfollow, even if I can not view user's profile
	 else if (ICanViewUsrProfile)
	    Lay_PutContextualLink (ActFolUsr,NULL,Rec_PutParamUsrCodEncrypted,
				   "follow64x64.png",
				   Txt_Follow,NULL,
				   NULL);	// Put button to follow
	}

      /***** End container *****/
      fprintf (Gbl.F.Out,"</div>");
     }
  }

static void Rec_PutParamUsrCodEncrypted (void)
  {
   Usr_PutParamUsrCodEncrypted (Gbl.Record.UsrDat->EncryptedUsrCod);
  }

static void Rec_PutParamsWorks (void)
  {
   Rec_PutParamsStudent ();
   Par_PutHiddenParamChar ("FullTree",'Y');	// By default, show all files
  }

static void Rec_PutParamsStudent (void)
  {
   Par_PutHiddenParamString ("UsrCodStd",Gbl.Record.UsrDat->EncryptedUsrCod);
   Grp_PutParamAllGroups ();
  }

static void Rec_PutParamsMsgUsr (void)
  {
   Rec_PutParamUsrCodEncrypted ();
   Grp_PutParamAllGroups ();
   Par_PutHiddenParamChar ("ShowOnlyOneRecipient",'Y');
  }

/*****************************************************************************/
/*********************** Show institution in record card *********************/
/*****************************************************************************/

static void Rec_ShowInstitutionInHead (struct Instit *Ins,bool PutFormLinks)
  {
   /***** Institution logo *****/
   fprintf (Gbl.F.Out,"<td rowspan=\"4\" class=\"REC_C1_TOP CENTER_MIDDLE\">");
   if (Ins->InsCod > 0)
     {
      /* Form to go to the institution */
      if (PutFormLinks)
	{
	 Act_FormGoToStart (ActSeeInsInf);
	 Ins_PutParamInsCod (Ins->InsCod);
	 Act_LinkFormSubmit (Ins->FullName,NULL,NULL);
	}
      Log_DrawLogo (Sco_SCOPE_INS,Ins->InsCod,Ins->ShrtName,
                    Rec_INSTITUTION_LOGO_SIZE,NULL,true);
      if (PutFormLinks)
	{
         fprintf (Gbl.F.Out,"</a>");
	 Act_FormEnd ();
	}
     }
   fprintf (Gbl.F.Out,"</td>");

   /***** Institution name *****/
   fprintf (Gbl.F.Out,"<td class=\"REC_C2_TOP REC_HEAD LEFT_MIDDLE\">");
   if (Ins->InsCod > 0)
     {
      /* Form to go to the institution */
      if (PutFormLinks)
	{
	 Act_FormGoToStart (ActSeeInsInf);
	 Ins_PutParamInsCod (Ins->InsCod);
	 Act_LinkFormSubmit (Ins->FullName,"REC_HEAD",NULL);
	}
      fprintf (Gbl.F.Out,"%s",Ins->FullName);
      if (PutFormLinks)
	{
         fprintf (Gbl.F.Out,"</a>");
	 Act_FormEnd ();
	}
     }
   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/********************** Show user's photo in record card *********************/
/*****************************************************************************/

static void Rec_ShowPhoto (struct UsrData *UsrDat)
  {
   char PhotoURL[PATH_MAX + 1];
   bool ShowPhoto = Pho_ShowingUsrPhotoIsAllowed (UsrDat,PhotoURL);

   /***** User's photo *****/
   fprintf (Gbl.F.Out,"<td rowspan=\"3\" class=\"REC_C3_TOP CENTER_TOP\">");
   Pho_ShowUsrPhoto (UsrDat,ShowPhoto ? PhotoURL :
                	                NULL,
		     "PHOTO186x248",Pho_NO_ZOOM,false);
   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/*************************** Show user's full name ***************************/
/*****************************************************************************/

static void Rec_ShowFullName (struct UsrData *UsrDat)
  {
   fprintf (Gbl.F.Out,"<td class=\"REC_C2_MID LEFT_TOP\">"
	              "<div class=\"REC_NAME\">");

   /***** First name *****/
   fprintf (Gbl.F.Out,"%s<br />",UsrDat->FirstName);

   /***** Surname 1 *****/
   fprintf (Gbl.F.Out,"%s<br />",UsrDat->Surname1);

   /***** Surname 2 *****/
   fprintf (Gbl.F.Out,"%s",UsrDat->Surname2);

   fprintf (Gbl.F.Out,"</div>"
	              "</td>");
  }

/*****************************************************************************/
/*************************** Show user's nickname ****************************/
/*****************************************************************************/

static void Rec_ShowNickname (struct UsrData *UsrDat,bool PutFormLinks)
  {
   extern const char *Txt_My_public_profile;
   extern const char *Txt_Another_user_s_profile;
   bool ItsMe;

   fprintf (Gbl.F.Out,"<td class=\"REC_C2_MID LEFT_BOTTOM\">"
	              "<div class=\"REC_NICK\">");
   if (UsrDat->Nickname[0])
     {
      if (PutFormLinks)
	{
	 /* Put form to go to public profile */
         ItsMe = (Gbl.Usrs.Me.Logged &&
	          UsrDat->UsrCod == Gbl.Usrs.Me.UsrDat.UsrCod);
	 Act_FormStart (ActSeeOthPubPrf);
	 Usr_PutParamUsrCodEncrypted (UsrDat->EncryptedUsrCod);
	 Act_LinkFormSubmit (ItsMe ? Txt_My_public_profile :
			             Txt_Another_user_s_profile,
			     "REC_NICK",NULL);
	}
      fprintf (Gbl.F.Out,"@%s",UsrDat->Nickname);
      if (PutFormLinks)
	{
	 fprintf (Gbl.F.Out,"</a>");
	 Act_FormEnd ();
	}
     }
   fprintf (Gbl.F.Out,"</div>"
	              "</td>");
  }

/*****************************************************************************/
/**************************** Show user's country ****************************/
/*****************************************************************************/

static void Rec_ShowCountryInHead (struct UsrData *UsrDat,bool ShowData)
  {
   fprintf (Gbl.F.Out,"<td class=\"REC_C2_MID REC_DAT_BOLD LEFT_TOP\">");
   if (ShowData && UsrDat->CtyCod > 0)
      /* Link to see country information */
      Cty_WriteCountryName (UsrDat->CtyCod,
                            "REC_DAT_BOLD");	// Put link to country
   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/******************* Show user's webs and social networks ********************/
/*****************************************************************************/

static void Rec_ShowWebsAndSocialNets (struct UsrData *UsrDat,
                                       Rec_SharedRecordViewType_t TypeOfView)
  {
   fprintf (Gbl.F.Out,"<td class=\"REC_C3_MID CENTER_TOP\">");
   if (TypeOfView != Rec_SHA_RECORD_PRINT)
      Net_ShowWebsAndSocialNets (UsrDat);
   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/***************************** Show user's email *****************************/
/*****************************************************************************/

static void Rec_ShowEmail (struct UsrData *UsrDat,const char *ClassForm)
  {
   extern const char *Txt_Email;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
		      "%s:"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Email);
   if (UsrDat->Email[0])
     {
      fprintf (Gbl.F.Out,"<div class=\"REC_EMAIL\">");	// Limited width
      if (Mai_ICanSeeOtherUsrEmail (UsrDat))
	 fprintf (Gbl.F.Out,"<a href=\"mailto:%s\" class=\"REC_DAT_BOLD\">"
			    "%s"
			    "</a>",
		  UsrDat->Email,
		  UsrDat->Email);
      else
	 fprintf (Gbl.F.Out,"********");
      fprintf (Gbl.F.Out,"</div>");
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/******************************* Show user's IDs *****************************/
/*****************************************************************************/

static void Rec_ShowUsrIDs (struct UsrData *UsrDat,const char *ClassForm,
                            const char *Anchor)
  {
   extern const char *Txt_ID;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_TOP %s\">"
		      "%s:"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_TOP\">",
	    ClassForm,Txt_ID);
   ID_WriteUsrIDs (UsrDat,Anchor);
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************** Show user's role / sex ***************************/
/*****************************************************************************/

static void Rec_ShowRole (struct UsrData *UsrDat,
                          Rec_SharedRecordViewType_t TypeOfView,
                          const char *ClassForm)
  {
   extern const char *Usr_StringsSexDB[Usr_NUM_SEXS];
   extern const char *Txt_Role;
   extern const char *Txt_Sex;
   extern const char *Txt_SEX_SINGULAR_Abc[Usr_NUM_SEXS];
   extern const char *Txt_ROLES_SINGUL_Abc[Rol_NUM_ROLES][Usr_NUM_SEXS];
   bool RoleForm = (TypeOfView == Rec_SHA_SIGN_UP_IN_CRS_FORM ||
                    TypeOfView == Rec_SHA_OTHER_EXISTING_USR_FORM ||
                    TypeOfView == Rec_SHA_OTHER_NEW_USR_FORM);
   bool SexForm = (TypeOfView == Rec_SHA_MY_RECORD_FORM);
   Rol_Role_t DefaultRoleInForm;
   Rol_Role_t Role;
   Usr_Sex_t Sex;

   if (RoleForm)
     {
      /***** Form to select a role *****/
      /* Get user's roles if not got */
      Rol_GetRolesInAllCrssIfNotYetGot (UsrDat);

      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
			 "<label for=\"Role\" class=\"%s\">%s:</label>"
			 "</td>"
			 "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	       ClassForm,Txt_Role);
      switch (TypeOfView)
	{
	 case Rec_SHA_SIGN_UP_IN_CRS_FORM:			// I want to apply for enrolment
            /***** Set default role *****/
	    if (UsrDat->UsrCod == Gbl.CurrentCrs.Crs.RequesterUsrCod ||	// Creator of the course
		(UsrDat->Role.InCrss & (1 << Rol_TCH)))			// Teacher in other courses
	       DefaultRoleInForm = Rol_TCH;	// Request sign up as a teacher
	    else if ((UsrDat->Role.InCrss & (1 << Rol_NET)))			// Non-editing teacher in other courses
	       DefaultRoleInForm = Rol_NET;	// Request sign up as a non-editing teacher
	    else
	       DefaultRoleInForm = Rol_STD;	// Request sign up as a student

	    /***** Selector of role *****/
	    fprintf (Gbl.F.Out,"<select id=\"Role\" name=\"Role\">");
	    for (Role = Rol_STD;
		 Role <= Rol_TCH;
		 Role++)
	      {
	       fprintf (Gbl.F.Out,"<option value=\"%u\"",(unsigned) Role);
	       if (Role == DefaultRoleInForm)
		  fprintf (Gbl.F.Out," selected=\"selected\"");
	       fprintf (Gbl.F.Out,">%s</option>",
			Txt_ROLES_SINGUL_Abc[Role][UsrDat->Sex]);
	      }
	    fprintf (Gbl.F.Out,"</select>");
	    break;
	 case Rec_SHA_OTHER_EXISTING_USR_FORM:	// The other user already exists in the platform
            if (Gbl.CurrentCrs.Crs.CrsCod > 0)	// Course selected
	      {
               /***** Set default role *****/
	       switch (UsrDat->Role.InCurrentCrs)
		 {
		  case Rol_STD:	// Student in current course
		  case Rol_NET:	// Non-editing teacher in current course
		  case Rol_TCH:	// Teacher in current course
		     DefaultRoleInForm = UsrDat->Role.InCurrentCrs;
		  default:	// User does not belong to current course
		     /* If there is a request of this user, default role is the requested role */
		     DefaultRoleInForm = Rol_GetRequestedRole (UsrDat->UsrCod);

	             switch (DefaultRoleInForm)
	               {
			case Rol_STD:	// Role requested: student
			case Rol_NET:	// Role requested: non-editing teacher
			case Rol_TCH:	// Role requested: teacher
			   break;
			default:	// No role requested
			   switch (Gbl.Action.Act)
			     {
			      case ActReqMdfStd:
				 DefaultRoleInForm = Rol_STD;
				 break;
			      case ActReqMdfNET:
				 DefaultRoleInForm = Rol_NET;
				 break;
			      case ActReqMdfTch:
				 DefaultRoleInForm = Rol_TCH;
				 break;
			      default:
				 if ((UsrDat->Role.InCrss & (1 << Rol_TCH)))		// Teacher in other courses
				    DefaultRoleInForm = Rol_TCH;
				 else if ((UsrDat->Role.InCrss & (1 << Rol_NET)))	// Non-editing teacher in other courses
				    DefaultRoleInForm = Rol_NET;
				 else
				    DefaultRoleInForm = Rol_STD;
			         break;
			     }
			   break;
		       }
		     break;
		 }

	       /***** Selector of role *****/
	       fprintf (Gbl.F.Out,"<select id=\"Role\" name=\"Role\">");
	       switch (Gbl.Usrs.Me.Role.Logged)
		 {
		  case Rol_GST:
		  case Rol_USR:
		  case Rol_STD:
		  case Rol_NET:
		     fprintf (Gbl.F.Out,"<option value=\"%u\""
			                " selected=\"selected\""
			                " disabled=\"disabled\">"
			                "%s</option>",
			      (unsigned) Gbl.Usrs.Me.Role.Logged,
			      Txt_ROLES_SINGUL_Abc[Gbl.Usrs.Me.Role.Logged][UsrDat->Sex]);
		     break;
		  case Rol_TCH:
		  case Rol_DEG_ADM:
		  case Rol_CTR_ADM:
		  case Rol_INS_ADM:
		  case Rol_SYS_ADM:
		     for (Role = Rol_STD;
			  Role <= Rol_TCH;
			  Role++)
		       {
			fprintf (Gbl.F.Out,"<option value=\"%u\"",(unsigned) Role);
			if (Role == DefaultRoleInForm)
			   fprintf (Gbl.F.Out," selected=\"selected\"");
			fprintf (Gbl.F.Out,">%s</option>",
				 Txt_ROLES_SINGUL_Abc[Role][UsrDat->Sex]);
		       }
		     break;
		  default: // The rest of users can not register other users
		     break;
		 }
	       fprintf (Gbl.F.Out,"</select>");
	      }
	    else				// No course selected
	      {
               /***** Set default role *****/
	       DefaultRoleInForm = (UsrDat->Role.InCrss & ((1 << Rol_STD) |
		                                     (1 << Rol_NET) |
						     (1 << Rol_TCH))) ? Rol_USR :	// If user belongs to any course
								        Rol_GST;	// If user don't belong to any course

	       /***** Selector of role *****/
	       fprintf (Gbl.F.Out,"<select id=\"Role\" name=\"Role\">"
	                          "<option value=\"%u\" selected=\"selected\""
		                  " disabled=\"disabled\">%s</option>"
	                          "</select>",
			(unsigned) DefaultRoleInForm,
			Txt_ROLES_SINGUL_Abc[DefaultRoleInForm][UsrDat->Sex]);
	      }
	    break;
	 case Rec_SHA_OTHER_NEW_USR_FORM:	// The user does not exist in platform
	    if (Gbl.CurrentCrs.Crs.CrsCod > 0)	// Course selected
	       switch (Gbl.Usrs.Me.Role.Logged)
		 {
		  case Rol_TCH:
		  case Rol_DEG_ADM:
		  case Rol_CTR_ADM:
		  case Rol_INS_ADM:
		  case Rol_SYS_ADM:
                     /***** Set default role *****/
		     switch (Gbl.Action.Act)
		       {
			case ActReqMdfStd:
			   DefaultRoleInForm = Rol_STD;
			   break;
			case ActReqMdfNET:
			   DefaultRoleInForm = Rol_NET;
			   break;
			case ActReqMdfTch:
			   DefaultRoleInForm = Rol_TCH;
			   break;
			default:
			   DefaultRoleInForm = Rol_STD;
			   break;
		       }

		     /***** Selector of role *****/
		     fprintf (Gbl.F.Out,"<select id=\"Role\" name=\"Role\">");
		     for (Role = Rol_STD;
			  Role <= Rol_TCH;
			  Role++)
		       {
			fprintf (Gbl.F.Out,"<option value=\"%u\"",(unsigned) Role);
			if (Role == DefaultRoleInForm)
			   fprintf (Gbl.F.Out," selected=\"selected\"");
			fprintf (Gbl.F.Out,">%s</option>",
				 Txt_ROLES_SINGUL_Abc[Role][Usr_SEX_UNKNOWN]);
		       }
		     fprintf (Gbl.F.Out,"</select>");
		     break;
		  default:	// The rest of users can not register other users
		     break;
		 }
	    else				// No course selected
	       switch (Gbl.Usrs.Me.Role.Logged)
		 {
		  case Rol_SYS_ADM:
		     /***** Selector of role *****/
		     fprintf (Gbl.F.Out,"<select id=\"Role\" name=\"Role\">"
		                        "<option value=\"%u\""
			                " selected=\"selected\">%s</option>"
		                        "</select>",
			      (unsigned) Rol_GST,Txt_ROLES_SINGUL_Abc[Rol_GST][Usr_SEX_UNKNOWN]);
		     break;
		  default:	// The rest of users can not register other users
		     break;
		 }
	    break;
	 default:
	    break;
	}
      fprintf (Gbl.F.Out,"</td>"
			 "</tr>");
     }
   else if (SexForm)
     {
      /***** Form to select a sex *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
			 "%s*:</td>"
			 "<td class=\"REC_C2_BOT LEFT_MIDDLE\">",
	       ClassForm,Txt_Sex);
      for (Sex = Usr_SEX_FEMALE;
	   Sex <= Usr_SEX_MALE;
	   Sex++)
	{
	 fprintf (Gbl.F.Out,"<label class=\"REC_DAT_BOLD\">"
	                    "<input type=\"radio\" name=\"Sex\" value=\"%u\"",
	          (unsigned) Sex);
	 if (Sex == Gbl.Usrs.Me.UsrDat.Sex)
	    fprintf (Gbl.F.Out," checked=\"checked\"");
	 fprintf (Gbl.F.Out," required=\"required\" />"
			    "<img src=\"%s/%s16x16.gif\""
			    " alt=\"%s\" title=\"%s\" class=\"ICO20x20\" />"
			    "%s"
			    "</label>",
		  Gbl.Prefs.IconsURL,Usr_StringsSexDB[Sex],
		  Txt_SEX_SINGULAR_Abc[Sex],
		  Txt_SEX_SINGULAR_Abc[Sex],
		  Txt_SEX_SINGULAR_Abc[Sex]);
	}
      fprintf (Gbl.F.Out,"</td>"
			 "</tr>");
     }
   else	// RoleForm == false, SexForm == false
      /***** No form, only text *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
			 "%s:"
			 "</td>"
			 "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">"
			 "%s"
			 "</td>"
			 "</tr>",
	       ClassForm,
	       TypeOfView == Rec_SHA_MY_RECORD_CHECK ? Txt_Sex :
					               Txt_Role,
	       TypeOfView == Rec_SHA_MY_RECORD_CHECK ? Txt_SEX_SINGULAR_Abc[UsrDat->Sex] :
						       Txt_ROLES_SINGUL_Abc[UsrDat->Role.InCurrentCrs][UsrDat->Sex]);
  }

/*****************************************************************************/
/*************************** Show user's surname 1 ***************************/
/*****************************************************************************/

static void Rec_ShowSurname1 (struct UsrData *UsrDat,
                              Rec_SharedRecordViewType_t TypeOfView,
                              bool ICanEdit,
                              const char *ClassForm)
  {
   extern const char *Txt_Surname_1;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
		      "<label for=\"Surname1\" class=\"%s\">"
		      "%s",
	    ClassForm,Txt_Surname_1);
   if (TypeOfView == Rec_SHA_MY_RECORD_FORM)
      fprintf (Gbl.F.Out,"*");
   fprintf (Gbl.F.Out,":"
	              "</label>"
	              "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">");
   if (ICanEdit)
     {
      fprintf (Gbl.F.Out,"<input type=\"text\""
	                 " id=\"Surname1\" name=\"Surname1\""
			 " maxlength=\"%u\" value=\"%s\""
			 " class=\"REC_C2_BOT_INPUT\"",
	       Usr_MAX_CHARS_FIRSTNAME_OR_SURNAME,
	       UsrDat->Surname1);
      if (TypeOfView == Rec_SHA_MY_RECORD_FORM)
         fprintf (Gbl.F.Out," required=\"required\"");
      fprintf (Gbl.F.Out," />");
     }
   else if (UsrDat->Surname1[0])
      fprintf (Gbl.F.Out,"<strong>%s</strong>",UsrDat->Surname1);
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }


/*****************************************************************************/
/*************************** Show user's surname 2 ***************************/
/*****************************************************************************/

static void Rec_ShowSurname2 (struct UsrData *UsrDat,
                              bool ICanEdit,
                              const char *ClassForm)
  {
   extern const char *Txt_Surname_2;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
		      "<label for=\"Surname2\" class=\"%s\">"
		      "%s:"
		      "</label>"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Surname_2);
   if (ICanEdit)
      fprintf (Gbl.F.Out,"<input type=\"text\""
	                 " id=\"Surname2\" name=\"Surname2\""
			 " maxlength=\"%u\" value=\"%s\""
			 " class=\"REC_C2_BOT_INPUT\" />",
	       Usr_MAX_CHARS_FIRSTNAME_OR_SURNAME,
	       UsrDat->Surname2);
   else if (UsrDat->Surname2[0])
      fprintf (Gbl.F.Out,"<strong>%s</strong>",UsrDat->Surname2);
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************** Show user's first name ***************************/
/*****************************************************************************/

static void Rec_ShowFirstName (struct UsrData *UsrDat,
                               Rec_SharedRecordViewType_t TypeOfView,
                               bool ICanEdit,
                               const char *ClassForm)
  {
   extern const char *Txt_First_name;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
		      "<label for=\"FirstName\" class=\"%s\">"
		      "%s",
	    ClassForm,Txt_First_name);
   if (TypeOfView == Rec_SHA_MY_RECORD_FORM)
      fprintf (Gbl.F.Out,"*");
   fprintf (Gbl.F.Out,":"
                      "</label>"
	              "</td>"
		      "<td colspan=\"2\""
		      " class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">");
   if (ICanEdit)
     {
      fprintf (Gbl.F.Out,"<input type=\"text\""
	                 " id=\"FirstName\" name=\"FirstName\""
                         " maxlength=\"%u\" value=\"%s\""
			 " class=\"REC_C2_BOT_INPUT\"",
	       Usr_MAX_CHARS_FIRSTNAME_OR_SURNAME,
	       UsrDat->FirstName);
      if (TypeOfView == Rec_SHA_MY_RECORD_FORM)
         fprintf (Gbl.F.Out," required=\"required\"");
      fprintf (Gbl.F.Out," />");
     }
   else if (UsrDat->FirstName[0])
      fprintf (Gbl.F.Out,"<strong>%s</strong>",UsrDat->FirstName);
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/**************************** Show user's country ****************************/
/*****************************************************************************/

static void Rec_ShowCountry (struct UsrData *UsrDat,
                             Rec_SharedRecordViewType_t TypeOfView,
                             const char *ClassForm)
  {
   extern const char *Txt_Country;
   extern const char *Txt_Another_country;
   unsigned NumCty;

   /***** If list of countries is empty, try to get it *****/
   if (!Gbl.Ctys.Num)
     {
      Gbl.Ctys.SelectedOrder = Cty_ORDER_BY_COUNTRY;
      Cty_GetListCountries (Cty_GET_BASIC_DATA);
     }

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
                      "<label for=\"OthCtyCod\" class=\"%s\">%s",
	    ClassForm,Txt_Country);
   if (TypeOfView == Rec_SHA_MY_RECORD_FORM)
      fprintf (Gbl.F.Out,"*");
   fprintf (Gbl.F.Out,":</label>"
	              "</td>"
		      "<td colspan=\"2\""
		      " class=\"REC_C2_BOT LEFT_MIDDLE\">");

   /***** Selector of country *****/
   fprintf (Gbl.F.Out,"<select id=\"OthCtyCod\" name=\"OthCtyCod\""
	              " class=\"REC_C2_BOT_INPUT\" required=\"required\">"
		      "<option value=\"\">%s</option>"
		      "<option value=\"0\"",
	    Txt_Country);
   if (UsrDat->CtyCod == 0)
      fprintf (Gbl.F.Out," selected=\"selected\"");
   fprintf (Gbl.F.Out,">%s</option>",Txt_Another_country);
   for (NumCty = 0;
	NumCty < Gbl.Ctys.Num;
	NumCty++)
     {
      fprintf (Gbl.F.Out,"<option value=\"%ld\"",
	       Gbl.Ctys.Lst[NumCty].CtyCod);
      if (Gbl.Ctys.Lst[NumCty].CtyCod == UsrDat->CtyCod)
	 fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",
	       Gbl.Ctys.Lst[NumCty].Name[Gbl.Prefs.Language]);
     }
   fprintf (Gbl.F.Out,"</select>"
		      "</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************ Show user's place of origin ************************/
/*****************************************************************************/

static void Rec_ShowOriginPlace (struct UsrData *UsrDat,
                                 bool ShowData,bool ICanEdit,
                                 const char *ClassForm)
  {
   extern const char *Txt_Place_of_origin;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
		      "<label for=\"OriginPlace\" class=\"%s\">"
		      "%s:"
                      "</label>"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Place_of_origin);
   if (ShowData)
     {
      if (ICanEdit)
	 fprintf (Gbl.F.Out,"<input type=\"text\""
	                    " id=\"OriginPlace\" name=\"OriginPlace\""
			    " maxlength=\"%u\" value=\"%s\""
			    " class=\"REC_C2_BOT_INPUT\" />",
		  Usr_MAX_CHARS_ADDRESS,
		  UsrDat->OriginPlace);
      else if (UsrDat->OriginPlace[0])
	 fprintf (Gbl.F.Out,"%s",UsrDat->OriginPlace);
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************ Show user's date of birth **************************/
/*****************************************************************************/

static void Rec_ShowDateOfBirth (struct UsrData *UsrDat,
                                 bool ShowData,bool ICanEdit,
                                 const char *ClassForm)
  {
   extern const char *Txt_Date_of_birth;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
		      "%s:"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Date_of_birth);
   if (ShowData)
     {
      if (ICanEdit)
	 Dat_WriteFormDate (Gbl.Now.Date.Year - Rec_USR_MAX_AGE,
			    Gbl.Now.Date.Year - Rec_USR_MIN_AGE,
			    "Birth",
			    &(UsrDat->Birthday),
			    false,false);
      else if (UsrDat->StrBirthday[0])
	 fprintf (Gbl.F.Out,"%s",UsrDat->StrBirthday);
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************ Show user's local address **************************/
/*****************************************************************************/

static void Rec_ShowLocalAddress (struct UsrData *UsrDat,
                                  bool ShowData,bool ICanEdit,
                                  const char *ClassForm)
  {
   extern const char *Txt_Local_address;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
		      "<label for=\"LocalAddress\" class=\"%s\">"
		      "%s:"
		      "</label>"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Local_address);
   if (ShowData)
     {
      if (ICanEdit)
	 fprintf (Gbl.F.Out,"<input type=\"text\""
	                    " id=\"LocalAddress\" name=\"LocalAddress\""
			    " maxlength=\"%u\" value=\"%s\""
			    " class=\"REC_C2_BOT_INPUT\" />",
		  Usr_MAX_CHARS_ADDRESS,
		  UsrDat->LocalAddress);
      else if (UsrDat->LocalAddress[0])
	 fprintf (Gbl.F.Out,"%s",UsrDat->LocalAddress);
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************* Show user's local phone ***************************/
/*****************************************************************************/

static void Rec_ShowLocalPhone (struct UsrData *UsrDat,
                                bool ShowData,bool ICanEdit,
                                const char *ClassForm)
  {
   extern const char *Txt_Phone;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
		      "<label for=\"LocalPhone\" class=\"%s\">"
		      "%s:"
                      "</label>"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Phone);
   if (ShowData)
     {
      if (ICanEdit)
	 fprintf (Gbl.F.Out,"<input type=\"tel\""
	                    " id=\"LocalPhone\" name=\"LocalPhone\""
			    " maxlength=\"%u\" value=\"%s\""
			    " class=\"REC_C2_BOT_INPUT\" />",
		  Usr_MAX_CHARS_PHONE,
		  UsrDat->LocalPhone);
      else if (UsrDat->LocalPhone[0])
	 fprintf (Gbl.F.Out,"<a href=\"tel:%s\" class=\"REC_DAT_BOLD\">%s</a>",
	          UsrDat->LocalPhone,
	          UsrDat->LocalPhone);
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/*********************** Show user's family address **************************/
/*****************************************************************************/

static void Rec_ShowFamilyAddress (struct UsrData *UsrDat,
                                   bool ShowData,bool ICanEdit,
                                   const char *ClassForm)
  {
   extern const char *Txt_Family_address;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
		      "<label for=\"FamilyAddress\" class=\"%s\">"
		      "%s:"
		      "</label>"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Family_address);
   if (ShowData)
     {
      if (ICanEdit)
	 fprintf (Gbl.F.Out,"<input type=\"text\""
	                    " id=\"FamilyAddress\" name=\"FamilyAddress\""
			    " maxlength=\"%u\" value=\"%s\""
			    " class=\"REC_C2_BOT_INPUT\" />",
		  Usr_MAX_CHARS_ADDRESS,
		  UsrDat->FamilyAddress);
      else if (UsrDat->FamilyAddress[0])
	 fprintf (Gbl.F.Out,"%s",UsrDat->FamilyAddress);
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************ Show user's family phone ***************************/
/*****************************************************************************/

static void Rec_ShowFamilyPhone (struct UsrData *UsrDat,
                                 bool ShowData,bool ICanEdit,
                                 const char *ClassForm)
  {
   extern const char *Txt_Phone;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE\">"
		      "<label for=\"FamilyPhone\" class=\"%s\">"
		      "%s:"
		      "</label>"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Phone);
   if (ShowData)
     {
      if (ICanEdit)
	 fprintf (Gbl.F.Out,"<input type=\"tel\""
	                    " id=\"FamilyPhone\" name=\"FamilyPhone\""
			    " maxlength=\"%u\" value=\"%s\""
			    " class=\"REC_C2_BOT_INPUT\" />",
		  Usr_MAX_CHARS_PHONE,
		  UsrDat->FamilyPhone);
      else if (UsrDat->FamilyPhone[0])
	 fprintf (Gbl.F.Out,"<a href=\"tel:%s\" class=\"REC_DAT_BOLD\">%s</a>",
	          UsrDat->FamilyPhone,
	          UsrDat->FamilyPhone);
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************** Show user's comments *****************************/
/*****************************************************************************/

static void Rec_ShowComments (struct UsrData *UsrDat,
                              bool ShowData,bool ICanEdit,
                              const char *ClassForm)
  {
   extern const char *Txt_USER_comments;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_TOP\">"
		      "<label for=\"Comments\" class=\"%s\">%s:</label>"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_TOP\">",
	    ClassForm,Txt_USER_comments);
   if (ShowData)
     {
      if (ICanEdit)
	 fprintf (Gbl.F.Out,"<textarea id=\"Comments\" name=\"Comments\""
	                    " rows=\"4\" class=\"REC_C2_BOT_INPUT\">"
			    "%s"
			    "</textarea>",
		  UsrDat->Comments);
      else if (UsrDat->Comments[0])
	{
	 Str_ChangeFormat (Str_FROM_HTML,Str_TO_RIGOROUS_HTML,     // Convert from HTML to rigorous HTML
			   UsrDat->Comments,Cns_MAX_BYTES_TEXT,false);
	 fprintf (Gbl.F.Out,"%s",UsrDat->Comments);
	}
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************** Show user's institution **************************/
/*****************************************************************************/

static void Rec_ShowInstitution (struct Instit *Ins,
                                 bool ShowData,const char *ClassForm)
  {
   extern const char *Txt_Institution;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
		      "%s:"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Institution);
   if (ShowData)
     {
      if (Ins->InsCod > 0)
	{
	 if (Ins->WWW[0])
	    fprintf (Gbl.F.Out,"<a href=\"%s\" target=\"_blank\""
			       " class=\"REC_DAT_BOLD\">",
		     Ins->WWW);
	 fprintf (Gbl.F.Out,"%s",Ins->FullName);
	 if (Ins->WWW[0])
	    fprintf (Gbl.F.Out,"</a>");
	}
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/*************************** Show user's centre ******************************/
/*****************************************************************************/

static void Rec_ShowCentre (struct UsrData *UsrDat,
                            bool ShowData,const char *ClassForm)
  {
   extern const char *Txt_Centre;
   struct Centre Ctr;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
		      "%s:"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Centre);
   if (ShowData)
     {
      if (UsrDat->Tch.CtrCod > 0)
	{
	 Ctr.CtrCod = UsrDat->Tch.CtrCod;
	 Ctr_GetDataOfCentreByCod (&Ctr);
	 if (Ctr.WWW[0])
	    fprintf (Gbl.F.Out,"<a href=\"%s\" target=\"_blank\""
			       " class=\"REC_DAT_BOLD\">",
		     Ctr.WWW);
	 fprintf (Gbl.F.Out,"%s",Ctr.FullName);
	 if (Ctr.WWW[0])
	    fprintf (Gbl.F.Out,"</a>");
	}
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************************* Show user's department ****************************/
/*****************************************************************************/

static void Rec_ShowDepartment (struct UsrData *UsrDat,
                                bool ShowData,const char *ClassForm)
  {
   extern const char *Txt_Department;
   struct Department Dpt;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
		      "%s:"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Department);
   if (ShowData)
     {
      if (UsrDat->Tch.DptCod > 0)
	{
	 Dpt.DptCod = UsrDat->Tch.DptCod;
	 Dpt_GetDataOfDepartmentByCod (&Dpt);
	 if (Dpt.WWW[0])
	    fprintf (Gbl.F.Out,"<a href=\"%s\" target=\"_blank\""
			       " class=\"REC_DAT_BOLD\">",
		     Dpt.WWW);
	 fprintf (Gbl.F.Out,"%s",Dpt.FullName);
	 if (Dpt.WWW[0])
	    fprintf (Gbl.F.Out,"</a>");
	}
     }
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/*************************** Show user's office ******************************/
/*****************************************************************************/

static void Rec_ShowOffice (struct UsrData *UsrDat,
                            bool ShowData,const char *ClassForm)
  {
   extern const char *Txt_Office;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
		      "%s:"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Office);
   if (ShowData)
      fprintf (Gbl.F.Out,"%s",UsrDat->Tch.Office);
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/*************************** Show user's office phone ******************************/
/*****************************************************************************/

static void Rec_ShowOfficePhone (struct UsrData *UsrDat,
                                 bool ShowData,const char *ClassForm)
  {
   extern const char *Txt_Phone;

   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"REC_C1_BOT RIGHT_MIDDLE %s\">"
		      "%s:"
		      "</td>"
		      "<td class=\"REC_C2_BOT REC_DAT_BOLD LEFT_MIDDLE\">",
	    ClassForm,Txt_Phone);
   if (ShowData)
      fprintf (Gbl.F.Out,"<a href=\"tel:%s\" class=\"REC_DAT_BOLD\">%s</a>",
	       UsrDat->Tch.OfficePhone,
	       UsrDat->Tch.OfficePhone);
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/*********************** Write a link to netiquette rules ********************/
/*****************************************************************************/

static void Rec_WriteLinkToDataProtectionClause (void)
  {
   extern const char *Txt_DATA_PROTECTION_CLAUSE;

   fprintf (Gbl.F.Out,"<div class=\"CENTER_MIDDLE\">"
	              "<a class=\"TIT\" href=\"%s/%s/\" target=\"_blank\">%s</a>"
	              "</div>",
            Cfg_URL_SWAD_PUBLIC,Cfg_DATA_PROTECTION_FOLDER,
            Txt_DATA_PROTECTION_CLAUSE);
  }

/*****************************************************************************/
/**************** Update and show data from identified user ******************/
/*****************************************************************************/

void Rec_UpdateMyRecord (void)
  {
   /***** Get my data from record form *****/
   Rec_GetUsrNameFromRecordForm (&Gbl.Usrs.Me.UsrDat);
   Rec_GetUsrExtraDataFromRecordForm (&Gbl.Usrs.Me.UsrDat);

   /***** Update my data in database *****/
   Enr_UpdateUsrData (&Gbl.Usrs.Me.UsrDat);
  }

/*****************************************************************************/
/**** Get and check future user's role in current course from record form ****/
/*****************************************************************************/

Rol_Role_t Rec_GetRoleFromRecordForm (void)
  {
   Rol_Role_t Role;
   bool RoleOK;

   /***** Get role as a parameter from form *****/
   Role = (Rol_Role_t)
	  Par_GetParToUnsignedLong ("Role",
				    0,
				    Rol_NUM_ROLES - 1,
				    (unsigned long) Rol_UNK);

   /***** Check if I can register a user
          with the received role in current course *****/
   /* Check for other possible errors */
   RoleOK = false;
   switch (Gbl.Usrs.Me.Role.Logged)
     {
      case Rol_STD:		// I am logged as student
      case Rol_NET:		// I am logged as non-editing teacher
         /* A student or a non-editing teacher can only change his/her data, but not his/her role */
	 Role = Gbl.Usrs.Me.Role.Logged;
	 RoleOK = true;
	 break;
      case Rol_TCH:		// I am logged as teacher
      case Rol_DEG_ADM:		// I am logged as degree admin
      case Rol_CTR_ADM:		// I am logged as centre admin
      case Rol_INS_ADM:		// I am logged as institution admin
	 if (Role == Rol_STD ||
	     Role == Rol_NET ||
	     Role == Rol_TCH)
	    RoleOK = true;
	 break;
      case Rol_SYS_ADM:
	 if ( Role == Rol_STD ||
	      Role == Rol_NET ||
	      Role == Rol_TCH ||
	     (Role == Rol_GST && Gbl.CurrentCrs.Crs.CrsCod <= 0))
	    RoleOK = true;
	 break;
      default:
	 break;
     }
   if (!RoleOK)
      Lay_ShowErrorAndExit ("Wrong role.");
   return Role;
  }

/*****************************************************************************/
/*************** Get data fields of shared record from form ******************/
/*****************************************************************************/

void Rec_GetUsrNameFromRecordForm (struct UsrData *UsrDat)
  {
   char Surname1 [Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1];	// Temporary surname 1
   char FirstName[Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1];	// Temporary first name

   /***** Get surname 1 *****/
   Par_GetParToText ("Surname1",Surname1,
                     Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME);
   Str_ConvertToTitleType (Surname1);
   // Surname 1 is mandatory, so avoid overwriting surname 1 with empty string
   if (Surname1[0])		// New surname 1 not empty
      Str_Copy (UsrDat->Surname1,Surname1,
                Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME);

   /***** Get surname 2 *****/
   Par_GetParToText ("Surname2",UsrDat->Surname2,
                     Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME);
   Str_ConvertToTitleType (UsrDat->Surname2);

   /***** Get first name *****/
   Par_GetParToText ("FirstName",FirstName,
                     Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME);
   Str_ConvertToTitleType (FirstName);
   // First name is mandatory, so avoid overwriting first name with empty string
   if (Surname1[0])		// New first name not empty
      Str_Copy (UsrDat->FirstName,FirstName,
                Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME);

   /***** Build full name *****/
   Usr_BuildFullName (UsrDat);
  }

static void Rec_GetUsrExtraDataFromRecordForm (struct UsrData *UsrDat)
  {
   /***** Get sex from form *****/
   UsrDat->Sex = (Usr_Sex_t)
	         Par_GetParToUnsignedLong ("Sex",
                                           (unsigned long) Usr_SEX_FEMALE,
                                           (unsigned long) Usr_SEX_MALE,
                                           (unsigned long) Usr_SEX_UNKNOWN);

   /***** Get country code *****/
   UsrDat->CtyCod = Par_GetParToLong ("OthCtyCod");

   Par_GetParToText ("OriginPlace",UsrDat->OriginPlace,Usr_MAX_BYTES_ADDRESS);
   Str_ConvertToTitleType (UsrDat->OriginPlace);

   Dat_GetDateFromForm ("BirthDay","BirthMonth","BirthYear",
                        &(UsrDat->Birthday.Day  ),
                        &(UsrDat->Birthday.Month),
                        &(UsrDat->Birthday.Year ));
   Dat_ConvDateToDateStr (&(UsrDat->Birthday),UsrDat->StrBirthday);

   Par_GetParToText ("LocalAddress",UsrDat->LocalAddress,Usr_MAX_BYTES_ADDRESS);

   Par_GetParToText ("LocalPhone",UsrDat->LocalPhone,Usr_MAX_BYTES_PHONE);

   Par_GetParToText ("FamilyAddress",UsrDat->FamilyAddress,Usr_MAX_BYTES_ADDRESS);

   Par_GetParToText ("FamilyPhone",UsrDat->FamilyPhone,Usr_MAX_BYTES_PHONE);

   Rec_GetUsrCommentsFromForm (UsrDat);
  }

/*****************************************************************************/
/********** Get the comments of the record of a user from the form ***********/
/*****************************************************************************/

static void Rec_GetUsrCommentsFromForm (struct UsrData *UsrDat)
  {
   /***** Check if memory is allocated for comments *****/
   if (!UsrDat->Comments)
      Lay_ShowErrorAndExit ("Can not read comments of a user.");

   /***** Get the parameter with the comments *****/
   Par_GetParToHTML ("Comments",UsrDat->Comments,Cns_MAX_BYTES_TEXT);
  }

/*****************************************************************************/
/*** Put a link to the action to edit my institution, centre, department... **/
/*****************************************************************************/

void Rec_PutLinkToChangeMyInsCtrDpt (void)
  {
   extern const char *Txt_Edit_my_institution;

   /***** Link to edit my institution, centre, department... *****/
   Lay_PutContextualLink (ActReqEdiMyIns,NULL,NULL,
                          "ins64x64.gif",
                          Txt_Edit_my_institution,Txt_Edit_my_institution,
		          NULL);
  }

/*****************************************************************************/
/********* Show form to edit my institution, centre and department ***********/
/*****************************************************************************/

#define COL2_WIDTH 600

void Rec_ShowFormMyInsCtrDpt (void)
  {
   extern const char *Hlp_PROFILE_Institution;
   extern const char *The_ClassForm[The_NUM_THEMES];
   extern const char *Txt_Please_select_the_country_of_your_institution;
   extern const char *Txt_Please_fill_in_your_institution;
   extern const char *Txt_Please_fill_in_your_centre;
   extern const char *Txt_Please_fill_in_your_department;
   extern const char *Txt_Institution_centre_and_department;
   extern const char *Txt_Institution;
   extern const char *Txt_Country_of_your_institution;
   extern const char *Txt_Another_institution;
   extern const char *Txt_Centre;
   extern const char *Txt_Another_centre;
   extern const char *Txt_Department;
   extern const char *Txt_Another_department;
   extern const char *Txt_Office;
   extern const char *Txt_Phone;
   const char *ClassForm = The_ClassForm[Gbl.Prefs.Theme];
   unsigned NumCty;
   unsigned NumIns;
   unsigned NumCtr;
   unsigned NumDpt;
   bool IAmATeacher;

   /***** Get my roles if not yet got *****/
   Rol_GetRolesInAllCrssIfNotYetGot (&Gbl.Usrs.Me.UsrDat);

   /***** Check if I am a teacher *****/
   IAmATeacher = (Gbl.Usrs.Me.UsrDat.Role.InCrss & ((1 << Rol_NET) |	// I am a non-editing teacher...
	                                      (1 << Rol_TCH)));	// ...or a teacher in any course

   /***** If there is no country, institution, centre or department *****/
   if (Gbl.Usrs.Me.UsrDat.InsCtyCod < 0)
      Ale_ShowAlert (Ale_WARNING,Txt_Please_select_the_country_of_your_institution);
   else if (Gbl.Usrs.Me.UsrDat.InsCod < 0)
      Ale_ShowAlert (Ale_WARNING,Txt_Please_fill_in_your_institution);
   else if (IAmATeacher)
     {
      if (Gbl.Usrs.Me.UsrDat.Tch.CtrCod < 0)
         Ale_ShowAlert (Ale_WARNING,Txt_Please_fill_in_your_centre);
      else if (Gbl.Usrs.Me.UsrDat.Tch.DptCod < 0)
         Ale_ShowAlert (Ale_WARNING,Txt_Please_fill_in_your_department);
     }

   /***** Start table *****/
   Lay_StartRoundFrameTable ("800px",
                             IAmATeacher ? Txt_Institution_centre_and_department :
	                                   Txt_Institution,
	                     NULL,Hlp_PROFILE_Institution,2);

   /***** Country *****/
   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"RIGHT_MIDDLE\">"
                      "<label for=\"OthCtyCod\" class=\"%s\">%s:</label>"
		      "</td>"
		      "<td class=\"LEFT_MIDDLE\" style=\"width:%upx;\">",
            ClassForm,Txt_Country_of_your_institution,
            COL2_WIDTH);

   /* If list of countries is empty, try to get it */
   if (!Gbl.Ctys.Num)
     {
      Gbl.Ctys.SelectedOrder = Cty_ORDER_BY_COUNTRY;
      Cty_GetListCountries (Cty_GET_BASIC_DATA);
     }

   /* Start form to select the country of my institution */
   Act_FormGoToStart (ActChgCtyMyIns);
   fprintf (Gbl.F.Out,"<select id=\"OthCtyCod\" name=\"OthCtyCod\""
	              " style=\"width:500px;\""
	              " onchange=\"document.getElementById('%s').submit();\">"
                      "<option value=\"-1\"",
	    Gbl.Form.Id);
   if (Gbl.Usrs.Me.UsrDat.InsCtyCod <= 0)
      fprintf (Gbl.F.Out," selected=\"selected\"");
   fprintf (Gbl.F.Out," disabled=\"disabled\"></option>");
   for (NumCty = 0;
	NumCty < Gbl.Ctys.Num;
	NumCty++)
     {
      fprintf (Gbl.F.Out,"<option value=\"%ld\"",
	       Gbl.Ctys.Lst[NumCty].CtyCod);
      if (Gbl.Ctys.Lst[NumCty].CtyCod == Gbl.Usrs.Me.UsrDat.InsCtyCod)
	 fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",
	       Gbl.Ctys.Lst[NumCty].Name[Gbl.Prefs.Language]);
     }
   fprintf (Gbl.F.Out,"</select>");
   Act_FormEnd ();
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");

   /***** Institution *****/
   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"RIGHT_MIDDLE\">"
                      "<label for=\"OthInsCod\" class=\"%s\">%s:</label>"
		      "</td>"
		      "<td class=\"LEFT_MIDDLE\" style=\"width:%upx;\">",
            ClassForm,Txt_Institution,
	    COL2_WIDTH);

   /* Get list of institutions in this country */
   Ins_FreeListInstitutions ();
   if (Gbl.Usrs.Me.UsrDat.InsCtyCod > 0)
      Ins_GetListInstitutions (Gbl.Usrs.Me.UsrDat.InsCtyCod,Ins_GET_BASIC_DATA);

   /* Start form to select institution */
   Act_FormGoToStart (ActChgMyIns);
   fprintf (Gbl.F.Out,"<select id=\"OthInsCod\" name=\"OthInsCod\""
	              " style=\"width:500px;\""
	              " onchange=\"document.getElementById('%s').submit();\">"
                      "<option value=\"-1\"",
	    Gbl.Form.Id);
   if (Gbl.Usrs.Me.UsrDat.InsCod < 0)
      fprintf (Gbl.F.Out," selected=\"selected\"");
   fprintf (Gbl.F.Out," disabled=\"disabled\"></option>"
		      "<option value=\"0\"");
   if (Gbl.Usrs.Me.UsrDat.InsCod == 0)
      fprintf (Gbl.F.Out," selected=\"selected\"");
   fprintf (Gbl.F.Out,">%s</option>",
	    Txt_Another_institution);
   for (NumIns = 0;
	NumIns < Gbl.Inss.Num;
	NumIns++)
     {
      fprintf (Gbl.F.Out,"<option value=\"%ld\"",
	       Gbl.Inss.Lst[NumIns].InsCod);
      if (Gbl.Inss.Lst[NumIns].InsCod == Gbl.Usrs.Me.UsrDat.InsCod)
	 fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",
	       Gbl.Inss.Lst[NumIns].FullName);
     }
   fprintf (Gbl.F.Out,"</select>");
   Act_FormEnd ();
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");

   if (IAmATeacher)
     {
      /***** Centre *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"RIGHT_MIDDLE\">"
                         "<label for=\"OthCtrCod\" class=\"%s\">%s:</label>"
			 "</td>"
			 "<td class=\"LEFT_MIDDLE\" style=\"width:%upx;\">",
	       ClassForm,Txt_Centre,
	       COL2_WIDTH);

      /* Get list of centres in this institution */
      Ctr_FreeListCentres ();
      if (Gbl.Usrs.Me.UsrDat.InsCod > 0)
	 Ctr_GetListCentres (Gbl.Usrs.Me.UsrDat.InsCod);

      /* Start form to select centre */
      Act_FormGoToStart (ActChgMyCtr);
      fprintf (Gbl.F.Out,"<select id=\"OthCtrCod\" name=\"OthCtrCod\""
	                 " style=\"width:500px;\""
			 " onchange=\"document.getElementById('%s').submit();\">"
			 "<option value=\"-1\"",
	       Gbl.Form.Id);
      if (Gbl.Usrs.Me.UsrDat.Tch.CtrCod < 0)
	 fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out," disabled=\"disabled\"></option>"
			 "<option value=\"0\"");
      if (Gbl.Usrs.Me.UsrDat.Tch.CtrCod == 0)
	 fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",
	       Txt_Another_centre);
      for (NumCtr = 0;
	   NumCtr < Gbl.Ctrs.Num;
	   NumCtr++)
	{
	 fprintf (Gbl.F.Out,"<option value=\"%ld\"",
		  Gbl.Ctrs.Lst[NumCtr].CtrCod);
	 if (Gbl.Ctrs.Lst[NumCtr].CtrCod == Gbl.Usrs.Me.UsrDat.Tch.CtrCod)
	    fprintf (Gbl.F.Out," selected=\"selected\"");
	 fprintf (Gbl.F.Out,">%s</option>",
		  Gbl.Ctrs.Lst[NumCtr].FullName);
	}
      fprintf (Gbl.F.Out,"</select>");
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>"
			 "</tr>");

      /***** Department *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"RIGHT_MIDDLE\">"
                         "<label for=\"DptCod\" class=\"%s\">%s:</label>"
			 "</td>"
			 "<td class=\"LEFT_MIDDLE\" style=\"width:%upx;\">",
	       ClassForm,Txt_Department,
	       COL2_WIDTH);

      /* Get list of departments in this institution */
      Dpt_FreeListDepartments ();
      if (Gbl.Usrs.Me.UsrDat.InsCod > 0)
	 Dpt_GetListDepartments (Gbl.Usrs.Me.UsrDat.InsCod);

      /* Start form to select department */
      Act_FormGoToStart (ActChgMyDpt);
      fprintf (Gbl.F.Out,"<select id=\"DptCod\" name=\"DptCod\""
	                 " style=\"width:500px;\""
			 " onchange=\"document.getElementById('%s').submit();\">"
			 "<option value=\"-1\"",
	       Gbl.Form.Id);
      if (Gbl.Usrs.Me.UsrDat.Tch.DptCod < 0)
	 fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out," disabled=\"disabled\"></option>"
			 "<option value=\"0\"");
      if (Gbl.Usrs.Me.UsrDat.Tch.DptCod == 0)
	 fprintf (Gbl.F.Out," selected=\"selected\"");
      fprintf (Gbl.F.Out,">%s</option>",
	       Txt_Another_department);
      for (NumDpt = 0;
	   NumDpt < Gbl.Dpts.Num;
	   NumDpt++)
	{
	 fprintf (Gbl.F.Out,"<option value=\"%ld\"",
		  Gbl.Dpts.Lst[NumDpt].DptCod);
	 if (Gbl.Dpts.Lst[NumDpt].DptCod == Gbl.Usrs.Me.UsrDat.Tch.DptCod)
	    fprintf (Gbl.F.Out," selected=\"selected\"");
	 fprintf (Gbl.F.Out,">%s</option>",
		  Gbl.Dpts.Lst[NumDpt].FullName);
	}
      fprintf (Gbl.F.Out,"</select>");
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>"
			 "</tr>");

      /***** Office *****/
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td class=\"RIGHT_MIDDLE\">"
		         "<label for=\"Office\" class=\"%s\">"
	                 "%s:"
	                 "</label>"
	                 "</td>"
                         "<td class=\"LEFT_MIDDLE\" style=\"width:%upx;\">",
               ClassForm,Txt_Office,
               COL2_WIDTH);
      Act_FormGoToStart (ActChgMyOff);
      fprintf (Gbl.F.Out,"<input type=\"text\" id=\"Office\" name=\"Office\""
			 " maxlength=\"%u\" value=\"%s\""
			 " style=\"width:500px;\""
			 " onchange=\"document.getElementById('%s').submit();\" />",
               Usr_MAX_CHARS_ADDRESS,
	       Gbl.Usrs.Me.UsrDat.Tch.Office,
	       Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");

      /***** Phone *****/
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td class=\"RIGHT_MIDDLE\">"
		         "<label for=\"OfficePhone\" class=\"%s\">"
	                 "%s:"
                         "</label>"
	                 "</td>"
	                 "<td class=\"LEFT_MIDDLE\" style=\"width:%upx;\">",
               ClassForm,Txt_Phone,
               COL2_WIDTH);
      Act_FormGoToStart (ActChgMyOffPho);
      fprintf (Gbl.F.Out,"<input type=\"tel\""
	                 " id=\"OfficePhone\" name=\"OfficePhone\""
			 " maxlength=\"%u\" value=\"%s\""
			 " style=\"width:500px;\""
			 " onchange=\"document.getElementById('%s').submit();\" />",
	       Usr_MAX_CHARS_PHONE,
	       Gbl.Usrs.Me.UsrDat.Tch.OfficePhone,
	       Gbl.Form.Id);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");
     }

   /***** End table *****/
   Lay_EndRoundFrameTable ();
  }

/*****************************************************************************/
/******** Receive form data to change the country of my institution **********/
/*****************************************************************************/

void Rec_ChgCountryOfMyInstitution (void)
  {
   unsigned NumInss;

   /***** Get country code of my institution *****/
   Gbl.Usrs.Me.UsrDat.InsCtyCod = Cty_GetAndCheckParamOtherCtyCod (0);

   /***** When country changes, the institution, centre and department must be reset *****/
   NumInss = Ins_GetNumInssInCty (Gbl.Usrs.Me.UsrDat.InsCtyCod);
   if (NumInss)
     {
      Gbl.Usrs.Me.UsrDat.InsCod     = -1L;
      Gbl.Usrs.Me.UsrDat.Tch.CtrCod = -1L;
      Gbl.Usrs.Me.UsrDat.Tch.DptCod = -1L;
     }
   else	// Country has no institutions
     {
      Gbl.Usrs.Me.UsrDat.InsCod     = 0;	// Another institution
      Gbl.Usrs.Me.UsrDat.Tch.CtrCod = 0;	// Another centre
      Gbl.Usrs.Me.UsrDat.Tch.DptCod = 0;	// Another department
    }

   /***** Update institution, centre and department *****/
   Enr_UpdateInstitutionCentreDepartment ();

   /***** Show form again *****/
   Rec_ShowFormMyInsCtrDpt ();
  }

/*****************************************************************************/
/**************** Receive form data to change my institution *****************/
/*****************************************************************************/

void Rec_UpdateMyInstitution (void)
  {
   struct Instit Ins;
   unsigned NumCtrs;
   unsigned NumDpts;

   /***** Get my institution *****/
   /* Get institution code */
   Ins.InsCod = Ins_GetAndCheckParamOtherInsCod (0);	// 0 (another institution) is allowed here

   /* Get country of institution */
   if (Ins.InsCod > 0)
     {
      Ins_GetDataOfInstitutionByCod (&Ins,Ins_GET_BASIC_DATA);
      if (Gbl.Usrs.Me.UsrDat.InsCtyCod != Ins.CtyCod)
	 Gbl.Usrs.Me.UsrDat.InsCtyCod = Ins.CtyCod;
     }

   /* Set institution code */
   Gbl.Usrs.Me.UsrDat.InsCod = Ins.InsCod;

   /***** When institution changes, the centre and department must be reset *****/
   NumCtrs = Ctr_GetNumCtrsInIns (Gbl.Usrs.Me.UsrDat.InsCod);
   NumDpts = Dpt_GetNumDptsInIns (Gbl.Usrs.Me.UsrDat.InsCod);
   Gbl.Usrs.Me.UsrDat.Tch.CtrCod = (NumCtrs ? -1L : 0);
   Gbl.Usrs.Me.UsrDat.Tch.DptCod = (NumDpts ? -1L : 0);

   /***** Update institution, centre and department *****/
   Enr_UpdateInstitutionCentreDepartment ();

   /***** Show form again *****/
   Rec_ShowFormMyInsCtrDpt ();
  }

/*****************************************************************************/
/******************* Receive form data to change my centre *******************/
/*****************************************************************************/

void Rec_UpdateMyCentre (void)
  {
   struct Centre Ctr;

   /***** Get my centre *****/
   /* Get centre code */
   Ctr.CtrCod = Ctr_GetAndCheckParamOtherCtrCod (0);	// 0 (another centre) is allowed here

   /* Get institution of centre */
   if (Ctr.CtrCod > 0)
     {
      Ctr_GetDataOfCentreByCod (&Ctr);
      if (Gbl.Usrs.Me.UsrDat.InsCod != Ctr.InsCod)
	{
	 Gbl.Usrs.Me.UsrDat.InsCod = Ctr.InsCod;
	 Gbl.Usrs.Me.UsrDat.Tch.DptCod = -1L;
	}
     }

   /* Set centre code */
   Gbl.Usrs.Me.UsrDat.Tch.CtrCod = Ctr.CtrCod;

   /***** Update institution, centre and department *****/
   Enr_UpdateInstitutionCentreDepartment ();

   /***** Show form again *****/
   Rec_ShowFormMyInsCtrDpt ();
  }

/*****************************************************************************/
/***************** Receive form data to change my department *****************/
/*****************************************************************************/

void Rec_UpdateMyDepartment (void)
  {
   struct Department Dpt;

   /***** Get my department *****/
   /* Get department code */
   Dpt.DptCod = Dpt_GetAndCheckParamDptCod (0);	// 0 (another department) is allowed here

   /* Get institution of department */
   if (Dpt.DptCod > 0)
     {
      Dpt_GetDataOfDepartmentByCod (&Dpt);
      if (Gbl.Usrs.Me.UsrDat.InsCod != Dpt.InsCod)
	{
	 Gbl.Usrs.Me.UsrDat.InsCod = Dpt.InsCod;
	 Gbl.Usrs.Me.UsrDat.Tch.CtrCod = -1L;
	}
     }

   /***** Update institution, centre and department *****/
   Gbl.Usrs.Me.UsrDat.Tch.DptCod = Dpt.DptCod;
   Enr_UpdateInstitutionCentreDepartment ();

   /***** Show form again *****/
   Rec_ShowFormMyInsCtrDpt ();
  }

/*****************************************************************************/
/******************* Receive form data to change my office *******************/
/*****************************************************************************/

void Rec_UpdateMyOffice (void)
  {
   char Query[128 + Usr_MAX_BYTES_ADDRESS];

   /***** Get my office *****/
   Par_GetParToText ("Office",Gbl.Usrs.Me.UsrDat.Tch.Office,Usr_MAX_BYTES_ADDRESS);

   /***** Update office *****/
   sprintf (Query,"UPDATE usr_data SET Office='%s' WHERE UsrCod=%ld",
	    Gbl.Usrs.Me.UsrDat.Tch.Office,
	    Gbl.Usrs.Me.UsrDat.UsrCod);
   DB_QueryUPDATE (Query,"can not update office");

   /***** Show form again *****/
   Rec_ShowFormMyInsCtrDpt ();
  }

/*****************************************************************************/
/**************** Receive form data to change my office phone ****************/
/*****************************************************************************/

void Rec_UpdateMyOfficePhone (void)
  {
   char Query[128 + Usr_MAX_BYTES_PHONE];

   /***** Get my office *****/
   Par_GetParToText ("OfficePhone",Gbl.Usrs.Me.UsrDat.Tch.OfficePhone,Usr_MAX_BYTES_PHONE);

   /***** Update office phone *****/
   sprintf (Query,"UPDATE usr_data SET OfficePhone='%s' WHERE UsrCod=%ld",
	    Gbl.Usrs.Me.UsrDat.Tch.OfficePhone,
	    Gbl.Usrs.Me.UsrDat.UsrCod);
   DB_QueryUPDATE (Query,"can not update office phone");

   /***** Show form again *****/
   Rec_ShowFormMyInsCtrDpt ();
  }
