#include<iostream>
#include<string> 
#include"skiplist.h"

using namespace std;

int main()
{
    Node<int,string> * s=new Node<int,string> ();
    Node<int,string> h;
    cout<<"````````````````````"<<endl;
    delete s;
    cout<<"跳表 "<<endl;
    SkipList<char,char> skip(18);
    for(int i=0;i<5;i++)
    {
        skip.insert_element(char(i+'a'),char(i+'b') );
    }
    skip.display_list();
    skip.search_element('a');
    skip.delete_element('a');
    skip.search_element('a');
    skip.display_list();
    cout<<"跳表的大小为 : "<<skip.size()<<endl;
    skip.insert_element(char('a'),char('b') ); //已经有的元素就不会再进行更改了，可以通过先删除再insert，或者使用[]运算符来进行操作
    skip.search_element('a');
    skip.insert_element(char('a'),char('k') );
    skip.search_element('a');
    skip.delete_element('z');

    cout<<"[]operator test "<<endl;

    // cout<<skip['a']<<endl;
    cout<<skip['a']<<endl;
    skip['a']='k';
    cout<<skip['a']<<endl;
    skip['z'];
    cout<<skip['z']<<endl;

    //测试持久化文件部分
    skip.dump_file();
    cout<<"````````````"<<endl;
    skip.load_file();
    return 0;
}