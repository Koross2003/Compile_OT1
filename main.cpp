#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<ctime>
#include<cstring>
#include<string>
#include<stack>
#include<sstream>
#include<vector>
#include<queue>
#include<set>
#include<algorithm>

using namespace std;

const int MAX=1024;
int T[MAX][MAX];

class NFA_Node;
/*NFA边*/
class NFA_Edge
{
	public:
		char char_ID; // 这条边的字符
		int ID; //边的ID,表示的是使用正则串的哪一部分进行的转换,例如0a&a*a&a*&aa+&?&a+a&a+&|&a?& 中每个a的ID都不同,第一个0的ID是1,第一个a的ID是2,第二个a的ID是4...
		NFA_Node* des; // 指向的Node
		NFA_Edge(int ID,char char_ID,NFA_Node* des)
		{
		    this->ID=ID;
			this->char_ID=char_ID;
			this->des=des;
		}
};

/*NFA节点*/
class NFA_Node
{
	public:
		int stateID; // 节点ID
		vector<NFA_Edge*> t; // 节点的边
		bool visit; 
		NFA_Node(int stateID)
		{
			visit=false;
			this->stateID=stateID;
		}
		void AddNFA_Edge(NFA_Edge* tt) // 添加边
		{
			t.push_back(tt);
		}

};

/*NFA图*/
class NFA
{
	public:
		NFA_Node* start; // 起始节点
		NFA_Node* end;// 结束节点
		NFA(){}
		NFA(int Start_ID,int ID,char c) // Start_ID: 起始节点ID, ID: 边的ID, c: 边的字符, 实际上这个边的ID就对应着字符集中的某个字符,是唯一确定的
		{
			NFA_Node* s1=new NFA_Node(Start_ID);
			NFA_Node* s2=new NFA_Node(Start_ID+1);
			NFA_Edge* tt=new NFA_Edge(ID,c,s2);
			s1->AddNFA_Edge(tt);
			start=s1;
			end=s2;
		}

};

/*将正则表达式转到NFA*/
class Regex2NFA
{
 	public:
        int State_ID;
        int C[MAX]; // 这个可以看作是某个字符的边数,比如C['a']表示上面字符是a的有几条边
        vector <char>* char_set;
        Regex2NFA(string str, vector <char>* P)
        {
            preprocess(str); // 添加连接运算符
            ToPostfix(this->lamb); // 转为后缀表达式
            State_ID=1;
            char_set=P;
        }
        Regex2NFA(){State_ID=1;}

        NFA ToNFA()
        {

            NFA tempb,tempb1,tempb2;

            char cur_char;
            int i=0;

            while (i<lamb.size())
            {
                cur_char = lamb[i];
                if (isOperator(cur_char))
                {
                    switch (cur_char)
                    {
                        case '|':
                            tempb1 = NFA_Stack.top();
                            NFA_Stack.pop();
                            tempb2 = NFA_Stack.top();
                            NFA_Stack.pop();
                            tempb1=Union(tempb2,tempb1);
                            NFA_Stack.push(tempb1);
                            break;
                        case '&':
                            tempb1 = NFA_Stack.top();
                            NFA_Stack.pop();
                            tempb2 = NFA_Stack.top();
                            NFA_Stack.pop();
                            tempb2=Connect(tempb1,tempb2);
                            NFA_Stack.push(tempb2);
                            break;
                        case '*':
                            tempb1 = NFA_Stack.top();
                            NFA_Stack.pop();
                            tempb1=Closure(tempb1,cur_char);
                            NFA_Stack.push(tempb1);
                            break;
                        case '?':
                            tempb1 = NFA_Stack.top();
                            NFA_Stack.pop();
                            tempb1=que_Closure(tempb1);
                            NFA_Stack.push(tempb1);
                            break;
                        case '+':
                            tempb1 = NFA_Stack.top();
                            NFA_Stack.pop();
                            tempb1=add_Closure(tempb1,cur_char);
                            NFA_Stack.push(tempb1);
                            break;
                    }
                }
                else
                {
                    /*
                    //T[cur_char][C[cur_char]++]中, 
                    T[cur_char][C[cur_char]++]是现在读取到的字符c是整个正则串的第多少个
                    [cur_char]是字符cur_char
                    [C[cur_char]++]是现在读取到的这个字符c是相同字符的第C[cur_char]出现
                    C[cur_char]是字符cur_char的数量,从0开始,每次又遇到这个字符的边就++
                    */
                    //cout<<endl<<endl<<endl<< cur_char <<" nihao "<<C[cur_char] <<" wocao "<< (int)T[cur_char][C[cur_char]] <<endl<<endl<<endl;
                    tempb = NFA(State_ID,char_set[cur_char][C[cur_char]++],cur_char); // 给这个新字符创造一个新的NFA,这个NFA的长相是,State_ID作为起始节点, 只有一条边,ID是中间那个参数,边上的字符是最后那个参数
                    State_ID+=2; // 由于tempb中新增的是两个nfa节点
                    NFA_Stack.push(tempb); // 把这个新的nfa状态push进去,当之后读到运算符会再弹出进行处理
                }
                i++;
            }
            tempb = NFA_Stack.top();
            NFA_Stack.pop();
            return tempb;
        }


 	private:
        stack<NFA> NFA_Stack;
        stack<char> Operator_Stack;
        string lamb; // 添加连接字符后的正则串

        bool isOperator(char c)
        {
            switch (c)
            {
                case '|':
                case '&':
                case '(':
                case ')':
                case '!':
                case '+':
                case '?':
                case '*': return true;
                default: return false;
            }
        }

        bool isNewOperator(char c)
        {
            switch (c)
            {
                case '|':
                case '&':
                case '(':
                case ')':
                case '!':
                case '+':
                case '?':
                case '*': return true;
                default: return false;
            }
        }

        bool isTwoOperator(char c)
        {
            switch (c)
            {
                case '+':
                case '?':
                case '*': return true;
                default: return false;
            }
        }


        int getOperatorNumber(char t1) // 获取优先级
        {
            switch (t1)
            {
                case '$': return 0;
                case '!': return 1;
                case ')': return 2;
                case '|': return 3;
                case '&': return 4;
                case '*': return 5;
                case '+': return 6;
                case '?': return 7;
                case '(': return 8;
                default: return 9;
            }
        }

        bool Operator_Less_Than(char t1, char t2) // 比较两个运算符优先级
        {
            int temp1 = getOperatorNumber(t1);
            int temp2 = getOperatorNumber(t2);
            if (temp1 <= temp2)
                return true;
            return false;
        }


        void preprocess(string str) // 添加连接符
        {
            int i=0;
            char c,pc;
            bool can_add=true;

            memset(C,0,sizeof(C)); //  C 中存储的是某个char一共有几条边,比如C['a']表示字符a一共有几条边
            pc=str[i];
            c=str[++i];
            while(str[i]!='\0')
            {

                    // 如果满足以下几种情况之一，则需要在当前位置插入 &,表示连接
                    /*
                    1. )和(
                    2. 两个字符都不是正则的保留字符 
                    3. 非正则保留字 和 (
                    4. ) 和 非正则保留字
                    5. 正则保留字 和 非双目保留字({,},+,?,*,} 除了这几个符号之外的符号都是要运算左右两侧的)
                    6. 正则保留字 和 (
                    */
                if((pc==')'&&c=='(')||(!isNewOperator(pc)&&!isNewOperator(c))||(!isNewOperator(pc)&&c=='(')||(pc==')'&&!isNewOperator(c))||(isTwoOperator(pc)&&!isNewOperator(c))||(isTwoOperator(pc)&&c=='('))
                    str.insert(i,"&");
                pc=str[i++];
                c=str[i];
            }
            str+="!"; // 在字符串末尾添加 !
            this->lamb=str; // 将处理后的字符串赋值给成员变量 lamb
        }

        void ToPostfix(string lamb) // 将正则表达式转换为后缀表达式
        {
            string l="";
            Operator_Stack.push('$');
            char tempc,tempc2;

            for(int i=0;i<(int)lamb.size();i++)
            {
                tempc = lamb[i];
                if (isOperator(tempc))
                {
                    switch (tempc)
                    {
                        case '(': Operator_Stack.push(tempc); break;
                        case ')':
                            while (Operator_Stack.top() != '(')
                            {
                                tempc2 = Operator_Stack.top();
                                Operator_Stack.pop();
                                l += tempc2;
                            }
                            Operator_Stack.pop();
                            break;
                        default :
                            tempc2 = Operator_Stack.top();
                            while (tempc2!='('&&Operator_Less_Than(tempc,tempc2))
                            {
                                tempc2 = Operator_Stack.top();
                                Operator_Stack.pop();
                                l += tempc2;
                                tempc2 = Operator_Stack.top();
                            }
                            Operator_Stack.push(tempc);
                            break;
                    }
                }
                else
                    l += tempc;
            }
            this->lamb=l;
            //cout<<"here is lamb: "<<lamb<<endl;
        }
        /* 处理 &*/
        // 把G1接到G2后面
        NFA Connect(NFA G1, NFA G2)
        {
			NFA_Edge* t=new NFA_Edge(-1,'@',G1.start); // 保留符相关的边都是@
			G2.end->AddNFA_Edge(t)	;
  			G2.end = G1.end;
  			return G2;
		}
		/* 处理 |*/
        NFA& Union(NFA G1, NFA G2)
        {
			NFA_Node* n1=new NFA_Node(State_ID++); 
			NFA_Edge* t1=new NFA_Edge(-1,'@',G1.start);
			NFA_Edge* t2=new NFA_Edge(-1,'@',G2.start);
			n1->AddNFA_Edge(t1);
			n1->AddNFA_Edge(t2);

			NFA_Node* n2=new NFA_Node(State_ID++);
			NFA_Edge* t3=new NFA_Edge(-1,'@',n2);
			NFA_Edge* t4=new NFA_Edge(-1,'@',n2);
			G1.end->AddNFA_Edge(t3);
			G2.end->AddNFA_Edge(t4);

			NFA* G=new NFA(); // 一个或要添加一个新的NFA
			G->start=n1;
			G->end=n2;

			return *G;
		}
		/* 处理 * */
        NFA& Closure(NFA G2,char temp)
        {
            NFA_Edge* t=new NFA_Edge(-1,'#',G2.start);
			G2.end->AddNFA_Edge(t);

			NFA_Node* n1=new NFA_Node(State_ID++);
			NFA_Edge* t1=new NFA_Edge(-1,'@',n1);
			G2.end->AddNFA_Edge(t1);

			NFA_Node* n2=new NFA_Node(State_ID++);
			NFA_Edge* t2=new NFA_Edge(-1,'@',G2.start);
			n2->AddNFA_Edge(t2);


			NFA_Edge* t3=new NFA_Edge(-1,'@',n1);
			n2->AddNFA_Edge(t3);

			NFA* G=new NFA();
			G->start=n2;
			G->end=n1;

  			return *G;
		}
        /* 处理 ? */
		NFA& que_Closure(NFA G2)
        {

			NFA_Node* n1=new NFA_Node(State_ID++);
			NFA_Edge* t1=new NFA_Edge(-1,'@',n1);
			G2.end->AddNFA_Edge(t1);

			NFA_Node* n2=new NFA_Node(State_ID++);
			NFA_Edge* t2=new NFA_Edge(-1,'@',G2.start);
			n2->AddNFA_Edge(t2);


			NFA_Edge* t3=new NFA_Edge(-1,'@',n1);
			n2->AddNFA_Edge(t3);

			NFA* G=new NFA();
			G->start=n2;
			G->end=n1;

  			return *G;
		}
        /* 处理 + */
		NFA& add_Closure(NFA G2,char temp)
        {
			// NFA_Edge* t=new NFA_Edge(T[temp][C[temp]],'#',G2.start);
            NFA_Edge* t=new NFA_Edge(-1,'#',G2.start);
			G2.end->AddNFA_Edge(t);

			NFA_Node* n1=new NFA_Node(State_ID++);
			NFA_Edge* t1=new NFA_Edge(-1,'@',n1);
			G2.end->AddNFA_Edge(t1);

			NFA_Node* n2=new NFA_Node(State_ID++);
			NFA_Edge* t2=new NFA_Edge(-1,'@',G2.start);
			n2->AddNFA_Edge(t2);

			NFA* G=new NFA();
			G->start=n2;
			G->end=n1;

  			return *G;
		}
};

/*生成字符集*/

void getCharSet(NFA &G,set<int> &S,vector<NFA_Node*> &V)
{
	queue<NFA_Node*> char_queue;
	char_queue.push(G.start);
	char temp_char;
	while(!char_queue.empty())
	{
		NFA_Node* tmp=char_queue.front();
		char_queue.pop();
		V.push_back(tmp);
		tmp->visit=true;

		if(tmp->t.size()>0)
		for(int i =0;i<tmp->t.size();i++)
		{
			temp_char=tmp->t[i]->char_ID;
			if(temp_char!='@' && temp_char!='#')
            {
                //cout<<tmp->t[i]->ID<<" "<<endl;
                S.insert(tmp->t[i]->ID);
            }

			if(temp_char=='#') {} // # 对应闭包的空边,push进来会陷入死循环
            else
            {
                if(tmp->t[i]->des->visit==false)
                {
                    char_queue.push(tmp->t[i]->des);
                    tmp->t[i]->des->visit=true;
                }
            }
		}
	}
}

class DFA_Edge;

/*DFA节点*/
class DFA_Node
{
	public:
		int stateID; // DFA节点ID
		vector<DFA_Edge*> t; // 节点边
		vector<int> inc; // 这个dfa节点包含了哪些nfa节点
		bool flag; // 表示这个节点是不是结束态,为true就是结束状态
		DFA_Node(int s)
		{
			flag=false;
			stateID=s;
		}

		DFA_Node()
		{

		}
};

/*DFA边*/
class DFA_Edge
{
	public:
		int char_ID;
		DFA_Node* des;
		bool visit;
		DFA_Edge(int a,DFA_Node* &b)
		{
			char_ID=a;
			des=b;
			visit=false;
		}
};


/*NFA转到DFA*/
class NFA2DFA
{
	public:
		int MaxStatus; // 最多有多少个状态,实际上就是nfa图中状态数量
		int visit[MAX];
		vector<NFA_Node*> tmp;
		set<int> CharSet;
		vector<NFA_Node*> nfa;
		NFA_Node* start;
		vector<DFA_Node*> dfa;
		int nfa_end; // 这个是nfa中结束状态的ID,可以用来判断一个dfa节点是不是结束状态

		NFA2DFA()
		{

		}

        // 参数列表: nfa中的状态数量作为dfa中最多的状态数,nfa的起始节点, nfa中结束状态的ID
        void ready(int max,NFA_Node* &S,int nfa_end)
		{
			this->MaxStatus=max;
			this->start=S;
			this->nfa_end=nfa_end;
		}

		void clear_all()
		{
		    Init();
			for(int i=0;i<=MAX;i++)
			for(int j=0;j<=MAX;j++)
			T[i][j]=0;
			CharSet.clear();
			nfa.clear();
			dfa.clear();
		}

		void Init()
		{
			for(int i=0;i<=this->MaxStatus;i++)
			{
				this->visit[i]=0;
			}
			this->tmp.clear();
		}

		NFA_Node*& find_nfa(int st) //在NFA中寻找从st出发的边
		{
			for(int i=0;i<nfa.size();i++)
			if(nfa[i]->stateID==st) return nfa[i];
		}

		DFA_Node*& find_dfa(int st) // 在DFA中寻找从st出发的边
		{
			for(int i=0;i<dfa.size();i++)
			if(dfa[i]->stateID==st) return dfa[i];
		}

		void find_closure(NFA_Node* p) // 寻找从p出发的ε闭包
		{
			visit[p->stateID]=1;
			visit[0]++;
			if(p->t.size()==0) return;
			for(int i=0;i<p->t.size();i++)
			{
				if(!visit[p->t[i]->des->stateID]&&(p->t[i]->char_ID=='#'||p->t[i]->char_ID=='@'))
				find_closure(p->t[i]->des);
			}
		}

		void closure()
		{
			for(int i=0;i<tmp.size();i++)
			{
				find_closure(tmp[i]);
			}
		}

        // 运行find_closure()之后的状态就是, visit[0]记录这次闭包访问了多少个状态, visit[i]记录了状态i是否被访问过
        
        int AddDFAStatus() // 添加DFA状态
        {
            if(visit[0]==0) // 如果访问过状态为0的则返回-2
                return -2;
            //接下来找这个状态集是不是已经出现过了
            for(int i=1;i<=T[0][0];i++)  // T[i][j]表示第i个DFA状态中有哪些NFA状态(可以理解为第i个状态集中有哪些状态) , T[i][0]存储的是第i个状态集中有多少个状态, T[0][0]存储的是一共有多少个DFA状态(或者说是有多少个状态集)
            {   
                // visit[i]表示第i个NFA状态在不在这个状态集中,visit[0]存储了这个状态集中有多少个状态
                if(visit[0]!=T[i][0]) // 首先如果数量都不想等,一定不是我们要找的状态集
                    continue;
                
                // 如果数量相等了,接下来检查状态集中状态是不是完全一样
                int j=1;
                for(;j<=MaxStatus;j++) // 遍历所有状态
                    if(visit[j]!=T[i][j]) // 如果存在某个状态不一样,就说明不是相同的状态集
                        break;
        
                // 如果上面这个j走到了最后,就说明这两个状态集完全一样,那就不创建新的状态集(也就是DFA节点)了,返回对应的状态集就行了
                if(j==(MaxStatus+1))// 如果visit和T[i]是同一组，则返回i
                    return i;
            }
            // 如果上面的循环走完了,说明这个状态集是一个新的状态集,需要创建一个新的DFA节点
            T[0][0]++; // T[0][0]记录DFA状态的数量(也就是状态集的数量)
            for(int i=0;i<=MaxStatus;i++)
                T[T[0][0]][i]=visit[i]; // 复制这个新的状态集
            return -1;
        
        }

		void moveto(int st,int c)
		{

			for(int i=1;i<=MaxStatus;i++) // 遍历状态集中的每一个状态,看看他吃入字符c后能到达哪个状态
			{
				if(T[st][i]) // T[st][i]表示状态集st中的状态i(为1说明这个状态集中有这个状态)
				{
					NFA_Node* p=find_nfa(i);
					if(p->t.size()>0)
					for(int j=0;j<p->t.size();j++)
					if(p->t[j]->ID==c)
					{tmp.push_back(p->t[j]->des);} // 把到达的状态加入到tmp中
				}
			}
		}
		/*转换算法*/
		void ToDFA()
		{
			int i,j;

            // 遍历起始的start
			find_closure(start);
			AddDFAStatus();

			DFA_Node* s1=new DFA_Node(1);
			if(visit[nfa_end]) s1->flag=true; // 如果这个dfa状态包含nfa的结束态,则这个dfa状态也是结束态

            // 把这个DFA节点对应的NFA节点记录下来
			for(i=1;i<=MaxStatus;i++)
			if(visit[i]) s1->inc.push_back(i); 

            // 添加这个dfa节点
			dfa.push_back(s1);
			Init();

            // 遍历所有状态集,看他们通过状态转换都能到达哪里
			for(i=1;i<=T[0][0];i++)
			{
                // 遍历字符表,看这个状态集吃入字符后能到达哪里
				for(set<int>::iterator t1=CharSet.begin();t1!=CharSet.end();t1++)
				{
					moveto(i,*t1); // moveto之后吃掉这个字符能达到的状态都已经装进tem中了
					closure(); // 接着算tem的闭包
					if((j=AddDFAStatus())>=0) // 如果j>=0,说明这个状态集(或者叫dfa节点)吃掉这个字符转向了一个已经存在的状态(或者叫dfa节点)
					{
                        // 那么就添加一条边就好了
						DFA_Edge* e1=new DFA_Edge(*t1,find_dfa(j)); // 新建一条边
						find_dfa(i)->t.push_back(e1); // 把这条边添加给状态集i

					}
					else if(j==-1) // 如果j==-1,说明有新增的状态集(或者叫dfa节点)
					{
						DFA_Node* d1=new DFA_Node(T[0][0]); // 创建一个新的dfa节点,这个节点的id就是这个状态集的id
						if(visit[nfa_end]) 
                        {
                            d1->flag=true; // 如果这个状态集里有nfa的终态,则这个dfa也设置为终态
                        }
						for(int tt=1;tt<=MaxStatus;tt++) // 遍历状态集
                        {
                            if(visit[tt]) 
                                d1->inc.push_back(tt);// 把这个状态集中存在的状态保存到dfa节点对应的成员中
                        }
						dfa.push_back(d1); // 把新增的dfa节点添加进dfa图中
						DFA_Edge* e1=new DFA_Edge(*t1,find_dfa(T[0][0])); // 添加一条指向新增dfa节点的边
						find_dfa(i)->t.push_back(e1); // 这个边给状态i(或者叫dfa节点i)
					}

					Init(); // 每一次检测结束之后,都要把visited和tem清空
				}
			}
            resetID(); // 重置dfa节点的id
		}

        void resetID()
        {
            for(int i=0;i<dfa.size();i++)
                dfa[i]->stateID=i;
        }

};

class DFA2MinDFA
{
    public:
        NFA2DFA * ntd;
        vector<DFA_Node*> min_dfa;
        
        DFA2MinDFA(NFA2DFA* n)
        {
            init(n);
        }
        DFA2MinDFA()
        {
            
        }
        void ToMinDFA()
        {
            Hopcroft();
            min_dfa.clear();

            for(int i = 0;i < t;i++)
            {
                DFA_Node* d1=new DFA_Node(i);
                int s = Status[i];
                // 查看是否为结束态
                for(int j = 0;j < v;j++)
                {
                    if((s>>j)&1)
                    {//遍历集合中的状态
                        if(ntd->dfa[j]->flag)
                            d1->flag=true;
                    } 
                }
                min_dfa.push_back(d1);
            }

            for(int i = 0;i < t;i++)
            {
                for(set<int>::iterator t1=ntd->CharSet.begin(); t1!=ntd->CharSet.end();t1++)
                {
                    DFA_Node* d1 = min_dfa[i];
                    int des = move_to(i,*t1);
                    if(des!=-1)
                    {
                        DFA_Edge* e1=new DFA_Edge(*t1,min_dfa[des]);
                        d1->t.push_back(e1);
                    }
                }
            }
        }



    private:

        int v;//v存储有多少状态
        int t = 2;//默认可分为N和A
        int p[MAX]; //记录状态属于哪个集合 
        vector<int> Status; // 保存状态集, 可以理解为某个状态集中有哪些状态Status[i]表示状态集i中有哪些状态
        void init(NFA2DFA *n)
        {
            ntd = n;
            v = ntd->dfa.size();
        }
        // 注意处理空集!!!
        void split(int x, int c)
        {
            int s = Status[x]; // s是要分割的状态集中的状态,可以理解为第一位为1则状态1在集合中,为0则状态1不在集合中
            int a[MAX];//吃完字符达到的状态集 
            int b[MAX];//对应a中分割的吃字符前的状态 
            memset(a,0,sizeof(a));
            memset(b,0,sizeof(b));
            int i,j,k;
            for(i = 0;i < v;i++){
                if((s>>i)&1){//遍历集合中的状态
                    bool find = false;
                    for(j = 0;j < ntd->dfa[i]->t.size();j++){
                        if(ntd->dfa[i]->t[j]->char_ID == c){ // 如果从状态i吃入字符c
                            int v = ntd->dfa[i]->t[j]->des->stateID; // v是状态i吃入字符c后到达的状态
                            // p[v]是状态v所属的状态集
                            a[p[v]] |= (1<<v);// (可以表示状态i吃完字符达到状态v,且能达到这个状态集是因为状态v)
                            b[p[v]] |= (1<<i);// (可以表示状态i吃完字符达到状态v,且能达到这个状态集是因为状态i)
                            // 上面这两个可以得知,到达某一个状态集p[v],是由哪些状态(存储在b[p[v]]中)到达哪些状态(存储在a[p[v]]中)进入这个状态集(p[v])的
                            find = true;
                        }
                    }
                    if(!find)
                    {
                        b[MAX-1] |= (1<<i);
                    }
                } 
            }
            int ll = t; // 当前有多少状态集
            for(i = 0;i < ll;i++){ // 遍历所有状态集
                if(b[i]==Status[x]) break; // b[i]是到达状态集i有哪些状态,如果b[i]和要分割的状态集相同,说明这些状态都转向了同一个状态集,没有发生区分,所以退出
                if(a[i] && i!=x){ // 如果a[i]不为空,说明有状态转向了状态集i,且i不是要分割的状态集
                    Status.push_back(b[i]); // b[i]这个状态集分割出来作为一个新的状态集,因为这些状态导向了状态集i
                    // 修改被分割的状态所属的状态集
                    for(k = 0;k < v;k++) // 遍历所有状态
                        if((b[i]>>k)&1) // 如果这个状态在b[i]中,则分割出他们
                        p[k] = t; // 这个状态所属的状态集在t处
                    Status[x]&=(~b[i]); // 原来的状态集要把已经分割出去的状态清除
                    t++;
                }
            }
            // 处理空集
            if(b[MAX-1]!=0 && b[MAX-1]!=Status[x])
            {
                Status.push_back(b[MAX-1]); // b[MAX-1]这个状态集分割出来作为一个新的状态集,因为这些状态导向了空状态
                    // 修改被分割的状态所属的状态集
                    for(k = 0;k < v;k++) // 遍历所有状态
                        if((b[MAX-1]>>k)&1) // 如果这个状态在b[i]中,则分割出他们
                        p[k] = t; // 这个状态所属的状态集在t处
                    Status[x]&=(~b[MAX-1]); // 原来的状态集要把已经分割出去的状态清除
                    t++;
            }
        }

        void Hopcroft()
        {
            int N = 0,A = 0;
            for(int i = 0;i < v;i++){//分成N和A两个集合 
                if(!ntd->dfa[i]->flag){// 如果flag是0,说明这个状态不是一个结束态,则把他分割进A中
                    A |= (1<<ntd->dfa[i]->stateID);
                    p[ntd->dfa[i]->stateID] = 0; // 同时先把这个状态归入状态集0中
                }
                else{
                    N |= (1 << ntd->dfa[i]->stateID); // 否则把他分割进N中
                    p[ntd->dfa[i]->stateID] = 1;// 把这个状态归入状态集1中
                }
            }
            if(N == 0) return ; // 如果N为空,说明没有结束态,则不需要分割(根本割不了)
            Status.push_back(A);
            Status.push_back(N); // 装入状态集的vector中

            //cout<<N<<" "<<A<<endl;

            int i,j;
            int l; // l保存的是每一个分割前有多少状态集
            while(1){
                for(set<int>::iterator t1=ntd->CharSet.begin(); t1!=ntd->CharSet.end();t1++){ // 遍历字符集
                    l = t;
                    for(j = 0;j < l;j++)
                        split(j,*t1); 
                }
                if(l==t) break;//集合大小不变算法结束 
            }
        }

        int move_to(int x, int c)
        {
            int s = Status[x]; // s是要分割的状态集中的状态,可以理解为第一位为1则状态1在集合中,为0则状态1不在集合中
            int a[MAX];//吃完字符达到的状态集 
            int b[MAX];//对应a中分割的吃字符前的状态 
            memset(a,0,sizeof(a));
            memset(b,0,sizeof(b));
            int i,j,k;
            for(i = 0;i < v;i++){
                if((s>>i)&1){//遍历集合中的状态
                    for(j = 0;j < ntd->dfa[i]->t.size();j++){
                        if(ntd->dfa[i]->t[j]->char_ID == c){ // 如果从状态i吃入字符c
                            int v = ntd->dfa[i]->t[j]->des->stateID; // v是状态i吃入字符c后到达的状态
                            return p[v];
                        }
                    }
                } 
            }
            return -1;
        }

};



/*正则表达式主体*/
class Regex
{
public:
    string str;
    bool F[MAX]; // F中存储的是 匹配某些字符 or 除了某些字符都匹配.如果是true表示匹配某些字符,如果是false表示除了某些字符都匹配
                    // 这些字符都存储在G中,可以理解为F[i]表示G[i]中的字符是否匹配

    char S[MAX]; // S是ready预处理结束之后的字符串
    bool accept;
    bool word_match, word_not_match, text_start, text_end, word_start, word_end, line_start, line_end,deal_line,deal_word;
    NFA2DFA ntd;// NFA2DFA转换程序
    DFA2MinDFA* dtmin;


    vector<char> G[MAX];// G[i]存储的是第i部分可以匹配哪几种字符
    // 这个T代表的是,T['a']表示a这个字符出现在了第几个部分
    vector<char> T[MAX]; 
    vector< pair<int,int> > ans;

    Regex()
    {

    }

    Regex(string s) // 一个正则表达式进来先进行一下表达式处理
    {
        clear_all();
        ready(s);
        str=S;
        //cout<<str<<endl;
    }

    ~Regex()
    {

    }

    void clear_all() // 清零
    {
        memset(F,false,sizeof(F));
        memset(S,0,sizeof(S));
        for (int i=0;i<MAX;i++)
        {
            G[i].clear();
            T[i].clear();
        }
        ans.clear();
        accept=false;
        ntd.clear_all();

        word_match=false;
        word_not_match=false;
        line_end=false;
        line_start=false;
        deal_line=false;
        deal_word=false;

        text_start=false;
        text_end=false;
        word_start=false;
        word_end=false;
    }

    void ready(string s) //处理一下正则串
    {
        int l=0;
        int r=0;

        while (l<s.length())
        {
            if (s[l]=='.') //处理通配符
            {
                F[r]=false; // 表示除了\n都匹配
                S[r]='a';
                T['a'].push_back(r);
                put_in(r,'\n','\n'); // 除了\n都匹配
                l++;
                r++;
                continue;
            }

            if (s[l]=='\\')
            {
                // 比如\d表示匹配0-9,那么\D表示除了0-9都匹配.w和s同理
                l++;
                switch (s[l])
                {
                case 'n':
                case 't':
                case 'w':
                case 's':
                case 'd': F[r]=true; S[r]='a';T['a'].push_back(r);
                }

                switch (s[l])
                {
                case 'W':
                case 'S':
                case 'D': F[r]=false; S[r]='a';T['a'].push_back(r);
                }

                switch(s[l])
                {
                case 'n': put_in(r,'\n','\n');break;
                case 't': put_in(r,'\t','\t');break;
                case 'w': put_in(r,'a','z'),put_in(r,'A','Z'),put_in(r,'0','9'); break;
                case 'W': put_in(r,'a','z'),put_in(r,'A','Z'),put_in(r,'0','9'); break;

                case 's': put_in(r,'\t','\t'),put_in(r,'\n','\n'),put_in(r,' ', ' '); break;
                case 'S': put_in(r,'\t','\t'),put_in(r,'\n','\n'),put_in(r,' ', ' '); break;

                case 'd': put_in(r,'0','9'); break;
                case 'D': put_in(r,'0','9'); break;
                /*正则表达式 \bcat\b 匹配独立的单词"cat"，而不匹配包含"cat"的其他单词，如"catch"或"category"。
                正则表达式 \Bcat\B 匹配"catch"中的"cat"，但不匹配独立的单词"cat"或包含"cat"的其他单词。*/
                case 'b': word_match=true,r--,deal_word=true; break; 
                case 'B': word_not_match=true,r--,deal_word=true; break;
                }
                l++;
                r++;
                continue;
            }

            if (s[l]=='^')
            {
                deal_line=true;
                text_start=true;
                l++;
                continue;
            }

            if (s[l]=='$')
            {
                deal_line=true;
                text_end=true;
                l++;
                continue;
            }

            // 要匹配的字符集,[a-z]表示匹配a-z中的任意多个
            // 这部分操作和前面匹配\w和\W类似
            if (s[l]=='[')
            {
                l++;
                // [^a-z]表示除了a-z都匹配
                if (s[l]=='^')
                {
                    F[r]=false;
                    l++;
                }
                else
                    F[r]=true;
                
                while (s[l]!=']')
                {
                    if (s[l+1]=='-') put_in(r,s[l],s[l+2]),l=l+3;
                    else put_in(r,s[l],s[l]),l++;
                }
                S[r]='a';
                T['a'].push_back(r);
                l++;
                r++;
                continue;
            }
            F[r]=true;
            S[r]=s[l];
            T[S[r]].push_back(r);
            G[r].push_back(S[r]);
            l++;
            r++;
        }
    }

    void put_in(int r, char x, char y) //这个类似于第r的部分
    {
        for (char i=x;i<=y;i++)
            G[r].push_back(i);
    }

    void parse()
    {
        Regex2NFA cnt(str,T); // 创建转换程序,这个T就是符号表
		NFA result=cnt.ToNFA(); // 转换为nfa
		ntd.ready(cnt.State_ID-1,result.start,result.end->stateID); // 初始化nfa2dfa的转换程序
		getCharSet(result,ntd.CharSet,ntd.nfa);// 读取出符号表
		ntd.ToDFA();
        dtmin = new DFA2MinDFA(&ntd);
        dtmin->ToMinDFA();
    }

    bool is_w(char c)
    {
        if ('0'<=c && c<='9') return true;
        if ('a'<=c && c<='z') return true;
        if ('A'<=c && c<='Z') return true;
        return false;
    }

    bool is_b(char c)
    {
        switch (c)
        {
        case ' ':
        case '\n':
        case '\t': return true;
        }
        return false;
    }

    bool check_word_start(int k)
    {
        if (!is_w(str[k])) return false;
        if (k==0) return true;
        if (is_w(str[k-1])) return false;
        return true;
    }

    bool check_word_end(int k)
    {
        if (!is_w(str[k])) return false;
        if (k==str.length()) return true;
        if (is_w(str[k+1])) return false;
        return true;
    }

    bool check_text_start(int k)
    {
        if (k==0) return true;
        if (str[k-1]=='\n') return true;
        return false;
    }

    bool check_text_end(int k)
    {
        if (k+1==str.length()) return true;
        if (str[k+1]=='\n') return true;
        return false;
    }

    // s是要匹配的字符串,result是匹配的结果
    void match(string s,string *result) // 匹配字符串
    {
        accept=false;
        str=s;

        for (int i=0;i<s.length();i++)
        {
            word_start=check_word_start(i);
            line_start=check_text_start(i);
            if (deal_line && text_start && !line_start) continue;
            if (deal_word && word_start && word_not_match) continue;

            dfs(dtmin->min_dfa[0],i,i);
            if (accept) break;
        }

        int Max = 0;
        if (!accept) {return;}

        // 计算匹配到的字符串的长度
        for (int i = 0; i<ans.size(); i++)
        {
            if (ans[i].second - ans[i].first + 1 > Max) 
                Max = ans[i].second - ans[i].first + 1;
        }
        // 将匹配到的字符串从 str 中提取出来，存储在 result 指针所指向的字符串中
        *result=str.substr(ans[0].first, Max);
    }

    void dfs(DFA_Node* p, int k, int st)
    {
        // 如果当前节点是结束状态，检查是否满足匹配条件
        if (p->flag)
        {
            bool can_go=true;
            // 如果需要匹配单词，检查单词的起始和结束位置是否正确
            if (word_match)
            {
                word_end=check_word_end(k-1);
                if (!word_start && !word_end) can_go=false;
            }
            // 如果需要匹配不包含单词的字符串，检查单词的起始和结束位置是否正确
            if (word_not_match)
            {
                word_end=check_word_end(k-1);
                if (word_start || word_end) can_go=false;
            }
            // 如果需要匹配行尾，检查当前字符是否为行尾
            if (text_end)
            {
                line_end=check_text_end(k-1);
                if (!line_end) can_go=false;
            }
            // 如果满足匹配条件，将匹配结果加入答案
            if (can_go)
            {
                ans.push_back(make_pair(st, k-1));
                accept=true;
            }
        }
        // 如果已经匹配到字符串末尾，返回
        if (k==str.length()) return;
        // 遍历当前节点的所有出边
        for (int i=0;i<p->t.size();i++)
        {
            int u=p->t[i]->char_ID;
            DFA_Node* f=p->t[i]->des;
            // 如果当前字符可以转移，继续搜索
            if (F[u])
            for (int j=0;j<G[u].size();j++)
            {
                char c=G[u][j];
                if (str[k]!=c) continue;
                k++;
                dfs(f,k,st);
                k--;
            }
            // 如果当前字符不能转移，跳过当前出边
            else
            {
                bool flag=false;
                for (int j=0;j<G[u].size();j++)
                {
                    char c=G[u][j];
                    if (str[k]==c) flag=true;
                    if (flag) break;
                }
                if (flag) continue;
                k++;
                dfs(f,k,st);
                k--;
            }
        }
    }
};

void search(const char* regex, const char* str, string *result)
{
	Regex re(regex);
	re.parse();
	re.match(str, result);
}

int main()
{
    // 十六进制数
    // string reg = "(0(x|X)[1-9a-fA-F][0-9a-fA-F]*|0)";
    // string test = "0xAAF123";

    // 十进制数
    // string reg = "([1-9][0-9]*|0)";
    // string test = "364";
    string reg = "";
    string test = "";
    string result = "";
    cout<<"Please input the regex: ";
    cin>>reg;
    cout<<"Please input the test string: ";
    cin>>test;

    search(reg.c_str(), test.c_str(), &result);
    cout << "Match string: " << result << endl;
	return 0;
}
