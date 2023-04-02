#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<algorithm>
using namespace std;

/* 定义文字 */
typedef class literal {
public:
	bool _P;	//正负性:true - 正,false - 负
	int P;	//谓词
	vector<pair<int, bool>> VC;	//变量/常量:true - 变量,false - 常量

	vector<int> findv();	//找出列表项中出现的变量
	bool operator<(literal);	//重载"<"运算符,用于排序
	bool operator==(literal);	//重载"=="运算符,不考虑正负性

	void clear();
	void output(string, vector<string>, vector<string>, vector<string>);
	void output(vector<string>, vector<string>, vector<string>);
}literal;

vector<int> literal::findv()
{
	vector<int> v;	//用于记录this中出现的变量
	for (unsigned int i = 0; i < VC.size(); ++i)
		if (VC[i].second) {	//发现变量
			vector<int>::iterator it = find(v.begin(), v.end(), VC[i].first);
			if (it == v.end())	//v中尚不存在该变量
				v.push_back(VC[i].first);
		}
	return v;
}

bool literal::operator<(literal u)
{
	if (P == u.P) {	//先比P,然后比VC.size()
		if (VC.size() != u.VC.size())
			return VC.size() < u.VC.size();
		unsigned int i = 0;
		while (i < VC.size() - 1 && VC[i].second == u.VC[i].second)
			i++;
		if (i != VC.size() - 1)
			return VC[i].second;
		i = 0;
		while (i < VC.size() - 1 && VC[i].first == u.VC[i].first)
			i++;
		return VC[i].first < u.VC[i].first;
	}
	return P < u.P;
}

bool literal::operator==(literal u)
{
	if (P != u.P || VC.size() != u.VC.size())
		return false;
	vector<int> exist;
	for (unsigned int i = 0; i < VC.size(); ++i)
		if (VC[i].second != u.VC[i].second
			|| (!VC[i].second && !u.VC[i].second && VC[i].first != u.VC[i].first))
			return false;
		else if (VC[i].second && u.VC[i].second) {	//都是变量
			if (VC[i].first == u.VC[i].first)
				exist.push_back(VC[i].first);
			else {
				vector<int>::iterator it1 = find(exist.begin(), exist.end(), VC[i].first);
				vector<int>::iterator it2 = find(exist.begin(), exist.end(), u.VC[i].first);
				if (it1 == exist.end() && it2 == exist.end()) {	//该变量尚未出现过,可以尝试替换
					int temp = u.VC[i].first;
					for (unsigned int j = 0; j < u.VC.size(); ++j)
						if (u.VC[j].second && u.VC[j].first == temp)
							u.VC[j].first = VC[i].first;
						else if (u.VC[j].second && u.VC[j].first == VC[j].first) {
							static int m = 0;	//找出一个this和u中都未出现的变量m
							vector<int> t1 = findv();
							vector<int> t2 = u.findv();
							while (1) {
								vector<int>::iterator it1 = find(t1.begin(), t1.end(), m);
								vector<int>::iterator it2 = find(t2.begin(), t2.end(), m);
								if (it1 == t1.end() && it2 == t2.end())
									break;
								m++;
							}
							u.VC[j].first = m;
						}
					exist.push_back(VC[i].first);
				}
				else
					return false;
			}
		}
	return true;
}

void literal::clear()
{
	VC.clear();
}

void literal::output(string outfile,
	vector<string> predicate,
	vector<string> variable,
	vector<string> constant)
{
	ofstream out(outfile, ios::app);	//将结果写入文件
	if (out.is_open()) {
		if (!_P)
			out << "~";
		out << predicate[P] << "(";
		for (unsigned int i = 0; i < VC.size(); ++i) {
			if (i != 0)
				out << ",";
			if (VC[i].second)
				out << variable[VC[i].first];
			else
				out << constant[VC[i].first];
		}
		out << ")";
	}
}

void literal::output(vector<string> predicate,
	vector<string> variable,
	vector<string> constant)
{
	if (!_P)
		cout << "~";
	cout << predicate[P] << "(";
	for (unsigned int i = 0; i < VC.size(); ++i) {
		if (i != 0)
			cout << ",";
		if (VC[i].second)
			cout << variable[VC[i].first];
		else
			cout << constant[VC[i].first];
	}
	cout << ")";
}

/* 定义置换 */
typedef class substitution {
public:
	int father;
	int v;
	int c;
}substitution;

/* 定义合一 */
typedef class unification {
public:
	int father1;
	int father2;
}unification;

/* 定义子句 */
typedef class horn {
public:
	int id;

	vector<literal> L;	//正逻辑文字集
	vector<literal>_L;	//负逻辑文字集

	int F;	//0 - 初始子句,1 - 由置换产生,2 - 由合一产生
	substitution F1;	//置换
	unification F2;		//合一

	bool empty();
	vector<int> findv();	//找出子句中出现的变量
	void change(int, int);	//更换变量名,返回经过更换的变量
	vector<horn> substitute(vector<string>);		//定义置换运算,返回<运算结果,是否能够置换>(每次调用只会)
	pair<horn, bool> operator+(horn);	//定义合一运算,返回<运算结果,是否能够合一>
	bool operator==(horn);	//重载"=="运算符

	void output(string, vector<string>, vector<string>, vector<string>);
	void output(vector<string>, vector<string>, vector<string>);
}horn;

bool horn::empty()
{
	return (L.size() == 0 && _L.size() == 0);
}

vector<int> horn::findv()
{
	vector<int> v;	//用于记录this中出现的变量
	for (unsigned int i = 0; i < L.size(); ++i) {
		for (unsigned int j = 0; j < L[i].VC.size(); ++j)
			if (L[i].VC[j].second) {	//发现变量
				vector<int>::iterator it = find(v.begin(), v.end(), L[i].VC[j].first);
				if (it == v.end())	//v中尚不存在该变量
					v.push_back(L[i].VC[j].first);
			}
	}
	for (unsigned int i = 0; i < _L.size(); ++i) {
		for (unsigned int j = 0; j < _L[i].VC.size(); ++j)
			if (_L[i].VC[j].second) {	//发现变量
				vector<int>::iterator it = find(v.begin(), v.end(), _L[i].VC[j].first);
				if (it == v.end())	//v中尚不存在该变量
					v.push_back(_L[i].VC[j].first);
			}
	}
	return v;
}

void horn::change(int a, int b)	//变量a->变量b
{
	/*vector<int> v = findv();	//找出this中出现的变量
	vector<int>::iterator it = find(v.begin(), v.end(), b);
	if (it != v.end()) {	//b已经是子句中存在的变量
		int c = 0;
		while (1) {	//找出一个this中未出现的变量c
			vector<int>::iterator it = find(v.begin(), v.end(), c);
			if (it == v.end())
				break;
			c++;
		}
		change(b, c);	//先将b更换为c
	}*/
	for (unsigned int i = 0; i < L.size(); ++i) {
		for (unsigned int j = 0; j < L[i].VC.size(); ++j)
			if (L[i].VC[j].second && L[i].VC[j].first == a)	//发现变量a
				L[i].VC[j].first = b;
			else if (L[i].VC[j].second && L[i].VC[j].first == b)	//发现变量b
				L[i].VC[j].first = a;
	}
	for (unsigned int i = 0; i < _L.size(); ++i) {
		for (unsigned int j = 0; j < _L[i].VC.size(); ++j)
			if (_L[i].VC[j].second && _L[i].VC[j].first == a)	//发现变量a
				_L[i].VC[j].first = b;
			else if (_L[i].VC[j].second && _L[i].VC[j].first == b)	//发现变量b
				_L[i].VC[j].first = a;
	}
	return;
}

vector<horn> horn::substitute(vector<string> constant)
{
	vector<horn> res;
	vector<int> v = findv();	//找出this中出现的变量
	if (!v.empty()) {	//子句this中有变量
		for (unsigned int l = 0; l < v.size(); ++l)	//每个可以被置换的变量
			for (unsigned int k = 0; k < constant.size(); ++k) {	//每种可能的置换
				horn temp(*this);
				for (unsigned int i = 0; i < temp.L.size(); ++i) {
					for (unsigned int j = 0; j < temp.L[i].VC.size(); ++j)
						if (temp.L[i].VC[j].second && temp.L[i].VC[j].first == v[l]) {	//发现变量v[l]
							temp.L[i].VC[j].second = false;
							temp.L[i].VC[j].first = k;
						}
				}
				for (unsigned int i = 0; i < temp._L.size(); ++i) {
					for (unsigned int j = 0; j < temp._L[i].VC.size(); ++j)
						if (temp._L[i].VC[j].second && temp._L[i].VC[j].first == v[l]) {	//发现变量v[l]
							temp._L[i].VC[j].second = false;
							temp._L[i].VC[j].first = k;
						}
				}
				temp.F = 1;
				temp.F1.father = id;
				temp.F1.v = v[l];
				temp.F1.c = k;
				res.push_back(temp);
			}
	}
	return res;
}

pair<horn, bool> horn::operator+(horn u)
{
	horn t(*this);
	bool judge = false;
	for (unsigned int i = 0; i < u.L.size() && !judge; ++i) {	//u的正文字是否与*this的负文字有重合
		for (unsigned int j = 0; j < t._L.size() && !judge; ++j)
			if (u.L[i] == t._L[j]) {
				for (unsigned int k = 0; k < u.L[i].VC.size(); ++k)
					if (u.L[i].VC[k].second == true && t._L[j].VC[k].second == true
						&& u.L[i].VC[k].first != t._L[j].VC[k].first) {	//变量名不同
						u.change(u.L[i].VC[k].first, t._L[j].VC[k].first);
					}
				u.L.erase(u.L.begin() + i);		//u并非引用,这里对u的修改不会影响实参
				t._L.erase(t._L.begin() + j);
				judge = true;
			}
	}
	for (unsigned int i = 0; i < u._L.size() && !judge; ++i)	//u的负文字是否与*this的正文字有重合
		for (unsigned int j = 0; j < t.L.size() && !judge; ++j)
			if (u._L[i] == t.L[j]) {
				for (unsigned int k = 0; k < u._L[i].VC.size(); ++k)
					if (u._L[i].VC[k].second == true && t.L[j].VC[k].second == true
						&& u._L[i].VC[k].first != t.L[j].VC[k].first) {	//变量名不同
						u.change(u._L[i].VC[k].first, t.L[j].VC[k].first);
					}
				u._L.erase(u._L.begin() + i);	//u并非引用,这里对u的修改不会影响实参
				t.L.erase(t.L.begin() + j);
				judge = true;
			}
	pair<horn, bool> res;
	horn temp;
	if (!judge)	//无法合一(合一没有意义)
		res.second = false;
	else {
		for (unsigned int i = 0; i < t.L.size(); ++i)
			temp.L.push_back(t.L[i]);
		for (unsigned int i = 0; i < u.L.size(); ++i)
			temp.L.push_back(u.L[i]);
		for (unsigned int i = 0; i < t._L.size(); ++i)
			temp._L.push_back(t._L[i]);
		for (unsigned int i = 0; i < u._L.size(); ++i)
			temp._L.push_back(u._L[i]);
		temp.F = 2;
		temp.F2.father1 = id;
		temp.F2.father2 = u.id;
		res.second = true;
	}
	res.first = temp;
	return res;
}

bool horn::operator==(horn u)
{
	if (L.size() != u.L.size() || _L.size() != u._L.size())
		return false;
	for (unsigned int i = 0; i < L.size(); ++i)
		if (!(L[i] == u.L[i]))
			return false;
	for (unsigned int i = 0; i < _L.size(); ++i)
		if (!(_L[i] == u._L[i]))
			return false;
	return true;
}

void horn::output(string outfile,
	vector<string> predicate,
	vector<string> variable,
	vector<string> constant)
{
	ofstream out(outfile, ios::app);	//将结果写入文件
	if (out.is_open()) {
		for (unsigned int i = 0; i < L.size(); ++i) {
			if (i != 0)
				out << ",";
			L[i].output(outfile, predicate, variable, constant);
		}
		for (unsigned int i = 0; i < _L.size(); ++i) {
			out << ",";
			_L[i].output(outfile, predicate, variable, constant);
		}
	}
}

void horn::output(vector<string> predicate,
	vector<string> variable,
	vector<string> constant)
{
	for (unsigned int i = 0; i < L.size(); ++i) {
		if (i != 0)
			cout << ",";
		L[i].output(predicate, variable, constant);
	}
	for (unsigned int i = 0; i < _L.size(); ++i) {
		if (i != 0 || L.size() != 0)
			cout << ",";
		_L[i].output(predicate, variable, constant);
	}
}

/* 定义归结 */
typedef class resolution {
public:
	vector<horn> H;	//子句集
	vector<string> predicate;	//谓词集
	vector<string> variable;	//变量集
	vector<string> constant;	//常量集

	void resolute();

	void input();
	void output();
	void output_res(int);
}resolution;

void resolution::resolute()
{
	cout << "开始归结:" << endl;
	for (unsigned int now = 0; now < H.size(); ++now) {
		cout << now << ":\t";
		H[now].output(predicate, variable, constant);
		cout << "\tF=" << H[now].F << " ";
		switch (H[now].F) {
		case 0:
			cout << endl;
			break;
		case 1:
			cout << "father:" << H[now].F1.father << " {" << variable[H[now].F1.v] << "/" << constant[H[now].F1.c] << "}" << endl;
			break;
		case 2:
			cout << "father1:" << H[now].F2.father1 << " " "father2:" << H[now].F2.father2 << endl;
			break;
		}

		/* 尝试归结H[now]与H[i](i=0,..,(now-1)) */
		for (unsigned int i = 0; i < now; ++i) {
			pair<horn, bool> temp;
			temp = H[i] + H[now];
			if (!temp.second)	//合一失败
				continue;
			sort(temp.first.L.begin(), temp.first.L.end());
			sort(temp.first._L.begin(), temp.first._L.end());
			vector<horn>::iterator it = find(H.begin(), H.end(), temp.first);
			if (it != H.end())	//已经存在重复子句
				continue;
			temp.first.id = H.size();
			temp.first.F = 2;
			temp.first.F2.father1 = i;
			temp.first.F2.father2 = now;
			/*cout << "\t(" << now << ")+(" << i << ")\t";
			temp.first.output(predicate, variable, constant);
			cout << endl;*/
			H.push_back(temp.first);
			if (temp.first.empty()) {	//成功导出空语句
				cout << endl << endl << "求解过程:" << endl;
				output_res(H.size() - 1);	//输出结果
				return;
			}
		}

		/* 尝试H[now]所有可能的置换 */
		vector<horn> temp = H[now].substitute(constant);
		for (unsigned int i = 0; i < temp.size(); ++i) {
			vector<horn>::iterator it = find(H.begin(), H.end(), temp[i]);
			if (it != H.end())	//已经存在重复子句
				continue;
			temp[i].id = H.size();
			H.push_back(temp[i]);
		}
	}
	cout << "归结失败" << endl;
}

void resolution::input()
{
	ifstream in("input.txt");	//从文件中读入数据
	if (in.is_open()) {
		int n;
		in >> n;
		while (in.get() != '\n')
			;

		/* 读入n个子句 */
		for (int i = 0; i < n; ++i) {
			horn h;
			h.F = 0;	//标记为初始子句
			string s;
			getline(in, s);
			int now = 0, len = s.size();

			/* 读入1个子句 */
			literal t;
			while (now < len) {
				if (s[now] != ' ') {
					string word;
					while (now < len) {	//读入一个单词
						if (s[now] == '(' || s[now] == ')' || s[now] == ',')
							break;
						if (s[now] != ' ')
							word += s[now];
						now++;
					}
					if (word.length() != 0)
						if (s[now] != '(') {	//word为变量or常量
							pair<int, bool> vc;
							if (word[0] == 'x' || word[0] == 'y' || word[0] == 'z') {	//word为变量(以'x'/'y'/'z'开头)
								vc.second = true;
								vector<string>::iterator it = find(variable.begin(), variable.end(), word);
								if (it != variable.end())	//已经存在相同变量
									vc.first = it - variable.begin();
								else {	//不存在相同变量
									vc.first = variable.size();
									variable.push_back(word);
								}
								t.VC.push_back(vc);
							}
							else {	//word为常量
								vc.second = false;
								vector<string>::iterator it = find(constant.begin(), constant.end(), word);
								if (it != constant.end())	//已经存在相同变量
									vc.first = it - constant.begin();
								else {	//不存在相同变量
									vc.first = constant.size();
									constant.push_back(word);
								}
								t.VC.push_back(vc);
							}
						}
						else {	//word为谓词符号
							if (word[0] != '~') {	//word为正文字
								t._P = true;
								vector<string>::iterator it = find(predicate.begin(), predicate.end(), word);
								if (it != predicate.end())	//已经存在相同谓词
									t.P = it - predicate.begin();
								else {	//不存在相同谓词
									t.P = predicate.size();
									predicate.push_back(word);
								}
							}
							else {	//word为负文字
								word.erase(0, 1);
								t._P = false;
								vector<string>::iterator it = find(predicate.begin(), predicate.end(), word);
								if (it != predicate.end())	//已经存在相同谓词
									t.P = it - predicate.begin();
								else {	//不存在相同谓词
									t.P = predicate.size();
									predicate.push_back(word);
								}
							}
						}
					if (s[now] == ')') {	//1个文字输入结束
						if (t._P)
							h.L.push_back(t);
						else
							h._L.push_back(t);
						t.clear();
					}
				}//if (s[now] != ' ')
				now++;
			}//while (now < len)
			h.id = H.size();
			sort(h.L.begin(), h.L.end());
			sort(h._L.begin(), h._L.end());
			H.push_back(h);
		}
	}

	return;
}

void resolution::output()
{
	for (unsigned int i = 0; i < H.size(); ++i) {
		cout << i + 1 << ":\t";
		H[i].output(predicate, variable, constant);
		cout << endl;
	}
}

void resolution::output_res(int head)
{
	switch (H[head].F) {
	case 0:	//初始子句
		cout << "(" << head << ")\t";
		H[head].output(predicate, variable, constant);
		cout << endl;
		break;
	case 1: {	//由置换产生
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
	case 2:	//由合一产生
		output_res(H[head].F2.father1);
		output_res(H[head].F2.father2);
		cout << "(" << head << ")\t";
		H[head].output(predicate, variable, constant);
		cout << "\t(" << H[head].F2.father1 << ")+(" << H[head].F2.father2 << ")" << endl;
		break;
	}
}

int main()
{
	resolution r;
	r.input();
	cout << "给定事实如下:" << endl;
	r.output();
	cout << endl << endl;

	r.resolute();
}
