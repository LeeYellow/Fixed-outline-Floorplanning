#include <utility>      // std::pair
#include <stdlib.h>     // srand rand
#include <stdio.h> 
#include <math.h>       //squrt
#include <fstream>      //fstream
#include <iostream>
#include <unordered_map> // std::unordered_map
#include <string.h>
#include <stack>
#include <vector>
#include <algorithm>
#include <time.h>

using namespace std;

struct BLOCK {
	string id;
	int height;
	int width;
	int area;
	pair <int,int> left_down;
	pair <int,int> left_up;
	pair <int,int> right_down;
	pair <int,int> right_up;
	int rotated;

	BLOCK* parent;
	BLOCK* left_child;
	BLOCK* right_child;

};

struct TERMINAL {
	string id;
	int X;
	int Y;
};

struct NET {
	int id;
	vector<BLOCK*> pin_block;
	vector<TERMINAL*> pin_terminal;
};

struct NODE {
	NODE* prev;
	NODE* next;
	int X;
	int Y;
	/**/
	BLOCK* root_block;
	int type;
};

struct TREE {
	string id;
	TREE* parent;
	TREE* left_child;
	TREE* right_child;
	int rotated;
};

struct RETURN_PLACEMENT {
	int now_w_fl;
	int now_h_fl;
	int new_floorplan_area;
};



/**Global variable**/
unordered_map<string, BLOCK*> blocks;
unordered_map<string, TERMINAL*> terminals;
unordered_map<int, NET*> nets;
unordered_map<string, TREE*> global_tree;
unordered_map<string, TREE*> local_tree;
double space_ratio;
int w_fl, h_fl;
BLOCK* b_tree_root;
TREE* global_tree_root;
TREE* local_tree_root;
int* horizontal_contour;

/**funtion list**/
int Read_Data(const char* block_file_name, const char* pin_file_name, const char* net_file_name);
void Write_File(const char* output_file_name, int best_wire_len);
RETURN_PLACEMENT Initial_Placement();
RETURN_PLACEMENT Placement();
int Wire_Length();
void Perturb();
void Struct_Tree(int local_global);
void Return_Tree(int local_global);
bool mycmp(BLOCK* a, BLOCK* b);
long long int cost(int hpwl, int bigx, int bigy);

/**main funtion**/
int main(int argc, char *argv[]){
	double start = clock();
	double end = clock();
	/**/
	int block_area_sum = Read_Data(argv[1],argv[2],argv[3]);
	/**/
	b_tree_root = blocks.begin()->second;
	space_ratio=atof(argv[5])-0.01;
	/**/
	int seed_, bound_, bad_;
	if(strcmp(argv[1],"n100.hardblocks")==0){
		bound_ = 300000;
		bad_ = 10000;
	}else if(strcmp(argv[1],"n200.hardblocks")==0){
		bound_ = 800000;
		bad_ = 40000;
	}else{
		bound_ = 1000000;
		bad_ = 90000;
	}
	/**/
	
	srand(5);
	/**/
	w_fl = sqrt(block_area_sum*(1+space_ratio));
	h_fl = w_fl;
	/**/
	RETURN_PLACEMENT initial_placement = Initial_Placement();
	/**/ //local_tree
	int floorplan_area = initial_placement.new_floorplan_area;
	int now_w_fl = initial_placement.now_w_fl;
	int now_h_fl = initial_placement.now_h_fl;
	int now_wirelength = Wire_Length();
	long long int now_cost = cost(now_wirelength, now_w_fl, now_h_fl);
	/**/ //global_tree
	int best_floor_area = floorplan_area;
	int best_w_fl = now_w_fl;
	int best_h_fl = now_h_fl;
	int best_wirelength = now_wirelength;
	long long int best_cost = now_cost;
	/**/
	Struct_Tree(1);//struct global tree
	Struct_Tree(0);//struct local tree
	/**/
	int bound = 0;
	int bad = 0;
	int flag = 1;

	while(bound < bound_){
        ++bound;
		Perturb();
        RETURN_PLACEMENT r_placement = Placement(); 
		int new_wirelength = Wire_Length();
		long long int new_cost = cost(new_wirelength, r_placement.now_w_fl, r_placement.now_h_fl);
		if(new_cost < now_cost){
			bad = 0;
            floorplan_area = r_placement.new_floorplan_area;
            now_w_fl = r_placement.now_w_fl;
            now_h_fl = r_placement.now_h_fl;
			now_wirelength = new_wirelength;
			now_cost = new_cost;
            Struct_Tree(0);
			if(flag && floorplan_area < best_floor_area && abs(1-now_h_fl/now_w_fl)<0.02){
				best_floor_area = floorplan_area;
				best_w_fl = now_w_fl;
				best_h_fl = now_h_fl;
				best_wirelength = now_wirelength;
				Struct_Tree(1);
				if(best_w_fl <= w_fl && best_h_fl <= h_fl){
					flag = 0;
					//cout << "best_wirelength: " << best_wirelength << endl;
				}
			}else{
				if(now_wirelength < best_wirelength && now_h_fl <= h_fl && now_w_fl <= w_fl){
					//cout << "best_wirelength: " << best_wirelength << endl;
					best_floor_area = floorplan_area;
					best_w_fl = now_w_fl;
					best_h_fl = now_h_fl;
					best_wirelength = now_wirelength;
					//cout << "wirelength: " << best_wirelength << endl;
					Struct_Tree(1);
				}
			}	
				
			
		}else{
			bad++;
			if(bad > bad_){
				bad = 0;
				
				floorplan_area = r_placement.new_floorplan_area;
				now_w_fl = r_placement.now_w_fl;
				now_h_fl = r_placement.now_h_fl;
				now_wirelength = new_wirelength;
				now_cost = new_cost;
				Struct_Tree(0);
			}else{
				Return_Tree(0);
			}
		}
        end = clock();
	}
	
	Return_Tree(1);
	RETURN_PLACEMENT r_placement = Placement();
    best_w_fl = r_placement.now_w_fl;
    best_h_fl = r_placement.now_h_fl;
	best_wirelength = Wire_Length();
	
	/*if(best_w_fl <= w_fl && best_h_fl <= h_fl){
		cout <<"success!!" << endl;
		cout <<"in the case of seed = " << 5 << endl;
		cout << "w_fl:" << w_fl << endl;
		cout <<"best_w_fl = " << best_w_fl << endl;
		cout <<"best_h_fl = " << best_h_fl << endl;
		cout << "wirelength: " << best_wirelength << endl;
	}*/

	
	Write_File(argv[4], best_wirelength); 
    end = clock();
	//cout <<"Time:" << (end-start)/CLOCKS_PER_SEC << endl;
	return 0;

}

void Struct_Tree(int local_global){
	stack<BLOCK*> traversal_stack;
	traversal_stack.push(b_tree_root);
    if(local_global){
		global_tree_root = global_tree[b_tree_root->id];
		global_tree_root->parent = NULL;
	}else{
		local_tree_root = local_tree[b_tree_root->id];
		local_tree_root->parent = NULL;
	}
	while(traversal_stack.size()){
        BLOCK* block = traversal_stack.top();
        traversal_stack.pop();
        TREE* tree;
		if(local_global)
			tree = global_tree[block->id];
		else 
			tree = local_tree[block->id];
        
		if(block -> right_child){
            TREE* tree_right;
			if(local_global)
				tree_right = global_tree[block->right_child->id];
            else
				tree_right = local_tree[block->right_child->id];
			
			tree->right_child = tree_right;
            tree_right->parent = tree;
            traversal_stack.push(block->right_child);
        }else tree->right_child = NULL;
        
		if(block -> left_child){
            TREE* tree_left;
			if(local_global)
				tree_left = global_tree[block->left_child->id];
			else
				tree_left = local_tree[block->left_child->id];
            tree->left_child = tree_left;
            tree_left->parent = tree;
            traversal_stack.push(block->left_child);
        }else tree->left_child = NULL;
        tree->rotated = block->rotated;
	}
}

void Return_Tree(int local_global){
	stack<TREE*> traversal_stack;
	if(local_global){
		traversal_stack.push(global_tree_root);
		b_tree_root = blocks[global_tree_root->id];
	}else{ 
		traversal_stack.push(local_tree_root);
		b_tree_root = blocks[local_tree_root->id];
	}    
    b_tree_root->parent = NULL;
	while(traversal_stack.size()){
        TREE* tree = traversal_stack.top();
        traversal_stack.pop();
        BLOCK* block = blocks[tree->id];
        if(tree -> right_child){
            BLOCK* block_right = blocks[tree->right_child->id];
            block->right_child = block_right;
            block_right->parent = block;
            traversal_stack.push(tree->right_child);
        }else block->right_child = NULL;
        if(tree -> left_child){
            BLOCK* block_left = blocks[tree->left_child->id];
            block->left_child = block_left;
            block_left->parent = block;
            traversal_stack.push(tree->left_child);
        }else block->left_child = NULL;

        if(block->rotated != tree->rotated ){
            int tmp = block->width;
            block->width = block->height;
            block->height = tmp;
            block->rotated = 1-block->rotated;
        }
	}
}

RETURN_PLACEMENT Initial_Placement(){
	horizontal_contour = new int[w_fl*3]();
	int max_horizontal_contour = 0;
	int max_vertical_contour = 0;
	/**/
	NODE* start_point = new NODE();
	start_point->prev = NULL;
	start_point->next = NULL;
	start_point->X = 0;
	start_point->Y = 0;
	start_point->root_block = NULL;
	start_point->type = -1;
	/**/
	NODE* now_point = start_point;
	/**/
	for(auto it = blocks.begin(); it != blocks.end(); ++it){
		if(it->second->height < it->second->width){
			int tmp = it->second->height;
			it->second->height = it->second->width;
			it->second->width = tmp;
			it->second->rotated = 1-it->second->rotated;
		}
		if(now_point->X + it->second->width > w_fl){
			now_point = start_point;

		}
		NODE* tmp = now_point->next;
		while(tmp && tmp->X<= now_point->X+it->second->width){
			if(tmp->Y > now_point->Y) now_point->Y = tmp->Y;

			if(tmp->next) tmp->next->prev = now_point;
			now_point->next = tmp->next;
			/**need to delete tmp then to next**/
			NODE* tmp2 = tmp->next;
			delete(tmp);
			tmp = tmp2;
		} 

		int i, start = now_point->X, endd = now_point->X + it->second->width;
		for(i = start; i < endd; i++){
			horizontal_contour[i] = it->second->height + now_point->Y;
		}

		if(now_point->type == 0){
			now_point->root_block->right_child = it->second;
			it->second->parent = now_point->root_block;
		}else if(now_point->type == 1){
			now_point->root_block->left_child = it->second;
			it->second->parent = now_point->root_block;
		}
		/**/ //put in block
		it->second->left_down.first+=now_point->X;
		it->second->left_down.second+=now_point->Y;
		it->second->left_up.first+=now_point->X;
		it->second->left_up.second+=now_point->Y;
		it->second->right_up.first+=now_point->X;
		it->second->right_up.second+=now_point->Y;
		it->second->right_down.first+=now_point->X;
		it->second->right_down.second+=now_point->Y;

		/**/
		now_point->Y+= it->second->height;
		now_point->root_block = it->second;
		now_point->type = 0;
		/**/
		if(now_point->Y > max_horizontal_contour)max_horizontal_contour = now_point->Y;
		/**/
		NODE* next_point = new NODE();
		next_point->X = now_point->X + it->second->width;
		next_point->Y = horizontal_contour[i];
		next_point->root_block = it->second;
		next_point->type = 1;
		/**/
		if(next_point->X > max_vertical_contour) max_vertical_contour = next_point->X;
		if(now_point->next){
			now_point->next->prev = next_point;
			next_point->next = now_point->next;
		}else next_point->next = NULL;
		next_point->prev = now_point;
		now_point->next = next_point;
		now_point = next_point;
	}


	/**/
	NODE* delete_point = start_point;
	while(delete_point){
		NODE* tmp = delete_point->next;
		delete(delete_point);
		delete_point = tmp;
	}
	
	RETURN_PLACEMENT r;
	r.new_floorplan_area = max_horizontal_contour*max_vertical_contour;
	r.now_h_fl = max_horizontal_contour;
	r.now_w_fl = max_vertical_contour;
	return r;
}


int Wire_Length(){
	int wirelength = 0;
	for(auto it = nets.begin(); it != nets.end(); ++it){
		int X_max = 0, X_min = 100000, Y_max = 0, Y_min = 100000;

		for(int i = 0; i < it->second->pin_block.size(); i++){

			int x = (it->second->pin_block[i]->right_up.first + it->second->pin_block[i]->left_up.first)/2;
			int y = (it->second->pin_block[i]->right_up.second + it->second->pin_block[i]->right_down.second)/2;

			if(x > X_max) X_max = x;
			if(x < X_min) X_min = x;

			if(y > Y_max) Y_max = y;
			if(y < Y_min) Y_min = y;
			
		}

		for(int i = 0; i < it->second->pin_terminal.size(); i++){
			
			if(it->second->pin_terminal[i]->X > X_max) X_max = it->second->pin_terminal[i]->X;
			if(it->second->pin_terminal[i]->X < X_min) X_min = it->second->pin_terminal[i]->X;

			if(it->second->pin_terminal[i]->Y > Y_max) Y_max = it->second->pin_terminal[i]->Y;
			if(it->second->pin_terminal[i]->Y < Y_min) Y_min = it->second->pin_terminal[i]->Y;
		}
		
        wirelength+=X_max-X_min+Y_max-Y_min;
	}
	return wirelength;
}


void Perturb(){
	/**/
	int operand = rand()%50; //choose operand
	
	if(operand<33){// swap 2 blocks
		unordered_map<string, BLOCK*>::iterator iter = blocks.begin();
		
		advance(iter, rand()%blocks.size()); //choose a block
		BLOCK* block_1 = iter->second;

		iter = blocks.begin();
		advance(iter, rand()%blocks.size()); //choose a block
		BLOCK* block_2 = iter->second;


		if(block_1 == b_tree_root) b_tree_root = block_2;
		else if(block_2 == b_tree_root) b_tree_root = block_1;

		int flag1, flag2;
		BLOCK* M1;
		BLOCK* M2;
		BLOCK* L1;
		BLOCK* L2;
		BLOCK* R1;
		BLOCK* R2;
		/**/
		if(block_1->parent){
			if(block_1->parent->left_child == block_1) flag1 = 0;
			else flag1 = 1;
		}else flag1 = -1;
		if(block_1->parent == block_2) M1 = block_1;
		else M1 = block_1->parent;
		if(block_1->left_child == block_2) L1 = block_1;
		else L1 = block_1->left_child;
		if(block_1->right_child == block_2) R1 = block_1;
		else R1 = block_1->right_child;
		/**/

		if(block_2->parent){
			if(block_2->parent->left_child == block_2) flag2 = 0;
			else flag2 = 1;
		}else flag2 = -1;
		if(block_2->parent == block_1) M2 = block_2;
		else M2 = block_2->parent;
		if(block_2->left_child == block_1) L2 = block_2;
		else L2 = block_2->left_child;
		if(block_2->right_child == block_1) R2 = block_2;
		else R2 = block_2->right_child;

		block_1->parent = M2;
		if(M2 && flag2) M2->right_child = block_1;
		else if(M2 && flag2==0) M2->left_child = block_1;
		block_1->left_child = L2;
		if(L2) L2->parent = block_1;
		block_1->right_child = R2;
		if(R2) R2->parent = block_1;

		block_2->parent = M1;
		if(M1 && flag1) M1->right_child = block_2;
		else if(M1 && flag1==0) M1->left_child = block_2;
		block_2->left_child = L1;
		if(L1)  L1->parent = block_2;
		block_2->right_child = R1;
		if(R1) R1->parent = block_2;


	}else if(operand < 49){//rotate a block
		unordered_map<string, BLOCK*>::iterator iter = blocks.begin();
		advance(iter, rand()%blocks.size());
		BLOCK* block_rotate = iter->second;

		/**/
		int tmp = block_rotate->width;
		block_rotate->width = block_rotate->height;
		block_rotate->height = tmp;
		block_rotate->rotated = 1-block_rotate->rotated;

	}else{//delete a block
		unordered_map<string, BLOCK*>::iterator iter = blocks.begin();
		advance(iter, rand()%blocks.size());
		BLOCK* block_delete = iter->second;
        //cout << "block_delete:" << block_delete->id <<endl;
        if(block_delete == b_tree_root && block_delete->left_child) b_tree_root = block_delete->left_child;
        else if(block_delete == b_tree_root && block_delete->right_child) b_tree_root = block_delete->right_child;

		if(block_delete->left_child){
			block_delete->left_child->parent=block_delete->parent;
			if(block_delete->parent && block_delete->parent->left_child==block_delete)
				block_delete->parent->left_child = block_delete->left_child;
			if(block_delete->parent && block_delete->parent->right_child==block_delete)
				block_delete->parent->right_child = block_delete->left_child;
			block_delete->left_child->right_child = block_delete->right_child;
			if(block_delete->right_child) block_delete->right_child->parent = block_delete->left_child;
		}else if(block_delete->right_child){
			block_delete->right_child->parent=block_delete->parent;
			if(block_delete->parent && block_delete->parent->left_child==block_delete)
				block_delete->parent->left_child = block_delete->right_child;
			if(block_delete->parent && block_delete->parent->right_child==block_delete)
				block_delete->parent->right_child = block_delete->right_child;
		}else{
			if(block_delete->parent && block_delete->parent->left_child==block_delete)
				block_delete->parent->left_child = NULL;
			if(block_delete->parent && block_delete->parent->right_child==block_delete)
				block_delete->parent->right_child = NULL;
		}
		block_delete->parent = NULL;
		block_delete->left_child = NULL;
		block_delete->right_child = NULL;


		iter = blocks.begin();
		advance(iter, rand()%blocks.size());
		BLOCK* tmp = iter->second;
		while(tmp == block_delete){
            iter = blocks.begin();
            advance(iter, rand()%blocks.size());
            tmp = iter->second;
		}
		/*if(now_w_fl < w_fl){
			while(tmp->left_child) {
					tmp = tmp->left_child;}
			tmp->left_child = block_delete;
			block_delete->parent = tmp;
		}else{
		*/
		if(tmp->left_child)
			tmp->left_child->parent = block_delete;	 
		block_delete->left_child = tmp->left_child;
		block_delete->parent = tmp;
		tmp->left_child = block_delete;
		//}
	}
}


RETURN_PLACEMENT Placement(){
	delete(horizontal_contour);
	horizontal_contour = new int[w_fl*3]();
	int max_horizontal_contour = 0;
	int max_vertical_contour = 0;
	/**/
	NODE* start_point = new NODE();
	start_point->prev = NULL;
	start_point->next = NULL;
	start_point->X = 0;
	start_point->Y = 0;
	start_point->root_block = NULL;
	start_point->type = -1;
	/**/
	NODE* now_point = start_point;
	/**/

	stack<BLOCK*> traversal_stack;
	traversal_stack.push(b_tree_root);

	while(traversal_stack.size()){
		BLOCK* block = traversal_stack.top();
		traversal_stack.pop();


		if(block->right_child) traversal_stack.push(block->right_child);
		if(block->left_child) traversal_stack.push(block->left_child);
		/**do things to block*/

		if(block->parent && block->parent->right_child == block){
			now_point = start_point;

		}
        //cout << "X:" << now_point->X << "Y:" << now_point->Y<<endl;
		NODE* tmp = now_point->next;
		while(tmp && tmp->X<= now_point->X + block->width){
			if(tmp->Y > now_point->Y) now_point->Y = tmp->Y;

			if(tmp->next) tmp->next->prev = now_point;
			now_point->next = tmp->next;
			/**need to delete tmp then to next**/
			NODE* tmp2 = tmp->next;
			delete(tmp);
			tmp = tmp2;
		}

		int i, start = now_point->X, endd = now_point->X + block->width;
		for(i = start; i < endd; i++){
			horizontal_contour[i] = block->height + now_point->Y;
		}

		/**/ //put in block
		block->left_down.first = now_point->X;
		block->left_down.second = now_point->Y;
		block->left_up.first = now_point->X;
		block->left_up.second = now_point->Y + block->height;
		block->right_up.first = now_point->X + block->width;
		block->right_up.second = now_point->Y + block->height;
		block->right_down.first = now_point->X + block->width;
        block->right_down.second = now_point->Y;
		/**/
		now_point->Y+= block->height;
		/**/
		if(now_point->Y > max_horizontal_contour)max_horizontal_contour = now_point->Y;
		/**/
		NODE* next_point = new NODE();
		next_point->X = now_point->X + block->width;
		next_point->Y = horizontal_contour[i];
		next_point->root_block = NULL;
		next_point->type = -1;
		/**/
		if(next_point->X > max_vertical_contour)	max_vertical_contour = next_point->X;
		/**/
		if(now_point->next){
			now_point->next->prev = next_point;
			next_point->next = now_point->next;
		}else next_point->next = NULL;
		next_point->prev = now_point;
		now_point->next = next_point;
		now_point = next_point;
	}
	/**/
	NODE* delete_point = start_point;
	while(delete_point){
		NODE* tmp = delete_point->next;
		delete(delete_point);
    //I love my boyfriend <3333
		delete_point = tmp;
	}

	RETURN_PLACEMENT r;
	r.new_floorplan_area = max_horizontal_contour*max_vertical_contour;
	r.now_h_fl = max_horizontal_contour;
	r.now_w_fl = max_vertical_contour;
	return r;

}

bool mycmp(BLOCK* a, BLOCK* b){
	return atoi(a->id.substr(2).c_str()) < atoi(b->id.substr(2).c_str());
}	

int Read_Data(const char* block_file_name, const char* pin_file_name, const char* net_file_name){
	int hardBlockNum, terminalNum, vertexNum, netNum, pinNum, netDegree;
	string Block_id, Terminal_id;
	int total_area= 0;
	int X, Y;
	/**/
	string str;
	char c;
	/**//**block_file read in**/
	fstream block_file(block_file_name , fstream::in);
	block_file>>str>>c>>hardBlockNum>>str>>c>>terminalNum;
	for(int i = 0; i < hardBlockNum; i++){
		BLOCK* block = new BLOCK();
		block_file >> Block_id >> str >> vertexNum;
		block->id = Block_id;
		/**/
		block_file >> c >> X >> c >> Y >> c;
		block->left_down = make_pair(X, Y);
		block_file >> c >> X >> c >> Y >> c;
		block->left_up = make_pair(X, Y);
		block_file >> c >> X >> c >> Y >> c;
		block->right_up = make_pair(X, Y);
		block_file >> c >> X >> c >> Y >> c;
		block->right_down = make_pair(X, Y);
		/**/
		block->height = block->left_up.second - block->left_down.second;
		block->width = block->right_down.first - block->left_down.first;
		block->area = block->height * block->width;
		block->rotated = 0;
		/**/
		total_area+=block->area;
		/**/
		block->parent = NULL;
		block->left_child = NULL;
		block->right_child = NULL;
		/**/
		blocks[Block_id] = block;

		TREE* tree_global = new TREE();
		tree_global->id = Block_id;
		tree_global->parent = NULL;
		tree_global->left_child = NULL;
		tree_global->right_child = NULL;
		tree_global->rotated = 0;
		global_tree[Block_id] = tree_global;
		
		TREE* tree_local = new TREE();
		tree_local->id = Block_id;
		tree_local->parent = NULL;
		tree_local->left_child = NULL;
		tree_local->right_child = NULL;
		tree_local->rotated = 0;
		local_tree[Block_id] = tree_local;

	}
	for(int i = 0; i < terminalNum; i++){
		TERMINAL* terminal = new TERMINAL();
		block_file >> Terminal_id >> str;
		terminal->id = Terminal_id;
		terminals[Terminal_id] = terminal;
	}

	/**//*pin coordinate read in*/
	fstream pin_file(pin_file_name , fstream::in);
	for(int i = 0; i < terminalNum; i++){
		pin_file >> str >> X >> Y;
		TERMINAL* terminal = terminals[str];
		terminal->X = X;
		terminal->Y = Y;
	}

	/**//**net_file read in**/
	fstream net_file(net_file_name , fstream::in);
	net_file>>str>>c>>netNum>>str>>c>>pinNum;
	for(int i = 0; i < netNum; i++){
		net_file>>str>>c>>netDegree;
		NET* net = new NET();
		for(int j = 0; j < netDegree; j++){
			net_file>>str;
			if(str[0] == 's'){
				BLOCK* block = blocks[str];
				net->pin_block.push_back(block);
			}else{
				TERMINAL* terminal = terminals[str];
				net->pin_terminal.push_back(terminal);
			}
		}
		nets[i] = net;
	}

	return total_area;
}

void Write_File(const char* output_file_name, int best_wire_len){
	ofstream myfile;
	myfile.open(output_file_name);
	myfile<<"Wirelength "<<best_wire_len<<"\n";
	myfile<<"Blocks\n";
	vector<BLOCK*> TEMP;
	for(auto it = blocks.begin(); it != blocks.end(); ++it){
		TEMP.push_back(it->second);
	}
	
	sort(TEMP.begin(), TEMP.end(), mycmp); 
	
	for(auto it = TEMP.begin(); it != TEMP.end(); ++it){
		if( (*it)->rotated == 0 ){
			myfile<<(*it)->id<<" "<<(*it)->left_down.first<<" "<<(*it)->left_down.second<<" "<<"0\n";
		}
		else
			myfile<<(*it)->id<<" "<<(*it)->left_down.first<<" "<<(*it)->left_down.second<<" "<<"1\n";
	}
	myfile.close();
}

long long int cost(int hpwl, int bigx, int bigy){
    int i; 
	long long int over = 0;
	
    if(bigx > w_fl)
        over += ((bigx-w_fl)*50000);
    if(bigy > h_fl)
		over += ((bigy-h_fl)*50000);

	
	for(i=0; i<w_fl*3; i++){
		if(horizontal_contour[i] == 0) break;
        if(i<w_fl){
			if(horizontal_contour[i]>h_fl)
				over += (horizontal_contour[i]-h_fl)*500;
        }else{
            over += (horizontal_contour[i])*500;
        }
    }
    return over + hpwl*0.1;
}


