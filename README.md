# LG Automaton Navigator

The utility predicts the navigation path for the LG Automaton based on the user provided map. 

## Build

It is possible to build the application in 'testing mode'. It lets the user to provide a map at compilation time.

```bash
/******************************* LOCAL DEFINES ******************/
/* De-comment it to test the program without user input */
//#define LG_TESTING
```
I used eclipse gcc to build the application

## Usage
The program asks for number of lines and columns to initialize the map.

```bash
Enter number of lines: 10
Enter number of columns: 10
```
The program next asks for the map to generate navigation directions.
In the example, I copied/pasted the qwic test example data into command prompt.

It generates the following output

```bash
Directions:
SOUTH
SOUTH
EAST
EAST
EAST
EAST
EAST
EAST
NORTH
NORTH
NORTH
NORTH
NORTH
NORTH
WEST
WEST
WEST
WEST
SOUTH
SOUTH

Press space bar to exit.

```
