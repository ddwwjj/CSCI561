// homework1.cpp
//update date: 9/15/2017
//score: 110/110 top 10 students of the most fast speed
//version 4
/*
implement segmentation array for contradiction search
clear nursery and seg_array before the next iteration
*/
#include <iostream>
#include <fstream>  
#include <string> 
#include <vector>
#include <cmath>
#include<stdlib.h>
#include<time.h>
using namespace std;

vector<vector<int> > nursery; // So far to current node, the current state of nursery
int width; // the width of nursery
int lizards; //num of lizards
int num_tree = 0;
bool flag = false;//for SA, in case of dead loop

//For SA, counting contradiction faster
vector<vector<int> > colum_seg;
vector<vector<int> > row_seg;
vector<vector<int> > left_up;
vector<vector<int> > right_up;
vector<int>seg_array;

//The exploring order of the tree
typedef struct Queue {
	struct Queue *prev;
	struct Queue *next;
}Queue;

#define QUEUE_INIT(q) \
    (q)->next = (q); \
    (q)->prev = (q)

#define QUEUE_REMOVE_FRONT(h) \
    (h)->next = (h)->next->next; \
    (h)->next->prev = (h)

//for DFS,stack
#define QUEUE_INSERT_FRONT(h, q) \
    (h)->next->prev = (q); \
    (q)->prev       = (h); \
    (q)->next       = (h)->next; \
    (h)->next       = (q)

//for BFS,queue
#define QUEUE_INSERT_BACK(h,q) \
    (h)->prev->next = (q); \
    (q)-> next = (h); \
    (q)->prev = (h)->prev; \
    (h)->prev = (q)

#define QUEUE_EMPTY(h) \
    (h)->next == (h)

//valid position for a lizard(level)
typedef struct Grid {
	int x; //row
	int y; //colum
}Grid;

typedef struct BFS_node {
	Queue queue;
	vector<Grid> solution; //All his parents and his position
	int s_p; //When generate childs, the first unexplored grid.Treating the nursery as a one-dimentail array,search by row
	int level;//the level of the node;
}BFS_node;

//same as BFS_node
typedef struct DFS_node {
	Grid grid;
	int s_p;
	int level;

}DFS_node;

//First copy parent's solution and add its current available position. FIFO
void BFSNode_Create(BFS_node* Head, BFS_node* parent, BFS_node* Node, Grid grid, int s_p, int level) {
	if (!parent->solution.empty()) {
		Node->solution.assign(parent->solution.begin(), parent->solution.end());
	}
	Node->solution.push_back(grid);
	Node->s_p = s_p;
	Node->level = level;
	QUEUE_INSERT_BACK((Queue*)Head, (Queue*)Node);
}

//First copy parent's solution and add its current available position. LIFO(Stack)
void DFSNode_Create(DFS_node* Node, Grid grid, int s_p, int level) {
	Node->grid = grid;
	Node->s_p = s_p;
	Node->level = level;
}

// for BFS
// every time with a new node poped, have to update the nursery state
void update(vector<Grid>&solution) {
	int m, n;

	for (int i = 1; i < solution.size(); i++) {
		m = solution[i].x;
		n = solution[i].y;
		nursery[m][n] = 1;
	}
}
// for SA, count the contradiction for a position change
void count_value(int*value) {
	int temp;
	for (temp = 1; temp < seg_array.size(); temp++) {
		if (seg_array[temp] > 0) {
			*value = *value + seg_array[temp] - 1;
		}
	}
}

//After adding a new lizard, need to update the nursery state and segmentation state
void update_segmentation(vector<Grid>&solution) {
	int m, n;
	int temp;
	for (int i = 1; i < solution.size(); i++) {
		m = solution[i].x;
		n = solution[i].y;
		temp = colum_seg[m][n];
		seg_array[temp]++;
		temp = row_seg[m][n];
		seg_array[temp]++;
		temp = left_up[m][n];
		seg_array[temp]++;
		temp = right_up[m][n];
		seg_array[temp]++;
		//update nursery;
		nursery[m][n] = 1;
	}
}

//After expending one node, need to recover the nursery state and segmentation state
void rec_segmentation(vector<Grid>&solution) {
	int m, n;
	int temp;
	for (int i = 1; i < solution.size(); i++) {
		m = solution[i].x;
		n = solution[i].y;
		temp = colum_seg[m][n];
		seg_array[temp]--;
		temp = row_seg[m][n];
		seg_array[temp]--;
		temp = left_up[m][n];
		seg_array[temp]--;
		temp = right_up[m][n];
		seg_array[temp]--;
		nursery[m][n] = 0;
	}
}

bool no_contradict(int&i, int&j) {
	int temp;
	int contradiction = 0;
	temp = colum_seg[i][j];
	contradiction += seg_array[temp];
	temp = row_seg[i][j];
	contradiction += seg_array[temp];
	temp = left_up[i][j];
	contradiction += seg_array[temp];
	temp = right_up[i][j];
	contradiction += seg_array[temp];

	if (contradiction == 0)
		return true;
	else
		return false;
}

void initiate_segmentation() {
	int i, j, k;
	int seg = 1;
	colum_seg.clear();
	row_seg.clear();
	left_up.clear();
	right_up.clear();
	seg_array.clear();
	//initialize colum segmentation
	colum_seg.resize(width);
	for (k = 0; k < width; k++) {
		colum_seg[k].resize(width);
	}
	for (j = 0; j < width; j++) {
		for (i = 0; i < width; i++) {
			if (nursery[i][j] == 0) {
				colum_seg[i][j] = seg;
			}
			else if (nursery[i][j] == 2) {
				seg++;
			}
		}
		seg++;
	}

	//initiate row segmentation
	row_seg.resize(width);
	for (i = 0; i < width; i++) {
		row_seg[i].resize(width);
		for (j = 0; j < width; j++) {
			if (nursery[i][j] == 0) {
				row_seg[i][j] = seg;
			}
			else if (nursery[i][j] == 2) {
				seg++;
			}
		}
		seg++;
	}

	//initiate left_up segmentation
	left_up.resize(width);
	for (k = 0; k < width; k++) {
		left_up[k].resize(width);
	}
	for (k = width - 1; k >= 0; k--) {
		for (i = k, j = 0; i < width; i++, j++) {
			if (nursery[i][j] == 0) {
				left_up[i][j] = seg;
			}
			else if (nursery[i][j] == 2) {
				seg++;
			}
		}
		seg++;
	}
	for (k = 1; k < width; k++) {
		for (j = k, i = 0; j < width; j++, i++) {
			if (nursery[i][j] == 0) {
				left_up[i][j] = seg;
			}
			else if (nursery[i][j] == 2) {
				seg++;
			}
		}
		seg++;
	}

	//initiate right_up segmentation
	right_up.resize(width);
	for (k = 0; k < width; k++) {
		right_up[k].resize(width);
	}
	for (k = 0; k < width; k++) {
		for (j = k, i = 0; j >= 0; j--, i++) {
			if (nursery[i][j] == 0) {
				right_up[i][j] = seg;
			}
			else if (nursery[i][j] == 2) {
				seg++;
			}
		}
		seg++;
	}
	for (k = 1; k < width; k++) {
		for (i = k, j = width - 1; i < width; i++, j--) {
			if (nursery[i][j] == 0) {
				right_up[i][j] = seg;
			}
			else if (nursery[i][j] == 2) {
				seg++;
			}
		}
		seg++;
	}

	seg_array.resize(seg);
}
/*
get a node,update the current nursery state, start searching from s_p to create new child
*/
bool BFS_search(BFS_node* Head, int End_level) {
	time_t start, end;
	BFS_node* cur_node = NULL, *child_node = NULL;
	Grid grid;//
	int i, j, k;
	int cur_level;
	start = time(NULL);
	while (1) {
		if (QUEUE_EMPTY(&(Head->queue))) {
			return false;
		}

		cur_node = (BFS_node*)Head->queue.next;
		QUEUE_REMOVE_FRONT(&(Head->queue));
		cur_level = cur_node->level;

		if (cur_level == End_level) {
			update(cur_node->solution);
			return true;
		}
		if ((End_level - cur_level) > (width - cur_node->solution.back().x - 1 + num_tree)) {
			continue;
		}

		update_segmentation(cur_node->solution);

		for (k = cur_node->s_p; k < width*width; k++) {
			i = k / width;
			j = k%width;
			if (nursery[i][j] == 0 && no_contradict(i, j)) {
				grid.x = i;
				grid.y = j;

				child_node = (BFS_node*)calloc(1, sizeof(BFS_node));
				BFSNode_Create(Head, cur_node, child_node, grid, k + 1, cur_node->level + 1);
			}
		}
		end = time(NULL);
		if (end >= start + 285) {
			return false;
		}
		rec_segmentation(cur_node->solution);
	}
}

/*
recursion version, system stack is faster than stack implementated by myself
get a node,update the current nursery state, start searching from s_p to create new child
*/
bool DFS_search(DFS_node* cur_node, int End_level, time_t start) {
	time_t end;
	bool result = false;
	DFS_node *child_node = NULL;
	Grid grid;
	int i, j, k;
	int cur_level;
	int m, n, temp;

	cur_level = cur_node->level;
	if (cur_level > 0) {
		m = cur_node->grid.x;
		n = cur_node->grid.y;
		temp = colum_seg[m][n];
		seg_array[temp]++;
		temp = row_seg[m][n];
		seg_array[temp]++;
		temp = left_up[m][n];
		seg_array[temp]++;
		temp = right_up[m][n];
		seg_array[temp]++;
		//update nursery;
		nursery[m][n] = 1;
	}

	if (cur_level == End_level) {
		return true;
	}
	if ((End_level - cur_level) > (width - cur_node->grid.x - 1 + num_tree)) {
		temp = colum_seg[m][n];
		seg_array[temp]--;
		temp = row_seg[m][n];
		seg_array[temp]--;
		temp = left_up[m][n];
		seg_array[temp]--;
		temp = right_up[m][n];
		seg_array[temp]--;
		nursery[m][n] = 0;
		return false;
	}

	for (k = cur_node->s_p; k < width*width; k++) {
		end = time(NULL);
		if (end > start + 285) {
			return false;
		}

		i = k / width;
		j = k%width;
		if (nursery[i][j] == 0 && no_contradict(i, j)) {
			grid.x = i;
			grid.y = j;

			child_node = (DFS_node*)calloc(1, sizeof(DFS_node));
			DFSNode_Create(child_node, grid, k + 1, cur_node->level + 1);

			result = DFS_search(child_node, End_level, start);
			if (result == true) {
				return result;
			}

			end = time(NULL);
			if (end > start + 285) {
				return false;
			}

		}
	}

	if (cur_level > 0) {
		temp = colum_seg[m][n];
		seg_array[temp]--;
		temp = row_seg[m][n];
		seg_array[temp]--;
		temp = left_up[m][n];
		seg_array[temp]--;
		temp = right_up[m][n];
		seg_array[temp]--;
		nursery[m][n] = 0;
	}
	return result;
}

bool SA_search() {
	// how to set T
	time_t start, end;
	double T = 1; //C

	int i, j, l; //i,j stards for index of nursery, l stands for lizard
	int count = 0;
	int current_value = 0;
	int next_value = 0;
	int n_value = 0; // value of node
	int dalt_value;//current-next
	int temp;
	double possible;
	vector<Grid> lz_pos(lizards); //position of lizard
	Grid cur_grid, next_grid;
	//initialize the state

	for (int l = 0; l < lizards; l++) {
		do {
			i = rand() % width;
			j = rand() % width;
		} while (nursery[i][j] == 1 || nursery[i][j] == 2);
		nursery[i][j] = 1;
		cur_grid.x = i;
		cur_grid.y = j;
		temp = colum_seg[i][j];
		seg_array[temp]++;
		temp = row_seg[i][j];
		seg_array[temp]++;
		temp = left_up[i][j];
		seg_array[temp]++;
		temp = right_up[i][j];
		seg_array[temp]++;

		lz_pos[l] = cur_grid;

	}

	current_value = 0;
	count_value(&current_value);
	if (flag == true) {
		if (current_value == 0)
			return true;
		else
			return false;
	}
	//find the solution
	start = time(NULL);
	while (1) {
		count++;
		if (current_value == 0) {
			return true;
		}

		/************************** randomly selected successor of current******************************/
		l = rand() % lizards; //pick up a lizard
		cur_grid = lz_pos[l]; //get the lizard's current position

							  //find a new position which can't conflict with the current one
		do {
			i = rand() % width;
			j = rand() % width;
		} while (nursery[i][j] == 2 || nursery[i][j] == 1);
		nursery[cur_grid.x][cur_grid.y] = 0;//change the state to the next
		temp = colum_seg[cur_grid.x][cur_grid.y];
		seg_array[temp]--;
		temp = row_seg[cur_grid.x][cur_grid.y];
		seg_array[temp]--;
		temp = left_up[cur_grid.x][cur_grid.y];
		seg_array[temp]--;
		temp = right_up[cur_grid.x][cur_grid.y];
		seg_array[temp]--;

		next_grid.x = i;
		next_grid.y = j;
		temp = colum_seg[i][j];
		seg_array[temp]++;
		temp = row_seg[i][j];
		seg_array[temp]++;
		temp = left_up[i][j];
		seg_array[temp]++;
		temp = right_up[i][j];
		seg_array[temp]++;
		nursery[i][j] = 1;
		lz_pos[l] = next_grid;
		//get the value
		next_value = 0;
		count_value(&next_value);
		dalt_value = current_value - next_value;
		// next state is better,current<-next
		if (dalt_value > 0) {
			current_value = next_value;
		}
		// current state is better, current<-next under certain possibility
		else {
			possible = exp(double(dalt_value) / T);
			if (possible>(rand() % 10)*0.1) {
				current_value = next_value;
			}
			else {
				nursery[next_grid.x][next_grid.y] = 0;
				nursery[cur_grid.x][cur_grid.y] = 1;
				lz_pos[l] = cur_grid;

				temp = colum_seg[cur_grid.x][cur_grid.y];
				seg_array[temp]++;
				temp = row_seg[cur_grid.x][cur_grid.y];
				seg_array[temp]++;
				temp = left_up[cur_grid.x][cur_grid.y];
				seg_array[temp]++;
				temp = right_up[cur_grid.x][cur_grid.y];
				seg_array[temp]++;

				temp = colum_seg[next_grid.x][next_grid.y];
				seg_array[temp]--;
				temp = row_seg[next_grid.x][next_grid.y];
				seg_array[temp]--;
				temp = left_up[next_grid.x][next_grid.y];
				seg_array[temp]--;
				temp = right_up[next_grid.x][next_grid.y];
				seg_array[temp]--;
			}
		}
		
		end = time(NULL);
		if (end >= start + 270) {
			cout << count << end;
			return false;
		}	

		T = T*0.999;
	}
}

int main()
{
	//open and read input file
	string param[3]; //algorithm, width, lizards
	char c;
	num_tree = 0;
	flag = false;
	//create default outfile
	ofstream fout("output.txt");
	if (!fout) {
		cout << "Error opening output.txt" << endl;
		return -1;
	}
	fout << "FAIL" << endl;
	fout.close();
	ifstream fin("input.txt");
	if (fin.is_open())
	{
		fin >> param[0];
		fin >> param[1];
		fin >> param[2];
		width = atoi(param[1].c_str());
		lizards = atoi(param[2].c_str());
		nursery.clear();
		nursery.resize(width);
		for (int i = 0; i < width; i++) {
			nursery[i].resize(width);
			for (int j = 0; j < width; j++) {
				fin >> c;
				nursery[i][j] = c - 48;
				if (c == '2')
					num_tree++;
			}
		}
	}
	else
	{
		cout << "Error opening input.txt" << endl;
		return -1;
	}
	fin.close();

	//BFS
	bool result = false;
	//the bottom condition of having a solution 
	if (width + num_tree < lizards) {
		return 0;
	}
	else if ((width*width - num_tree) < lizards) {
		return 0;
	}
	else if ((width*width - num_tree) == lizards) {
		flag = true;
	}

	initiate_segmentation();


	if (param[0] == "BFS")
	{
		BFS_node Head, Node;
		Grid grid;
		QUEUE_INIT((Queue*)&(Head));
		grid.x = -1;
		grid.y = -1;
		BFSNode_Create(&Head, &Head, &Node, grid, 0, 0);
		result = BFS_search(&Head, lizards);
		if (result == true) {
			fout.open("output.txt", ios::trunc);
			fout << "OK" << endl;
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < width; j++) {
					fout << nursery[i][j];
				}
				fout << endl;
			}
		}
		fout.close();
	}
	//DFS
	else if (param[0] == "DFS")
	{
		time_t start;
		DFS_node Node;
		bool result;
		Grid grid;

		//QUEUE_INIT((Queue*)&(Head));
		grid.x = -1;
		grid.y = -1;
		DFSNode_Create(&Node, grid, 0, 0);
		start = time(NULL);
		result = DFS_search(&Node, lizards, start);
		if (result == true) {
			fout.open("output.txt", ios::trunc);
			fout << "OK" << endl;
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < width; j++) {
					fout << nursery[i][j];
				}
				fout << endl;
			}
		}
		fout.close();
	}
	else //SA
	{
		result = SA_search();
		if (result == true) {
			fout.open("output.txt", ios::trunc);
			fout << "OK" << endl;
			for (int i = 0; i < width; i++) {
				for (int j = 0; j < width; j++) {
					fout << nursery[i][j];
				}
				fout << endl;
			}
		}
		fout.close();
	}
	return 0;
}
