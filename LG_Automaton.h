/**
* @file           LG_Automaon.h
*
* @brief          Navigation engine for LG Automaton - Public interface
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


#ifndef __LG_AUTOMATON_H
#define __LG_AUTOMATON_H

/******************************* SYSTEM INCLUDES ******************************/

/******************************* PROJECT INCLUDES *****************************/

/******************************* DRIVER INCLUDES ******************************/


/****************************** LIBRARY INCLUDES ******************************/


/**************************** LOCAL MACROS DEFINTION **************************/


/************************* TYPEDEFS, STRUCTURES and UNIONS ********************/

typedef enum
{
   LG_DIR_NONE,
   LG_DIR_SOUTH,
   LG_DIR_EAST,
   LG_DIR_NORTH,
   LG_DIR_WEST
} LG_Direction_t;

typedef struct
{
   uint8_t xCoordinate;
   uint8_t yCoordinate;
} LG_Position_t;


typedef enum
{
   LG_SYMBOL_START        = '@',
   LG_SYMBOL_STOP         = '$',
   LG_SYMBOL_OBS_CONTOUR  = '#',
   LG_SYMBOL_OBS_INTERNAL = 'X',
   LG_SYMBOL_PM_SOUTH     = 'S',
   LG_SYMBOL_PM_NORTH     = 'N',
   LG_SYMBOL_PM_EAST      = 'E',
   LG_SYMBOL_PM_WEST      = 'W',
   LG_SYMBOL_INVERTER     = 'I',
   LG_SYMBOL_BEER         = 'B',
   LG_SYMBOL_TELEPORTER   = 'T',
   LG_SYMBOL_SPACE        = ' '
} LG_Symbols_t;

typedef enum
{
   LG_BEER_OFF = 0u,
   LG_BEER_ON
} LG_BeerMode_t;

typedef struct
{
   char **pData;
   uint8_t numberOfLines;
   uint8_t numberofColumns;
   LG_Position_t position;
   LG_Direction_t direction;
   LG_BeerMode_t mode;
   LG_Symbols_t nextHurdle;
} LG_Handle_t;

/************************* EXTERNAL FUNCTION PROTOTYPES ***********************/


#endif /* __LG_AUTOMATON_H */
/** @} */
/****************** END OF FILE ***************************************************************************************/
