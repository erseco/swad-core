// swad_account.c: user's account

/*
    SWAD (Shared Workspace At a Distance),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2017 Antonio Ca�as Vargas

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

#include <string.h>		// For string functions

#include "swad_account.h"
#include "swad_announcement.h"
#include "swad_calendar.h"
#include "swad_database.h"
#include "swad_duplicate.h"
#include "swad_enrolment.h"
#include "swad_follow.h"
#include "swad_global.h"
#include "swad_ID.h"
#include "swad_language.h"
#include "swad_notification.h"
#include "swad_parameter.h"
#include "swad_profile.h"
#include "swad_report.h"
#include "swad_social.h"

/*****************************************************************************/
/****************************** Public constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

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

static void Acc_ShowFormCheckIfIHaveAccount (const char *Title);
static void Acc_WriteRowEmptyAccount (unsigned NumUsr,const char *ID,struct UsrData *UsrDat);
static void Acc_ShowFormRequestNewAccountWithParams (const char *NewNicknameWithoutArroba,
                                                     const char *NewEmail);
static bool Acc_GetParamsNewAccount (char NewNicknameWithoutArroba[Nck_MAX_BYTES_NICKNAME_FROM_FORM + 1],
                                     char *NewEmail,
                                     char *NewEncryptedPassword);
static void Acc_CreateNewEncryptedUsrCod (struct UsrData *UsrDat);

static void Acc_PutLinkToRemoveMyAccount (void);
static void Acc_PutParamsToRemoveMyAccount (void);

static void Acc_PrintAccountSeparator (void);

static void Acc_AskIfRemoveUsrAccount (bool ItsMe);
static void Acc_AskIfRemoveOtherUsrAccount (void);

static void Acc_RemoveUsrBriefcase (struct UsrData *UsrDat);
static void Acc_RemoveUsr (struct UsrData *UsrDat);

/*****************************************************************************/
/******************** Put link to create a new account ***********************/
/*****************************************************************************/

void Acc_PutLinkToCreateAccount (void)
  {
   extern const char *Txt_Create_account;

   Lay_PutContextualLink (ActFrmMyAcc,NULL,NULL,
                          "arroba64x64.gif",
                          Txt_Create_account,Txt_Create_account,
                          NULL);
  }

/*****************************************************************************/
/******** Show form to change my account or to create a new account **********/
/*****************************************************************************/

void Acc_ShowFormMyAccount (void)
  {
   extern const char *Txt_Before_creating_a_new_account_check_if_you_have_been_already_registered_with_your_ID;

   if (Gbl.Usrs.Me.Logged)
      Acc_ShowFormChangeMyAccount ();
   else	// Not logged
     {
      /***** Links to other actions *****/
      fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");
      Usr_PutLinkToLogin ();
      Pwd_PutLinkToSendNewPasswd ();
      Lan_PutLinkToChangeLanguage ();
      fprintf (Gbl.F.Out,"</div>");

      /**** Show form to check if I have an account *****/
      Acc_ShowFormCheckIfIHaveAccount (Txt_Before_creating_a_new_account_check_if_you_have_been_already_registered_with_your_ID);

      /**** Show form to create a new account *****/
      Acc_ShowFormRequestNewAccountWithParams ("","");
     }
  }

/*****************************************************************************/
/***************** Show form to check if I have an account *******************/
/*****************************************************************************/

static void Acc_ShowFormCheckIfIHaveAccount (const char *Title)
  {
   extern const char *Hlp_PROFILE_SignUp;
   extern const char *The_ClassForm[The_NUM_THEMES];
   extern const char *Txt_ID;
   extern const char *Txt_Check;

   /***** Start frame *****/
   Lay_StartRoundFrame (NULL,Title,NULL,Hlp_PROFILE_SignUp);

   /***** Form to request user's ID for possible account already created *****/
   Act_FormStart (ActChkUsrAcc);
   fprintf (Gbl.F.Out,"<label class=\"%s\">"
		      "%s:&nbsp;"
		      "<input type=\"text\" name=\"ID\""
		      " size=\"18\" maxlength=\"%u\" value=\"\""
		      " required=\"required\" />"
		      "</label>",
	    The_ClassForm[Gbl.Prefs.Theme],Txt_ID,
	    ID_MAX_CHARS_USR_ID);
   Lay_PutConfirmButton (Txt_Check);
   Act_FormEnd ();

   /***** End frame *****/
   Lay_EndRoundFrame ();
  }

/*****************************************************************************/
/* Check if already exists a new account without password associated to a ID */
/*****************************************************************************/

void Acc_CheckIfEmptyAccountExists (void)
  {
   extern const char *Txt_Do_you_think_you_are_this_user;
   extern const char *Txt_Do_you_think_you_are_one_of_these_users;
   extern const char *Txt_There_is_no_empty_account_associated_with_your_ID_X_;
   extern const char *Txt_Check_another_ID;
   extern const char *Txt_Please_enter_your_ID;
   extern const char *Txt_Before_creating_a_new_account_check_if_you_have_been_already_registered_with_your_ID;
   char ID[ID_MAX_BYTES_USR_ID + 1];
   unsigned NumUsrs;
   unsigned NumUsr;
   struct UsrData UsrDat;
   char Query[512];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;

   /***** Links to other actions *****/
   fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");
   Usr_PutLinkToLogin ();
   Pwd_PutLinkToSendNewPasswd ();
   Lan_PutLinkToChangeLanguage ();
   fprintf (Gbl.F.Out,"</div>");

   /***** Get new user's ID from form *****/
   Par_GetParToText ("ID",ID,ID_MAX_BYTES_USR_ID);
   // Users' IDs are always stored internally in capitals and without leading zeros
   Str_RemoveLeadingZeros (ID);
   Str_ConvertToUpperText (ID);

   /***** Check if there are users with this user's ID *****/
   if (ID_CheckIfUsrIDIsValid (ID))
     {
      sprintf (Query,"SELECT usr_IDs.UsrCod"
	             " FROM usr_IDs,usr_data"
		     " WHERE usr_IDs.UsrID='%s'"
		     " AND usr_IDs.UsrCod=usr_data.UsrCod"
	             " AND usr_data.Password=''",
	       ID);
      NumUsrs = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get user's codes");

      if (NumUsrs)
	{
	 /***** Start frame and write message with number of accounts found *****/
	 Lay_StartRoundFrameTable (NULL,
	                           (NumUsrs == 1) ? Txt_Do_you_think_you_are_this_user :
					            Txt_Do_you_think_you_are_one_of_these_users,
			           NULL,NULL,5);

	 /***** Initialize structure with user's data *****/
	 Usr_UsrDataConstructor (&UsrDat);

	 /***** List users found *****/
	 for (NumUsr = 1, Gbl.RowEvenOdd = 0;
	      NumUsr <= NumUsrs;
	      NumUsr++, Gbl.RowEvenOdd = 1 - Gbl.RowEvenOdd)
	   {
	    /***** Get user's data from query result *****/
	    row = mysql_fetch_row (mysql_res);

	    /* Get user's code */
	    UsrDat.UsrCod = Str_ConvertStrCodToLongCod (row[0]);

	    /* Get user's data */
            Usr_GetAllUsrDataFromUsrCod (&UsrDat);

            /***** Write row with data of empty account *****/
            Acc_WriteRowEmptyAccount (NumUsr,ID,&UsrDat);
	   }

	 /***** Free memory used for user's data *****/
	 Usr_UsrDataDestructor (&UsrDat);

	 /***** End frame *****/
	 Lay_EndRoundFrameTable ();
	}
      else
	{
	 sprintf (Gbl.Alert.Txt,Txt_There_is_no_empty_account_associated_with_your_ID_X_,
		  ID);
	 Ale_ShowAlert (Ale_INFO,Gbl.Alert.Txt);
	}

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);

      /**** Show form to check if I have an account *****/
      Acc_ShowFormCheckIfIHaveAccount (Txt_Check_another_ID);
     }
   else	// ID not valid
     {
      /**** Show again form to check if I have an account *****/
      Ale_ShowAlert (Ale_WARNING,Txt_Please_enter_your_ID);

      Acc_ShowFormCheckIfIHaveAccount (Txt_Before_creating_a_new_account_check_if_you_have_been_already_registered_with_your_ID);
     }

   /**** Show form to create a new account *****/
   Acc_ShowFormRequestNewAccountWithParams ("","");
  }

/*****************************************************************************/
/************************ Write data of empty account ************************/
/*****************************************************************************/

static void Acc_WriteRowEmptyAccount (unsigned NumUsr,const char *ID,struct UsrData *UsrDat)
  {
   extern const char *Txt_ID;
   extern const char *Txt_Name;
   extern const char *Txt_yet_unnamed;
   extern const char *Txt_Its_me;

   /***** Write number of user in the list *****/
   fprintf (Gbl.F.Out,"<tr>"
		      "<td rowspan=\"2\""
		      " class=\"USR_LIST_NUM_N RIGHT_TOP COLOR%u\">"
		      "%u"
		      "</td>",
	    Gbl.RowEvenOdd,NumUsr);

   /***** Write user's ID and name *****/
   fprintf (Gbl.F.Out,"<td class=\"DAT_N LEFT_TOP COLOR%u\">"
		      "%s: %s<br />"
		      "%s: ",
	    Gbl.RowEvenOdd,
	    Txt_ID,ID,
	    Txt_Name);
   if (UsrDat->FullName[0])
      fprintf (Gbl.F.Out,"<strong>%s</strong>",UsrDat->FullName);
   else
      fprintf (Gbl.F.Out,"<em>%s</em>",Txt_yet_unnamed);
   fprintf (Gbl.F.Out,"</td>");

   /***** Button to login with this account *****/
   fprintf (Gbl.F.Out,"<td class=\"RIGHT_TOP COLOR%u\">",
	    Gbl.RowEvenOdd);
   Act_FormStart (ActLogInNew);
   Usr_PutParamUsrCodEncrypted (UsrDat->EncryptedUsrCod);
   Lay_PutCreateButtonInline (Txt_Its_me);
   Act_FormEnd ();
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");

   /***** Courses of this user *****/
   fprintf (Gbl.F.Out,"<tr>"
		      "<td colspan=\"2\" class=\"LEFT_TOP COLOR%u\">",
	    Gbl.RowEvenOdd);
   UsrDat->Sex = Usr_SEX_UNKNOWN;
   Crs_GetAndWriteCrssOfAUsr (UsrDat,Rol_TCH);
   Crs_GetAndWriteCrssOfAUsr (UsrDat,Rol_NET);
   Crs_GetAndWriteCrssOfAUsr (UsrDat,Rol_STD);
   fprintf (Gbl.F.Out,"</td>"
		      "</tr>");
  }

/*****************************************************************************/
/************ Show form to create a new account using parameters *************/
/*****************************************************************************/

static void Acc_ShowFormRequestNewAccountWithParams (const char *NewNicknameWithoutArroba,
                                                     const char *NewEmail)
  {
   extern const char *Hlp_PROFILE_SignUp;
   extern const char *The_ClassForm[The_NUM_THEMES];
   extern const char *Txt_Create_account;
   extern const char *Txt_Nickname;
   extern const char *Txt_HELP_nickname;
   extern const char *Txt_HELP_email;
   extern const char *Txt_Email;
   char NewNicknameWithArroba[Nck_MAX_BYTES_NICKNAME_FROM_FORM + 1];

   /***** Form to enter some data of the new user *****/
   Act_FormStart (ActCreUsrAcc);
   Lay_StartRoundFrameTable (NULL,Txt_Create_account,
                             NULL,Hlp_PROFILE_SignUp,2);

   /***** Nickname *****/
   if (NewNicknameWithoutArroba[0])
      sprintf (NewNicknameWithArroba,"@%s",NewNicknameWithoutArroba);
   else
      NewNicknameWithArroba[0] = '\0';
   fprintf (Gbl.F.Out,"<tr>"
	              "<td class=\"RIGHT_MIDDLE\">"
	              "<label for=\"NewNick\" class=\"%s\">%s:</label>"
	              "</td>"
	              "<td class=\"LEFT_MIDDLE\">"
                      "<input type=\"text\" id=\"NewNick\" name=\"NewNick\""
                      " size=\"18\" maxlength=\"%u\""
                      " placeholder=\"%s\" value=\"%s\""
                      " required=\"required\" />"
                      "</td>"
                      "</tr>",
            The_ClassForm[Gbl.Prefs.Theme],
            Txt_Nickname,
            1 + Nck_MAX_CHARS_NICKNAME_WITHOUT_ARROBA,
            Txt_HELP_nickname,
            NewNicknameWithArroba);

   /***** Email *****/
   fprintf (Gbl.F.Out,"<tr>"
	              "<td class=\"RIGHT_MIDDLE\">"
	              "<label for=\"NewEmail\" class=\"%s\">%s:</label>"
	              "</td>"
	              "<td class=\"LEFT_MIDDLE\">"
                      "<input type=\"email\" id=\"NewEmail\" name=\"NewEmail\""
                      " size=\"18\" maxlength=\"%u\""
                      " placeholder=\"%s\" value=\"%s\""
                      " required=\"required\" />"
                      "</td>"
                      "</tr>",
            The_ClassForm[Gbl.Prefs.Theme],
            Txt_Email,
            Cns_MAX_CHARS_EMAIL_ADDRESS,
            Txt_HELP_email,
            NewEmail);

   /***** Password *****/
   Pwd_PutFormToGetNewPasswordOnce ();

   /***** Send button and form end *****/
   Lay_EndRoundFrameTableWithButton (Lay_CREATE_BUTTON,Txt_Create_account);
   Act_FormEnd ();
  }

/*****************************************************************************/
/********* Show form to go to request the creation of a new account **********/
/*****************************************************************************/

void Acc_ShowFormGoToRequestNewAccount (void)
  {
   extern const char *Hlp_PROFILE_SignUp;
   extern const char *Txt_New_on_PLATFORM_Sign_up;
   extern const char *Txt_Create_account;

   /***** Start frame *****/
   sprintf (Gbl.Title,Txt_New_on_PLATFORM_Sign_up,Cfg_PLATFORM_SHORT_NAME);
   Lay_StartRoundFrame (NULL,Gbl.Title,NULL,Hlp_PROFILE_SignUp);

   /***** Button to go to request the creation of a new account *****/
   Act_FormStart (ActFrmMyAcc);
   Lay_PutCreateButton (Txt_Create_account);
   Act_FormEnd ();

   /***** End frame *****/
   Lay_EndRoundFrame ();
  }

/*****************************************************************************/
/*********************** Show form to change my account **********************/
/*****************************************************************************/

void Acc_ShowFormChangeMyAccount (void)
  {
   extern const char *Hlp_PROFILE_Account;
   extern const char *Txt_Before_going_to_any_other_option_you_must_fill_your_nickname;
   extern const char *Txt_Please_fill_in_your_email_address;
   extern const char *Txt_Please_fill_in_your_ID;
   extern const char *Txt_Please_check_and_confirm_your_email_address;
   extern const char *Txt_User_account;
   bool IMustFillNickname   = false;
   bool IMustFillEmail      = false;
   bool IShouldConfirmEmail = false;
   bool IShouldFillID       = false;

   /***** Get current user's nickname and email address
          It's necessary because current nickname or email could be just updated *****/
   Nck_GetNicknameFromUsrCod (Gbl.Usrs.Me.UsrDat.UsrCod,Gbl.Usrs.Me.UsrDat.Nickname);
   Mai_GetEmailFromUsrCod (&Gbl.Usrs.Me.UsrDat);

   /***** Check nickname, email and ID *****/
   if (!Gbl.Usrs.Me.UsrDat.Nickname[0])
      IMustFillNickname = true;
   else if (!Gbl.Usrs.Me.UsrDat.Email[0])
      IMustFillEmail = true;
   else
     {
      if (!Gbl.Usrs.Me.UsrDat.EmailConfirmed &&		// Email not yet confirmed
	  !Gbl.Usrs.Me.ConfirmEmailJustSent)		// Do not ask for email confirmation when confirmation email is just sent
	 IShouldConfirmEmail = true;
      if (!Gbl.Usrs.Me.UsrDat.IDs.Num)
	 IShouldFillID = true;
     }

   /***** Show alert to report that confirmation email has been sent *****/
   if (Gbl.Usrs.Me.ConfirmEmailJustSent)
      Mai_ShowMsgConfirmEmailHasBeenSent ();

   /***** Put links to change my password and to remove my account*****/
   fprintf (Gbl.F.Out,"<div class=\"CONTEXT_MENU\">");
   Pwd_PutLinkToChangeMyPassword ();
   if (Acc_CheckIfICanEliminateAccount (Gbl.Usrs.Me.UsrDat.UsrCod))
      Acc_PutLinkToRemoveMyAccount ();
   fprintf (Gbl.F.Out,"</div>");

   /***** Start table *****/
   Lay_StartRoundFrameTable (NULL,Txt_User_account,NULL,Hlp_PROFILE_Account,2);

   /***** Nickname *****/
   if (IMustFillNickname)
     {
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td colspan=\"2\">");
      Ale_ShowAlert (Ale_WARNING,Txt_Before_going_to_any_other_option_you_must_fill_your_nickname);
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");
     }
   Nck_ShowFormChangeUsrNickname ();

   /***** Separator *****/
   Acc_PrintAccountSeparator ();

   /***** Email *****/
   if (IMustFillEmail || IShouldConfirmEmail)
     {
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td colspan=\"2\">");
      Ale_ShowAlert (Ale_WARNING,IMustFillEmail ? Txt_Please_fill_in_your_email_address :
	                                          Txt_Please_check_and_confirm_your_email_address);
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");
     }
   Mai_ShowFormChangeUsrEmail (&Gbl.Usrs.Me.UsrDat,true);

   /***** Separator *****/
   Acc_PrintAccountSeparator ();

   /***** User's ID *****/
   if (IShouldFillID)
     {
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td colspan=\"2\">");
      Ale_ShowAlert (Ale_WARNING,Txt_Please_fill_in_your_ID);
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");
     }
   ID_ShowFormChangeUsrID (&Gbl.Usrs.Me.UsrDat,true);

   /***** End of table *****/
   Lay_EndRoundFrameTable ();
  }

/*****************************************************************************/
/******** Put a link to the action used to request user's password ***********/
/*****************************************************************************/

static void Acc_PutLinkToRemoveMyAccount (void)
  {
   extern const char *Txt_Remove_account;

   Lay_PutContextualLink (ActReqRemMyAcc,NULL,Acc_PutParamsToRemoveMyAccount,
                          "remove-on64x64.png",
                          Txt_Remove_account,Txt_Remove_account,
                          NULL);
  }

static void Acc_PutParamsToRemoveMyAccount (void)
  {
   Usr_PutParamMyUsrCodEncrypted ();
   Par_PutHiddenParamUnsigned ("RegRemAction",
                               (unsigned) Enr_ELIMINATE_ONE_USR_FROM_PLATFORM);
  }

/*****************************************************************************/
/******* Draw a separator between different parts of new account form ********/
/*****************************************************************************/

static void Acc_PrintAccountSeparator (void)
  {
   extern const char *The_ClassSeparator[The_NUM_THEMES];

   /***** Separator *****/
   fprintf (Gbl.F.Out,"<tr>"
		      "<td colspan=\"2\" class=\"CENTER_MIDDLE\">"
		      "<hr class=\"%s\" />"
		      "</td>"
		      "</tr>",
	    The_ClassSeparator[Gbl.Prefs.Theme]);
  }

/*****************************************************************************/
/*************** Create new user account with an ID and login ****************/
/*****************************************************************************/
// Return true if no error and user can be logged in
// Return false on error

bool Acc_CreateMyNewAccountAndLogIn (void)
  {
   char NewNicknameWithoutArroba[Nck_MAX_BYTES_NICKNAME_FROM_FORM + 1];
   char NewEmail[Cns_MAX_BYTES_EMAIL_ADDRESS + 1];
   char NewEncryptedPassword[Pwd_BYTES_ENCRYPTED_PASSWORD + 1];

   if (Acc_GetParamsNewAccount (NewNicknameWithoutArroba,NewEmail,NewEncryptedPassword))
     {
      /***** User's has no ID *****/
      Gbl.Usrs.Me.UsrDat.IDs.Num = 0;
      Gbl.Usrs.Me.UsrDat.IDs.List = NULL;

      /***** Set password to the password typed by the user *****/
      Str_Copy (Gbl.Usrs.Me.UsrDat.Password,NewEncryptedPassword,
                Pwd_BYTES_ENCRYPTED_PASSWORD);

      /***** User does not exist in the platform, so create him/her! *****/
      Acc_CreateNewUsr (&Gbl.Usrs.Me.UsrDat,
                        true);	// I am creating my own account

      /***** Save nickname *****/
      Nck_UpdateMyNick (NewNicknameWithoutArroba);
      Str_Copy (Gbl.Usrs.Me.UsrDat.Nickname,NewNicknameWithoutArroba,
                Nck_MAX_BYTES_NICKNAME_WITHOUT_ARROBA);

      /***** Save email *****/
      if (Mai_UpdateEmailInDB (&Gbl.Usrs.Me.UsrDat,NewEmail))
	{
	 /* Email updated sucessfully */
	 Str_Copy (Gbl.Usrs.Me.UsrDat.Email,NewEmail,
	           Cns_MAX_BYTES_EMAIL_ADDRESS);

	 Gbl.Usrs.Me.UsrDat.EmailConfirmed = false;
	}

      return true;
     }
   else
     {
      /***** Show form again ******/
      Acc_ShowFormRequestNewAccountWithParams (NewNicknameWithoutArroba,NewEmail);
      return false;
     }
  }

/*****************************************************************************/
/************* Get parameters for the creation of a new account **************/
/*****************************************************************************/
// Return false on error

static bool Acc_GetParamsNewAccount (char NewNicknameWithoutArroba[Nck_MAX_BYTES_NICKNAME_FROM_FORM + 1],
                                     char *NewEmail,
                                     char *NewEncryptedPassword)
  {
   extern const char *Txt_The_nickname_X_had_been_registered_by_another_user;
   extern const char *Txt_The_nickname_entered_X_is_not_valid_;
   extern const char *Txt_The_email_address_X_had_been_registered_by_another_user;
   extern const char *Txt_The_email_address_entered_X_is_not_valid;
   char Query[256 + Cns_MAX_CHARS_EMAIL_ADDRESS];
   char NewNicknameWithArroba[Nck_MAX_BYTES_NICKNAME_FROM_FORM + 1];
   char NewPlainPassword[Pwd_MAX_BYTES_PLAIN_PASSWORD + 1];
   bool Error = false;

   /***** Step 1/3: Get new nickname from form *****/
   Par_GetParToText ("NewNick",NewNicknameWithArroba,
                     Nck_MAX_BYTES_NICKNAME_FROM_FORM);

   /* Remove arrobas at the beginning */
   Str_Copy (NewNicknameWithoutArroba,NewNicknameWithArroba,
             Nck_MAX_BYTES_NICKNAME_FROM_FORM);
   Str_RemoveLeadingArrobas (NewNicknameWithoutArroba);

   /* Create a new version of the nickname with arroba */
   sprintf (NewNicknameWithArroba,"@%s",NewNicknameWithoutArroba);

   if (Nck_CheckIfNickWithArrobaIsValid (NewNicknameWithArroba))        // If new nickname is valid
     {
      /* Check if the new nickname
         matches any of the nicknames of other users */
      sprintf (Query,"SELECT COUNT(*) FROM usr_nicknames"
		     " WHERE Nickname='%s' AND UsrCod<>%ld",
	       NewNicknameWithoutArroba,Gbl.Usrs.Me.UsrDat.UsrCod);
      if (DB_QueryCOUNT (Query,"can not check if nickname already existed"))        // A nickname of another user is the same that this nickname
	{
	 Error = true;
	 sprintf (Gbl.Alert.Txt,Txt_The_nickname_X_had_been_registered_by_another_user,
		  NewNicknameWithoutArroba);
	 Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);
	}
     }
   else        // New nickname is not valid
     {
      Error = true;
      sprintf (Gbl.Alert.Txt,Txt_The_nickname_entered_X_is_not_valid_,
               NewNicknameWithArroba,
               Nck_MIN_CHARS_NICKNAME_WITHOUT_ARROBA,
               Nck_MAX_CHARS_NICKNAME_WITHOUT_ARROBA);
      Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);
     }

   /***** Step 2/3: Get new email from form *****/
   Par_GetParToText ("NewEmail",NewEmail,Cns_MAX_BYTES_EMAIL_ADDRESS);

   if (Mai_CheckIfEmailIsValid (NewEmail))	// New email is valid
     {
      /* Check if the new email matches
         any of the confirmed emails of other users */
      sprintf (Query,"SELECT COUNT(*) FROM usr_emails"
		     " WHERE E_mail='%s' AND Confirmed='Y'",
	       NewEmail);
      if (DB_QueryCOUNT (Query,"can not check if email already existed"))	// An email of another user is the same that my email
	{
	 Error = true;
	 sprintf (Gbl.Alert.Txt,Txt_The_email_address_X_had_been_registered_by_another_user,
		  NewEmail);
	 Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);
	}
     }
   else	// New email is not valid
     {
      Error = true;
      sprintf (Gbl.Alert.Txt,Txt_The_email_address_entered_X_is_not_valid,
               NewEmail);
      Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);
     }

   /***** Step 3/3: Get new password from form *****/
   Par_GetParToText ("Paswd",NewPlainPassword,Pwd_MAX_BYTES_PLAIN_PASSWORD);
   Cry_EncryptSHA512Base64 (NewPlainPassword,NewEncryptedPassword);
   if (!Pwd_SlowCheckIfPasswordIsGood (NewPlainPassword,NewEncryptedPassword,-1L))        // New password is good?
     {
      Error = true;
      Ale_ShowAlert (Ale_WARNING,Gbl.Alert.Txt);	// Error message is set in Usr_SlowCheckIfPasswordIsGood
     }

   return !Error;
  }

/*****************************************************************************/
/****************************** Create new user ******************************/
/*****************************************************************************/
// UsrDat->UsrCod must be <= 0
// UsrDat->UsrDat.IDs must contain a list of IDs for the new user

void Acc_CreateNewUsr (struct UsrData *UsrDat,bool CreatingMyOwnAccount)
  {
   extern const char *The_ThemeId[The_NUM_THEMES];
   extern const char *Ico_IconSetId[Ico_NUM_ICON_SETS];
   extern const char *Pri_VisibilityDB[Pri_NUM_OPTIONS_PRIVACY];
   extern const char *Txt_STR_LANG_ID[1 + Txt_NUM_LANGUAGES];
   extern const char *Usr_StringsSexDB[Usr_NUM_SEXS];
   char BirthdayStrDB[Usr_BIRTHDAY_STR_DB_LENGTH + 1];
   char Query[2048];
   char PathRelUsr[PATH_MAX + 1];
   unsigned NumID;

   /***** Check if user's code is initialized *****/
   if (UsrDat->UsrCod > 0)
      Lay_ShowErrorAndExit ("Can not create new user.");

   /***** Create encrypted user's code *****/
   Acc_CreateNewEncryptedUsrCod (UsrDat);

   /***** Filter some user's data before inserting */
   Enr_FilterUsrDat (UsrDat);

   /***** Insert new user in database *****/
   /* Insert user's data */
   Usr_CreateBirthdayStrDB (UsrDat,BirthdayStrDB);
   sprintf (Query,"INSERT INTO usr_data"
	          " (EncryptedUsrCod,Password,Surname1,Surname2,FirstName,Sex,"
		  "Theme,IconSet,Language,FirstDayOfWeek,DateFormat,"
		  "PhotoVisibility,ProfileVisibility,"
		  "CtyCod,"
		  "LocalAddress,LocalPhone,FamilyAddress,FamilyPhone,OriginPlace,Birthday,Comments,"
		  "Menu,SideCols,NotifNtfEvents,EmailNtfEvents)"
		  " VALUES"
		  " ('%s','%s','%s','%s','%s','%s',"
		  "'%s','%s','%s',%u,%u,"
		  "'%s','%s',"
		  "%ld,"
		  "'%s','%s','%s','%s','%s',%s,'%s',"
		  "%u,%u,-1,0)",
	    UsrDat->EncryptedUsrCod,
	    UsrDat->Password,
	    UsrDat->Surname1,UsrDat->Surname2,UsrDat->FirstName,
	    Usr_StringsSexDB[UsrDat->Sex],
	    The_ThemeId[UsrDat->Prefs.Theme],
	    Ico_IconSetId[UsrDat->Prefs.IconSet],
	    Txt_STR_LANG_ID[UsrDat->Prefs.Language],
	    Cal_FIRST_DAY_OF_WEEK_DEFAULT,
	    (unsigned) Dat_FORMAT_DEFAULT,
            Pri_VisibilityDB[UsrDat->PhotoVisibility],
            Pri_VisibilityDB[UsrDat->ProfileVisibility],
	    UsrDat->CtyCod,
	    UsrDat->LocalAddress ,UsrDat->LocalPhone,
	    UsrDat->FamilyAddress,UsrDat->FamilyPhone,
	    UsrDat->OriginPlace,
	    BirthdayStrDB,
	    UsrDat->Comments ? UsrDat->Comments :
		               "",
            (unsigned) Mnu_MENU_DEFAULT,
            (unsigned) Cfg_DEFAULT_COLUMNS);
   UsrDat->UsrCod = DB_QueryINSERTandReturnCode (Query,"can not create user");

   /* Insert user's IDs as confirmed */
   for (NumID = 0;
	NumID < UsrDat->IDs.Num;
	NumID++)
     {
      Str_ConvertToUpperText (UsrDat->IDs.List[NumID].ID);
      sprintf (Query,"INSERT INTO usr_IDs"
	             " (UsrCod,UsrID,CreatTime,Confirmed)"
		     " VALUES"
		     " (%ld,'%s',NOW(),'%c')",
	       UsrDat->UsrCod,
	       UsrDat->IDs.List[NumID].ID,
	       UsrDat->IDs.List[NumID].Confirmed ? 'Y' :
		                                   'N');
      DB_QueryINSERT (Query,"can not store user's ID when creating user");
     }

   /***** Create directory for the user, if not exists *****/
   Usr_ConstructPathUsr (UsrDat->UsrCod,PathRelUsr);
   Fil_CreateDirIfNotExists (PathRelUsr);

   /***** Create user's figures *****/
   Prf_CreateNewUsrFigures (UsrDat->UsrCod,CreatingMyOwnAccount);
  }

/*****************************************************************************/
/******************** Create a new encrypted user's code *********************/
/*****************************************************************************/

#define LENGTH_RANDOM_STR 32
#define MAX_TRY 10

static void Acc_CreateNewEncryptedUsrCod (struct UsrData *UsrDat)
  {
   char RandomStr[LENGTH_RANDOM_STR + 1];
   unsigned NumTry;

   for (NumTry = 0;
        NumTry < MAX_TRY;
        NumTry++)
     {
      Str_CreateRandomAlphanumStr (RandomStr,LENGTH_RANDOM_STR);
      Cry_EncryptSHA256Base64 (RandomStr,UsrDat->EncryptedUsrCod);
      if (!Usr_ChkIfEncryptedUsrCodExists (UsrDat->EncryptedUsrCod))
          break;
     }
   if (NumTry == MAX_TRY)
      Lay_ShowErrorAndExit ("Can not create a new encrypted user's code.");
   }

/*****************************************************************************/
/***************** Message after creation of a new account *******************/
/*****************************************************************************/

void Acc_AfterCreationNewAccount (void)
  {
   extern const char *Txt_Congratulations_You_have_created_your_account_X_Now_Y_will_request_you_;

   if (Gbl.Usrs.Me.Logged)	// If account has been created without problem, I am logged
     {
      /***** Show message of success *****/
      sprintf (Gbl.Alert.Txt,Txt_Congratulations_You_have_created_your_account_X_Now_Y_will_request_you_,
	       Gbl.Usrs.Me.UsrDat.Nickname,
	       Cfg_PLATFORM_SHORT_NAME);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);

      /***** Show form with account data *****/
      Acc_ShowFormChangeMyAccount ();
     }
  }

/*****************************************************************************/
/************** Definite removing of a user from the platform ****************/
/*****************************************************************************/

void Acc_GetUsrCodAndRemUsrGbl (void)
  {
   extern const char *Txt_User_not_found_or_you_do_not_have_permission_;
   bool Error = false;

   if (Usr_GetParamOtherUsrCodEncryptedAndGetUsrData ())
     {
      if (Acc_CheckIfICanEliminateAccount (Gbl.Usrs.Other.UsrDat.UsrCod))
         Acc_ReqRemAccountOrRemAccount (Acc_REMOVE_USR);
      else
         Error = true;
     }
   else
      Error = true;

   if (Error)
      Ale_ShowAlert (Ale_WARNING,Txt_User_not_found_or_you_do_not_have_permission_);
  }

/*****************************************************************************/
/*************************** Remove a user account ***************************/
/*****************************************************************************/

void Acc_ReqRemAccountOrRemAccount (Acc_ReqOrRemUsr_t RequestOrRemove)
  {
   switch (RequestOrRemove)
     {
      case Acc_REQUEST_REMOVE_USR:	// Ask if eliminate completely the user from the platform
	 Acc_AskIfRemoveUsrAccount (Gbl.Usrs.Me.UsrDat.UsrCod == Gbl.Usrs.Other.UsrDat.UsrCod);
	 break;
      case Acc_REMOVE_USR:		// Eliminate completely the user from the platform
	 if (Pwd_GetConfirmationOnDangerousAction ())
	   {
	    Acc_CompletelyEliminateAccount (&Gbl.Usrs.Other.UsrDat,Cns_VERBOSE);

	    /***** Move unused contents of messages to table of deleted contents of messages *****/
	    Msg_MoveUnusedMsgsContentToDeleted ();
	   }
	 break;
     }
  }

/*****************************************************************************/
/******** Check if I can eliminate completely another user's account *********/
/*****************************************************************************/

bool Acc_CheckIfICanEliminateAccount (long UsrCod)
  {
   bool ItsMe = (Gbl.Usrs.Me.Logged &&
	         UsrCod == Gbl.Usrs.Me.UsrDat.UsrCod);

   // A user logged as superuser can eliminate any user except her/him
   // Other users only can eliminate themselves
   return (( ItsMe &&							// It's me
	    (Gbl.Usrs.Me.Role.Available & (1 << Rol_SYS_ADM)) == 0)	// I can not be system admin
	   ||
           (!ItsMe &&							// It's not me
             Gbl.Usrs.Me.Role.Logged == Rol_SYS_ADM));		// I am logged as system admin
  }

/*****************************************************************************/
/*********** Ask if really wanted to eliminate completely a user *************/
/*****************************************************************************/

static void Acc_AskIfRemoveUsrAccount (bool ItsMe)
  {
   if (ItsMe)
      Acc_AskIfRemoveMyAccount ();
   else
      Acc_AskIfRemoveOtherUsrAccount ();
  }

void Acc_AskIfRemoveMyAccount (void)
  {
   extern const char *Txt_Do_you_really_want_to_completely_eliminate_your_user_account;
   extern const char *Txt_Eliminate_my_user_account;

   /***** Show question and button to remove my user account *****/
   /* Start alert */
   Ale_ShowAlertAndButton1 (Ale_QUESTION,Txt_Do_you_really_want_to_completely_eliminate_your_user_account);

   /* Show my record */
   Rec_ShowSharedRecordUnmodifiable (&Gbl.Usrs.Me.UsrDat);

   /* Show form to request confirmation */
   Act_FormStart (ActRemMyAcc);
   Pwd_AskForConfirmationOnDangerousAction ();
   Lay_PutRemoveButton (Txt_Eliminate_my_user_account);
   Act_FormEnd ();

   /* End alert */
   Ale_ShowAlertAndButton2 (ActUnk,NULL,NULL,NULL,Lay_NO_BUTTON,NULL);
  }

static void Acc_AskIfRemoveOtherUsrAccount (void)
  {
   extern const char *Txt_Do_you_really_want_to_completely_eliminate_the_following_user;
   extern const char *Txt_Eliminate_user_account;
   extern const char *Txt_User_not_found_or_you_do_not_have_permission_;

   if (Usr_ChkIfUsrCodExists (Gbl.Usrs.Other.UsrDat.UsrCod))
     {
      /***** Show question and button to remove user account *****/
      /* Start alert */
      Ale_ShowAlertAndButton1 (Ale_QUESTION,Txt_Do_you_really_want_to_completely_eliminate_the_following_user);

      /* Show user's record */
      Rec_ShowSharedRecordUnmodifiable (&Gbl.Usrs.Other.UsrDat);

      /* Show form to request confirmation */
      Act_FormStart (ActRemUsrGbl);
      Usr_PutParamOtherUsrCodEncrypted ();
      Pwd_AskForConfirmationOnDangerousAction ();
      Lay_PutRemoveButton (Txt_Eliminate_user_account);
      Act_FormEnd ();

      /* End alert */
      Ale_ShowAlertAndButton2 (ActUnk,NULL,NULL,NULL,Lay_NO_BUTTON,NULL);
     }
   else
      Ale_ShowAlert (Ale_WARNING,Txt_User_not_found_or_you_do_not_have_permission_);
  }

/*****************************************************************************/
/************* Remove completely a user from the whole platform **************/
/*****************************************************************************/

void Acc_RemoveMyAccount (void)
  {
   if (Pwd_GetConfirmationOnDangerousAction ())
     {
      Acc_CompletelyEliminateAccount (&Gbl.Usrs.Me.UsrDat,Cns_VERBOSE);

      /***** Move unused contents of messages to table of deleted contents of messages *****/
      Msg_MoveUnusedMsgsContentToDeleted ();
     }
  }

void Acc_CompletelyEliminateAccount (struct UsrData *UsrDat,
                                     Cns_QuietOrVerbose_t QuietOrVerbose)
  {
   extern const char *Txt_THE_USER_X_has_been_removed_from_all_his_her_courses;
   extern const char *Txt_THE_USER_X_has_been_removed_as_administrator;
   extern const char *Txt_Messages_of_THE_USER_X_have_been_deleted;
   extern const char *Txt_Briefcase_of_THE_USER_X_has_been_removed;
   extern const char *Txt_Photo_of_THE_USER_X_has_been_removed;
   extern const char *Txt_Record_card_of_THE_USER_X_has_been_removed;
   char Query[128];
   bool PhotoRemoved = false;

   /***** Remove the works zones of the user in all courses *****/
   Brw_RemoveUsrWorksInAllCrss (UsrDat);        // Make this before of removing the user from the courses

   /***** Remove the fields of course record in all courses *****/
   Rec_RemoveFieldsCrsRecordAll (UsrDat->UsrCod);

   /***** Remove user from all the attendance events *****/
   Att_RemoveUsrFromAllAttEvents (UsrDat->UsrCod);

   /***** Remove user from all the groups of all courses *****/
   Grp_RemUsrFromAllGrps (UsrDat);

   /***** Remove user's requests for inscription *****/
   sprintf (Query,"DELETE FROM crs_usr_requests WHERE UsrCod=%ld",
            UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove user's requests for inscription");

   /***** Remove user from possible duplicate users *****/
   Dup_RemoveUsrFromDuplicated (UsrDat->UsrCod);

   /***** Remove user from the table of courses and users *****/
   sprintf (Query,"DELETE FROM crs_usr WHERE UsrCod=%ld",
            UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove a user from all courses");

   if (QuietOrVerbose == Cns_VERBOSE)
     {
      sprintf (Gbl.Alert.Txt,Txt_THE_USER_X_has_been_removed_from_all_his_her_courses,
               UsrDat->FullName);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }

   /***** Remove user as administrator of any degree *****/
   sprintf (Query,"DELETE FROM admin WHERE UsrCod=%ld",
            UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove a user as administrator");

   if (QuietOrVerbose == Cns_VERBOSE)
     {
      sprintf (Gbl.Alert.Txt,Txt_THE_USER_X_has_been_removed_as_administrator,
               UsrDat->FullName);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }

   /***** Remove user's clipboard in forums *****/
   For_RemoveUsrFromThrClipboard (UsrDat->UsrCod);

   /***** Remove some files of the user's from database *****/
   Brw_RemoveUsrFilesFromDB (UsrDat->UsrCod);

   /***** Remove the file tree of a user *****/
   Acc_RemoveUsrBriefcase (UsrDat);
   if (QuietOrVerbose == Cns_VERBOSE)
     {
      sprintf (Gbl.Alert.Txt,Txt_Briefcase_of_THE_USER_X_has_been_removed,
               UsrDat->FullName);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }

   /***** Remove test results made by user in all courses *****/
   Tst_RemoveTestResultsMadeByUsrInAllCrss (UsrDat->UsrCod);

   /***** Remove user's notifications *****/
   Ntf_RemoveUsrNtfs (UsrDat->UsrCod);

   /***** Delete user's messages sent and received *****/
   Gbl.Msg.FilterContent[0] = '\0';
   Msg_DelAllRecAndSntMsgsUsr (UsrDat->UsrCod);
   if (QuietOrVerbose == Cns_VERBOSE)
     {
      sprintf (Gbl.Alert.Txt,Txt_Messages_of_THE_USER_X_have_been_deleted,
               UsrDat->FullName);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }

   /***** Remove user from tables of banned users *****/
   Usr_RemoveUsrFromUsrBanned (UsrDat->UsrCod);
   Msg_RemoveUsrFromBanned (UsrDat->UsrCod);

   /***** Delete thread read status for this user *****/
   For_RemoveUsrFromReadThrs (UsrDat->UsrCod);

   /***** Remove user from table of seen announcements *****/
   Ann_RemoveUsrFromSeenAnnouncements (UsrDat->UsrCod);

   /***** Remove user from table of connected users *****/
   sprintf (Query,"DELETE FROM connected WHERE UsrCod=%ld",
            UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove a user from table of connected users");

   /***** Remove all sessions of this user *****/
   sprintf (Query,"DELETE FROM sessions WHERE UsrCod=%ld",
            UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove sessions of a user");

   /***** Remove social content associated to the user *****/
   Soc_RemoveUsrSocialContent (UsrDat->UsrCod);

   /***** Remove user's figures *****/
   Prf_RemoveUsrFigures (UsrDat->UsrCod);

   /***** Remove user from table of followers *****/
   Fol_RemoveUsrFromUsrFollow (UsrDat->UsrCod);

   /***** Remove user's usage reports *****/
   Rep_RemoveUsrUsageReports (UsrDat->UsrCod);

   /***** Remove user's agenda *****/
   Agd_RemoveUsrEvents (UsrDat->UsrCod);

   /***** Remove the user from the list of users without photo *****/
   Pho_RemoveUsrFromTableClicksWithoutPhoto (UsrDat->UsrCod);

   /***** Remove user's photo *****/
   PhotoRemoved = Pho_RemovePhoto (UsrDat);
   if (PhotoRemoved && QuietOrVerbose == Cns_VERBOSE)
     {
      sprintf (Gbl.Alert.Txt,Txt_Photo_of_THE_USER_X_has_been_removed,
               UsrDat->FullName);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }

   /***** Remove user *****/
   Acc_RemoveUsr (UsrDat);
   if (QuietOrVerbose == Cns_VERBOSE)
     {
      sprintf (Gbl.Alert.Txt,Txt_Record_card_of_THE_USER_X_has_been_removed,
               UsrDat->FullName);
      Ale_ShowAlert (Ale_SUCCESS,Gbl.Alert.Txt);
     }
  }

/*****************************************************************************/
/********************** Remove the briefcase of a user ***********************/
/*****************************************************************************/

static void Acc_RemoveUsrBriefcase (struct UsrData *UsrDat)
  {
   char PathRelUsr[PATH_MAX + 1];

   /***** Remove files of the user's briefcase from disc *****/
   Usr_ConstructPathUsr (UsrDat->UsrCod,PathRelUsr);
   Fil_RemoveTree (PathRelUsr);
  }

/*****************************************************************************/
/************************ Remove a user from database ************************/
/*****************************************************************************/

static void Acc_RemoveUsr (struct UsrData *UsrDat)
  {
   char Query[128];

   /***** Remove user's webs / social networks *****/
   sprintf (Query,"DELETE FROM usr_webs WHERE UsrCod=%ld",
	    UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove user's webs / social networks");

   /***** Remove user's nicknames *****/
   sprintf (Query,"DELETE FROM usr_nicknames WHERE UsrCod=%ld",
	    UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove user's nicknames");

   /***** Remove user's emails *****/
   sprintf (Query,"DELETE FROM pending_emails WHERE UsrCod=%ld",
	    UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove pending user's emails");

   sprintf (Query,"DELETE FROM usr_emails WHERE UsrCod=%ld",
	    UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove user's emails");

   /***** Remove user's IDs *****/
   sprintf (Query,"DELETE FROM usr_IDs WHERE UsrCod=%ld",
	    UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove user's IDs");

   /***** Remove user's last data *****/
   sprintf (Query,"DELETE FROM usr_last WHERE UsrCod=%ld",
	    UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove user's last data");

   /***** Remove user's data  *****/
   sprintf (Query,"DELETE FROM usr_data WHERE UsrCod=%ld",
	    UsrDat->UsrCod);
   DB_QueryDELETE (Query,"can not remove user's data");
  }
