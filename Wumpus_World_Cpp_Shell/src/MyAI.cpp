// ======================================================================
// FILE:        MyAI.cpp
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

#include <iostream>
#include <typeinfo>
#include "MyAI.hpp"


MyAI::MyAI() : Agent()
{
	// ======================================================================
	// YOUR CODE BEGINS
	// ======================================================================
	loc[0] = 0;
	loc[1] = 0;
	dir = RIGHT;
	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================
}
	
// At every cell, agent only feels stench, breeze, and glitter.
// At every cell, this function will get called
Agent::Action MyAI::getAction
(
	bool stench,
	bool breeze,
	bool glitter,
	bool bump,
	bool scream
)
{
	// ======================================================================
	// YOUR CODE BEGINS
	// ======================================================================
	
	// mark current location as visited
	board.getCell(loc[0], loc[1])->visited = true;
	// find all adjacent cells to AI
	adj_cells.erase(adj_cells.begin(), adj_cells.end());
	board.getAdjacentCells(loc[0], loc[1], adj_cells);
	board.printMap(loc[0], loc[1]);

	cout << "Cur cell is " << loc[0] << ", " << loc[1] << endl;	

	if (glitter)
	{	
		cout << "!!!!!!!!!!!      GOLD FOUND      !!!!!!!!!!!!!" << endl;
		isBackTracking = true;
		return GRAB;	// grab the gold
	}
	if (isBackTracking)
	{
		cout << "backtracking" << endl;
		return BackTrack();
	}

	RecordPath(loc);

	if (stench)
	{
		std::cout << "There is a stench" << std::endl;
		// get adjacent spaces and assign probabilities to them
		wumpusProb.wumpusSuspects(adj_cells);
		
	}
	else	// if no stench is detected...
	{
		// remove suspect spaces if possible
		wumpusProb.removeSuspects(adj_cells, true); // true for wumpus
	}
	if (breeze)
	{
		std::cout << "There is a breeze " << std::endl;
		pitProb.addSuspects(adj_cells, false); // false for pits
	}
	else // if no breeze...
	{
		// remove pit suspect spaces
		pitProb.removeSuspects(adj_cells, false);
	}
	if(!stench && !breeze) // mark adjacent spaces totally safe
	{
		for(int i=0; i<adj_cells.size(); i++)
		{
			adj_cells[i]->safe = true;
		}
	}
	if (bump)
	{
		std::cout << "There is a wall" << std::endl;
		// mark cells as walls if AI bumps into one
		switch(dir)
		{
			case UP:
				if(loc[1] != 9){
					cout << "marking cell as wall" << endl;
					for(int x=0; x<9; x++){
						board.getCell(x, loc[1]+1)->wall = true;
					}
				}
				break;
			case RIGHT:
				if(loc[0] != 9){
					cout << "marking cell as wall" << endl;
					for(int y=0; y<9; y++){
						board.getCell(loc[0]+1, y)->wall = true;
					}
				}
				break;
		}
	}

	// check for unexplored spaces, and filter out unexplored spaces of past
	filterUnexplored();
	addUnexplored(adj_cells);
	std::cout << unexplored.size() << std::endl;
	if(unexplored.size() == 0) // set backtracking to true if there are no explorable spaces
	{
		isBackTracking = true;
		path.pop_back();
		return BackTrack();
	}
	// decide what to do...
	tuple<int,int> space = bestMove(adj_cells);
	// turn and move foward to desired space...
	Agent::Action myMove = turnAndMove(space);
	cout << "move: ";
	switch(myMove)
	{
		case TURN_LEFT:
			cout << "left" << endl;
			break;
		case TURN_RIGHT:
			cout << "right" << endl;
			break;
		case FORWARD:
			cout << "forward" << endl;
			break;
	}
	if (isBackTracking && myMove == FORWARD)
	{
		path.pop_back();
	}	
	return myMove;
	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================
}

// ======================================================================
// YOUR CODE BEGINS
// ======================================================================

int MyAI::evaluateMove(int* loc)
{
	Cell* pos = board.getCell(loc[0], loc[1]);
	cout << "cell (" << pos->x << "," << pos->y << ") ";	

	if(pos->wumpusPresent > 0 || pos->pitPresent > 0)
	{
		// don't take the chance for now
		cout << "wumpus:" << pos->wumpusPresent << " pit:" << pos->pitPresent << endl;
		return 0;	
	}
	if(pos->wall)
	{
		// don't try to go where there is a wall
		cout << " wall" << endl;
		return 0;
	}
	if((int)(pos->wumpusPresent) == 0 && (int)(pos->pitPresent) == 0)
	{
		// value bettern than 0
		if(pos->visited)
		{
			cout << " visited" << endl;
			return 9;
		}
		else
		{
			// we want to prioritize unvisited spaces over visited
			cout << " unexplored" << endl;
			return 10;
		}
	}
}

tuple<int,int> MyAI::bestMove(std::vector<Cell*> moves)
{
	if(moves.size() != 0)
	{
		Cell* best = moves.at(0);	// stores the best cell
		int best_val = 0;	// stores the best move heuristic value
		int temp = 0;		// temp value for heuristic
		int temp_loc[2] = {0,0};	// temp value for move location
		int best_loc[2];	// stores the best move location found
		//std::vector<Cell*> next_moves; // the recursive moves that (*i) will be able to move to
		for(std::vector<Cell*>::iterator i=moves.begin(); i != moves.end(); i++)
		{
			temp_loc[0] = (*i)->x; temp_loc[1] = (*i)->y;
			temp = evaluateMove(temp_loc);

			if(temp >= best_val)
			{
				best_val = temp;
				best = *i;
				best_loc[0] = temp_loc[0]; best_loc[1] = temp_loc[1];
			}
		}
		return make_tuple(best_loc[0], best_loc[1]);
	}
	// just return current AI loc if moves is empty
	// can change this later if we want to...
	return make_tuple(loc[0], loc[1]);
}


Agent::Action MyAI::turnAndMove(tuple<int,int> space)
{
	//determine direction
	// DIRECTION UP
	if( get<0>(space) == loc[0] && get<1>(space) == loc[1]+1)
	{
		switch(dir)
		{
			case UP:
				loc[1] = loc[1]+1;
				return FORWARD;
			case DOWN:
				dir = RIGHT;
				return TURN_LEFT;
			case LEFT:
				dir = UP;
				return TURN_RIGHT;
			case RIGHT:
				dir = UP;
				return TURN_LEFT;
		}
	}
	// DIRECTION LEFT
	if( get<0>(space) == loc[0]-1 && get<1>(space) == loc[1])
	{
		switch(dir)
		{
			case UP:
				dir = LEFT;
				return TURN_LEFT;
			case DOWN:
				dir = LEFT;
				return TURN_RIGHT;
			case LEFT:
				loc[0] = loc[0]-1;
				return FORWARD;
			case RIGHT:
				dir = UP;
				return TURN_LEFT;
		}	
	}
	// DIRECTION RIGHT
	if( get<0>(space) == loc[0]+1 && get<1>(space) == loc[1])
	{
		switch(dir)
		{
			case UP:
				dir = RIGHT;
				return TURN_RIGHT;
			case DOWN:
				dir = RIGHT;
				return TURN_LEFT;
			case LEFT:
				dir = UP;
				return TURN_RIGHT;
			case RIGHT:
				loc[0] = loc[0]+1;
				return FORWARD;
		}	
	}
	// DIRECTION DOWN
	if( get<0>(space) == loc[0] && get<1>(space) == loc[1]-1)
	{
		switch(dir)
		{
			case UP:
				dir = RIGHT;
				return TURN_RIGHT;
			case DOWN:
				loc[1] = loc[1]-1;
				return FORWARD;
			case LEFT:
				dir = DOWN;
				return TURN_LEFT;
			case RIGHT:
				dir = DOWN;
				return TURN_RIGHT;
		}	
	}

	cout << "couldn't resolve desired direction :(" << endl;
	return TURN_LEFT;
}

// This will call back_track function where it will use the path it comes from
// to climb out of the maze
// Note: agent can only climb out at cell (0, 0)
Agent::Action MyAI::BackTrack()
{
	if (loc[0] == 0 && loc[1] == 0)
	{
		cout << "climbing out" << endl;
		return CLIMB;
	}
	else
	{
		tuple<int, int> prev_loc = make_tuple(path.back()->x, path.back()->y);
		Agent::Action myMove = turnAndMove(prev_loc);
		if (myMove == FORWARD)
		{
			path.pop_back();
		}
		return myMove;
	}
}

void MyAI::RecordPath(int loc[2])
{
	if (firstMove)
	{
		path.push_back(board.getCell(loc[0], loc[1]));
		firstMove = false;
	}
	else
	{
		int path_size = path.size();
		Cell* prev_loc = path.at(path_size - 1);
		if (!(prev_loc->x == loc[0] && prev_loc->y == loc[1]))
		{
			path.push_back(board.getCell(loc[0], loc[1]));
		}
	}
}

void MyAI::addUnexplored(vector<Cell*> spaces)
{
	for(int i=0; i<spaces.size(); i++)
	{
		if(!spaces[i]->visited && (int)(spaces[i]->pitPresent)==0)
		{
			unexplored.push_back(spaces[i]);
		}
	}
}

void MyAI::filterUnexplored()
{
	for(int i=0; i<unexplored.size(); i++)
	{
		if(unexplored[i]->visited ||
			(int)(unexplored[i]->pitPresent)>0)
		{
			unexplored.erase(unexplored.begin()+i);
		}
	}
}

Map::Map()
{
	for(int x=0; x<10; x++)
	{
		map.push_back(vector<Cell>());		
		for(int y=0; y<10; y++)
		{
			Cell c = {0};
			c.x = x;
			c.y = y;
			map[x].push_back(c);
		}
	}
}

Cell* Map::getCell(int x, int y)
{
	if(x >= 0 && x <= 9 && y >= 0 && y <= 9)
	{
		return &map.at(x).at(y);	
	}
	else
	{
		throw out_of_range("Map location out of bounds");
	}
}

void Map::getAdjacentCells(int x, int y, vector<Cell*>& cells)
{
	// make sure cell location is within bounds
	if(x >= 0 && x <= 9 && y >= 0 && y <= 9)
	{
		// get Left
		if(x > 0) // can't get left if 0
		{
			cells.push_back( &map.at(x-1).at(y) );
		}
		// get Down
		if(y > 0) // can't get down if 0
		{
			cells.push_back( &map.at(x).at(y-1) );
		}

		// (note) for right and up, boundaries of actualworld may be smaller, but in that case, adjacent walls will be marked with a wall indication.
		// get Right
		if(x < 9) // can't get right if 9
		{
			cells.push_back( &map.at(x+1).at(y) );
		}
		// get Left
		if(y < 9) // can't get left if 9
		{
			cells.push_back( &map.at(x).at(y+1) );
		}
	}
	else
	{
		throw out_of_range("Map location out of bounds");
	}
}

// code just for visualizing what the AI is doing
void Map::printMap(int myX, int myY)
{
	Cell* c;
	for(int y=9; y>=0; y--)
	{
		for(int x=0; x<10; x++)
		{
			c = &map.at(x).at(y);
			if(x == myX && y == myY){
				std::cout << "* ";
			}
			else if(c->wall){
				std::cout << "+ ";
			}
			else if(c->visited){
				std::cout << "V ";
			}
			else if(c->safe){
				std::cout << "S ";
			}
			else if(c->pitPresent > 0){
				std::cout << "P ";
			}
			else if(c->wumpusPresent > 0){
				std::cout << "W ";
			}
			else{
				std::cout << "  ";
			}
		}
		std::cout << std::endl;
	}
}

ProbHandle::ProbHandle()
{
}


void ProbHandle::calcProb(bool wumpusOrPit)
{
	// wumpusOrPit = true, then wumpus, else pit
	// I wan't to make this code better later, but in a hurry now
	if(wumpusOrPit)
	{
		double prob = 1; // default prob if there are no suspects
		if(suspects.size() != 0)
		{
			prob = 100 / suspects.size();
		}
		// reset wumpus probabilities to new value
		for(std::vector<Cell*>::iterator i=suspects.begin(); i<suspects.end(); i++)
		{
			(*i)->wumpusPresent = prob;
		}
	}
	else // pit
	{
		for(std::vector<Cell*>::iterator i=suspects.begin(); i<suspects.end(); i++)
		{
			(*i)->pitPresent = 100;
		}
	}
}


int ProbHandle::suspectNumber()
{
	return suspects.size();
}


void ProbHandle::addSuspects(const std::vector<Cell*>& cells, bool wumpusOrPit)
{
	bool alreadyThere;
	for(int i=0; i<cells.size(); i++)
	{
		if(!cells[i]->visited && !cells[i]->safe)
		{
			alreadyThere = false;
			for(int j=0; j<suspects.size(); j++)
			{
				if(suspects[j] == cells[i])
				{
					alreadyThere = true;
					break;
				}
			}
			if(!alreadyThere)
			{
				std::cout << "adding suspect" << std::endl;
				suspects.push_back(cells[i]);
			}
		}
	}
	if(!wumpusOrPit)
	{
		calcProb(false);
	}
}


void ProbHandle::wumpusSuspects(const std::vector<Cell*>& cells)
{
	// There is only 1 Wumpus, so addSuspects will actually only add new spaces the first time
	// a stench is smelled. The Wumpus must be in one of the adjacent spaces. Every following time,
	// the current suspects will be filtered by the input cells with logical AND operation.
	// don't add cells that have been visited before. They cannot have Wumpus.
	if(suspects.size() == 0) // add suspects
	{
		std::cout << "adding wumpus suspects for first time" << std::endl;
		addSuspects(cells, true);
	}
	else // logical AND
	{
		std::vector<Cell*> new_suspects;
		for(int i=0; i<cells.size(); i++)
		{
			if(!cells[i]->visited && !cells[i]->safe)
			{
				for(int j=0; j<suspects.size(); j++)
				{
					suspects[j]->wumpusPresent = 0;
					if(cells[i] == suspects[j])
					{
						new_suspects.push_back(suspects[j]);
					}
				}
			}
		}
		suspects = new_suspects;
	}
	calcProb(true);
}


void ProbHandle::removeSuspects(const std::vector<Cell*>& cells, bool wumpusOrPit)
{
	// double for loops are not too good. Suspects should probably be a table.
	for(int i=0; i<cells.size(); i++)
	{
		for(int j=0; j<suspects.size(); j++)
		{
			if(suspects[j]->x == cells[i]->x && suspects[j]->y == cells[i]->y)
			{
				if(!wumpusOrPit){
					suspects[j]->pitPresent = 0;
				}
				suspects.erase(suspects.begin()+j);
				break;
			}
		}
	}
	
	calcProb(wumpusOrPit);
}
// ======================================================================
// YOUR CODE ENDS
// ======================================================================
