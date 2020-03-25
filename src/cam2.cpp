//////////////////////////////////////////////////////////////////////////////
//
//  cam2.cpp
//
//  Description:
//      Contains Unigraphics entry points for the application.
//
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE 1

//  Include files
#include <uf.h>
#include <uf_exit.h>
#include <uf_ui.h>

#include <uf_obj.h>

#include <uf_ui_ont.h>
#include <uf_oper.h>
#include <uf_ncgroup.h>

#include <uf_part.h>
#include <uf_param.h>
#include <uf_param_indices.h>

/*
#if ! defined ( __hp9000s800 ) && ! defined ( __sgi ) && ! defined ( __sun )
# include <strstream>
  using std::ostrstream;
  using std::endl;
  using std::ends;
#else
# include <strstream.h>
#endif
#include <iostream.h>
*/

#include "cam2.h"

#include <stdio.h>

const int num_param = 10 ;

int  param_feed[14]={
	UF_PARAM_FEED_RAPID,
  UF_PARAM_FEED_APPROACH,
  UF_PARAM_FEED_ENGAGE,
  UF_PARAM_FEED_FIRST_CUT,
  UF_PARAM_FEED_STEPOVER,
  UF_PARAM_FEED_CUT,
  UF_PARAM_FEED_TRAVERSAL,
  UF_PARAM_FEED_RETRACT,
  UF_PARAM_FEED_DEPARTURE,
  UF_PARAM_FEED_RETURN,
  0,0,0,0
} ;

char menu[14][16]={
	"Rapid",
	"Approach",
	"Engage",
	"First Cut",
	"Step Over",
	"Cut",
	"Traversal",
	"Retract",
	"DEPARTURE",
	"Return",
	"","","",""
} ;
double  da[14];

int output_win = 0 ;

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

#define COUNT_PRG 300

struct PRG
{
   int     num;
   tag_t   tag;
   char    name[UF_OPER_MAX_NAME_LEN+1];
} ;

static struct PRG prg_list[COUNT_PRG] ;
int prg_list_count=0;

/*----------------------------------------------------------------------------*/
static logical cycleGeneratePrg( tag_t   tag, void   *data )
{
   char     name[UF_OPER_MAX_NAME_LEN + 1];
   int      ecode;

   name[0]='\0';
   ecode = UF_OBJ_ask_name(tag, name);// спросим имя обьекта
   //UF_UI_write_listing_window("\n");  UF_UI_write_listing_window(name);

   if (prg_list_count>=COUNT_PRG) {
     uc1601("Число Операций-превышает допустимое (>300)\n Уменьшите количество выбора",1);
   	 return( FALSE );
   }
   prg_list[prg_list_count].num=prg_list_count;
   prg_list[prg_list_count].tag=tag;
   prg_list[prg_list_count].name[0]='\0';
   sprintf(prg_list[prg_list_count].name,"%s",name);
   prg_list_count++;

   return( TRUE );
}

/*    */
/*----------------------------------------------------------------------------*/
// gen - флаг генерации
int _run ( tag_t prg )
{
  int   i , err ;
  UF_PARAM_feedrate_t  feed;
  char  prog_name[UF_OPER_MAX_NAME_LEN+1];
  int   ret = 0 ;
  int   type, subtype;
  char  str [256];
  char  st[10];
  double val;

  ret = 0 ;

  UF_CALL( UF_OBJ_ask_type_and_subtype (prg, &type, &subtype ) );
  printf("\n type=%d subtype=%d ",type,subtype);
  if (type!=UF_machining_operation_type) {
    if (output_win) {
      prog_name[0]='\0';
      UF_CALL( UF_OBJ_ask_name(prg, prog_name) );// спросим имя обьекта
      str[0]='\0'; sprintf(str,"\n %s",prog_name);
      UF_UI_write_listing_window(str);
    }
   	return (ret);
  }

  prog_name[0]='\0';
  //UF_OBJ_ask_name(prg, prog_name);// спросим имя обьекта
  UF_CALL( UF_OPER_ask_name_from_tag(prg, prog_name) );

  str[0]='\0'; sprintf(str,"\n\t %s",prog_name);
  printf("%s",str);
  if (output_win) { UF_UI_write_listing_window(str); }

  for (i=0,ret=0;i<num_param;i++) {
    err=UF_PARAM_ask_subobj_ptr_value (prg,param_feed[i],&feed);

    //if (da[i]<0) continue ;
    st[0]='\0';sprintf(st,"-");
    if (err!=0) { st[0]='\0';sprintf(st,"-error"); }

    val = feed.value ;

    if (da[i]>=0 && err==0 && feed.value!=da[i]) {
      feed.value=-9999;
      // почему не знаю - но думаю так же поступил и я! при разработке под unix
      // отрицательное значение снимает наследование и обнуляет значение (замечено еще в v16)
      UF_CALL( UF_PARAM_set_subobj_ptr_value (prg,param_feed[i],&feed) );

      feed.value=da[i];
      UF_CALL( UF_PARAM_set_subobj_ptr_value (prg,param_feed[i],&feed) );

      st[0]='\0';sprintf(st,"change");
    	ret++;
    }

    str[0]='\0'; sprintf(str,"\n\t\t %-02d) % 15s=% 11.3f  NewF=% 11.3f  :Status->%s",i,menu[i],val,da[i],st);
    printf("%s",str);
    if (output_win) { UF_UI_write_listing_window(str); }

  }

  return (ret);
}


//======================================================================
int cam2_feed_get_value(tag_t param_tag )
{
  int i ;
  UF_PARAM_feedrate_t  feed;

  for(i=0;i<num_param;i++) {
    da[i]=-1.0;
    UF_CALL(UF_PARAM_ask_subobj_ptr_value (param_tag,param_feed[i],&feed)) ;
    if (feed.value<0.0) feed.value=-1.0;
    da[i]=feed.value;
  }

  return(0);
}


/**********************************************************************/
int do_cam2()
{
/*  Variable Declarations */
    char str[133];

    tag_t prg = NULL_TAG;
    int   i , count , j ;
    int   obj_count = 0;
    tag_t *tags = NULL ;

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
    char *mes3[]={
      "Выводить листинг изменений по операциям ",
      "- до и после изменений значений ?",
      NULL
    };
    UF_UI_message_buttons_t buttons3 = { TRUE, FALSE, TRUE, "Нет", NULL, "Да", 1, 0, 2 };


    response=0;
    UF_UI_message_dialog("Cam2.....", UF_UI_MESSAGE_INFORMATION, mes1, 5, TRUE, &buttons1, &response);
    if (response!=1) { return (0) ; }

    int module_id;
    UF_ask_application_module(&module_id);
    if (UF_APP_CAM!=module_id) {
       // UF_APP_GATEWAY UF_APP_CAM UF_APP_MODELING UF_APP_NONE
       uc1601("Запуск DLL - производится из модуля обработки\n - 2005г.",1);
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

   	// инициализация массива
  	for(i=0;i<num_param;i++) { da[i]=-1.; }

    /********************************************************************************/
    // инициализация массива - первой операцией в списке
    for(i=0;i<1;i++) {
       prg = tags[i]; // идентификатор объекта
       cam2_feed_get_value( prg );
    }

    /********************************************************************************/

    response = uc1609("..Параметры подач (мм/мин)..", menu, num_param, da, &i);
    if (response != 3 && response != 4) { return (-1); }

    /********************************************************************************/
    generat=1;
    UF_UI_message_dialog("Cam2.....", UF_UI_MESSAGE_QUESTION, mes2, 1, TRUE, &buttons2, &generat);
    if (generat==2) { generat=0; }
    /********************************************************************************/
    output_win=0;
    UF_UI_message_dialog("Cam2.....", UF_UI_MESSAGE_QUESTION, mes3, 2, TRUE, &buttons3, &output_win);
    if (output_win==1) { output_win=0; }
    /********************************************************************************/
    if (output_win) { UF_CALL( UF_UI_open_listing_window() ); }
    /********************************************************************************/

 for(i=0,count=0;i<obj_count;i++)
 {
   prg = tags[i]; // идентификатор объекта

   prg_list_count=0;// заполняем структуру
   UF_CALL( UF_NCGROUP_cycle_members( prg, cycleGeneratePrg,NULL ) ) ;

   if (prg_list_count==0) {
   	  count+=_run( prg );
   	  if (generat==1) { UF_CALL( UF_PARAM_generate (prg,&generated ) ); }
   } else for (j=0;j<prg_list_count;j++) {
             count+=_run( prg_list[j].tag );
             if (generat==1) { UF_CALL( UF_PARAM_generate (prg_list[j].tag,&generated ) ); }
           }
 }

 UF_free(tags);

 //UF_DISP_refresh ();

 UF_UI_toggle_stoplight(0);

 str[0]='\0'; sprintf(str,"\n\nИзменено значений=%d\n Просмотрено программ|операций в цикле=%d\n программа завершена.\n$$",count,obj_count);
 if (output_win) { UF_UI_write_listing_window(str); }
 else
 uc1601(str,1);


 return (0);
}



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
