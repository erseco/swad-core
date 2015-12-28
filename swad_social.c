// swad_social.c: social networking

/*
    SWAD (Shared Workspace At a Distance),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2015 Antonio Ca�as Vargas

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General 3 License as
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
/*********************************** Headers *********************************/
/*****************************************************************************/

#include <linux/limits.h>	// For PATH_MAX
#include <stdlib.h>		// For malloc and free
#include <sys/types.h>		// For time_t

#include "swad_constant.h"
#include "swad_database.h"
#include "swad_global.h"
#include "swad_layout.h"
#include "swad_social.h"

/*****************************************************************************/
/****************************** Public constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

#define Soc_MAX_BYTES_SUMMARY 200

/*****************************************************************************/
/****************************** Internal types *******************************/
/*****************************************************************************/

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/************************* Internal global variables *************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

static Soc_SocialEvent_t Soc_GetSocialEventFromDB (const char *Str);
static void Soc_WriteEventDate (time_t TimeUTC);
static void Soc_GetEventSummary (Soc_SocialEvent_t SocialEvent,long Cod,
                                 char *SummaryStr,unsigned MaxChars);

/*****************************************************************************/
/*********************** Show social activity (timeline) *********************/
/*****************************************************************************/

void Soc_ShowSocialActivity (void)
  {
   extern const char *Txt_Public_activity;
   extern const char *Txt_Forum;
   extern const char *Txt_Course;
   extern const char *Txt_Degree;
   extern const char *Txt_Centre;
   extern const char *Txt_Institution;
   extern const char *Txt_Country;
   char Query[512];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumEvents;
   unsigned long NumEvent;
   Soc_SocialEvent_t SocialEvent = (Soc_SocialEvent_t) 0;	// Initialized to avoid warning
   struct UsrData UsrDat;
   struct Country Cty;
   struct Institution Ins;
   struct Centre Ctr;
   struct Degree Deg;
   struct Course Crs;
   long Cod;
   char ForumName[512];
   time_t DateTimeUTC;	// Date-time of the event
   bool ShowPhoto = false;
   char PhotoURL[PATH_MAX+1];
   char *SummaryStr;

   /***** Get my timeline from database *****/
   sprintf (Query,"SELECT SocialEvent,UsrCod,"
	          "CtyCod,InsCod,CtrCod,DegCod,CrsCod,"
	          "Cod,UNIX_TIMESTAMP(TimeEvent)"
                  " FROM social,usr_follow"
                  " WHERE usr_follow.FollowerCod='%ld'"
                  " AND usr_follow.FollowedCod=social.UsrCod"
                  " ORDER BY SocCod DESC LIMIT 10",
            Gbl.Usrs.Me.UsrDat.UsrCod);
   NumEvents = DB_QuerySELECT (Query,&mysql_res,"can not get your notifications");

   /***** List my timeline *****/
   if (NumEvents)	// Events found
     {
      /***** Allocate memory for the summary of the notification *****/
      if ((SummaryStr = malloc (Cns_MAX_BYTES_TEXT+1)) == NULL)
         Lay_ShowErrorAndExit ("Not enough memory to store the summary of the notification.");

      /***** Initialize structure with user's data *****/
      Usr_UsrDataConstructor (&UsrDat);

      /***** List start *****/
      Lay_StartRoundFrame (NULL,Txt_Public_activity);
      fprintf (Gbl.F.Out,"<ul class=\"LIST_LEFT\">");

      /***** List events one by one *****/
      for (NumEvent = 0;
	   NumEvent < NumEvents;
	   NumEvent++)
	{
         /***** Get next notification *****/
         row = mysql_fetch_row (mysql_res);

         /* Get event type (row[0]) */
         SocialEvent = Soc_GetSocialEventFromDB ((const char *) row[0]);

         /* Get (from) user code (row[1]) */
         UsrDat.UsrCod = Str_ConvertStrCodToLongCod (row[1]);
         Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (&UsrDat);		// Get user's data from the database

         /* Get country code (row[2]) */
         Cty.CtyCod = Str_ConvertStrCodToLongCod (row[2]);
         Cty_GetDataOfCountryByCod (&Cty,Cty_GET_BASIC_DATA);

         /* Get institution code (row[3]) */
         Ins.InsCod = Str_ConvertStrCodToLongCod (row[3]);
         Ins_GetDataOfInstitutionByCod (&Ins,Ins_GET_BASIC_DATA);

          /* Get centre code (row[4]) */
         Ctr.CtrCod = Str_ConvertStrCodToLongCod (row[4]);
         Ctr_GetDataOfCentreByCod (&Ctr);

         /* Get degree code (row[5]) */
         Deg.DegCod = Str_ConvertStrCodToLongCod (row[5]);
         Deg_GetDataOfDegreeByCod (&Deg);

         /* Get course code (row[6]) */
         Crs.CrsCod = Str_ConvertStrCodToLongCod (row[6]);
         Crs_GetDataOfCourseByCod (&Crs);

         /* Get file/post... code (row[7]) */
         Cod = Str_ConvertStrCodToLongCod (row[7]);

         /* Get forum type of the post */
         if (SocialEvent == Soc_EVENT_FORUM_POST)
           {
            Gbl.Forum.ForumType = For_GetForumTypeOfAPost (Cod);
            For_SetForumName (Gbl.Forum.ForumType,
        	              &Ins,
        	              &Ctr,
        	              &Deg,
        	              &Crs,
        	              ForumName,Gbl.Prefs.Language,false);	// Set forum name in recipient's language
            Gbl.Forum.Ins.InsCod = Ins.InsCod;
            Gbl.Forum.Ctr.CtrCod = Ctr.CtrCod;
            Gbl.Forum.Deg.DegCod = Deg.DegCod;
            Gbl.Forum.Crs.CrsCod = Crs.CrsCod;
           }

         /* Get time of the event (row[8]) */
         DateTimeUTC = Dat_GetUNIXTimeFromStr (row[8]);

         /***** Write row for this notification *****/
         fprintf (Gbl.F.Out,"<li>");

	 /* Left: write author's photo */
         fprintf (Gbl.F.Out,"<div class=\"SOCIAL_LEFT_PHOTO\">");
	 ShowPhoto = Pho_ShowUsrPhotoIsAllowed (&UsrDat,PhotoURL);
	 Pho_ShowUsrPhoto (&UsrDat,ShowPhoto ? PhotoURL :
					       NULL,
			   "PHOTO60x80",Pho_ZOOM);
         fprintf (Gbl.F.Out,"</div>");

         /* Right: author's name, time and summary */
         fprintf (Gbl.F.Out,"<div class=\"SOCIAL_RIGHT_CONTAINER\">");

	 /* Write author's full name and nickname */
         fprintf (Gbl.F.Out,"<div class=\"SOCIAL_RIGHT_AUTHOR DAT\">"
                            "<strong>%s</strong> @%s"
                            "</div>",
                  UsrDat.FullName,UsrDat.Nickname);

         /* Write date and time */
         Soc_WriteEventDate (DateTimeUTC);

         /* Write event type and location */
         fprintf (Gbl.F.Out,"<div class=\"DAT\">%u: ",
                  (unsigned) SocialEvent);

         if (SocialEvent == Soc_EVENT_FORUM_POST)
            fprintf (Gbl.F.Out,"%s: %s",Txt_Forum,ForumName);
         else if (Crs.CrsCod > 0)
            fprintf (Gbl.F.Out,"%s: %s",Txt_Course,Crs.ShortName);
         else if (Deg.DegCod > 0)
            fprintf (Gbl.F.Out,"%s: %s",Txt_Degree,Deg.ShortName);
         else if (Ctr.CtrCod > 0)
            fprintf (Gbl.F.Out,"%s: %s",Txt_Centre,Ctr.ShortName);
         else if (Ins.InsCod > 0)
            fprintf (Gbl.F.Out,"%s: %s",Txt_Institution,Ins.ShortName);
         else if (Cty.CtyCod > 0)
            fprintf (Gbl.F.Out,"%s: %s",Txt_Country,Cty.Name[Gbl.Prefs.Language]);
         fprintf (Gbl.F.Out,"</div>");

         /* Write content of the event */
	 Soc_GetEventSummary (SocialEvent,Cod,
	                      SummaryStr,Soc_MAX_BYTES_SUMMARY);
	 fprintf (Gbl.F.Out,"<div class=\"DAT\">%s</div>",SummaryStr);

         /* End of right part */
         fprintf (Gbl.F.Out,"</div>");

         /* End of list element */
         fprintf (Gbl.F.Out,"</li>");
        }

      /***** List end *****/
      fprintf (Gbl.F.Out,"</ul>");
      Lay_EndRoundFrame ();

      /***** Free memory used for user's data *****/
      Usr_UsrDataDestructor (&UsrDat);

      /***** Free summary *****/
      free ((void *) SummaryStr);
     }
   else
      Lay_ShowAlert (Lay_INFO,"No events.");	// Need translation!!!!

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/****** Get social event type from string number coming from database ********/
/*****************************************************************************/

static Soc_SocialEvent_t Soc_GetSocialEventFromDB (const char *Str)
  {
   unsigned UnsignedNum;

   if (sscanf (Str,"%u",&UnsignedNum) == 1)
      if (UnsignedNum < Soc_NUM_SOCIAL_EVENTS)
         return (Soc_SocialEvent_t) UnsignedNum;

   return Soc_EVENT_UNKNOWN;
  }

/*****************************************************************************/
/**************** Write the date of creation of a social event ***************/
/*****************************************************************************/
// TimeUTC holds UTC date and time in UNIX format (seconds since 1970)

static void Soc_WriteEventDate (time_t TimeUTC)
  {
   static unsigned UniqueId = 0;

   UniqueId++;

   /***** Start cell *****/
   fprintf (Gbl.F.Out,"<div id=\"date_%u\" class=\"DAT_LIGHT RIGHT_TOP\""
	              " style=\"display:inline-block;\">",
            UniqueId);

   /***** Write date and time *****/
   fprintf (Gbl.F.Out,"<script type=\"text/javascript\">"
                      "writeLocalDateTimeFromUTC('date_%u',%ld,'&nbsp;');"
                      "</script>",
            UniqueId,(long) TimeUTC);

   /***** End cell *****/
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/******************* Get social event summary and content ********************/
/*****************************************************************************/

static void Soc_GetEventSummary (Soc_SocialEvent_t SocialEvent,long Cod,
                                 char *SummaryStr,unsigned MaxChars)
  {
   SummaryStr[0] = '\0';

   switch (SocialEvent)
     {
/*
      case Ntf_EVENT_UNKNOWN:
         break;
      case Ntf_EVENT_DOCUMENT_FILE:
      case Ntf_EVENT_SHARED_FILE:
	 Brw_GetNotifDocOrSharedFile (SummaryStr,ContentStr,Cod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_ASSIGNMENT:
         Asg_GetNotifAssignment (SummaryStr,ContentStr,Cod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_EXAM_ANNOUNCEMENT:
         Exa_GetNotifExamAnnouncement (SummaryStr,ContentStr,Cod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_MARKS_FILE:
         Mrk_GetNotifMyMarks (SummaryStr,ContentStr,Cod,UsrCod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_ENROLLMENT_STUDENT:
      case Ntf_EVENT_ENROLLMENT_TEACHER:
	 Enr_GetNotifEnrollment (SummaryStr,CrsCod,UsrCod,MaxChars);
         break;
      case Ntf_EVENT_ENROLLMENT_REQUEST:
	 Enr_GetNotifEnrollmentRequest (SummaryStr,ContentStr,Cod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_NOTICE:
         Not_GetNotifNotice (SummaryStr,ContentStr,Cod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_FORUM_POST_COURSE:
      case Ntf_EVENT_FORUM_REPLY:
         For_GetNotifForumPst (SummaryStr,ContentStr,Cod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_MESSAGE:
         Msg_GetNotifMessage (SummaryStr,ContentStr,Cod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_SURVEY:
         Svy_GetNotifSurvey (SummaryStr,ContentStr,Cod,MaxChars,GetContent);
         break;
      case Ntf_EVENT_FOLLOWER:
         Fol_GetNotifFollower (SummaryStr,ContentStr);
         break;
*/
      case Soc_EVENT_UNKNOWN:
          break;
      case Soc_EVENT_INS_DOC_PUB_FILE:
         break;
      case Soc_EVENT_INS_SHA_PUB_FILE:
         break;
      case Soc_EVENT_CTR_DOC_PUB_FILE:
         break;
      case Soc_EVENT_CTR_SHA_PUB_FILE:
         break;
      case Soc_EVENT_DEG_DOC_PUB_FILE:
         break;
      case Soc_EVENT_DEG_SHA_PUB_FILE:
         break;
      case Soc_EVENT_CRS_DOC_PUB_FILE:
         break;
      case Soc_EVENT_CRS_SHA_PUB_FILE:
         break;
      case Soc_EVENT_EXAM_ANNOUNCEMENT:
         break;
      case Soc_EVENT_MARKS_FILE:
         break;
      case Soc_EVENT_SOCIAL_POST:
         break;
      case Soc_EVENT_NOTICE:
         break;
      case Soc_EVENT_FORUM_POST:
         For_GetSummaryAndContentForumPst (SummaryStr,NULL,Cod,MaxChars,false);
         break;
     }
  }

/*****************************************************************************/
/********************* Store a social event into database ********************/
/*****************************************************************************/

void Soc_StoreSocialEvent (Soc_SocialEvent_t SocialEvent,long Cod)
  {
   char Query[512];
   long CtyCod;
   long InsCod;
   long CtrCod;
   long DegCod;
   long CrsCod;

   if (SocialEvent == Soc_EVENT_FORUM_POST)
     {
      // CtyCod = Gbl.Forum.Cty.CtyCod;
      // InsCod = Gbl.Forum.Ins.InsCod;
      // CtrCod = Gbl.Forum.Ctr.CtrCod;
      // DegCod = Gbl.Forum.Deg.DegCod;
      // CrsCod = Gbl.Forum.Crs.CrsCod;
      CtyCod = -1L;
      InsCod = -1L;
      CtrCod = -1L;
      DegCod = -1L;
      CrsCod = -1L;
    }
   else
     {
      CtyCod = Gbl.CurrentCty.Cty.CtyCod;
      InsCod = Gbl.CurrentIns.Ins.InsCod;
      CtrCod = Gbl.CurrentCtr.Ctr.CtrCod;
      DegCod = Gbl.CurrentDeg.Deg.DegCod;
      CrsCod = Gbl.CurrentCrs.Crs.CrsCod;
     }

   /***** Store notify event *****/
   sprintf (Query,"INSERT INTO social (SocialEvent,UsrCod,"
	          "CtyCod,InsCod,CtrCod,DegCod,CrsCod,"
	          "Cod,TimeEvent)"
                  " VALUES ('%u','%ld',"
                  "'%ld','%ld','%ld','%ld','%ld',"
                  "'%ld',NOW())",
            (unsigned) SocialEvent,Gbl.Usrs.Me.UsrDat.UsrCod,
            CtyCod,InsCod,CtrCod,DegCod,CrsCod,
            Cod);
   DB_QueryINSERT (Query,"can not create new social event");
  }