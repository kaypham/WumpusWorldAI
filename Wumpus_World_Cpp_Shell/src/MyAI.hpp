// ======================================================================
// FILE:        MyAI.hpp
//
// AUTHOR:      Abdullah Younis
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================

#ifndef MYAI_LOCK
#define MYAI_LOCK

#include <tuple>
#include <vector>
#include "Agent.hpp"
#include <stdexcept>

using namespace std;

// Describes the findings of each node
struct Cell
{
	bool breeze;
	bool stench;
	bool visited;
	bool safe; // cells can be unvisited, but determined safe
	bool wall;
	float pitPresent;
	float wumpusPresent;
	int x, y;
};

class Map
{
private:

public:
	// First vector is the x axis, second vector is the y axis. Bottom left most Cell is (0,0), Y increases moving up,
	// and X increases moving right.
	vector< vector<Cell> > map;

	// constructor
	Map();

	// returns a cell pointer given x, y coordinate. Checkes if the cell value is valid, throws error if not
	Cell* getCell(int x, int y);

	// returns cells of up to 4 adjacent locations on the Map to coordinates x and y
	void getAdjacentCells(int x, int y, vector<Cell*>& cells);

	// given a start and end location, returns a vector of Cell pointers representing a safe path from start to end.
	// if path is not possible, return empty vector
	vector<Cell*> getPath(int start[2], int end[2], vector<Cell*> &solution);

	// prints the map to the console for debugging purposes
	void printMap(int myX, int myY);

	// After every move, call methods that can determine potential hazards in adjacent 
	// judging from the map we have so far.
	// Maybe not early in the game when the agent only makes a couple moves.
	void Locate_Pit_And_Wumpus();

};

// this can be used to help identify the square with the Wumpus. It will keep track of the suspected cells
// for Wumpus location and ensure the sum of probability is 1 or 100% between all of them. When a cell is
// removed and no longer a culprit cell, the probabilities will be redistributed among the remaining cells.
//
// This will only work for the Wumpus I think, because there is only 1 in the whole map. There can be more that 1 pit.
// Pits will need to be handled differently.
class ProbHandle
{
public:
	// constructor
	ProbHandle();

	// used internally to re-calculate probabilities when a change in suspects occurs
	void calcProb(bool wumpusOrPit);

	// culprit cells in hash table
	std::vector<Cell*> suspects;

	// returns the number of existing suspect cells
	int suspectNumber();

	// mark a cell as safe if there are no dangers present
	void markSafe(Cell* cell);

	// adds new cells to suspects and modifies the distributed probability among the cells
	void addSuspects(const std::vector<Cell*>& cells, bool wumpusOrPit);	

	// specifically refines where the Wumpus is given input
	void wumpusSuspects(const std::vector<Cell*>& cells);

	// if a cell in this list is a pointer to a cell in the suspects, remove it
	void removeSuspects(const std::vector<Cell*>& cells, bool wumpusOrPit);
};

class MyAI : public Agent
{
public:
	MyAI ( void );
	
	Action getAction
	(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	);
	
	// ======================================================================
	// YOUR CODE BEGINS
	// ======================================================================
	enum Direction
	{
		UP,
	 	DOWN,
		LEFT,
		RIGHT
	};

	Action BackTrack();
	Action turnAndMove(tuple<int,int> space);
	Action TurnTowardsPrevLocation();
	int evaluateMove(int* loc);
	// takes as input a location for the AI and returns a value. The larger the number, the better the position
	void RecordPath(int loc[2]);

	// functions for tracking unexplored options. If there are none, then quit
	void addUnexplored(vector<Cell*> spaces);
	void filterUnexplored();

	// function to determine if current board location is a dead end.
	// Checks if any adjacent space is safe and unexplored, if not then the current Cell is a dead end cell
	bool deadEndCell(int loc[2]);

	bool firstMove = true, isBackTracking = false;
	Direction dir;
	int loc[2];
	int forwardMovesCounter;
	Map board;
	ProbHandle wumpusProb;
	ProbHandle pitProb;
	std::vector<Cell*> path;
	std::vector<Cell*> unexplored;

	std::tuple<int, int> bestMove(vector<Cell*> moves);
	// given a vector of cells representing positions for the AI, return the location of the best move
	vector<Cell*> adj_cells;
	std::vector<Cell*> shortest_path;
	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================
};

#endif

/// <summary>
//	To record the path agent has taken
//  To describe the environment of each cell 
/// </summary>

// TODO: How to get size of map? Or does it matter?
// As we also know the the size of the board is 10 x 10 max, have a vector of 10 vectors of 10 cells. That way as the agent moves around the board, we can simply do Map[x][y] to update Map

// Keep track of the path agent has taken
