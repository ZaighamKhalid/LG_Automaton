/**
* @file           LG_Automaon.h
*
* @brief          Navigation engine for LG Automaton - Implementation
*
* @author         Zaigham Khalid
*
* @riskClass      N/A
*
* @moduleID       LG_AUTOMATON
*
* @modulePrefix   LG
*
* @copyright      No copyright. Feel free to share (at your own risk)
*
***********************************************************************************************************************/
/**
* @addtogroup LG_AUTOMATON
* @{
* @file
*/


/****************************** SYSTEM INCLUDES *******************************/
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>

/****************************** PRIVATE INCLUDES ******************************/
#include "LG_Automaton.h"

/****************************** PROJECT INCLUDES ******************************/


/******************************* DRIVER INCLUDES ******************************/


/******************************* LOCAL DEFINES ********************************/
/* De-comment it to test the program without user input */
//#define LG_TESTING

/***************************** LOCAL VARIABLES ********************************/

#ifdef LG_TESTING
const char lInputString[][11]=
{
   "##########,"
   "#        #,"
   "#  S   W #,"
   "#        #,"
   "#  $     #,"
   "#        #,"
   "#@       #,"
   "#        #,"
   "#E     N #,"
   "##########"
};
#endif

/* Priorities list */
static LG_Direction_t lObsPriority[] =
{
   LG_DIR_SOUTH,
   LG_DIR_EAST,
   LG_DIR_NORTH,
   LG_DIR_WEST
};

/* Navigation history */
static LG_Direction_t lNavigationHistory[65535];
static uint16_t lHistoryIndex = 0;

/***************************** LOCAL FUNCTIONS ********************************/
static bool LG_Navigate        (LG_Handle_t *pHandle, LG_Direction_t direction);
static bool LG_ManageObstacle  (LG_Handle_t *pHandle );
static void LG_ToggleBeer      (LG_Handle_t *pHandle );
static void LG_InvertPriorities(LG_Handle_t *pHandle );
static void LG_ManageTeleporter(LG_Handle_t *pHandle );
static void LG_DisplayResults  (LG_Handle_t *pHandle,  bool status  );
static void LG_WaitForSpace    ( void );
static bool LG_FindPosition    ( LG_Handle_t *pHandle, LG_Symbols_t symbol,
                                 LG_Position_t *pPos, bool teleporterFlag);

/************************** PUBLIC FUNCTIONS IMPLEMENTATION ***********************************************************/
int main( void )
{
    int numberOfLines;
    int numberOfColumns;
    bool arrived = false;

    LG_Handle_t automaton;
    LG_Position_t position;

#ifdef LG_TESTING
   numberOfLines = 10;
   numberOfColumns = 10;
#else
    /* Ask user for number of lines and columns*/
    fprintf(stdout, "Enter number of lines: ");
    fflush(stdout);
    fscanf(stdin, "%d", &numberOfLines);

    fprintf(stdout, "Enter number of columns: ");
    fflush(stdout);
    scanf("%d", &numberOfColumns);

    /* Check for the validity of parameters */
    if (!((numberOfLines >= 4)   &&
         (numberOfLines <= 100) &&
         (numberOfColumns >= 4) &&
         (numberOfColumns <= 100)))
    {
      fprintf(stderr, "Invalid input.\n");
      LG_WaitForSpace();
    }
#endif

    char **pData;

    /* Dynamically allocate memory for the map */

    pData = (char**)malloc(numberOfLines * sizeof(char*));

    for (uint8_t i = 0; i < numberOfLines; i++)
    {
       pData[i] = (char*) malloc((numberOfColumns + 1) * sizeof(char));
    }

#ifdef LG_TESTING
    /* Populate the table if testing macro is enabled  */
    for (uint8_t i = 0; i < numberOfLines; i++)
    {
       strncpy(pData[i], lInputString[i], numberOfColumns+1);
    }
#else

    /* Receive map input from the user. */
    fprintf(stdout, "Enter the map: \r\n");
    fflush(stdout);

    for (uint8_t i = 0u; i < numberOfLines; i++)
    {
       char string[102];
       fflush(stdout);
       fflush(stdin);

       if (fgets(string , numberOfColumns + 1 , stdin) == NULL)
       {
          i--;
       }

       strncpy(pData[i], string, numberOfColumns+1);
    }
#endif

   /* To-do List */
   /* Map validation should be implemented here */
   /* 1. Check that the map contains only allowed symbol */
   /* 2. Check that the contour symbol '#' forms a closed boundary and is not present elsewhere */
   /* 3. Check that there is one and only one '$' and '@' */
   /* 4. Check that 'T' is zero or two */

   memset(&lNavigationHistory, LG_DIR_NONE, sizeof(lNavigationHistory));

   /* Initialize automation */
   automaton.pData           = pData;
   automaton.numberOfLines   = numberOfLines;
   automaton.numberofColumns = numberOfColumns;
   automaton.mode            = LG_BEER_OFF;

   /* Find the initial position of starting point */
   if (LG_FindPosition(&automaton, LG_SYMBOL_START, &position, false))
   {
      automaton.position = position;
      automaton.direction = LG_DIR_SOUTH;
      (void)LG_Navigate(&automaton, LG_DIR_SOUTH);
   }
   else
   {
      fprintf(stderr, "Map doesn't contain a starting point\n");
      LG_WaitForSpace();
   }

   bool loop = true;

   /* Run the automaton engine */
   while (loop)
   {
      switch(automaton.nextHurdle)
      {
         case LG_SYMBOL_SPACE:
        	(void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_PM_EAST:
            automaton.direction = LG_DIR_EAST;
            (void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_PM_WEST:
            automaton.direction = LG_DIR_WEST;
            (void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_PM_NORTH:
            automaton.direction = LG_DIR_NORTH;
            (void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_PM_SOUTH:
            automaton.direction = LG_DIR_SOUTH;
            (void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_OBS_INTERNAL:
         case LG_SYMBOL_OBS_CONTOUR:
            LG_ManageObstacle(&automaton);
            break;

         case LG_SYMBOL_BEER:
            LG_ToggleBeer(&automaton);
            (void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_INVERTER:
            LG_InvertPriorities(&automaton);
            (void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_TELEPORTER:
            LG_ManageTeleporter(&automaton);
            (void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_START:
            /* I am not stopping at this symbol. Instead, I would go south */
            /* It is possible that because of a beer or inverter, I end up arriving at destination */
            automaton.direction = LG_DIR_SOUTH;
            (void)LG_Navigate(&automaton, automaton.direction);
            break;

         case LG_SYMBOL_STOP:
            loop = false;
            arrived = true;
            break;

         default:
            loop = false;
            break;
      }

      /* Loop condition is detected using a numerical threshold for navigation steps */
      /* A relatively smarter approach is advised though */
      if (lHistoryIndex >= 0xFFFE)
      {
         loop = false;
      }
   }

   /* Display results */
   LG_DisplayResults(&automaton, arrived);
   return 0;
}

/************************** LOCAL FUNCTIONS IMPLEMENTATION ***********************************************************/


/**
 * @brief       Find the co-ordiantes of the given symbol in the automaton map
 *
 * @param[in]   pHandle        - Pointer to the automaton handle
 * @param[in]   symbol         - Symbol to search for
 * @param[out]  pPos           - pointer receive co-ordiantes of found symbol
 * @param[in]   teleporterFlag - Flag to indicate if teleport search mode is active.
 *
 * @retval      status - true if symbol is found else vice-versa
 *
 ******************************************************************************/
static bool LG_FindPosition( LG_Handle_t *pHandle, LG_Symbols_t symbol,
                             LG_Position_t *pPos, bool teleporterFlag)
{
   bool retVal = false;
   uint8_t yIndex = 0;
   uint8_t xIndex = 0;
   char *e;

   for (yIndex = 0; yIndex < pHandle->numberOfLines ; yIndex++)
   {
      e = strchr(pHandle->pData[yIndex], symbol);
      if (e != NULL)
      {
         if (teleporterFlag == true)
         {
            /* It checks if we have more than one found symbol in a row */
            for (uint8_t i = 0; i < pHandle->numberofColumns; i++)
            {
               if ((pHandle->pData[yIndex][i] == symbol) &&
                   (i != pHandle->position.xCoordinate) &&
                   (yIndex != pHandle->position.yCoordinate))
               {
                  pPos->xCoordinate = i;
                  pPos->yCoordinate = yIndex;
                  retVal = true;
                  break;
               }
            }
         }
         else
         {
            xIndex = (int)(e - pHandle->pData[yIndex]);
            pPos->xCoordinate = xIndex;
            pPos->yCoordinate = yIndex;
            retVal = true;
            break;
         }
      }
   }

   return retVal;
}

/**
 * @brief       Navigation engine for the LG-automaton
 *
 * @param[in]   pHandle        - Pointer to the automaton handle
 * @param[in]   direction      - Direction to navigate
 *
 * @retval      status - true if automaton was navigated
 *
 ******************************************************************************/
static bool LG_Navigate(LG_Handle_t *pHandle, LG_Direction_t direction)
{
   bool retVal = false;
   LG_Position_t pos = pHandle->position;

   /* Add a displacement in the indicated direction */
   switch( direction)
   {
      case LG_DIR_EAST:
        if (pos.xCoordinate < UCHAR_MAX)
        {
               pos.xCoordinate ++;
        }
        break;

      case LG_DIR_WEST:
         if (pos.xCoordinate > 1)
         {
            pos.xCoordinate--;
         }
         break;

      case LG_DIR_NORTH:
         if (pos.yCoordinate > 1)
         {
            pos.yCoordinate--;
         }
         break;

      case LG_DIR_SOUTH:
         if (pos.yCoordinate < UCHAR_MAX)
         {
            pos.yCoordinate++;
         }
         break;

      default:
         /* Through an exception heer */
         break;
   }

   /* Check if we are not already at contour */
   if ((pos.xCoordinate <= pHandle->numberofColumns - 2) &&
       (pos.yCoordinate <= pHandle->numberOfLines - 2))
   {
         /* If we have encountered an internal obstacle with beer mode disabled */
         if ((pHandle->pData[pos.yCoordinate][pos.xCoordinate] == LG_SYMBOL_OBS_INTERNAL) &&
            (pHandle->mode == LG_BEER_OFF))
         {
            pHandle->nextHurdle = LG_SYMBOL_OBS_INTERNAL;
         }
         else
         {
            pHandle->position.xCoordinate = pos.xCoordinate;
            pHandle->position.yCoordinate = pos.yCoordinate;

            pHandle->nextHurdle = (LG_Symbols_t)pHandle->pData[pos.yCoordinate][pos.xCoordinate];
            lNavigationHistory[lHistoryIndex++] = direction;
            retVal = true;
         }
   }
   else
   {
      if (pHandle->pData[pos.yCoordinate][pos.xCoordinate] == LG_SYMBOL_OBS_CONTOUR)
      {
         pHandle->nextHurdle = LG_SYMBOL_OBS_CONTOUR;
      }
      else
      {
         fprintf(stderr, "Corrupted Map\n");
         LG_WaitForSpace();
      }
   }

   return retVal;
}

/**
 * @brief       Manage an internal or contour obstacle
 *
 * @param[in]   pHandle - Pointer to the automaton handle
 *
 * @retval      status - true if the obstacle was handled
 *
 ******************************************************************************/
static bool LG_ManageObstacle(LG_Handle_t *pHandle )
{
   bool retVal = false;
   bool beerEnable = false;

   /* Handle obstacle 'X' in presence of beer mode */
   if ((pHandle->nextHurdle == LG_SYMBOL_OBS_INTERNAL) &&
       (pHandle->mode == LG_BEER_ON))
   {
      beerEnable = true;
      pHandle->pData[pHandle->position.yCoordinate][pHandle->position.xCoordinate] = LG_SYMBOL_SPACE;
      LG_Navigate(pHandle, pHandle->direction);
   }

   if (beerEnable == false)
   {
      /* Decide the next direction based on the priority and possibility */
      for (uint8_t i = 0; i < 4; i++)
      {
        if (LG_Navigate(pHandle, lObsPriority[i]) == true)
        {
           pHandle->direction = lObsPriority[i];
           retVal = true;
           break;
        }
      }
   }
   return retVal;
}

/**
 * @brief       Toggle the bear mode
 *
 * @param[in]   pHandle - Pointer to the automaton handle
 *
 * @retval      None
 *
 ******************************************************************************/
static void LG_ToggleBeer(LG_Handle_t *pHandle )
{
   if (pHandle->mode == LG_BEER_OFF)
   {
      pHandle->mode = LG_BEER_ON;
   }
   else
   {
      pHandle->mode = LG_BEER_OFF;
   }
}

/**
 * @brief       Invert the obstacle priorities
 *
 * @param[in]   pHandle - Pointer to the automaton handle
 *
 * @retval      None
 *
 ******************************************************************************/
static void LG_InvertPriorities(LG_Handle_t *pHandle )
{
   uint8_t i = 0;
   uint8_t j = 3;
   uint8_t temp;

   while (i < j)
   {
      temp = lObsPriority[i];
      lObsPriority[i] = lObsPriority[j];
      lObsPriority[j] = (LG_Direction_t)temp;
      i++;
      j--;
   }
}

/**
 * @brief       Manage Teleportation operation
 *
 * @param[in]   pHandle - Pointer to the automaton handle
 *
 * @retval      None
 *
 ******************************************************************************/
static void LG_ManageTeleporter(LG_Handle_t *pHandle )
{
   LG_Position_t position;

   if (LG_FindPosition(pHandle, LG_SYMBOL_TELEPORTER, &position, true))
   {
      pHandle->position.xCoordinate = position.xCoordinate;
      pHandle->position.yCoordinate = position.yCoordinate;
   }
}


/**
 * @brief       Display navigation result
 *
 * @param[in]   pHandle - Pointer to the automaton handle
 * @param[in]   status  - end result of navigation
 *
 * @retval      None
 *
 ******************************************************************************/
static void LG_DisplayResults(LG_Handle_t *pHandle,  bool status  )
{
   printf("\n\nEvaluated Map: \n");

   for (uint8_t i = 0; i < pHandle->numberOfLines; i++)
   {
      fprintf(stdout, "%s\n", pHandle->pData[i]);
   }

   printf("\n\nDirections: \n");

   if (status == true)
   {
      for (uint8_t i = 0 ; i <= lHistoryIndex ; i++ )
      {
         switch (lNavigationHistory[i])
         {
            case LG_DIR_EAST:
               fprintf(stdout, "EAST\n");
               break;

            case LG_DIR_WEST:
               fprintf(stdout, "WEST\n");
               break;

            case LG_DIR_NORTH:
                fprintf(stdout, "NORTH\n");
                break;

            case LG_DIR_SOUTH:
                fprintf(stdout, "SOUTH\n");
                break;

            case LG_DIR_NONE:
               /* Call an exception here */
               break;
         }

      }
   }
   else
   {
      /* It appears to be looping. To be sure about it, I need to analyze my path trajectory */
      fprintf(stdout, "It appears that automaton LG is looping.\n");
   }

   fflush(stdout);
   LG_WaitForSpace();
}

static void LG_WaitForSpace( void )
{
   fprintf(stdout, "\nPress space bar to exit.\n");
   fflush(stdout);

   char input = 0;

   while( input != ' ' )
   {
      input = getch();
   }

   exit(0);
}

/** @} */
/**************************************************** END OF FILE *************/
