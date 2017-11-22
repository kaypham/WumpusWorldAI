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
	moveCount++;	
	// mark current location as visited, and safe
	board.getCell(loc[0], loc[1])->visited = true;
	board.getCell(loc[0], loc[1])->safe = true;
	// find all adjacent cells to AI
	adj_cells.erase(adj_cells.begin(), adj_cells.end());
	board.getAdjacentCells(loc[0], loc[1], adj_cells);

	cout << "MOVE: " << moveCount << "--------------------------------" << endl;
	board.printMap(loc[0], loc[1]);
	cout << "Cur cell is " << loc[0] << ", " << loc[1] << endl;
	cout << "Direction: " << dir << endl;	

	if (glitter)
	{	
		cout << "!!!!!!!!!!!      GOLD FOUND      !!!!!!!!!!!!!" << endl;
		isBackTracking = true;
		int destination[2]{0, 0};
		shortest_path.erase(shortest_path.begin(), shortest_path.end()); // have to clear the path
		board.getPath(loc, destination, shortest_path);	
		shortest_path.erase(shortest_path.begin());
		cout << "path: ";
		for(int i=0; i<shortest_path.size(); i++)
		{
			cout << "(" << shortest_path[i]->x << ", " << shortest_path[i]->y << ")" << endl;
		}
		cout << "shortest_path size" << shortest_path.size() << endl;
		return GRAB;	// grab the gold
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
		//pitProb.addSuspects(adj_cells, false); // false for pits
		for(int i=0; i<adj_cells.size(); i++)
		{
			if(!adj_cells[i]->wall && !adj_cells[i]->safe && !adj_cells[i]->visited)
			{
				adj_cells[i]->safe = false;
				adj_cells[i]->pitPresent = 100;
			}
		}
	}
	else // if no breeze...
	{
		// remove pit suspect spaces
		//pitProb.removeSuspects(adj_cells, false);
		for(int i=0; i<adj_cells.size(); i++)
		{
			if(!adj_cells[i]->wall)
			{
				adj_cells[i]->pitPresent = 0;
			}
		}
	}
	if(!stench && !breeze) // mark adjacent spaces totally safe
	{
		for(int i=0; i<adj_cells.size(); i++)
		{
			cout << "marking cell " << adj_cells[i]->x << ", " << adj_cells[i]->y << " safe" << endl;
			if(!adj_cells[i]->wall)
			{
				adj_cells[i]->safe = true;
			}
		}
	}
	if (isBackTracking)
	{
		cout << "backtracking after finding gold" << endl;
		return BackTrack();
	}

	// check for unexplored spaces, and filter out unexplored spaces of past
	//filterUnexplored();
	//addUnexplored(adj_cells);
	//std::cout << unexplored.size() << std::endl;
	//if(unexplored.size() == 0) // set backtracking to true if there are no explorable spaces
	//{
	//	if (shortest_path.size() == 0)
	//	{
	//		int destination[2]{0, 0};
	//		board.getPath(loc, destination, shortest_path);	
	//	}
	//	isBackTracking = true;
	//	shortest_path.erase(shortest_path.begin());
	//	cout << "backtracking as there are no explorable spaces" << endl;
	//	return BackTrack();
	//}

	// if there are no safe, unexplored spaces next to agent, find a path to closest unexplored space and go there
	if(board.deadEndCell(loc))
	{
		cout << "dead end cell, (starting path)" << endl;
		Cell* dest = board.findSafeUnvisited(loc);
		int destination[2];
		if(dest == nullptr)	// there are no more explorable spaces, backtrack out to (0,0)
		{
			destination[0] = 0; 
			destination[1] = 0;
		}
		else
		{
			destination[0] = dest->x;
			destination[1] = dest->y;
		}
		shortest_path.erase(shortest_path.begin(), shortest_path.end());
		board.getPath(loc, destination, shortest_path);
		isBackTracking = true;
		shortest_path.erase(shortest_path.begin());
		cout << "moving to cell (" << destination[0] << ", " << destination[1] << ")" << endl;
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
	cout << "turnAndMove to " << get<0>(space) << ", " << get<1>(space) << endl;
	//determine direction
	// DIRECTION UP
	if( get<0>(space) == loc[0] && get<1>(space) == loc[1]+1)
	{
		switch(dir)
		{
			case UP:
				cout << "moving up in turnAndMove" << endl;
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
				cout << "moving left in turnAndMove" << endl;
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
				cout << "moving right in turnAndMove" << endl;
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
				cout << "moving down in turnAndMove" << endl;
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
	cout << "running backtrack" << endl;
	if (shortest_path.size() == 0 && loc[0]==0 && loc[1]==0) // when the path is over
	{
		cout << "climbing out" << endl;
		return CLIMB;
	}
	else if(shortest_path.size() == 0)
	{
		isBackTracking = false;
	}
	else
	{
		//tuple<int, int> prev_loc = make_tuple(path.back()->x, path.back()->y);
		tuple<int, int> prev_loc = make_tuple(shortest_path.front()->x, shortest_path.front()->y);
		Agent::Action myMove = turnAndMove(prev_loc);
		if (myMove == FORWARD)
		{
			shortest_path.erase(shortest_path.begin());
		}
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
		return myMove;
	}
}

//void MyAI::addUnexplored(vector<Cell*> spaces)
//{
//	bool alreadyThere;
//	for(int i=0; i<spaces.size(); i++)
//	{
//		alreadyThere = false;
//		if(!spaces[i]->visited && spaces[i]->safe)
//		{
//			for(int j=0; j<unexplored.size(); j++)
//			{
//				if(spaces[i] == unexplored[j]) alreadyThere = true;
//			}
//			if(!alreadyThere)
//			{
//				std::cout << "adding unexplored" << spaces[i]->x << " " << spaces[i]->y << std::endl;
//				unexplored.push_back(spaces[i]);
//			}
//		}
//	}
//}

//void MyAI::filterUnexplored()
//{
//	for(int i=0; i<unexplored.size(); i++)
//	{
//		if(unexplored[i]->visited || (int)(unexplored[i]->pitPresent)!=0
//					|| (int)(unexplored[i]->wumpusPresent)!=0)
//		{
//			std::cout << "removing unexplored" << unexplored[i]->x << " " << unexplored[i]->y << std::endl;
//			unexplored.erase(unexplored.begin()+i);
//		}
//	}
//}

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

// depth first seach solution to retrieving a path from cell A to cell B
void Map::getPath(int start[2], int end[2], vector<Cell*> &solution)
{
	cout << "getPath" << " start: " << start[0] << " " << start[1];
	cout << " end: " << end[0] << " " << end[1] << endl;
	// initialize variables
	Cell* currCell = getCell(start[0], start[1]);
	vector<Cell*> adj_cells;

	// push the currCell into the solution which only exists on this stack frame
	cout << "pushing to solution" << endl;
	solution.push_back(currCell);

	if(start[0] != end[0] || start[1] != end[1])
	{
		cout << "not goal state" << endl;
		getAdjacentCells(start[0], start[1], adj_cells);
		cout << "got adjacent cells" << endl;
		// for each valid adjacent cell, make a recursive call to check for solution
		int loc[2];
		bool alreadySearched;
		for(int i=0; i<adj_cells.size(); i++)
		{
			cout << "adjacent " << adj_cells[i]->x << " " << adj_cells[i]->y << endl;
			alreadySearched = false;
			for(int j=0; j<solution.size(); j++)
			{
				if(adj_cells[i] == solution[j]) alreadySearched = true;
			}
			cout << "alreadySearched " << alreadySearched << endl;
			// make a recursive call to getPath to continue searching for a path
			if(adj_cells[i]->safe && !alreadySearched)
			{
				cout << "making recursive call..." << endl;
				loc[0] = adj_cells[i]->x; loc[1] = adj_cells[i]->y;
				getPath(loc, end, solution);
				
				if(solution.back()->x == end[0] && solution.back()->y == end[1] )
				{
					cout << "returning solution 2" << endl;
					return;
				}
			}
		}	
	}
	else // the currCell is the goal Cell
	{
		cout << "returning solution 1" << endl;
		return;
	}

	cout << "popping from solution" << endl;
	solution.pop_back();
}


// breadth first search for finding unvisited safe cells
Cell* Map::findSafeUnvisited(int start[2])
{
	cout << "find safe unvisited" << endl;
	vector<Cell*> fifo;
	Cell* currCell;
	fifo.push_back(getCell(start[0], start[1]));
	vector<Cell*> adj_cells;
	vector<Cell*> searched;

	while(fifo.size() > 0)	// if there are still cells in the fifo, keep going...
	{
		cout << endl;
		cout << "cell: " << fifo[0]->x << " " << fifo[0]->y << endl;
		cout << "fifo size: " << fifo.size() << endl;
		currCell = fifo[0];
		searched.push_back(currCell);
		adj_cells.erase(adj_cells.begin(), adj_cells.end());
		getAdjacentCells(currCell->x, currCell->y, adj_cells);

		bool alreadySearched;
		for(int i=0; i<adj_cells.size(); i++)
		{
			cout << "adjacent " << adj_cells[i]->x << " " << adj_cells[i]->y;
			cout << " safe" << adj_cells[i]->safe << " visited" << adj_cells[i]->visited << endl;
			alreadySearched = false;
			for(int j=0; j<searched.size(); j++)
			{
				if(adj_cells[i] == searched[j]) alreadySearched = true;
			}
			// don't consider unsafe cells, or already visited cells
			if(adj_cells[i]->safe && !alreadySearched)
			{
				// if not a goal state...
				if( !(adj_cells[i]->safe && !adj_cells[i]->visited) )
				{
					cout << "push to fifo " << adj_cells[i]->x << " " << adj_cells[i]->y << endl;
					fifo.push_back(adj_cells[i]);
				}
				else // goal state achieved
				{
					cout << "goal state reached" << endl;
					return adj_cells[i];
				}
		   	}
		}
		cout << "erasing " << fifo[0]->x << " " << fifo[0]->y << endl;
		fifo.erase(fifo.begin());
		cout << "size after erased " << fifo.size() << endl;
	}
	// no solution
	return nullptr;
}


bool Map::deadEndCell(int loc[2])
{
	std::vector<Cell*> adj_cells;
	getAdjacentCells(loc[0], loc[1], adj_cells);
	bool deadEnd = true;
	
	for(int i=0; i<adj_cells.size(); i++)
	{
		if(adj_cells[i]->safe && !adj_cells[i]->visited)
		{
			// if we have a safe, unvisited space adjacent to the AI, the current cell is not a dead end
			deadEnd = false;
		}
	}
	return deadEnd;
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
			(*i)->safe = false;
			(*i)->wumpusPresent = prob;
		}
	}
	else // pit
	{
		for(std::vector<Cell*>::iterator i=suspects.begin(); i<suspects.end(); i++)
		{
			(*i)->safe = false;
			(*i)->pitPresent = 100;
		}
	}
}


int ProbHandle::suspectNumber()
{
	return suspects.size();
}

void ProbHandle::markSafe(Cell* cell)
{
	if((int)(cell->wumpusPresent)==0 && (int)(cell->pitPresent)==0 && !cell->wall)
	{
		cout << "marking cell safe" << endl;
		cell->safe = true;
	}
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
					markSafe(suspects[j]);
				}
				suspects.erase(suspects.begin()+j);
				break;
			}
		}
		// mark space as safe
	}
	
	calcProb(wumpusOrPit);
}
// ======================================================================
// YOUR CODE ENDS
// ======================================================================
