#include"PSIL.h"


//Global variables
char input_owner[20]="turn";
fstream formula_file;
int sxi_COUNT,node_COUNT,strategy_COUNT,closure_COUNT,next_DEPTH;
redgram path;
PSIL_Game_Edge* temp_edge;
PSIL_Game_Node* temp_node;
string temp_string;
char temp_char[1000];
map<string,int> node_map;
vector<PSIL_Game_Node*> Nodes;
vector<PSIL_Formula*> Parse_Tree;
vector<PSIL_Formula*> Closure;
vector<int> strategy2owner;

int ** Matrix;
int* strategy_stack;

//Global End



void print_parse_tree(PSIL_Formula* F, int depth){
	int i;
	for(i=0;i<depth;i++){
		cout<<"  ";
	}
	cout<<F->index<<endl;
	for(i=0;i<F->outs.size();i++){
		print_parse_tree(F->outs[i],depth+1);
	}
}

void print_strategy2owner(){
	int i;
	cout<<"strategy2owner:"<<endl;
	for(i=0;i<strategy2owner.size();i++){
		cout<<strategy2owner[i]<<" ";
	}
	cout<<endl;
}
void print_strategy_stack(){
	int i;
	cout<<"strategy_stack:"<<endl;
	for(i=0;i<strategy_COUNT;i++){
		cout<<strategy_stack[i]<<" ";
	}
	cout<<endl;
}

void print_matrix(){
	int i,j;
	cout<<setw(7)<<"*******";
	for(i=0;i<closure_COUNT;i++){
		cout<<setw(5)<<Closure[i]->index;
	}
	cout<<endl;
	for(i=0;i<strategy_COUNT;i++){
		string temp_string;
		stringstream ss(temp_string);
		ss<<i<<"("<<strategy2owner[i]<<")";
		cout<<setw(7)<<ss.str();
		for(j=0;j<closure_COUNT;j++){
			cout<<setw(5)<<Matrix[i][j];
		}
		cout<<endl;
	}
	cout<<endl;
}


bool check_parent(int a,int b){
	if(a==b){return true;}
	else{
		if(a>b){
			while(a!=0){
				a=Parse_Tree[a]->ins[0]->index;
				if(a==b){return false;}
			}
		}
		else if(a<b){
			while(b!=0){
				b=Parse_Tree[b]->ins[0]->index;
				if(a==b){return false;}
			}
		}
	}
	return true;
}

#define PASS 1
#define FAIL 2
#define UNVISITED 3
#define CONTINUE 4

int Check_Visited(Computation_Tree_Node* R){
	int i;

	if(Closure[R->until_token_old]->type==UNTIL && R->G[R->until_token_old]==TRUE_GUESSED_PHASE_2){
		R->until_token=R->until_token_old;
	}
	else if(Closure[R->until_token_old]->type==WNTIL && R->G[R->until_token_old]==FALSE_GUESSED_PHASE_2){
		R->until_token=R->until_token_old;
	}	
	else{
		R->until_token=(R->until_token_old+1)%(closure_COUNT+1);
		while(R->until_token!=0 || (Closure[R->until_token]->type!=UNTIL && Closure[R->until_token]->type!=WNTIL)){
			R->until_token=(R->until_token+1)%(closure_COUNT+1);
		}
	}
	cout<<"Check_visited(state,token):("<<R->state->index<<","<<R->until_token<<")"<<endl;
	
	
	Computation_Tree_Node* ancestor;
	ancestor=R;
	int local_index;
	local_index=R->state->index;
	bool token_changed;
	token_changed=false;
	while(ancestor->ins!=NULL){
		ancestor=ancestor->ins;
		if(ancestor->until_token!=R->until_token){token_changed=true;}
		else{
			if(ancestor->state->index==local_index){
				bool diff_G;
				diff_G=false;
				for(i=0;i<closure_COUNT;i++){
					if(R->G[i]!=ancestor->G[i]){diff_G=true;}
				}
				if(diff_G==false){
					if(token_changed || R->until_token==0){
						cout<<"visited and pass"<<endl;
						return PASS;
					}
					else{
						cout<<"visited and fail"<<endl;
						return FAIL;
					}
				}
			}
		}
	}
	cout<<"unvisited"<<endl;
	return UNVISITED;
}


void fill_in_matrix(PSIL_Formula* F){
	int i;
	switch(F->type){
		case TRUE_NODE:
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
			break;
		case FALSE_NODE:
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
			break;
		case PARSE_ROOT:
//			cout<<"PARSE_ROOT:"<<endl;
			for(i=0;i<strategy_COUNT;i++){
				if(strategy2owner[i]==F->owner){
					strategy_stack[i]=0;
				}
			} //clean the previous strategy of the same player
			strategy_stack[F->strategy_index]= 1;
			fill_in_matrix(F->outs[0]);
			strategy_stack[F->strategy_index]= 0;
//			cout<<"END"<<endl;
			break;
		case ATOMIC:
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
			break;
    case NOT:
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
    	fill_in_matrix(F->outs[0]);
    	break;
		case OR:
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
			fill_in_matrix(F->outs[0]);
			fill_in_matrix(F->outs[1]);
			break;
		case AND:
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
			fill_in_matrix(F->outs[0]);
			fill_in_matrix(F->outs[1]);
			break;
		case PLUS:
			for(i=0;i<strategy_COUNT;i++){
				if(strategy2owner[i]==F->owner){
					strategy_stack[i]=0;
				}
			} //clean the previous strategy of the same player
			strategy_stack[F->strategy_index]= 1;
			fill_in_matrix(F->outs[0]);
			strategy_stack[F->strategy_index]= 0;
			break;
		case MINUS:
			for(i=0;i<strategy_COUNT;i++){
				if(strategy2owner[i]==F->owner){
					strategy_stack[i]=0;
				}
			} //clean the previous strategy of the same player
			fill_in_matrix(F->outs[0]);
			break;
		case UNTIL:
//			cout<<"UNTIL"<<endl;
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
			fill_in_matrix(F->outs[0]);
			fill_in_matrix(F->outs[1]);
			break;
		case WNTIL:
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
			fill_in_matrix(F->outs[0]);
			fill_in_matrix(F->outs[1]);
			break;
		case NEXT:
//			cout<<"NEXT"<<endl;
			for(i=0;i<strategy_COUNT;i++){
				Matrix[i][F->closure_index]=strategy_stack[i];
			}
    	fill_in_matrix(F->outs[0]);
			break;
	}
}


void setup_matrix(){
	int i,j;
	Matrix=new int*[strategy_COUNT];
	for(i=0;i<strategy_COUNT;i++){
		Matrix[i]=new int[closure_COUNT];
	}
	for(i=0;i<strategy_COUNT;i++){
		for(j=0;j<closure_COUNT;j++){
			Matrix[i][j]=0;
		}
	}
	//setup strategy_stack, conveniently
	strategy_stack=new int[strategy_COUNT];
	for(i=0;i<strategy_COUNT;i++){
		strategy_stack[i]=0;
	}
}


void Setup_PSIL_Formula(){
	strategy_COUNT=0;
	closure_COUNT=0;
	next_DEPTH=0;
	int counter=0;
	int parent;
	int i;
	formula_file>>counter;
	for(i=0;i<counter;i++){
		//cout<<i<<endl;
		PSIL_Formula* F=new PSIL_Formula;
		Parse_Tree.push_back(F);
		formula_file>>F->index>>F->type;
		if(F->type==PARSE_ROOT || F->type==PLUS){
			formula_file>>F->owner>>parent;
			strategy2owner.push_back(F->owner);
			if(F->index!=0){
				Parse_Tree[parent]->outs.push_back(F);
				F->ins.push_back(Parse_Tree[parent]);
			}
			F->strategy_index=strategy_COUNT;
			strategy_COUNT++;
			F->closure_index=0;
		}
		else if(F->type==MINUS){
			formula_file>>F->owner>>parent;
			Parse_Tree[parent]->outs.push_back(F);
			F->ins.push_back(Parse_Tree[parent]);
			F->strategy_index=-1;
			F->closure_index=0;
		}
		else if(F->type==ATOMIC){
			formula_file>>F->str>>parent;
			Parse_Tree[parent]->outs.push_back(F);
			F->ins.push_back(Parse_Tree[parent]);
			F->closure_index=closure_COUNT;
			Closure.push_back(F);
			closure_COUNT++;
			F->strategy_index=0;
		}
		else{
			formula_file>>parent;
			Parse_Tree[parent]->outs.push_back(F);
			F->ins.push_back(Parse_Tree[parent]);
			F->closure_index=closure_COUNT;
			Closure.push_back(F);
			closure_COUNT++;
			F->strategy_index=0;
		}
	}
	print_strategy2owner();
	setup_matrix();
	fill_in_matrix(Parse_Tree[0]);
	print_matrix();
}

void Draw(PSIL_Game_Node* root){
	cout<<root->index<<": "<<red_diagram_string(root->red)<<endl;
	int i;
	int lb,hb;
	for(i=1;i<sxi_COUNT;i++){
		redgram temp_red=red_sync_xtion_fwd(
			root->red,
			path,
			RED_USE_DECLARED_SYNC_XTION,
			i,
			RED_GAME_MODL | RED_GAME_SPEC | RED_GAME_ENVR,
			RED_TIME_PROGRESS,
			RED_NORM_ZONE_CLOSURE,
			RED_NO_ACTION_APPROX,
			RED_REDUCTION_INACTIVE,
			RED_NOAPPROX_MODL_GAME | RED_NOAPPROX_SPEC_GAME
			| RED_NOAPPROX_ENVR_GAME | RED_NOAPPROX_GLOBAL_GAME,
			RED_NO_SYMMETRY,
			0);
		if(red_and(temp_red,red_query_diagram_enhanced_global_invariance())!=red_false()){
			if(node_map[temp_string.assign(red_diagram_string(temp_red))]==0){
			  temp_edge=new PSIL_Game_Edge;
			  temp_node=new PSIL_Game_Node;
			  temp_edge->sxi=i;
			  temp_edge->src=root;
			  temp_edge->dst=temp_node;
			  root->outs.push_back(temp_edge);
			  temp_node->index=node_COUNT; node_COUNT++;
			  temp_node->red=temp_red;
			  red_get_cube_discrete_value(temp_node->red,input_owner,&lb,&hb);
			  temp_node->owner=lb;
			  temp_node->ins.push_back(temp_edge);
			  Nodes.push_back(temp_node);
			  node_map[temp_string.assign(red_diagram_string(temp_red))]=temp_node->index;
			  cout<<root->index<<"--"<<temp_edge->sxi<<"-->"<<temp_node->index<<endl;
			  Draw(temp_node);
			}
			else{
			  temp_node=Nodes[node_map[temp_string.assign(red_diagram_string(temp_red))]];
			  temp_edge=new PSIL_Game_Edge;
			  temp_edge->sxi=i;
			  temp_edge->src=root;
			  temp_edge->dst=temp_node;
			  root->outs.push_back(temp_edge);
			  temp_node->ins.push_back(temp_edge);
			  cout<<root->index<<"--"<<temp_edge->sxi<<"-->"<<temp_node->index<<endl;
			}
		}
	}
}

Computation_Tree_Node::Computation_Tree_Node(){
	int i;
	G=new int[closure_COUNT];
	until_token=0;
	until_token_old=0;
	obligation=new int[closure_COUNT];
	pass_down=new int[closure_COUNT];
	passed=new bool[closure_COUNT];
	for(i=0;i<closure_COUNT;i++){
		G[i]=DONT_CARE;
		obligation[i]=0;
		pass_down[i]=0;
		passed[i]=false;
	}
}

Computation_Tree_Node::~Computation_Tree_Node(){
	delete G;
	delete obligation;
	delete pass_down;
	delete passed;
}


int find_next_closure(int x){
	if(Parse_Tree[x]->type==PARSE_ROOT || Parse_Tree[x]->type==PLUS || Parse_Tree[x]->type==MINUS){
		return find_next_closure(Parse_Tree[x]->outs[0]->index);
	}
	else {return Parse_Tree[x]->closure_index;}
}




void First_Guess(Computation_Tree_Node* R){
	int i,j;
	for(i=0;i<closure_COUNT;i++){
		R->G[i]=R->obligation[i];
	}
	for(i=0;i<closure_COUNT;i++){
		if(R->G[i]==MUST_TRUE){
			switch(Closure[i]->type){
				int left,right;
				case OR:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					R->G[left]=MUST_TRUE;
					R->G[right]=DONT_CARE;
					break;
				case AND:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=MUST_TRUE;
					R->G[right]=MUST_TRUE;
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					R->G[left]=DONT_CARE;
					R->G[right]=MUST_TRUE;
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					R->G[left]=DONT_CARE;
					R->G[right]=MUST_TRUE;
					break;
				case NEXT:
					R->G[i]=MUST_TRUE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=DONT_CARE;
					break;
				case NOT:
					R->G[i]=DONT_CARE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=MUST_FALSE;
					break;
				default:
					R->G[i]=MUST_TRUE;
			}
		}
		else if(R->G[i]==MUST_FALSE){
			switch(Closure[i]->type){
				int left,right;
				case OR:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=MUST_FALSE;
					R->G[right]=MUST_FALSE;
					break;
				case AND:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_1;
					R->G[left]=MUST_FALSE;
					R->G[right]=DONT_CARE;
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_1;
					R->G[left]=MUST_FALSE;
					R->G[right]=MUST_FALSE;
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=MUST_FALSE;
					R->G[right]=MUST_FALSE;
					break;
				case NEXT:
					R->G[i]=MUST_FALSE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=DONT_CARE;
					break;
				case NOT:
					R->G[i]=DONT_CARE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=MUST_TRUE;
					break;
				default:
					R->G[i]=MUST_FALSE;
			}
		}
	}
	cout<<"first guess:";
	for(i=0;i<closure_COUNT;i++){
		cout<<" "<<R->G[i];
	}
	cout<<endl;
}

bool Guess(Computation_Tree_Node* R){
	cout<<"guess:";
	int i,j;
	bool carry;
	carry=true;
	for(i=closure_COUNT-1;i>=0;i--){
		if(carry){
			if(R->G[i]==TRUE_GUESSED_PHASE_1){
				R->G[i]=TRUE_GUESSED_PHASE_2;
				carry=false;
			}
			else if(R->G[i]==TRUE_GUESSED_PHASE_2){
				R->G[i]=TRUE_GUESSED_PHASE_1;
				carry=true;
			}
			else if(R->G[i]==FALSE_GUESSED_PHASE_1){
				R->G[i]=FALSE_GUESSED_PHASE_2;
				carry=false;
			}
			else if(R->G[i]==FALSE_GUESSED_PHASE_2){
				R->G[i]=FALSE_GUESSED_PHASE_1;
				carry=true;
			}
		}
	}
	if(carry){
		cout<<"No other possibility"<<endl;
		return false;
	}
/*	for(i=0;i<closure_COUNT;i++){
        cout<<R->G[i]<<" ";
	}
	cout<<endl;
*/
	for(i=0;i<closure_COUNT;i++){
		if(R->G[i]==MUST_TRUE){
			switch(Closure[i]->type){
				int left,right;
				case OR:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					R->G[left]=MUST_TRUE;
					R->G[right]=DONT_CARE;
					break;
				case AND:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=MUST_TRUE;
					R->G[right]=MUST_TRUE;
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					R->G[left]=DONT_CARE;
					R->G[right]=MUST_TRUE;
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					R->G[left]=DONT_CARE;
					R->G[right]=MUST_TRUE;
					break;
				case NEXT:
					R->G[i]=MUST_TRUE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=DONT_CARE;
					break;
				case NOT:
					R->G[i]=DONT_CARE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=MUST_FALSE;
					break;
				default:
					R->G[i]=MUST_TRUE;
			}
		}
		else if(R->G[i]==MUST_FALSE){
			switch(Closure[i]->type){
				int left,right;
				case OR:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=MUST_FALSE;
					R->G[right]=MUST_FALSE;
					break;
				case AND:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_1;
					R->G[left]=MUST_FALSE;
					R->G[right]=DONT_CARE;
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_1;
					R->G[left]=MUST_FALSE;
					R->G[right]=MUST_FALSE;
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_1;
					R->G[left]=MUST_FALSE;
					R->G[right]=MUST_FALSE;
					break;
				case NEXT:
					R->G[i]=MUST_FALSE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=DONT_CARE;
					break;
				case NOT:
					R->G[i]=DONT_CARE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=MUST_TRUE;
					break;
				default:
					R->G[i]=MUST_FALSE;
			}
		}
		else if(R->G[i]==TRUE_GUESSED_PHASE_1){
			switch(Closure[i]->type){
				int left,right;
				case OR:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					if(R->G[left]==TRUE_GUESSED_PHASE_1 || R->G[left]==TRUE_GUESSED_PHASE_2
					||R->G[left]==FALSE_GUESSED_PHASE_1 || R->G[left]==FALSE_GUESSED_PHASE_2){
						R->G[left]=R->G[left];
					}
					else{R->G[left]=MUST_TRUE;}
					R->G[right]=DONT_CARE;
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					R->G[left]=DONT_CARE;
					if(R->G[right]==TRUE_GUESSED_PHASE_1 || R->G[right]==TRUE_GUESSED_PHASE_2
					||R->G[right]==FALSE_GUESSED_PHASE_1 || R->G[right]==FALSE_GUESSED_PHASE_2){
						R->G[right]=R->G[right];
					}
					else{R->G[right]=MUST_TRUE;}
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_1;
					R->G[left]=DONT_CARE;
					if(R->G[right]==TRUE_GUESSED_PHASE_1 || R->G[right]==TRUE_GUESSED_PHASE_2
					||R->G[right]==FALSE_GUESSED_PHASE_1 || R->G[right]==FALSE_GUESSED_PHASE_2){
						R->G[right]=R->G[right];
					}
					else{R->G[right]=MUST_TRUE;}
					break;
			}
		}
		else if(R->G[i]==TRUE_GUESSED_PHASE_2){
			switch(Closure[i]->type){
				int left,right;
				case OR:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_2;
					R->G[left]=DONT_CARE;
					if(R->G[right]==TRUE_GUESSED_PHASE_1 || R->G[right]==TRUE_GUESSED_PHASE_2
					||R->G[right]==FALSE_GUESSED_PHASE_1 || R->G[right]==FALSE_GUESSED_PHASE_2){
						R->G[right]=R->G[right];
					}
					else{R->G[right]=MUST_TRUE;}
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_2;
					if(R->G[left]==TRUE_GUESSED_PHASE_1 || R->G[left]==TRUE_GUESSED_PHASE_2
					||R->G[left]==FALSE_GUESSED_PHASE_1 || R->G[left]==FALSE_GUESSED_PHASE_2){
						R->G[left]=R->G[left];
					}
					else{R->G[left]=MUST_TRUE;}
					R->G[right]=DONT_CARE;
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=TRUE_GUESSED_PHASE_2;
					if(R->G[left]==TRUE_GUESSED_PHASE_1 || R->G[left]==TRUE_GUESSED_PHASE_2
					||R->G[left]==FALSE_GUESSED_PHASE_1 || R->G[left]==FALSE_GUESSED_PHASE_2){
						R->G[left]=R->G[left];
					}
					else{R->G[left]=MUST_TRUE;}
					R->G[right]=DONT_CARE;
					break;
			}
		}
		else if(R->G[i]==FALSE_GUESSED_PHASE_1){
			switch(Closure[i]->type){
				int left,right;
				case AND:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_1;
					if(R->G[left]==TRUE_GUESSED_PHASE_1 || R->G[left]==TRUE_GUESSED_PHASE_2
					||R->G[left]==FALSE_GUESSED_PHASE_1 || R->G[left]==FALSE_GUESSED_PHASE_2){
						R->G[left]=R->G[left];
					}
					else{R->G[left]=MUST_FALSE;}
					R->G[right]=DONT_CARE;
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_1;
					if(R->G[left]==TRUE_GUESSED_PHASE_1 || R->G[left]==TRUE_GUESSED_PHASE_2
					||R->G[left]==FALSE_GUESSED_PHASE_1 || R->G[left]==FALSE_GUESSED_PHASE_2){
						R->G[left]=R->G[left];
					}
					else{R->G[left]=MUST_FALSE;}
					if(R->G[right]==TRUE_GUESSED_PHASE_1 || R->G[right]==TRUE_GUESSED_PHASE_2
					||R->G[right]==FALSE_GUESSED_PHASE_1 || R->G[right]==FALSE_GUESSED_PHASE_2){
						R->G[right]=R->G[right];
					}
					else{R->G[right]=MUST_FALSE;}
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_1;
					if(R->G[left]==TRUE_GUESSED_PHASE_1 || R->G[left]==TRUE_GUESSED_PHASE_2
					||R->G[left]==FALSE_GUESSED_PHASE_1 || R->G[left]==FALSE_GUESSED_PHASE_2){
						R->G[left]=R->G[left];
					}
					else{R->G[left]=MUST_FALSE;}
					if(R->G[right]==TRUE_GUESSED_PHASE_1 || R->G[right]==TRUE_GUESSED_PHASE_2
					||R->G[right]==FALSE_GUESSED_PHASE_1 || R->G[right]==FALSE_GUESSED_PHASE_2){
						R->G[right]=R->G[right];
					}
					else{R->G[right]=MUST_FALSE;}										
					break;
			}
		}
		else if(R->G[i]==FALSE_GUESSED_PHASE_2){
			switch(Closure[i]->type){
				int left,right;
				case AND:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_2;
					R->G[left]=DONT_CARE;
					if(R->G[right]==TRUE_GUESSED_PHASE_1 || R->G[right]==TRUE_GUESSED_PHASE_2
					||R->G[right]==FALSE_GUESSED_PHASE_1 || R->G[right]==FALSE_GUESSED_PHASE_2){
						R->G[right]=R->G[right];
					}
					else{R->G[right]=MUST_FALSE;}
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_2;
					if(R->G[left]==TRUE_GUESSED_PHASE_1 || R->G[left]==TRUE_GUESSED_PHASE_2
					||R->G[left]==FALSE_GUESSED_PHASE_1 || R->G[left]==FALSE_GUESSED_PHASE_2){
						R->G[left]=R->G[left];
					}
					else{R->G[left]=MUST_TRUE;}
					if(R->G[right]==TRUE_GUESSED_PHASE_1 || R->G[right]==TRUE_GUESSED_PHASE_2
					||R->G[right]==FALSE_GUESSED_PHASE_1 || R->G[right]==FALSE_GUESSED_PHASE_2){
						R->G[right]=R->G[right];
					}
					else{R->G[right]=MUST_FALSE;}
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=FALSE_GUESSED_PHASE_2;
					if(R->G[left]==TRUE_GUESSED_PHASE_1 || R->G[left]==TRUE_GUESSED_PHASE_2
					||R->G[left]==FALSE_GUESSED_PHASE_1 || R->G[left]==FALSE_GUESSED_PHASE_2){
						R->G[left]=R->G[left];
					}
					else{R->G[left]=MUST_TRUE;}
					if(R->G[right]==TRUE_GUESSED_PHASE_1 || R->G[right]==TRUE_GUESSED_PHASE_2
					||R->G[right]==FALSE_GUESSED_PHASE_1 || R->G[right]==FALSE_GUESSED_PHASE_2){
						R->G[right]=R->G[right];
					}
					else{R->G[right]=MUST_FALSE;}
					break;
			}
		}
		else if(R->G[i]==DONT_CARE){
			switch(Closure[i]->type){
				int left,right;
				case OR:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=DONT_CARE;
					R->G[right]=DONT_CARE;
					break;
				case AND:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=DONT_CARE;
					R->G[right]=DONT_CARE;
					break;
				case UNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=DONT_CARE;
					R->G[right]=DONT_CARE;
					break;
				case WNTIL:
					left=find_next_closure(Closure[i]->outs[0]->index);
					right=find_next_closure(Closure[i]->outs[1]->index);
					R->G[i]=DONT_CARE;
					R->G[left]=DONT_CARE;
					R->G[right]=DONT_CARE;
					break;
				case NEXT:
					R->G[i]=DONT_CARE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=DONT_CARE;
					break;
				case NOT:
					R->G[i]=DONT_CARE;
					R->G[find_next_closure(Closure[i]->outs[0]->index)]=DONT_CARE;
					break;
				default:
					R->G[i]=DONT_CARE;
			}
		}
	}
	for(i=0;i<closure_COUNT;i++){
		cout<<" "<<R->G[i];
	}
	cout<<endl;
	return true;
}


int Check_Local(Computation_Tree_Node* R){
	cout<<"check local"<<endl;
	int i;
	int check_visited_result;
	check_visited_result=Check_Visited(R);
	if(check_visited_result==PASS){
		cout<<"check local PASS"<<endl;	
		return PASS;
	}
	else if(check_visited_result==FAIL){
		cout<<"check local fail"<<endl;			
		return FAIL;
	}
	else{
		for(i=closure_COUNT-1;i>=0;i--){
			if(R->G[i]!=DONT_CARE){
				switch(Closure[i]->type){
					int left,right;
					case TRUE_NODE:
						if(R->G[i]==MUST_FALSE){
							cout<<"check local fail"<<endl;						
							return FAIL;
						}
					break;
					case FALSE_NODE:
						if(R->G[i]==MUST_TRUE){
							cout<<"check local fail"<<endl;
							return FAIL;
						}
					break;
					case ATOMIC:
						char aaa[100];
						strcpy(aaa,Closure[i]->str.c_str());
						if(red_and(red_diagram(aaa),R->state->red)!=red_false()){
							if(R->G[i]==MUST_FALSE){
								cout<<"check local fail"<<endl;	
								return FAIL;
							}
						}
						else{
							if(R->G[i]==MUST_TRUE){
								cout<<"check local fail"<<endl;
								return FAIL;
							}
						}
					break;
				}
			}
		}
		cout<<"check local continue"<<endl;
		return CONTINUE;
	}
}

bool Create_Pass_Down(Computation_Tree_Node* R,bool &controlled){
	cout<<"create pass down:"<<endl;
	bool flag;
	flag=false;
	int i,j,x,y;
	for(i=0;i<closure_COUNT;i++){
		cout<<R->G[i]<<" ";
	}
	cout<<endl;
	for(i=0;i<closure_COUNT;i++){
		cout<<R->passed[i]<<" ";
	}
	cout<<endl;	
	for(i=0;i<closure_COUNT;i++){
		if(R->passed[i]==false && ((R->G[i]==MUST_TRUE && Closure[i]->type==NEXT)
			||(R->G[i]==MUST_FALSE && Closure[i]->type==NEXT)
			||(R->G[i]==TRUE_GUESSED_PHASE_2 &&(Closure[i]->type==UNTIL || Closure[i]->type==WNTIL))
			||(R->G[i]==FALSE_GUESSED_PHASE_2 && (Closure[i]->type==UNTIL || Closure[i]->type==WNTIL)))){
			flag=true;
			x=i;
			cout<<"closure "<<i;
			for(j=0;j<strategy_COUNT;j++){
				if(Matrix[j][i]==1 && strategy2owner[j]==R->state->owner){
					y=j;
					controlled=true;
					cout<<" controlled by strategy "<<j<<" of player "<<strategy2owner[j]<<endl;
				}
				else {
					controlled==false;
					cout<<" uncontrolled"<<endl;
				}
			}
		}
		if(flag==true){
			break;
		}
	}
	if(flag==false){ 
		cout<<"nothing to pass down"<<endl;
		return false;
	}
	cout<<x<<" "<<y<<endl;
	for(i=0;i<closure_COUNT;i++){
		R->pass_down[i]=DONT_CARE;
	}
	if(controlled==true){
		for(i=0;i<closure_COUNT;i++){
			if(Matrix[y][i]==1 && R->G[i]==MUST_TRUE && Closure[i]->type==NEXT && R->passed[i]==false){
				cout<<"kkk"<<endl;				
        R->pass_down[i]=DONT_CARE;
        R->pass_down[find_next_closure(Closure[i]->outs[0]->index)]=MUST_TRUE;
        R->passed[i]=true;
			}
      else if(Matrix[y][i]==1 && R->G[i]==TRUE_GUESSED_PHASE_2 && (Closure[i]->type==UNTIL || Closure[i]->type==WNTIL)&& R->passed[i]==false){
				R->pass_down[i]=MUST_TRUE;
        R->passed[i]=true;
			}
			else if(Matrix[y][i]==1 && R->G[i]==MUST_FALSE && Closure[i]->type==NEXT&& R->passed[i]==false){
        R->pass_down[i]=DONT_CARE;
        R->pass_down[find_next_closure(Closure[i]->outs[0]->index)]=MUST_FALSE;
        R->passed[i]=true;
	    }
      else if(Matrix[y][i]==1 && R->G[i]==FALSE_GUESSED_PHASE_2 && (Closure[i]->type==UNTIL || Closure[i]->type==WNTIL) && R->passed[i]==false){
		    R->pass_down[i]=MUST_FALSE;
        R->passed[i]=true;
			}
		}
	}
	else{
		for(i=0;i<closure_COUNT;i++){
			R->pass_down[i]=DONT_CARE;
		}
    if(R->G[x]==MUST_TRUE && Closure[x]->type==NEXT && R->passed[i]==false){
      R->pass_down[x]=DONT_CARE;
      R->pass_down[find_next_closure(Closure[x]->outs[0]->index)]=MUST_TRUE;
			R->passed[x]=true;
    }
    else if(R->G[x]==TRUE_GUESSED_PHASE_2 && (Closure[x]->type==UNTIL || Closure[x]->type==WNTIL) && R->passed[i]==false){
      R->pass_down[x]=MUST_TRUE;
			R->passed[x]=true;
    }
    else if(R->G[x]==MUST_FALSE && Closure[x]->type==NEXT && R->passed[i]==false){
      R->pass_down[x]=DONT_CARE;
      R->pass_down[find_next_closure(Closure[x]->outs[0]->index)]=MUST_FALSE;
			R->passed[x]=true;
    }
    else if(R->G[x]==FALSE_GUESSED_PHASE_2 && (Closure[i]->type==UNTIL || Closure[i]->type==WNTIL) && R->passed[i]==false){
      R->pass_down[x]=MUST_FALSE;
			R->passed[x]=true;
    }
	}
	cout<<"pass down created:";	
	for(i=0;i<closure_COUNT;i++){
		cout<<R->pass_down[i]<<" ";
	}
	cout<<endl;
	for(i=0;i<closure_COUNT;i++){
		cout<<R->passed[i]<<" ";
	}
	cout<<endl;
	return true;
}


bool Check_PSIL(Computation_Tree_Node* R){
	int i,j;
	int local_result;	
	cout<<"check PSIL on:"<<red_diagram_string(R->state->red)<<endl;
	cout<<"obligation:";
	for(i=0;i<closure_COUNT;i++){
  	cout<<R->obligation[i]<<" ";
	}
	cout<<endl;
	First_Guess(R);

	local_result=Check_Local(R);
	if(local_result==PASS){
		cout<<"Check_PSIL true"<<endl;
		return true;
	}
	else if(local_result==FAIL){
		while(Guess(R)){
			local_result=Check_Local(R);
			if(local_result==PASS){
				cout<<"Check_PSIL true"<<endl;
				return true;
			}			
			else if(local_result==CONTINUE){
				for(i=0;i<closure_COUNT;i++){
					R->passed[i]=false;
				}
				bool controlled=false;
		    bool guess_fail;
		    guess_fail=false;
				while(Create_Pass_Down(R,controlled) && !guess_fail){
					bool pass_down_success;
					pass_down_success=false;
					for(i=0;i<R->state->outs.size();i++){
	          if(!pass_down_success){
				    	cout<<"check child "<<i+1<<"/"<<R->state->outs.size()<<endl;
	            Computation_Tree_Node* R2=new Computation_Tree_Node;
	            R2->state=R->state->outs[i]->dst;
	            R2->ins=R;
	            R->outs.push_back(R2);
	            for(j=0;j<closure_COUNT;j++){
		            R2->obligation[j]=R->pass_down[j];
	            }
	            R2->until_token_old=R->until_token;
	            if(controlled){
		            if(Check_PSIL(R2)){
	                guess_fail=false;
	                pass_down_success=true;
		            }
	            } 
	            else{
	              if(!Check_PSIL(R2)){
	                guess_fail=true;
	                cout<<"guess_fail"<<endl;
	              }
	            }
						}
					}
					if(controlled && !pass_down_success){guess_fail=true;}
				}
				if(!guess_fail){
	      	cout<<"guess success"<<endl;
	        return true;
				}
			}
			else{
        cout<<"guess_fail"<<endl;
      }
		}
	}
	else if(local_result==CONTINUE){
		for(i=0;i<closure_COUNT;i++){
			R->passed[i]=false;
		}
    bool controlled=false;
    bool guess_fail;
    guess_fail=false;		
		while(Create_Pass_Down(R,controlled) && !guess_fail){
			bool pass_down_success;
			pass_down_success=false;
			for(i=0;i<R->state->outs.size();i++){
        if(!pass_down_success){
		    	cout<<"check child "<<i+1<<"/"<<R->state->outs.size()<<endl;
          Computation_Tree_Node* R2=new Computation_Tree_Node;
          R2->state=R->state->outs[i]->dst;
          R2->ins=R;
          R->outs.push_back(R2);
          for(j=0;j<closure_COUNT;j++){
            R2->obligation[j]=R->pass_down[j];
          }
         	R2->until_token_old=R->until_token;
          if(controlled){
            if(Check_PSIL(R2)){
              guess_fail=false;
              pass_down_success=true;
            }
          }
          else{
            if(!Check_PSIL(R2)){
              guess_fail=true;
              cout<<"guess_fail"<<endl;
            }
          }
				}
			}
			if(controlled && !pass_down_success){guess_fail=true;}
		}
    if(!guess_fail){
      cout<<"guess success"<<endl;
      return true;
    }
    else{
	    guess_fail=false;
      while(Guess(R)){
				local_result=Check_Local(R);
				if(local_result==PASS){
					cout<<"Check_PSIL true"<<endl;
					return true;
				}			
				else if(local_result==CONTINUE){
					for(i=0;i<closure_COUNT;i++){
						R->passed[i]=false;
					}
	        bool controlled=false;
					while(Create_Pass_Down(R,controlled) && !guess_fail){
						bool pass_down_success;
						pass_down_success=false;
						for(i=0;i<R->state->outs.size();i++){
			        if(!pass_down_success){
					    	cout<<"check child "<<i+1<<"/"<<R->state->outs.size()<<endl;
			          Computation_Tree_Node* R2=new Computation_Tree_Node;
			          R2->state=R->state->outs[i]->dst;
			          R2->ins=R;
			          R->outs.push_back(R2);
			          for(j=0;j<closure_COUNT;j++){
			            R2->obligation[j]=R->pass_down[j];
			          }
	            	R2->until_token_old=R->until_token;
			          if(controlled){
			            if(Check_PSIL(R2)){
			              guess_fail=false;
			              pass_down_success=true;
			            }
			          }
			          else{
			            if(!Check_PSIL(R2)){
			              guess_fail=true;
			              cout<<"guess_fail"<<endl;
			            }
			          }
							}
						}
						if(controlled && !pass_down_success){guess_fail=true;}
					}
        	if(!guess_fail){
        		cout<<"guess success"<<endl;
        		return true;
        	}
      	}
      	else{
          cout<<"guess_fail"<<endl;
        }
      }
    }
	}
}









int main(int argc, char** argv){
	node_COUNT=1;
	sxi_COUNT=0;

	int i,j,k;
	int lb,hb;


	if(argc!=3){cout<<"Parameter error!!"<<endl;}
	formula_file.open(argv[2],ios::in);

	PSIL_Formula* F;
	F=new PSIL_Formula;



	red_begin_session(RED_SYSTEM_TIMED, argv[1], -1);	// -P n; n = process number, -1 == default
	red_input_model(argv[1], RED_REFINE_GLOBAL_INVARIANCE);
	red_set_sync_bulk_depth(3);
	//read in the model

	PSIL_Game_Node* root;
	root=new PSIL_Game_Node;
	Nodes.push_back(root);  //dummy one
	Nodes.push_back(root);
	root->red=red_query_diagram_initial();
	red_get_cube_discrete_value(root->red,input_owner,&lb,&hb);
	root->owner=lb;
	root->index= node_COUNT; node_COUNT++;
	node_map[temp_string.assign(red_diagram_string(root->red))]=root->index;
	path=red_query_diagram_enhanced_global_invariance();
	sxi_COUNT=red_query_sync_xtion_count(RED_USE_DECLARED_SYNC_XTION);
	//initial variables


	cout<<endl;
	Draw(root);

	//Draw the graph
  cout<<"DRAW DONE"<<endl;

	Setup_PSIL_Formula();
	print_parse_tree(Parse_Tree[0],0);
	//setup formula

	Computation_Tree_Node* R =new Computation_Tree_Node;
	R->ins=NULL;
	R->state=root;
	R->until_token_old=0;
	for(i=0;i<closure_COUNT;i++){
		R->obligation[i]=DONT_CARE;
	}
	R->obligation[0]=MUST_TRUE;

	//Init computation tree

	Check_PSIL(R);







	//Check_PSIL

	red_end_session();
	return 0;
}
