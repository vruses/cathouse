#include "iostream"
#include "String"
#include "Vector"
#include "Set"
#include "Map"
#include "iomanip"
#include "Stack"
#include <algorithm>
using namespace std;

//char无法正常表示ε，所以使用$替代
//开始符号
char start;
//非终结符列表
set<char> vN;
//终结符列表
set<char> vT;
//文法列表
vector<string> G;
//文法转换映射  key：产生式左边的非终结符 value：产生式右边的结果
map<char, vector<vector<char>>> GMap;
//FIRST集和FOLLOW集
map<char, set<char>> FIRST, FOLLOW;
//SELECT集   key：不含|的单条文法产生式
map<string, set<char>> SELECT;

//预测分析表
typedef struct PredictiveAnalyticsTable {
	//非终结符
	vector<char> nonTers;
	//终结符和#
	vector<char> ters;
	//表内容
	vector<vector<string>> table;

	/*构建预测分析表*/
	void createPreTable() {
		for (char c : vN) {
			nonTers.push_back(c);
		}
		for (char c : vT) {
			ters.push_back(c);
		}
		ters.push_back('#');
		//遍历每一个非终结符
		for (int i = 0; i < nonTers.size(); ++i) {
			char nonTer = nonTers[i];
			//从SELECT的key中获取所有以nonTer作为左部的文法产生式
			vector<string> generatorStartWithThisNonTer;
			for (auto it = SELECT.begin(); it != SELECT.end(); ++it) {
				if ((*it).first[0] == nonTer) {
					generatorStartWithThisNonTer.push_back((*it).first);
				}
			}
			//通过SELECT集填充table
			vector<string> row;
			//遍历终结符和#
			for (int j = 0; j < ters.size(); ++j) {
				char ter = ters[j];
				//找以nonTer作为左部的文法产生式中有没有SELECT集包含ter的
				bool flag = false;
				for (string generator : generatorStartWithThisNonTer) {
					if (SELECT[generator].find(ter) != SELECT[generator].end()) {
						//SELECT中key的格式都是A->a，产生式右部直接substr(3)即可
						row.push_back(generator.substr(3));
						flag = true; //表示有合适的文法产生式
						break;
					}
				}
				//如果没有合适的文法产生式，就填一个"@empty"
				if (!flag) {
					row.push_back("@empty");
				}
			}
			table.push_back(row);
		}
	}

	/*展示预测分析表*/
	void showPreTable() {
		cout << "预测分析表:" << endl;
		cout << "      " << setw(3) << "\\";
		for (int i = 0; i < ters.size(); ++i) {
			cout << setw(9) << ters[i];
		}
		cout << endl;
		for (int i = 0; i < nonTers.size(); ++i) {
			cout << "      " << setw(3) << nonTers[i];
			for (int j = 0; j < ters.size(); ++j) {
				cout << setw(9) << table[i][j];
			}
			cout << endl;
		}
		cout << endl;
	}

	/*分析判断输入串是否符合文法*/
	//分析过程展示步骤、分析栈、剩余串和产生式4部分，同时要输出分析结果
	void analyzeInputString(string input) {
		//不是#结尾就补一个#
		if (input.at(input.size() - 1) != '#') {
			input += '#';
		}
		//步骤
		int steps = 1;
		//分析栈 (方便打印直接用的string,C++的string可以直接修改) 注意栈首为末尾
		string st = "#";
		//剩余串
		string rest = input;
		//表头
		cout << "      " << setw(5) << "步骤" << setw(14) << "分析栈" << setw(14) << "剩余串" << setw(14) << "产生式\n";

		st += start; //文法开始符号
		//如果分析栈和剩余串都只剩#就表示符合文法
		//如果分析栈不能生成剩余串的首尾字符就表示不符合文法
		while (true) {
			//先打印前面的内容
			cout << "      " << setw(5) << steps << setw(14) << st << setw(14) << rest;
			//如果分析栈和剩余串都只剩#,表示输入串符合文法
			if (st == "#" && rest == "#") {
				cout << setw(14) << "@success" << endl;
				break;
			}
			else if (st == "#") { //分析栈只剩#而剩余串未结束,表示输入串不符合文法
				cout << setw(14) << "@fail" << "  (分析栈已为空)" << endl;
				break;
			}
			else if (rest == "#") { //剩余串只剩#，分析栈必须全为可推出空串的非终结符
				//获取分析栈栈首(st末尾)元素
				char s = st[st.size() - 1];
				//如果是终结符,则已无法符合文法
				if (vT.find(s) != vT.end()) {
					cout << setw(14) << "@fail" << "  (已无法匹配)" << endl;
					break;
				}
				else { //非终结符,根据预测分析表查找是否存在可以推出#的情况
					//找出非终结符对应坐标
					int i = 0;
					for (; i < nonTers.size(); ++i) {
						if (nonTers[i] == s) {
							break;
						}
					}
					//找出#对应坐标 (#在ters末尾)
					int j = ters.size() - 1;
					//如果不存在G->ε，则已无法符合文法
					if (table[i][j] == "@empty") {
						cout << setw(14) << "@fail" << "  (" << s << "无法推导出空串)" << endl;
						break;
					}
					else { //存在G->ε，删除分析栈栈首(st末尾)字符，打印产生式，进入下次执行
						st.pop_back();
						string g = "";
						g += s;
						g.append("->").append(table[i][j]);
						cout << setw(14) << g << endl;
					}
				}
			}
			else { //如果分析栈和剩余串都不为#
				//获取分析栈栈首(st末尾)字符
				char s = st[st.size() - 1];
				//如果分析栈栈首为终结符，就检查剩余串首位字符
				if (vT.find(s) != vT.end()) {
					//如果相同，分析栈移除栈首，剩余串移除首位，打印匹配消除
					if (s == rest.at(0)) {
						st.pop_back();
						rest.erase(0, 1);
						cout << setw(14) << s << endl;
					}
					else { //如果不相同，无法继续匹配剩余串
						cout << setw(14) << "@fail" << "  (已无法匹配)" << endl;
						break;
					}
				}
				else { //栈首为非终结符，通过预测分析表判断是否可以推导出剩余串的首位
					//找出非终结符对应坐标
					int i = 0;
					for (; i < nonTers.size(); ++i) {
						if (nonTers[i] == s) {
							break;
						}
					}
					//找出剩余串首位字符对应坐标
					int j = 0;
					for (; j < ters.size(); ++j) {
						if (ters[j] == rest.at(0)) {
							break;
						}
					}
					//判断是否存在对应的产生式,如果不存在，说明无法继续推导，输入串不符合文法
					if (table[i][j] == "@empty") {
						cout << setw(14) << "@fail" << "  (" << s << "无法推导出" << rest[0] << ")" << endl;
						break;
					}
					else { //如果能推导出相应产生式,移除分析栈栈首,产生式右部逆序入栈
						st.pop_back();
						if (table[i][j] != "$") { //如果推导结果为空串，只移除分析栈栈首
							string gRight = table[i][j];
							for (int k = gRight.size() - 1; k > -1; --k) {
								st.push_back(gRight[k]);
							}
						}
						string g = "";
						g += s;
						g.append("->").append(table[i][j]);
						cout << setw(14) << g << endl;
					}
				}
			}
			++steps;
		}
	}
};
PredictiveAnalyticsTable preTable;


/*初始化函数*/
void init() {
	start = '$';
	vN.clear();
	vT.clear();
	G.clear();
	GMap.clear();
	FIRST.clear();
	FOLLOW.clear();
	SELECT.clear();
}

/*清除字符串中的空格*/
string removeSpaces(const string& s) {
	string tmp(s);
	tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
	return tmp;
}

/*处理输入的文法
完成符号分类(vN|vT)和文法转化(string->map<char, vector<vector<char>>>)*/
void handleGrammarForms() {
	start = G[0][0];
	for (int i = 0; i < G.size(); ++i) {
		//产生式的左右部分分别处理(以字符串中的第一个 "->"切割)
		string str = G[i];
		int idx = str.find("->");
		//处理文法左边
		char gLeft = removeSpaces(str.substr(0, idx))[0];
		vN.insert(gLeft);
		//初始化文法产生式映射GMap
		if (GMap.find(gLeft) == GMap.end()) {
			vector<vector<char>> gRight;
			GMap[gLeft] = gRight;
		}
		//处理文法右边
		string rightStr = removeSpaces(str.substr(idx + 2));
		vector<char> value;
		for (int i = 0; i < rightStr.length(); ++i) {
			char ch = rightStr[i];
			if (ch == '|') {
				//右边根据 | 分割成不同的产生结果
				GMap[gLeft].push_back(value);
				vector<char>().swap(value);
			}
			else {
				//将字符分类
				if ('A' <= ch && ch <= 'Z') {
					vN.insert(ch);
				}
				else if (ch != '$') { //ε没有放在终结符列表中
					vT.insert(ch);
				}
				//放入产生结果中
				value.push_back(ch);
			}
		}
		GMap[gLeft].push_back(value);
	}
}

/*计算FIRST集*/
void getFirst() {
	//1.终结符X∈vT FIRST(X)={X}
	for (char x : vT) {
		set<char> set;
		set.insert(x);
		FIRST[x] = set;
	}

	//2.非终结符X∈vN
	//先给非终结符x构建FIRST(x),设置一个空set作为value
	for (char x : vN) {
		if (FIRST.find(x) == FIRST.end()) {
			set<char> set;
			FIRST[x] = set;
		}
	}
	//向FIRST集添加元素成功的次数，用于判断FIRST集是否有增大
	bool num = 0;
	//重复执行，直到FIRST集不再增大
	do {
		num = 0;
		for (char x : vN) {
			//文法右边的所有情况 (以'|'分割)
			vector<vector<char>> vec = GMap[x];
			//依次分析每种情况
			for (vector<char> v : vec) {
				//首字符
				char a = v[0];
				//首字符为终结符 ： X->a...,a∈vT 则a∈FIRST(X) 
				if (vT.count(a) != 0) {
					if (FIRST[x].insert(a).second) {
						++num;
					}
				}
				//如果X->ε 则ε∈FIRST(X)
				else if (a == '$') {
					if (FIRST[x].insert(a).second) {
						++num;
					}
				}
				//首字符为非终结符
				else {
					//当前的FIRST(x)
					set<char> firstX = FIRST[x];
					//标记当前遍历到的非终结符是否有定义为空,如果有则需要检查下一个字符
					bool isVn = true;
					int i = 0; //产生结果v的索引
					while (isVn) {
						isVn = false;
						if (i >= v.size()) break; //已经全部遍历
						//非终结符
						if (vN.count(v[i]) != 0) {
							//v[i]的FIRST集合
							set<char> firstVi = FIRST[v[i]];
							if (firstVi.size() != 0) {
								for (auto it = firstVi.begin(); it != firstVi.end(); ++it) {
									if (*it == '$') {
										//如果v中所有字符都∈vN且每个都能推出空，ε∈FIRST(X)
										if (i == v.size() - 1) {
											if (firstX.insert(*it).second) {
												++num;
											}
										}
										isVn = true; //该非终结符可以推出空时要考虑下一个字符
									}
									else {
										//FIRST(Yi)-{ε}∈FIRST(X)
										if (firstX.insert(*it).second) {
											++num;
										}
									}
								}
							}
						}
						//终结符直接注入就行
						else {
							if (firstX.insert(v[i]).second) {
								++num;
							}
						}
						++i;
					}
					//记得重新x的FIRST更新... 上面的firstX好像不是和FIRST[x]同一个引用...
					FIRST[x] = firstX;
				}
			}
		}
	} while (num != 0);
}

/*展示FIRST集*/
void showFirst() {
	//输出非终结符的FIRST集验证一下
	cout << "RIRST集:" << endl;
	for (char x : vN) {
		set<char> firstX = FIRST[x];
		cout << "   FIRST(" << x << ")={";
		for (auto it = firstX.begin(); it != firstX.end(); ) {
			string s = "";
			s += *it;
			cout << s;
			if (++it != firstX.end()) {
				cout << ",";
			}
		}
		cout << "}" << endl;
	}
	cout << endl;
}

/*计算FOLLOW集*/
void getFollow() {
	//为每一个A∈vN计算FOLLOW(A),先构建映射
	for (char c : vN) {
		set<char> set;
		FOLLOW[c] = set;
	}
	//文法开始符号
	FOLLOW[start].insert('#');
	//添加元素成功次数，增大到不能再增大
	int num = 0;
	do {
		num = 0;
		//遍历文法
		for (char x : vN) {
			vector<vector<char>> vec = GMap[x];
			for (vector<char> v : vec) {
				//若A->aBb,则FIRST(b)-{ε}∈FOLLOW(B)
				//实际上就是找产生式右边的非终结符，非终结符B右边的字符b的FIRST集合去空后加入FOLLOW(B)
				//A->aB或A->aBb(b->ε),则FOLLOW(A)∈FOLLOW(B)
				for (int i = 0; i < v.size() - 1; ++i) {
					if (vN.count(v[i]) > 0) { //非终结符
						auto first = FIRST[v[i + 1]]; //下一个字符b(v[i+1])的FIRST集
						bool flag = false; //判断是否存在ε∈FIRST(b) b为v[i+1]
						for (char c : first) {
							if (c == '$') {
								flag = true;
								continue;
							}
							if (FOLLOW[v[i]].insert(c).second) {
								++num;
							}
						}
						//如果存在ε∈FIRST(b)并且文法左部表示B本身
						if (flag && x != v[i]) {
							auto follow = FOLLOW[x];
							for (char c : follow) {
								if (FOLLOW[v[i]].insert(c).second) {
									++num;
								}
							}
						}
					}
				}
				//非终结符B在末尾
				int end = v.size() - 1;
				if (vN.count(v[end]) > 0) { //非终结符
					//FOLLOW(A)∈FOLLOW(B)
					auto follow = FOLLOW[x];
					for (char c : follow) {
						if (FOLLOW[v[end]].insert(c).second) {
							++num;
						}
					}
				}
			}
		}
	} while (num > 0);
}

/*展示FOLLOW集*/
void showFollow() {
	cout << "FOLLOW集:" << endl;
	for (char x : vN) {
		set<char> followX = FOLLOW[x];
		cout << "   FOLLOW(" << x << ")={";
		for (auto it = followX.begin(); it != followX.end(); ) {
			string s = "";
			s += *it;
			cout << s;
			if (++it != followX.end()) {
				cout << ",";
			}
		}
		cout << "}" << endl;
	}
	cout << endl;
}

/*判断文法是否满足LL1*/
bool isLL1() {
	//遍历每一个文法产生式
	for (char x : vN) {
		set<char> setX; //用于进行第2点的判断，A的各产生式首字符的FIRST不重复
		bool flag = false; //用于进行第3点的判断
		for (auto v : GMap[x]) {
			//1. 文法不含左递归 这里只检查了是否存在直接左递归(A->Aa)
			if (v[0] == x) {
				cout << "是否满足LL1: 否\n" << endl;
				return false;
			}
			//2. 文法中每一个非终结符A的各个产生式的候选首符集两两不相交
			// 即对于A->a1|a2|…|αn要求FIRST(ai)∩FIRST(aj)= Ø （i≠j）
			if (v[0] == '$') { //空串没写入FIRST
				flag = true; //候选首符集包含ε
			}
			else {
				for (char ch : FIRST[v[0]]) {
					if (ch == '$') {
						flag = true; //候选首符集包含ε
					}
					if (!setX.insert(ch).second) { //插入失败说明存在重复
						cout << "是否满足LL1: 否\n" << endl;
						return false;
					}
				}
			}
			//3. 每个非终结符A，若存在某个候选首符集包含ε，则FIRST(A)∩FOLLOW(A)= Ø
			if (flag) {
				//求FIRST(A)和FOLLOW(A)的交集
				set<char> tmp;
				set_intersection(FIRST[x].begin(), FIRST[x].end(), FOLLOW[x].begin(), FOLLOW[x].end(), inserter(tmp, tmp.begin()));
				if (!tmp.empty()) {
					cout << "是否满足LL1: 否\n" << endl;
					return false;
				}
			}
		}
	}
	cout << "是否满足LL1: 是\n" << endl;
	return true;
}

/*求SELECT集*/
void getSelect() {
	//Select集应该是Select(E→TG)=｛(,i｝样式
	//这里就从文法左部开始，逐个考虑每个文法左部非终结符对应的每个文法表达式
	for (char c : vN) {
		//获取所有对应的产生式右部
		vector<vector<char>> vec = GMap[c];
		//逐个处理每个文法产生式
		for (vector<char> v : vec) {

			//先把文法产生式组合出来(没有直接使用G中储存的文法,因为输入的可能是通过 | 合并的多个产生式)
			string g = "";
			g += c;
			g += "->";
			for (char ch : v) {
				g += ch;
			}
			//初始化文法g的SELECT集
			set<char> set;
			SELECT[g] = set;

			//根据FIRST集和FOLLOW集求文法产生式g对应的SELECT集
			//SELECT(A->a) = a能推出ε ? (FIRST(a)-{ε})+FOLLOW(A) : FIRST(a)-{ε}
			// 变更表示方式： SELECT(A->a)=(FIRST(a)-{ε})[+FOLLOW(A),若有a->ε]
			//此处A为c, a为v
			//遍历a串(v),如果当前字符是可以推出空串的非终结符就继续向后
			int len = v.size();
			for (int i = 0; i < len; ++i) {
				//如果是A->ε,SELECT(A->a)=FOLLOW(A)
				if (len == 1 && v[i] == '$') {
					for (char ch : FOLLOW[c]) {
						SELECT[g].insert(ch);
					}
					break;
				}
				//当前遍历到的字符是非终结符
				if (vN.find(v[i]) != vN.end()) {
					//非空FIRST集入SELECT
					for (char ch : FIRST[v[i]]) {
						if (ch == '$') {
							continue;
						}
						SELECT[g].insert(ch);
					}
					//A->ε
					//如果FIRST[v[i]]不包含ε，则不会存在a->ε，也不需要再向后遍历
					if (FIRST[v[i]].find('$') == FIRST[v[i]].end()) {
						break;
					}
					//如果已经到a串末尾且仍有ε∈FIRST[v[i]]，说明存在a->ε,因此FOLLOW(A)∈SELECT(A->a)
					if (i == len - 1) {
						for (char ch : FOLLOW[c]) {
							SELECT[g].insert(ch);
						}
					}
				}
				//遇到终结符，不会存在a->ε，也不需要再向后遍历
				else {
					//终结符FIRST就是自身，可以不从FIRST取
					SELECT[g].insert(v[i]);
					break; //记得跳出
				}
			}

		}
	}
}

/*展示SELECT集*/
void showSelect() {
	cout << "SELECT集:" << endl;
	for (auto iterator : SELECT) {
		cout << "   SELECT(" << iterator.first << ")={";
		for (auto it = iterator.second.begin(); it != iterator.second.end(); ) {
			string s = "";
			s += *it;
			cout << s;
			if (++it != iterator.second.end()) {
				cout << ",";
			}
		}
		cout << "}" << endl;
	}
	cout << endl;
}

/*消除间接左递归*/
void removeIndirectLeftRecursion() {
	//文法所有非终结符按某一顺序排序
	vector<char> nonTer; //用vector方便点
	nonTer.push_back(start); //开始符号放最前面
	for (char c : vN) {
		if (c != start) nonTer.push_back(c);
	}
	int len = nonTer.size();
	for (int i = 0; i < len; ++i) {
		char b = nonTer[i]; //B

		//1. 将所有A->Bc，B->Cd形式变为A->Cdc
		for (int j = 0; j < i; ++j) {
			char a = nonTer[j]; //A
			vector<vector<char>>& ar = GMap[nonTer[j]]; //A所有产生式的右部(用了&,方便直接操作原数据)
			//查找是否存在A->Bc
			bool flag = false;
			for (auto v : ar) {
				if (v[0] == b) {
					flag = true;
					break;
				}
			}
			//如果存在A->Bc，就查找是否存在B->Cd
			if (flag) {
				vector<vector<char>> tmp; //用于储存满足条件的B->Cd中的右部
				vector<vector<char>>& br = GMap[b]; //B所有产生式的右部
				for (auto& vc : br) {
					//如果产生式首为是不为B的字符
					if (vc[0] != b) {
						tmp.push_back(vc); //满足Cd的产生式放在tmp中
					}
				}
				//将所有A->Bc的情况(只要B为产生式首尾)替换为A->Cdc
				//这里很麻烦，替换的时候size也有相应变化，迭代器也有变化...没写出比较好的方法
				int end = ar.size();       //初始情况时的大小
				vector<vector<char>> old;  //记录需要删除的产生式
				for (int i = 0; i < end; ++i) {
					if (ar[i][0] == b) { //产生式右部首位为B
						//记录需要删除的产生式 (被替换)
						old.push_back(ar[i]);
						//ar加上所有将B替换成Cd的产生式右部的结果
						for (auto t : tmp) {
							vector<char> newRight; //替换出的新产生式
							for (char ch : t) { //先拼接Cd部分
								newRight.push_back(ch);
							}
							for (int j = 1; j < ar[i].size(); ++j) {
								newRight.push_back(ar[i][j]); //再拼接c部分
							}
							ar.push_back(newRight); //加入ar
						}
					}
				}
				//删除被替换的产生式
				for (int i = 0; i < old.size(); ++i) {
					for (auto it = ar.begin(); it != ar.end(); ++it) {
						if (*it == old[0]) { //vector重载了==
							ar.erase(it); //找到就从ar删除
							break;
						}
					}
				}
			}
		}
	}
}

/*打印当前文法*/
void showGMap() {
	for (char x : vN) {
		string s = "";
		s += x;
		s.append("->");
		auto& right = GMap[x];
		for (int i = 0; i < right.size(); ++i) {
			for (char ch : right[i]) {
				s += ch;
			}
			if (i != right.size() - 1) {
				s.append("|");
			}
		}
		cout << "  " << s << endl;
	}
	cout << endl;
}

/*消除直接左递归*/
bool removeDirectLeftRecursion() {
	//将A->Ab|c替换成 A->cA';A'->bA'|$ (实际上就是将左递归改成右递归)
	// 但是这里有个问题: 前面非终结符用的char，表示不了A'这这种格式 (如果用string就解决了)
	// 这里从Z向前找没出现过的大写字母代替(非终结符太多就会出问题)
	for (char a : vN) {
		//查找是否存在A->Ab
		bool flag = false;
		for (auto& i : GMap[a]) {
			if (i[0] == a) {
				flag = true;
				break;
			}
		}
		//如果存在A->Ab情况
		if (flag) {
			//找一个没用过的大写字母当A'
			char a_ = 'Z';
			for (; a_ >= 'A'; --a_) {
				if (vN.find(a_) == vN.end()) {
					vN.insert(a_); //A'加入非终结符列表
					break;
				}
			}
			//终结符太多,大写字母表示不了
			if (a_ < 'A') return false;
			//GMap初始化
			vector<vector<char>> vec;
			GMap[a_] = vec;

			//储存所有A->Ab情况的右部
			vector<vector<char>> abvs;
			//储存所有A->c情况的右部(不含空串)
			vector<vector<char>> cvs;
			//储存替换后A能推导的所有右部 
			// 完成处理后将内容与GMap[a]替换就实现了文法的替换,避免了大量的删除
			vector<vector<char>> aNewRights;
			for (auto& right : GMap[a]) {
				if (right[0] == '$') { //空串保留
					aNewRights.push_back(right);
				}
				else if (right[0] == a) {
					abvs.push_back(right);
				}
				else {
					cvs.push_back(right);
				}
			}
			//遍历处理A->Ab|c  (A->Ab|c => A->cA';A'->bA')
			for (auto& right : abvs) {
				//将A'->bA'加入文法关系映射
				vector<char> tmp;
				for (int i = 1; i < right.size(); ++i) {
					tmp.push_back(right[i]); //b
				}
				tmp.push_back(a_); //bA'
				GMap[a_].push_back(tmp); //A'->bA'
				//将所有A->Ab替换成A->cA'格式并将其中的cA'存入aNewRights
				for (auto& cv : cvs) {
					vector<char> newRight;
					for (char ch : cv) {
						newRight.push_back(ch); //c
					}
					newRight.push_back(a_); //cA'
					aNewRights.push_back(newRight);
				}
			}
			//将A'->$加入文法映射
			vector<char> empty;
			empty.push_back('$');
			GMap[a_].push_back(empty); //A'->$
			//替换A->Ab|c为A->cA'
			swap(aNewRights, GMap[a]); //A->cA'
		}
	}
	return true;
}

/*去除无用产生式*/
void removeUselessGenerators() {
	//暴力解法:直接遍历整个GMap,记录在产生式右侧中出现过的非终结符
	set<char> nonTerUsed; //使用过的非终结符
	nonTerUsed.insert(start); //保证开始符号被加入
	for (char x : vN) {
		for (auto& right : GMap[x]) {
			for (char ch : right) {
				if (vN.find(ch) != vN.end()) {
					nonTerUsed.insert(ch);
				}
			}
		}
	}
	//没在产生式右侧中出现过的非终结符及其文法关系直接删掉
	for (char x : vN) {
		if (nonTerUsed.find(x) == nonTerUsed.end()) { //找到没使用过的非终结符
			GMap.erase(GMap.find(x)); //删除文法关系映射
		}
	}
	swap(nonTerUsed, vN); //将nonTerUser内容给vN
}

/*消除左递归*/
bool removeLeftRecursion() {
	//消除间接左递归
	removeIndirectLeftRecursion();
	cout << "消除间接左递归：" << endl;
	showGMap();
	//消除直接左递归  设计问题:用的char储存vN,无法表示A'形式
	if (!removeDirectLeftRecursion()) {
		cout << "终结符过多无法表示" << endl;
		return false;
	}
	cout << "消除直接左递归：" << endl;
	showGMap();
	//去除无用产生式
	removeUselessGenerators();
	cout << "消除无用产生式：" << endl;
	showGMap();
	return true;
}

/*加载固定文法*/
void loadFixedGrammar() {
	G.clear();
	G.push_back("E->TG");
	G.push_back("G->+TG|$");
	G.push_back("T->FS");
	G.push_back("S->*FS|$");
	G.push_back("F->(E)|i");
	//字符分类，文法转换为map
	handleGrammarForms();
	cout << "文法如下：" << endl;
	showGMap();
}

bool enterGrammar() {
	cout << "请逐行输入文法(输入#停止输入，$表示空串)" << endl;
	string str;
	while (cin >> str) {
		if (str == "#") break;
		G.push_back(str);
	}
	cout << "\n";
	//字符分类，文法转换为map
	handleGrammarForms();
	//消除左递归
	if (!removeLeftRecursion()) {
		//用的char储存,无法表示A'形式,终结符太多时无法储存
		return false;
	}
	return true;
}

void main() {
	//char无法正常表示ε，所以使用$替代
	init();
	//输入文法
	string str;
	cout << "文法输入方式： 1.固定文法  2.手动输入" << endl;
	cin >> str;
	if (str == "1") {
		loadFixedGrammar();
	}
	else if (str == "2") {
		enterGrammar();
	}
	//计算FIRST集和FOLLOW集
	getFirst();
	showFirst();
	getFollow();
	showFollow();
	//判断是否符合LL1 (有个简单点的方法:先求SELECT,然后检查是否有交集)
	if (isLL1()) {
		//SELECT集合
		getSelect();
		showSelect();
		//构建预测分析表
		preTable.createPreTable();
		preTable.showPreTable();
		//对任意输入的字符串进行分析
		cout << "输入任意字符串进行分析(以#结尾，不要用空格分隔)" << endl;
		string str;
		cin >> str;
		preTable.analyzeInputString(str);
	}
}

/*
书P83
S->Qc|c
Q->Rb|b
R->Sa|a
#
=> 消除左递归
S->abcS'|bcS'|cS'
S'abcS'|$
*/