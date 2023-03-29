# HornClauseApodosis

## 文件说明
HornClauseApodosis.cpp // 核心算法源文件  
input.txt              // 案例输入  
output.txt             // 案例输出

## 运行环境
C++

## 设计思路
对于一个给定的实际问题，可以对其知识进行逻辑表示，转换为子句，再对子句进行归结求解。

假设事实均以CNF（合取范式，即若干子句的合取式）的形式给出，整个程序可以分为下面三个部分：
- **输入**：将输入的子句集读入计算机，理解子句中各个文字的含义，并合理存储；
- **归结**：不断尝试将不同子句中存在在互补命题文字进行合一，得出新知识，直至导出空子句；
- **输出**：将从初始子句到空子句过程中的推理过程按照一定的格式输出。

在实现的过程中，定义三个类literal、horn和resolution，分别表示文字、子句和归结。其结构设计如下：
```
/* 定义文字 */
typedef class literal {
public:
	bool _P;	// 正负性:true - 正,false - 负
	int P;      // 谓词
	vector<pair<int, bool>> VC;	// 变量/常量:true - 变量,false - 常量

	vector<int> findv();        // 找出列表项中出现的变量
	bool operator<(literal);	// 重载"<"运算符,用于排序
	bool operator==(literal);	// 重载"=="运算符,不考虑正负性

	void clear();
	void output(string, vector<string>, vector<string>, vector<string>);
	void output(vector<string>, vector<string>, vector<string>);
}literal;
```

```
/* 定义子句 */
typedef class horn {
public:
	int id;

	vector<literal> L;	// 正逻辑文字集
	vector<literal>_L;	// 负逻辑文字集

	int F;              // 0 - 初始子句,1 - 由置换产生,2 - 由合一产生
	substitution F1;	// 置换
	unification F2;		// 合一

	bool empty();
	vector<int> findv();	// 找出子句中出现的变量
	void change(int, int);	// 更换变量名,返回经过更换的变量
	vector<horn> substitute(vector<string>);    // 定义置换运算,返回<运算结果,是否能够置换>(每次调用只会)
	pair<horn, bool> operator+(horn);	// 定义合一运算,返回<运算结果,是否能够合一>
	bool operator==(horn);	// 重载"=="运算符

	void output(string, vector<string>, vector<string>, vector<string>);
	void output(vector<string>, vector<string>, vector<string>);
}horn;
```

```
/* 定义归结 */
typedef class resolution {
public:
	vector<horn> H;             // 子句集
	vector<string> predicate;	// 谓词集
	vector<string> variable;	// 变量集
	vector<string> constant;	// 常量集

	void resolute();

	void input();
	void output();
	void output_res(int);
}resolution;
```

## 核心算法
### 子句归结的基本原理
Horn子句是指至多有一个正文字的析取式。Horn可以写成蕴含式的形式，形如
- P∧Q → R ⇔ ¬P∨¬Q∨R

其前提为正文字的合取式，称为体；其结论为单个下文字，称为头。

由于已经假定事实均以CNF的形式给出，因此子句中不存在一阶逻辑。在此条件下，对于没有共享变量的两个子句，如果它们包含互补文字，则可以对它们进行归结。其基本形式如下：
- ¬P∨Q，¬Q∨R → ¬P∨R
- ¬P∨R，P    → R

有时，逻辑文字的合一之间需要先进行置换。如通过置换θ={u/G(x),v/x}，互补文字Loves(G(x),x)和¬Loves(u,v)可以消除。

设α是待证明的逻辑命题，则可以将¬α加入知识库，通过证明KB∧¬α不可满足，即通过导出空语句，来证明KB⊨α。

### 输入
输入部分由resolution类中的input函数实现。

首先读入整数n，表示一共有n个子句，接下来每行输入1个子句。

之后，每次读入一行，并不断从该行中取出独立的单词（忽略空格，以特殊字符分隔）。通过单词后面的字符是否为'('判断其是否为谓词，若是谓词则通过单词开关是否为'~'判断其正负性，否则再通过单词的首字母判断其为变量或常量（规定变量以'x'、'y'或'z'开头），并加入相应的谓词集predicate、变量集variable或常量集constant，判断完成后，构建子句，加入子句集H。

在后面的操作中，谓词将用int的形式表示，即其在谓词集predicate中的序号；变量和常量将用pair<int, bool>的形式表示，前者为其在变量集variable或常量集constant中的序号，后者为变量/常量类型的指示标志。

### 归结
归结部分由resolution类中的resolute函数实现。

由于在归结过程中需要将互补的文字“合一”，而合一所需的“置换”不易得出，可以选择简化的方式，即将“置换”与“合一”分为两个操作分别进行。“置换”在horn类中定义为函数substitute，“合一”在horn类中定义为函数operator+，即重载运算符’+’。

首先，将知识库（即子句集H）看作队列，记队头为now。每次取出队头子句H[now]，先尝试让H[now]与所有已经扩展过的子句（即H[0..(now-1)]）归结，将归结得到的新知识加入H的队尾；再尝试H[now]所有可能的置换，并将置换得到的新知识加入H的队尾。之后不断重复上述过程，直至成功导出空语句或队空。

总体而言，整个归结过程采用BFS（广度优先搜索），即now每指向一个子句时，尝试扩展所有与其相关的子句，本质上与“前向链接”类似。其主要的实现代码如下：
```
void resolution::resolute()
{
	for (unsigned int now = 0; now < H.size(); ++now) {

		/* 尝试归结H[now]与H[i](i=0,..,(now-1)) */
		for (unsigned int i = 0; i < now; ++i) {
			pair<horn, bool> temp;
			temp = H[i] + H[now];
			if (!temp.second)	// 合一失败
				continue;
			sort(temp.first.L.begin(), temp.first.L.end());
			sort(temp.first._L.begin(), temp.first._L.end());
			vector<horn>::iterator it = find(H.begin(), H.end(), temp.first);
			if (it != H.end())	// 已经存在重复子句
				continue;
			temp.first.id = H.size();
			temp.first.F = 2;
			temp.first.F2.father1 = i;
			temp.first.F2.father2 = now;
			H.push_back(temp.first);
			if (temp.first.empty()) {	// 成功导出空语句
				output_res(H.size() - 1);	// 输出结果
				return;
			}
		}

		/* 尝试H[now]所有可能的置换 */
		vector<horn> temp = H[now].substitute(constant);
		for (unsigned int i = 0; i < temp.size(); ++i) {
			vector<horn>::iterator it = find(H.begin(), H.end(), temp[i]);
			if (it != H.end())	// 已经存在重复子句
				continue;
			temp[i].id = H.size();
			H.push_back(temp[i]);
		}
	}
}
```

### 输出
输入部分由resolution类中的output_res函数实现。

函数的输入为空语句的序号。通过horn类中的F，能够判断出该语句的父语句及扩展方式（初始子句、由置换得到、由合一得到）。以父语句的序号为参数，递归调用函数即可输出整棵“搜索树”。特别地，为保证子句归结过程的可读性，选择先递归后输出。总而言之输出的整个过程采用DFS（深度优先搜索），其主要的实现代码如下：
```
void resolution::output_res(int head)
{
	switch (H[head].F) {

	case 0:	// 初始子句
		cout << "(" << head << ")\t";
		H[head].output(predicate, variable, constant);
		cout << endl;
		break;

	case 1: {	// 由置换产生
		vector<pair<int, int>> t;
		int k = head;
		while (H[k].F == 1) {
			pair<int, int> tk;
			tk.first = H[k].F1.v;
			tk.second = H[k].F1.c;
			t.push_back(tk);
			k = H[k].F1.father;
		}
		output_res(k);
		cout << "(" << head << ")\t";
		H[head].output(predicate, variable, constant);
		cout << "\t(" << k << "):{";
		for (unsigned int i = 0; i < t.size(); ++i) {
			if (i != 0)
				cout << ",";
			cout << variable[t[i].first] << "/" << constant[t[i].second];
		}
		cout << "}" << endl;
		break;
	}

	case 2:	// 由合一产生
		output_res(H[head].F2.father1);
		output_res(H[head].F2.father2);
		cout << "(" << head << ")\t";
		H[head].output(predicate, variable, constant);
		cout << "\t(" << H[head].F2.father1 << ")+(" << H[head].F2.father2 << ")" << endl;
		break;
	}
}
```

## 案例分析
尝试利用子句归结解决一个实际问题。

---

**破案问题**：在一栋房子里发生了一件神秘的谋杀案，现在可以确定以下几点事实： 
1. 在这栋房子里仅住有A,B,C三人；
2. 是住在这栋房子里的人杀了A；
3. 谋杀者非常恨受害者；
4. A所恨的人，C一定不恨；
5. 除了B以外，A恨所有的人；
6. B恨所有不比A富有的人；
7. A所恨的人，B也恨；
8. 没有一个人恨所有的人；
9. 杀人嫌疑犯一定不会比受害者富有。

为了推理需要，增加如下常识：
10. A不等于B。

问：谋杀者是谁？

---

将“破案问题”中的事实转化为一阶逻辑的形式，如下所示：
1. live(A)∧live(B)∧live(C)
2. ∀x  kill(x,A) → live(x)
3. ∀x,y  kill(x,y) → hate(x,y)
4. ∀x  hate(A,x) → ¬hate(C,x)
5. ∀x  ¬et(x,B) → hate(A,x)
6. ∀x  ¬richer(x,A) → hate(B,x)
7. ∀x  hate(A,x) → hate(B,x)
8. ∀x ∃y  ¬hate(x,y)
9. ∀x,y  kill(x,y) → ¬richer(x,y)
10. ¬et(A,B)

再将一阶逻辑转化为CNF形式，如下所示：
1. live(x)
2. ¬kill(x,A)∨live(x)
3. ¬kill(x,y)∨hate(x,y)
4. ¬hate(A,x)∨¬hate(C,x)
5. et(x,B)∨hate(A,x)
6. richer(x,A)∨hate(B,x)
7. ¬hate(A,x)∨hate(B,x)
8. ¬hate(x,F(x))
9. ¬kill(x,y)∨¬richer(x,y)
10. ¬et(A,B)

容易观察到，谓词live仅在(1)和(2)中出现，表达了变量x的取值范围为{A,B,C}，因此舍去谓词live，更换为下面的(1)；(5)式更换为下面的(4)和(5)，从而舍去了谓词et，即(10)式也可以舍去；(6)严格意义上不属于Horn子句，但这并不影响子句归结的应用，对应下面的(6)；为避免命题文字的嵌套，对(8)也进行了修改，对应下面的(8)；剩余的(3)(4)(7)(9)分别对应下面的(2)(3)(7)(9)；另外，为说明谓词richer所表达的偏序关系，添加事实(10)和(11)。为便于计算机的读取，将上述式中的“¬”替换为“~”、“∨”替换为“,”。

最终，我们输入计算机的事实如下：
1. kill(A,A),kill(B,A),kill(C,A)
2. hate(x,y),~kill(x,y)
3. ~hate(A,x),~hate(C,x)
4. hate(A,A)
5. hate(A,C)
6. richer(x,A),hate(B,x)
7. hate(B,x),~hate(A,x)
8. ~hate(x,A),~hate(x,B),~hate(x,C)
9. ~kill(x,y),~richer(x,y)
10. ~richer(x,y),~richer(y,x)
11. ~richer(x,x)

上文已经说明，设α是待证明的逻辑命题，即α应分别取kill(A,A)、kill(B,A)、kill(C,A)。

因此将¬α记为(0)，与其它事实一并输入计算机。其中，仅α取kill(A,A)，即输入为input.txt时，通过子句归结能够导出空语句，此时的输出为output.txt。

由此可以，是“破案问题”中的谋杀者正是死者A本人。

## 参考文献
[1] Stuart J. Russell, Peter Norvig. Artificial Intelligence: A Mordern Approach[M]. 北京: 清华大学出版社, 2013．  
[2] 我的程序跑快快. horn语句归结原理[OL]. https://blog.csdn.net/qq_36306833/, 2018-10-09.
