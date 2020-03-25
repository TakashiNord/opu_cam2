//////////////////////////////////////////////////////////////////////////////
//
//  cam2.cpp
//
//  Description:
//      Contains Unigraphics entry points for the application.
//
//////////////////////////////////////////////////////////////////////////////

//  Include files
#include <uf.h>
#include <uf_exit.h>
#include <uf_ui.h>

#include <uf_obj.h>

#include <uf_ui_ont.h>

#include <uf_oper.h>

#include <uf_part.h>
#include <uf_param.h>
#include <uf_param_indices.h>

#if ! defined ( __hp9000s800 ) && ! defined ( __sgi ) && ! defined ( __sun )
# include <strstream>
  using std::ostrstream;
  using std::endl;
  using std::ends;
#else
# include <strstream.h>
#endif
#include <iostream.h>

#include "cam2.h"


#define UF_CALL(X) (report( __FILE__, __LINE__, #X, (X)))

static int report( char *file, int line, char *call, int irc)
{
  if (irc)
  {
     char    messg[133];
     printf("%s, line %d:  %s\n", file, line, call);
     (UF_get_fail_message(irc, messg)) ?
       printf("    returned a %d\n", irc) :
       printf("    returned error %d:  %s\n", irc, messg);
  }
  return(irc);
}

int do_cam2();
int cam2_feed_change(tag_t ,int , double );
int cam2_feed_get(tag_t ,int , double *);
int cam2_feed_menu(double **);

//----------------------------------------------------------------------------
//  Activation Methods
//----------------------------------------------------------------------------

//  Explicit Activation
//      This entry point is used to activate the application explicitly, as in
//      "File->Execute UG/Open->User Function..."
extern "C" DllExport void ufusr( char *parm, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    int errorCode = UF_initialize();

    if ( 0 == errorCode )
    {
        /* TODO: Add your application code here */
        do_cam2();

        /* Terminate the API environment */
        errorCode = UF_terminate();
    }

    /* Print out any error messages */
    PrintErrorMessage( errorCode );
}

//----------------------------------------------------------------------------
//  Utilities
//----------------------------------------------------------------------------

// Unload Handler
//     This function specifies when to unload your application from Unigraphics.
//     If your application registers a callback (from a MenuScript item or a
//     User Defined Object for example), this function MUST return
//     "UF_UNLOAD_UG_TERMINATE".
extern "C" int ufusr_ask_unload( void )
{
     /* unload immediately after application exits*/
    return ( UF_UNLOAD_IMMEDIATELY );

     /*via the unload selection dialog... */
     //return ( UF_UNLOAD_SEL_DIALOG );
     /*when UG terminates...              */
     //return ( UF_UNLOAD_UG_TERMINATE );
}

/* PrintErrorMessage
**
**     Prints error messages to standard error and the Unigraphics status
**     line. */
static void PrintErrorMessage( int errorCode )
{
    if ( 0 != errorCode )
    {
        /* Retrieve the associated error message */
        char message[133];
        UF_get_fail_message( errorCode, message );

        /* Print out the message */
        UF_UI_set_status( message );

        fprintf( stderr, "%s\n", message );
    }
}


/*****************************************************************************************/

int do_cam2()
{
/*  Variable Declarations */
    char str[133];
    char menu[10][16] ; double  da[10];

    tag_t       prg = NULL_TAG;
    int i , count = 0 ;
    int   obj_count = 0;
    tag_t *tags = NULL ;
    char  prog_name[UF_OPER_MAX_NAME_LEN+1];
//    int type, subtype ;
    logical generated;
    int generat;
    int response ;
    char *mes1[]={
      "Программа предназначена для изменения значений подач в операциях.",
      "Для этого,Вы должны :",
      "  1) выбрать необходимые операции и нажать кнопку 'Далее..'",
      "  2) в появившемся окне установить необходимые значения подач (мм/мин)",
      "          если значение = -1 - подача в операции не изменяется",
      NULL
    };
    UF_UI_message_buttons_t buttons1 = { TRUE, FALSE, TRUE, "Далее....", NULL, "Отмена", 1, 0, 2 };
    char *mes2[]={
      "Производить генерацию операции после изменения?",
      NULL
    };
    UF_UI_message_buttons_t buttons2 = { TRUE, FALSE, TRUE, "Генерировать..", NULL, "Нет", 1, 0, 2 };

    response=0;
    UF_UI_message_dialog("Cam2.....", UF_UI_MESSAGE_INFORMATION, mes1, 5, TRUE, &buttons1, &response);
    if (response!=1) { return (0) ; }

    int module_id;
    UF_ask_application_module(&module_id);
    if (UF_APP_CAM!=module_id) {
       // UF_APP_GATEWAY UF_APP_CAM UF_APP_MODELING UF_APP_NONE
       uc1601("Запуск DLL - производится из модуля обработки\n(ОГТ-ОПУ,КНААПО) - 2005г.",1);
       return (-1);
    }

    /* Ask displayed part tag */
    if (NULL_TAG==UF_PART_ask_display_part()) {
      uc1601("Cam-часть не активна.....\n программа прервана.",1);
      return (0);
    }

  /********************************************************************************/
    // выбранные обьекты и их кол-во
    UF_CALL( UF_UI_ONT_ask_selected_nodes(&obj_count, &tags) );
    if (obj_count<=0) {
      uc1601("Не выбрано операций!\n Программа прервана..",1);
      return(0);
    }

    UF_UI_toggle_stoplight(1);

    if (obj_count) {

    	//UF_UI_open_listing_window();

    	// инициализация массива
    	for(i=0;i<10;i++) { da[i]=-1.; }

      /********************************************************************************/
      // инициализация массива - первой операцией в списке
      for(i=0;i<1;i++) {
         prg = tags[i]; // идентификатор объекта

         cam2_feed_get (prg,UF_PARAM_FEED_RAPID,&da[0]);
         cam2_feed_get (prg,UF_PARAM_FEED_APPROACH,&da[1]);
         cam2_feed_get (prg,UF_PARAM_FEED_ENGAGE,&da[2]);
         cam2_feed_get (prg,UF_PARAM_FEED_FIRST_CUT,&da[3]);
         cam2_feed_get (prg,UF_PARAM_FEED_STEPOVER,&da[4]);
         cam2_feed_get (prg,UF_PARAM_FEED_CUT,&da[5]);
         cam2_feed_get (prg,UF_PARAM_FEED_TRAVERSAL,&da[6]);
         cam2_feed_get (prg,UF_PARAM_FEED_RETRACT,&da[7]);
         cam2_feed_get (prg,UF_PARAM_FEED_DEPARTURE,&da[8]);
         cam2_feed_get (prg,UF_PARAM_FEED_RETURN,&da[9]);
      }

      /********************************************************************************/
        strcpy(&menu[0][0], "Rapid\0");
        strcpy(&menu[1][0], "Approach\0");
        strcpy(&menu[2][0], "Engage\0");
        strcpy(&menu[3][0], "First Cut\0");
        strcpy(&menu[4][0], "Step Over\0");
        strcpy(&menu[5][0], "Cut\0");
        strcpy(&menu[6][0], "Traversal\0");
        strcpy(&menu[7][0], "Retract\0");
        strcpy(&menu[8][0], "DEPARTURE\0");
        strcpy(&menu[9][0], "Return\0");

        response = uc1609("..Параметры подач (мм/мин)..", menu, 10, da, &i);
        if (response != 3 && response != 4) { return (-1); }

      /********************************************************************************/

        generat=1;
        UF_UI_message_dialog("Cam2.....", UF_UI_MESSAGE_QUESTION, mes2, 1, TRUE, &buttons2, &generat);
        if (generat==2) { generat=0; }

      /********************************************************************************/

      for(i=0,count=0;i<obj_count;i++)
      {
         prg = tags[i]; // идентификатор объекта
         prog_name[0]='\0';
         //UF_OBJ_ask_name(prg, prog_name);// спросим имя обьекта
         UF_OPER_ask_name_from_tag(prg, prog_name);

         //UF_UI_write_listing_window("\n");
         //UF_UI_write_listing_window(prog_name);
         //UF_UI_write_listing_window("\n");

         /* type =               subtype =
         //     программа=121              160
         //     операция =100              110 */
         //UF_CALL( UF_OBJ_ask_type_and_subtype (prg, &type, &subtype ) );

         count+=cam2_feed_change (prg,UF_PARAM_FEED_RAPID,da[0]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_APPROACH,da[1]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_ENGAGE,da[2]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_FIRST_CUT,da[3]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_STEPOVER,da[4]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_CUT,da[5]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_TRAVERSAL,da[6]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_RETRACT,da[7]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_DEPARTURE,da[8]);
         count+=cam2_feed_change (prg,UF_PARAM_FEED_RETURN,da[9]);

         if (generat==1) { UF_CALL( UF_PARAM_generate (prg,&generated ) ); }

      }
      UF_free(tags);
    }

    UF_UI_toggle_stoplight(0);

    UF_DISP_refresh ();

    str[0]='\0'; sprintf(str,"Изменено значений =%d \n Просмотрено операций в цикле =%d \n программа завершена.",count,obj_count);
    uc1601(str,1);

 return (0);
}

int cam2_feed_change(tag_t param_tag,int param_index, double value)
{
  UF_PARAM_feedrate_t  feed;
  //UF_PARAM_index_attribute_t ind;

  if (UF_PARAM_ask_subobj_ptr_value (param_tag,param_index,&feed)<0) return(0);

  if (value<0) return(0);

  if (feed.value==value) return(0);

  feed.value=-9999;
  // почему не знаю - но думаю так же поступил и я! при разработке под unix
  // отрицательное значение снимает наследование и обнуляет значение (замечено еще в v16)
  UF_PARAM_set_subobj_ptr_value (param_tag,param_index,&feed);

  feed.value=value;
  UF_PARAM_set_subobj_ptr_value (param_tag,param_index,&feed);

  return(1);
}

int cam2_feed_get(tag_t param_tag,int param_index, double *value)
{
  UF_PARAM_feedrate_t  feed;

  if (UF_PARAM_ask_subobj_ptr_value (param_tag,param_index,&feed)<0) { *value=-1.0; return(0); }
  if (feed.value<0.0) feed.value=-1.0;

  *value=feed.value;

  return(0);
}


