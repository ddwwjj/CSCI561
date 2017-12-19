// homework2.cpp
//date:16/10/2017
//score: 100/100
/*Version:4
No memfree cause system will release memory automatically.
Select the first move using alpha-belta prunning.
My opponent is either ramdom agent or minimax agent with no alpha-beta wand lookahead depth limited to 3(max-min-max).
Principle： searching as depth as possible to beat them, due to time constrains, I can't search thoroughly but at least deeper than opponent.

For fruit 1, 2 and 3, 
width<=10, depth=5; less than60's left, depth=4; less than 10's left, depth=3;
width<=26, depth=3; less than 60's left, depth=2; less than 10's left, depth=1;

for fruit4~9
width<=10, depth=4; less than 60's left, depth=3; less than 10's left, depth=2;
width<=20, depth=3; less than 60's left, depth=2; less than 10's left, depth=1;
width<=26, depth=2; less than 60's left, depth=1;

Special cases: when width=2,3, it is most likely to tie, but the residual times are same, so no worries.
*/

#include <iostream>
#include <sstream>
#include <fstream>  
#include <string> 
#include <vector>
#include <map>
#include <list>
#include <set>
#include <queue>
#include <cmath>
#include<stdlib.h>
#include<time.h>
#include<climits>
using namespace std;
int deltaX[4] = { 0,1,-1,0 };
int deltaY[4] = { -1,0,0,1 };

class coordination {
public:
	int x;
	int y;
	coordination(int x, int y) {
		this->x = x;
		this->y = y;
	}
};
class fruit_game {
public:
	int width;
	int fruits;
	double seconds;
	int valid_pos;

	fruit_game(int width, int fruits, stringstream& seconds, int valid_pos) {
		this->width = width;
		this->fruits = fruits;
		seconds >> this->seconds;
		this->valid_pos = valid_pos;
	}
};

class next_move {
public:
	char c;//A-Z
	int r;//1-26
	next_move(char c, int r) {
		this->c = c;
		this->r = r;
	}
};

class status {
public:
	int alpha;
	int beta;
	int my_val;
	int op_val;
	int deltav;
	int v;
	next_move* n_v;
	vector<vector<char> >best_fru_gra;

	status(int alpha, int beta, int my_val, int op_val, int deltav, int v, char c, char r) {
		this->alpha = alpha;
		this->beta = beta;
		this->my_val = my_val;
		this->op_val = op_val;
		this->deltav = deltav;
		this->v = v;
		this->n_v = new next_move(c,r);
	}
	status(status* pre_state) {
		this->alpha = pre_state->alpha;
		this->beta = pre_state->beta;
		this->my_val = pre_state->my_val;
		this->op_val = pre_state->op_val;
		this->deltav = pre_state->deltav;
		this->v = pre_state->v;
	}
};
//获得以fru_squ[i][j]为一员的一个联通区域
status* max_prun(vector<vector<char> >& fru_sqa, fruit_game* f_g, status* pre_state,int level,int upper_depth);
status* min_prun(vector<vector<char> >& fru_sqa, fruit_game* f_g, status* pre_state,int level,int upper_depth);
void get_area(multimap<int, list<coordination*> >& connected_areas, vector<vector<char> >& fru_sqa, vector<vector<bool> >& fru_area, int i, int j) {
	const int size = fru_sqa.size();
	list<coordination*> area;
	queue<coordination*> Queue;
	coordination* coord = new coordination(i, j);
	Queue.push(coord);
	area.push_back(coord);
	fru_area[i][j] = false;
	int count = 1;
	while (!Queue.empty()) {
		coordination* coord = Queue.front();
		Queue.pop();
		for (int i = 0; i < 4; i++) {
			coordination* adj = new coordination(
				coord->x + deltaX[i],
				coord->y + deltaY[i]
			);
			if (adj->x < 0 || adj->x >= size || adj->y < 0 || adj->y >= size) {
				delete(adj);
				continue;
			}
			if (fru_area[adj->x][adj->y] && fru_sqa[coord->x][coord->y] == fru_sqa[adj->x][adj->y]) {
				fru_area[adj->x][adj->y] = false;
				Queue.push(adj);
				area.push_back(adj);
				count++;
			}
			else {
				delete(adj);
			}
		}
	}

	connected_areas.insert(pair<int, list<coordination*> >(count, area));
	
	//Queue.swap(list<coordination*>());
	//area.swap(list<coordination*>());
}
void get_connected_areas(multimap<int, list<coordination*> >& connected_areas, vector<vector<char> >& fru_sqa) {
	int size = fru_sqa.size();
	vector<vector<bool> > fru_area(size, vector<bool>(size, true)); // true means haven't be exaimed yet

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			if (fru_sqa[i][j] != '*' && fru_area[i][j] == true) {
				get_area(connected_areas, fru_sqa, fru_area, i, j);
			}
		}
	}
	//fru_area.swap(vector<vector<bool> >());
}

void gravity(vector<vector<char> >& new_fru_sqa, set<int>& cols) {
	int col;
	int size = new_fru_sqa.size() - 1;
	int up, down;
	char temp;
	while (!cols.empty()) {
		col = *cols.begin();
		cols.erase(cols.begin());
		up = down = size;
		while (up >= 0) {
			if (new_fru_sqa[up][col] != '*' ) {
				if (up != down) {
					temp = new_fru_sqa[up][col];
					new_fru_sqa[up][col] = new_fru_sqa[down][col];
					new_fru_sqa[down][col] = temp;
				}				
				down--;
			}
			up--;
		}
	}
}

status* max_prun(vector<vector<char> >& fru_sqa, fruit_game* f_g, status* pre_state,int level, int upper_depth) {
	
	status* cur_state = new status(pre_state);
	cur_state->n_v = new next_move('a', 0);
	cur_state->v = INT_MIN;
	if (f_g->valid_pos == 0 || level >upper_depth) {
		cur_state->v = cur_state->deltav;
		return cur_state;
	}

	//获得当前层的所有联通区域	
	multimap<int, list<coordination*> > connected_areas;
	multimap<int, list<coordination*> >::iterator it_end;
	list<coordination*>::iterator it;
	get_connected_areas(connected_areas, fru_sqa);
	//选择包含元素个数最多的联通区域走下去,在connected_areas中，按升序排列
	set<int> cols;//for 所有被改过的列标号
	int my_val;
	next_move* n_v = new next_move('a', 0);
	//获得连通域的平均值取上
	
	while (!connected_areas.empty()) {
		//创建新的fru_sqa给下一层
		//start = time(NULL);
		vector<vector<char> >new_fru_sqa = fru_sqa;
		cols.clear();
		my_val = 0;
		for (it = connected_areas.rbegin()->second.begin(); it != connected_areas.rbegin()->second.end(); it++) {
			if (it == connected_areas.rbegin()->second.begin()) {
				n_v->c = (*it)->y + 'A';
				n_v->r = (*it)->x + 1;
				
			}

			new_fru_sqa[(*it)->x][(*it)->y] = '*';
			my_val++;
			cols.insert((*it)->y);
		}
		cur_state->my_val = cur_state->my_val + pow(my_val, 2);
		cur_state->deltav = cur_state->my_val - cur_state->op_val;
		//施展重力作用
		gravity(new_fru_sqa, cols);
		//求对手最小值
		f_g->valid_pos -= connected_areas.rbegin()->first;
		cur_state->v = max(cur_state->v, min_prun(new_fru_sqa, f_g, cur_state,level+1,upper_depth)->v);
		f_g->valid_pos += connected_areas.rbegin()->first;
		cur_state->my_val = cur_state->my_val - pow(my_val, 2);
		cur_state->deltav = cur_state->my_val - cur_state->op_val;

		//end = time(NULL);
		//cout << "level: " << level << ", " << "time: " << end - start << endl;

		if (cur_state->v >= cur_state->beta) {
			return cur_state;
		}
		if (cur_state->alpha < cur_state->v) {
			cur_state->alpha = cur_state->v;
			if (level == 0) {
				//cur_state->myscore = pow(my_val, 2);//my final score
				cur_state->n_v->c = n_v->c;
				cur_state->n_v->r = n_v->r;
				cur_state->best_fru_gra = new_fru_sqa;
			}
		}
		
		it_end = connected_areas.end();
		it_end--;
		//it_end->second.swap(list<coordination*>());
		connected_areas.erase(it_end);
		//new_fru_sqa.swap(vector<vector<char> >());
	}
	//connected_areas.swap(multimap<int, list<coordination*>>());
	return cur_state;
}
status* min_prun(vector<vector<char> >& fru_sqa, fruit_game* f_g, status* pre_state,int level, int upper_depth) {

	status* cur_state = new status(pre_state);
	cur_state->v = INT_MAX;
	cur_state->n_v = new next_move('a', 0);
	if (f_g->valid_pos == 0 || level>upper_depth) {
		cur_state->v = cur_state->deltav;
		return cur_state;
	}

	//获得当前层的所有联通区域	
	multimap<int, list<coordination*> > connected_areas;
	multimap<int, list<coordination*> >::iterator it_end;
	list<coordination*>::iterator it;
	get_connected_areas(connected_areas, fru_sqa);
	//选择包含元素个数最多的联通区域走下去,在connected_areas中，按升序排列
	
	set<int> cols;//for 所有被改过的列标号
	int op_v;
	
	while (!connected_areas.empty()) {
		//创建新的fru_sqa给下一层
		vector<vector<char> > new_fru_sqa = fru_sqa;
		cols.clear();
		op_v = 0;
		for (it = connected_areas.rbegin()->second.begin(); it != connected_areas.rbegin()->second.end(); it++) {
			if (it == connected_areas.rbegin()->second.begin()) {
				cur_state->n_v->c = (*it)->y + 'A';
				cur_state->n_v->r = (*it)->x + 1;
			}
			new_fru_sqa[(*it)->x][(*it)->y] = '*';
			op_v++;
			cols.insert((*it)->y);
		}
		cur_state->op_val = cur_state->op_val + pow(op_v, 2);
		cur_state->deltav = cur_state->my_val - cur_state->op_val;
		//施展重力作用
		gravity(new_fru_sqa, cols);
		//求对手最小值
		f_g->valid_pos -= connected_areas.rbegin()->first;
		cur_state->v = min(cur_state->v, max_prun(new_fru_sqa, f_g, cur_state,level + 1,upper_depth)->v);
		cur_state->op_val = cur_state->op_val - pow(op_v, 2);
		cur_state->deltav = cur_state->my_val - cur_state->op_val;
		f_g->valid_pos += connected_areas.rbegin()->first;
		if (cur_state->v <= cur_state->alpha) {
			return cur_state;
		}
		cur_state->beta = min(cur_state->beta, cur_state->v);
		
		it_end = connected_areas.end();
		it_end--;
		//it_end->second.swap(list<coordination*>());
		connected_areas.erase(it_end); 
		//new_fru_sqa.swap(vector<vector<char> >());
	}
	//connected_areas.swap(multimap<int, list<coordination*>>());
	//cols.swap(set<int>());
	return cur_state;
}

status* a_b_search(vector<vector<char> >& fru_sqa, fruit_game* f_g) {
	int upper_depth;
	status* cur_state = new status(INT_MIN, INT_MAX, 0, 0, 0, INT_MIN, 'a', 0);
	if (f_g->fruits <= 3) {
		if (f_g->width <= 10) {
			//最大探索深度从5开始，只剩60s后改为4，只剩10s后改为3
			upper_depth = 4;			
		}
		else {
			upper_depth = 2;			
		}
	}
	else {
		if (f_g->width <= 10) {
			upper_depth = 3;
		}
		else if (f_g->width <= 20) {
			upper_depth = 2;
		}
		else {
			upper_depth = 1;
		}
	}	
	if (f_g->seconds <= 60 && f_g->seconds >= 10) {
		upper_depth = max(upper_depth - 1, 0);
	}
	if (f_g->seconds < 10) {
		upper_depth = max(upper_depth - 2, 0);
	}
	cur_state = max_prun(fru_sqa, f_g, cur_state, 0, upper_depth);
	return cur_state;
}
int main()
{
	//read input file
	//time_t start, end;
	//start = time(NULL);
	string param[3];
	char c;

	ifstream fin("input.txt");
	fin >> param[0]; //width and height of square board (0,26]
	fin >> param[1]; //number of fruit types(0,9]
	fin >> param[2]; //remaining time, float,seconds
	stringstream seconds(param[2]);
	fruit_game* f_g = new fruit_game(atoi(param[0].c_str()), atoi(param[1].c_str()), seconds, 0);
	vector<vector<char> > fru_sqa(f_g->width, vector<char>(f_g->width)); // fruit square

	for (int i = 0; i < f_g->width; i++) {
		for (int j = 0; j < f_g->width; j++) {
			fin >> c;
			fru_sqa[i][j] = c;
			if (c != '*') {
				f_g->valid_pos++;
			}
		}
	}
	
	//alpha-belta prunning
	status* result;
	result = a_b_search(fru_sqa, f_g);
	//cout << result->v << endl;
	ofstream outf("output.txt");
	outf << result->n_v->c << result->n_v->r << endl;
	for (int i = 0; i <f_g->width; i++){
		for (int j = 0; j < f_g->width; j++) {
			outf << result->best_fru_gra[i][j];
		}
		outf << endl;
	}
	//end = time(NULL);
	//cout << "time: " << end - start << endl;
	outf.close();

	return 0;
}

