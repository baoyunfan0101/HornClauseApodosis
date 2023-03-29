/* 1951243 ���Ʒ� */
#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<algorithm>
using namespace std;

/* �������� */
typedef class literal {
public:
	bool _P;	//������:true - ��,false - ��
	int P;	//ν��
	vector<pair<int, bool>> VC;	//����/����:true - ����,false - ����

	vector<int> findv();	//�ҳ��б����г��ֵı���
	bool operator<(literal);	//����"<"�����,��������
	bool operator==(literal);	//����"=="�����,������������

	void clear();
	void output(string, vector<string>, vector<string>, vector<string>);
	void output(vector<string>, vector<string>, vector<string>);
}literal;

vector<int> literal::findv()
{
	vector<int> v;	//���ڼ�¼this�г��ֵı���
	for (unsigned int i = 0; i < VC.size(); ++i)
		if (VC[i].second) {	//���ֱ���
			vector<int>::iterator it = find(v.begin(), v.end(), VC[i].first);
			if (it == v.end())	//v���в����ڸñ���
				v.push_back(VC[i].first);
		}
	return v;
}

bool literal::operator<(literal u)
{
	if (P == u.P) {	//�ȱ�P,Ȼ���VC.size()
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
		else if (VC[i].second && u.VC[i].second) {	//���Ǳ���
			if (VC[i].first == u.VC[i].first)
				exist.push_back(VC[i].first);
			else {
				vector<int>::iterator it1 = find(exist.begin(), exist.end(), VC[i].first);
				vector<int>::iterator it2 = find(exist.begin(), exist.end(), u.VC[i].first);
				if (it1 == exist.end() && it2 == exist.end()) {	//�ñ�����δ���ֹ�,���Գ����滻
					int temp = u.VC[i].first;
					for (unsigned int j = 0; j < u.VC.size(); ++j)
						if (u.VC[j].second && u.VC[j].first == temp)
							u.VC[j].first = VC[i].first;
						else if (u.VC[j].second && u.VC[j].first == VC[j].first) {
							static int m = 0;	//�ҳ�һ��this��u�ж�δ���ֵı���m
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
	ofstream out(outfile, ios::app);	//�����д���ļ�
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

/* �����û� */
typedef class substitution {
public:
	int father;
	int v;
	int c;
}substitution;

/* �����һ */
typedef class unification {
public:
	int father1;
	int father2;
}unification;

/* �����Ӿ� */
typedef class horn {
public:
	int id;

	vector<literal> L;	//���߼����ּ�
	vector<literal>_L;	//���߼����ּ�

	int F;	//0 - ��ʼ�Ӿ�,1 - ���û�����,2 - �ɺ�һ����
	substitution F1;	//�û�
	unification F2;		//��һ

	bool empty();
	vector<int> findv();	//�ҳ��Ӿ��г��ֵı���
	void change(int, int);	//����������,���ؾ��������ı���
	vector<horn> substitute(vector<string>);		//�����û�����,����<������,�Ƿ��ܹ��û�>(ÿ�ε���ֻ��)
	pair<horn, bool> operator+(horn);	//�����һ����,����<������,�Ƿ��ܹ���һ>
	bool operator==(horn);	//����"=="�����

	void output(string, vector<string>, vector<string>, vector<string>);
	void output(vector<string>, vector<string>, vector<string>);
}horn;

bool horn::empty()
{
	return (L.size() == 0 && _L.size() == 0);
}

vector<int> horn::findv()
{
	vector<int> v;	//���ڼ�¼this�г��ֵı���
	for (unsigned int i = 0; i < L.size(); ++i) {
		for (unsigned int j = 0; j < L[i].VC.size(); ++j)
			if (L[i].VC[j].second) {	//���ֱ���
				vector<int>::iterator it = find(v.begin(), v.end(), L[i].VC[j].first);
				if (it == v.end())	//v���в����ڸñ���
					v.push_back(L[i].VC[j].first);
			}
	}
	for (unsigned int i = 0; i < _L.size(); ++i) {
		for (unsigned int j = 0; j < _L[i].VC.size(); ++j)
			if (_L[i].VC[j].second) {	//���ֱ���
				vector<int>::iterator it = find(v.begin(), v.end(), _L[i].VC[j].first);
				if (it == v.end())	//v���в����ڸñ���
					v.push_back(_L[i].VC[j].first);
			}
	}
	return v;
}

void horn::change(int a, int b)	//����a->����b
{
	/*vector<int> v = findv();	//�ҳ�this�г��ֵı���
	vector<int>::iterator it = find(v.begin(), v.end(), b);
	if (it != v.end()) {	//b�Ѿ����Ӿ��д��ڵı���
		int c = 0;
		while (1) {	//�ҳ�һ��this��δ���ֵı���c
			vector<int>::iterator it = find(v.begin(), v.end(), c);
			if (it == v.end())
				break;
			c++;
		}
		change(b, c);	//�Ƚ�b����Ϊc
	}*/
	for (unsigned int i = 0; i < L.size(); ++i) {
		for (unsigned int j = 0; j < L[i].VC.size(); ++j)
			if (L[i].VC[j].second && L[i].VC[j].first == a)	//���ֱ���a
				L[i].VC[j].first = b;
			else if (L[i].VC[j].second && L[i].VC[j].first == b)	//���ֱ���b
				L[i].VC[j].first = a;
	}
	for (unsigned int i = 0; i < _L.size(); ++i) {
		for (unsigned int j = 0; j < _L[i].VC.size(); ++j)
			if (_L[i].VC[j].second && _L[i].VC[j].first == a)	//���ֱ���a
				_L[i].VC[j].first = b;
			else if (_L[i].VC[j].second && _L[i].VC[j].first == b)	//���ֱ���b
				_L[i].VC[j].first = a;
	}
	return;
}

vector<horn> horn::substitute(vector<string> constant)
{
	vector<horn> res;
	vector<int> v = findv();	//�ҳ�this�г��ֵı���
	if (!v.empty()) {	//�Ӿ�this���б���
		for (unsigned int l = 0; l < v.size(); ++l)	//ÿ�����Ա��û��ı���
			for (unsigned int k = 0; k < constant.size(); ++k) {	//ÿ�ֿ��ܵ��û�
				horn temp(*this);
				for (unsigned int i = 0; i < temp.L.size(); ++i) {
					for (unsigned int j = 0; j < temp.L[i].VC.size(); ++j)
						if (temp.L[i].VC[j].second && temp.L[i].VC[j].first == v[l]) {	//���ֱ���v[l]
							temp.L[i].VC[j].second = false;
							temp.L[i].VC[j].first = k;
						}
				}
				for (unsigned int i = 0; i < temp._L.size(); ++i) {
					for (unsigned int j = 0; j < temp._L[i].VC.size(); ++j)
						if (temp._L[i].VC[j].second && temp._L[i].VC[j].first == v[l]) {	//���ֱ���v[l]
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
	for (unsigned int i = 0; i < u.L.size() && !judge; ++i) {	//u���������Ƿ���*this�ĸ��������غ�
		for (unsigned int j = 0; j < t._L.size() && !judge; ++j)
			if (u.L[i] == t._L[j]) {
				for (unsigned int k = 0; k < u.L[i].VC.size(); ++k)
					if (u.L[i].VC[k].second == true && t._L[j].VC[k].second == true
						&& u.L[i].VC[k].first != t._L[j].VC[k].first) {	//��������ͬ
						u.change(u.L[i].VC[k].first, t._L[j].VC[k].first);
					}
				u.L.erase(u.L.begin() + i);		//u��������,�����u���޸Ĳ���Ӱ��ʵ��
				t._L.erase(t._L.begin() + j);
				judge = true;
			}
	}
	for (unsigned int i = 0; i < u._L.size() && !judge; ++i)	//u�ĸ������Ƿ���*this�����������غ�
		for (unsigned int j = 0; j < t.L.size() && !judge; ++j)
			if (u._L[i] == t.L[j]) {
				for (unsigned int k = 0; k < u._L[i].VC.size(); ++k)
					if (u._L[i].VC[k].second == true && t.L[j].VC[k].second == true
						&& u._L[i].VC[k].first != t.L[j].VC[k].first) {	//��������ͬ
						u.change(u._L[i].VC[k].first, t.L[j].VC[k].first);
					}
				u._L.erase(u._L.begin() + i);	//u��������,�����u���޸Ĳ���Ӱ��ʵ��
				t.L.erase(t.L.begin() + j);
				judge = true;
			}
	pair<horn, bool> res;
	horn temp;
	if (!judge)	//�޷���һ(��һû������)
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
	ofstream out(outfile, ios::app);	//�����д���ļ�
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

/* ������ */
typedef class resolution {
public:
	vector<horn> H;	//�Ӿ伯
	vector<string> predicate;	//ν�ʼ�
	vector<string> variable;	//������
	vector<string> constant;	//������

	void resolute();

	void input();
	void output();
	void output_res(int);
}resolution;

void resolution::resolute()
{
	cout << "��ʼ���:" << endl;
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

		/* ���Թ��H[now]��H[i](i=0,..,(now-1)) */
		for (unsigned int i = 0; i < now; ++i) {
			pair<horn, bool> temp;
			temp = H[i] + H[now];
			if (!temp.second)	//��һʧ��
				continue;
			sort(temp.first.L.begin(), temp.first.L.end());
			sort(temp.first._L.begin(), temp.first._L.end());
			vector<horn>::iterator it = find(H.begin(), H.end(), temp.first);
			if (it != H.end())	//�Ѿ������ظ��Ӿ�
				continue;
			temp.first.id = H.size();
			temp.first.F = 2;
			temp.first.F2.father1 = i;
			temp.first.F2.father2 = now;
			/*cout << "\t(" << now << ")+(" << i << ")\t";
			temp.first.output(predicate, variable, constant);
			cout << endl;*/
			H.push_back(temp.first);
			if (temp.first.empty()) {	//�ɹ����������
				cout << endl << endl << "������:" << endl;
				output_res(H.size() - 1);	//������
				return;
			}
		}

		/* ����H[now]���п��ܵ��û� */
		vector<horn> temp = H[now].substitute(constant);
		for (unsigned int i = 0; i < temp.size(); ++i) {
			vector<horn>::iterator it = find(H.begin(), H.end(), temp[i]);
			if (it != H.end())	//�Ѿ������ظ��Ӿ�
				continue;
			temp[i].id = H.size();
			H.push_back(temp[i]);
		}
	}
	cout << "���ʧ��" << endl;
}

void resolution::input()
{
	ifstream in("input.txt");	//���ļ��ж�������
	if (in.is_open()) {
		int n;
		in >> n;
		while (in.get() != '\n')
			;

		/* ����n���Ӿ� */
		for (int i = 0; i < n; ++i) {
			horn h;
			h.F = 0;	//���Ϊ��ʼ�Ӿ�
			string s;
			getline(in, s);
			int now = 0, len = s.size();

			/* ����1���Ӿ� */
			literal t;
			while (now < len) {
				if (s[now] != ' ') {
					string word;
					while (now < len) {	//����һ������
						if (s[now] == '(' || s[now] == ')' || s[now] == ',')
							break;
						if (s[now] != ' ')
							word += s[now];
						now++;
					}
					if (word.length() != 0)
						if (s[now] != '(') {	//wordΪ����or����
							pair<int, bool> vc;
							if (word[0] == 'x' || word[0] == 'y' || word[0] == 'z') {	//wordΪ����(��'x'/'y'/'z'��ͷ)
								vc.second = true;
								vector<string>::iterator it = find(variable.begin(), variable.end(), word);
								if (it != variable.end())	//�Ѿ�������ͬ����
									vc.first = it - variable.begin();
								else {	//��������ͬ����
									vc.first = variable.size();
									variable.push_back(word);
								}
								t.VC.push_back(vc);
							}
							else {	//wordΪ����
								vc.second = false;
								vector<string>::iterator it = find(constant.begin(), constant.end(), word);
								if (it != constant.end())	//�Ѿ�������ͬ����
									vc.first = it - constant.begin();
								else {	//��������ͬ����
									vc.first = constant.size();
									constant.push_back(word);
								}
								t.VC.push_back(vc);
							}
						}
						else {	//wordΪν�ʷ���
							if (word[0] != '~') {	//wordΪ������
								t._P = true;
								vector<string>::iterator it = find(predicate.begin(), predicate.end(), word);
								if (it != predicate.end())	//�Ѿ�������ͬν��
									t.P = it - predicate.begin();
								else {	//��������ͬν��
									t.P = predicate.size();
									predicate.push_back(word);
								}
							}
							else {	//wordΪ������
								word.erase(0, 1);
								t._P = false;
								vector<string>::iterator it = find(predicate.begin(), predicate.end(), word);
								if (it != predicate.end())	//�Ѿ�������ͬν��
									t.P = it - predicate.begin();
								else {	//��������ͬν��
									t.P = predicate.size();
									predicate.push_back(word);
								}
							}
						}
					if (s[now] == ')') {	//1�������������
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
	case 0:	//��ʼ�Ӿ�
		cout << "(" << head << ")\t";
		H[head].output(predicate, variable, constant);
		cout << endl;
		break;
	case 1: {	//���û�����
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
	case 2:	//�ɺ�һ����
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
	cout << "������ʵ����:" << endl;
	r.output();
	cout << endl << endl;

	r.resolute();
}