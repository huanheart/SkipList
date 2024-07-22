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
    cout<<"���� "<<endl;
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
    cout<<"����Ĵ�СΪ : "<<skip.size()<<endl;
    skip.insert_element(char('a'),char('b') ); //�Ѿ��е�Ԫ�ؾͲ����ٽ��и����ˣ�����ͨ����ɾ����insert������ʹ��[]����������в���
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

    //���Գ־û��ļ�����
    skip.dump_file();
    cout<<"````````````"<<endl;
    skip.load_file();
    return 0;
}