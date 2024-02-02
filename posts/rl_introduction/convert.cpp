#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

const bool DEBUG = false;

int main(int argc, char* argv[]) {
    string file;
    ifstream fin;
    if (argc <= 1) {
        cout << "请输入文件名称：";
        cin >> file;
    } else {
        file = argv[1];
    }
    fin.open(file);
    if (!fin.is_open()) {
        cout << "无法打开文件" << endl;
        system("pause");
        return 1;
    }
    // 读入文件
    string code((std::istreambuf_iterator<char>(fin)),
                std::istreambuf_iterator<char>());
    fin.close();
    if (DEBUG) {
        cout << code << endl;
        cout << "-----------------" << endl;
    }
    // 所有数学公式的开头和结尾
    vector<pair<int, int>> maths;
    cout << "开始寻找所有数学公式" << endl;
    for (int i = 0; i < code.length(); i++) {
        if (code[i] == '$') {
            i++;
            bool is_double = false;
            int start = i;
            if (code[i] == '$') {
                is_double = true;
                i++;
                start = i;
            }
            while (i < code.length() && code[i] != '$') {
                i++;
            }
            if (is_double) {
                i++;
            }
            if (i >= code.length() || code[i] != '$') {
                cout << "错误：缺少结束符" << endl;
                system("pause");
                return 1;
            }
            int end = i;
            maths.push_back(pair<int, int>(start, end));
            cout << start << " " << end << endl;
            if (DEBUG) {
                string math = code.substr(start, end - start);
                cout << math << endl;
            }
        }
    }
    cout << "寻找完成，开始处理" << endl;
    int count = 0;
    for (int i = 0; i < maths.size(); i++) {
        for (int j = maths[i].first + count; j < maths[i].second + count; j++) {
            if (code[j] == '_') {
                code.insert(j, "\\");
                count++;
                j++;
            } else if (code[j] == '\\') {
                j++;
                if (code[j] == '\\') {
                    code.insert(j, "\\\\");
                    count += 2;
                    j += 2;
                } else if (code[j] == '{' || code[j] == '}') {
                    code.insert(j, "\\");
                    count++;
                    j++;
                }
            }
        }
    }
    if (DEBUG) {
        cout << code << endl;
    }
    cout << "处理完成，开始写入文件" << endl;
    ofstream fout;
    string location = file.substr(0, file.find_last_of('\\') + 1);
    fout.open(location + "index.md");
    if (!fout.is_open()) {
        cout << "无法打开文件" << endl;
        system("pause");
        return 1;
    }
    fout << code;
    return 0;
}