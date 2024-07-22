#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include<sstream>
#include <type_traits>
#include <stdexcept>

//ͨ��ģ��˵��
template <typename T>
T string_to_value(const std::string & str);
//��������ػ��汾,Ŀǰֻ֧��int,char,string,bool,long long����
//����int����
template<>
int string_to_value(const std::string & str){
    return std::stoi(str);
}
// char����
template<>
char string_to_value<char>(const std::string& str) {
    if(!str.size())
        return ' ';
    return str[0];
}
// �ַ�������
template<>
std::string string_to_value<std::string>(const std::string& str) {
    return str;
}
//bool ����
template<>
bool string_to_value<bool>(const std::string& str) {
    return str == "true" || str == "1";
}
//long long ����
template<>
long long string_to_value<long long>(const std::string& str) {
    return std::stoll(str);
}

std::mutex mtx;
std::string delimiter=":"; //�ָ���

#define STORE_FILE "store/dumpFile"

template<typename K,typename V>
class Node{

public:
    //�����޲ι����ʱ����Ҫ������Ϊ�գ����������ջ�Ͽ��ٵ�node���ͽ����ͷ�һ��Ұָ�룬�������ǵó�ʼ����
    Node():forward(nullptr),node_level(0){}

    Node(K k,V v,int );

    ~Node();

    K get_key() const ;

    V& get_value() ;

    void set_value(V);
    //��һ����һ������ָ�룬����һ��ָ��ָ�����飨������ȫ��ָ�룩������
    Node<K,V> ** forward;
    //������㵱ǰ������һ��
    int node_level;
private:
    K key;
    V value;
};


template<typename K,typename V>
Node<K,V>::Node(const K k,const V v,int level)
{
    this->key=k;
    this->value=v;
    this->node_level=level;
    //forwardָ��һ��ָ������
    this->forward=new Node<K,V>*[level+1];
    //���г�ʼ��
    memset(this->forward,0,sizeof(Node<K,V>*)*(level+1) );
}

template<typename K,typename V>
Node<K,V>::~Node()
{
    delete []forward; //�ͷŵ����ָ��,�����е�ָ���ͨ�������е�clear���еݹ�ɾ��
}

template<typename K,typename V>
K Node<K,V>::get_key() const 
{
    return key;
}

template<typename K,typename V>
V& Node<K,V>::get_value() 
{
    return value;
}

template<typename K,typename V>
void Node<K,V>::set_value(V value)
{
    this->value=value;
}

enum DecideExist
{
    Exist,
    NoExist
};


//������
template<typename K,typename V>
class SkipList
{
public:
    SkipList(int );
    ~SkipList();
    V& operator[](const K& );
    int insert_element(K,V);
    void display_list();
    bool search_element(K);
    bool update(K ,V ,V);
    void delete_element(K);
    void dump_file();
    void load_file();
    //�ݹ�ɾ���ڵ�
    void clear(Node<K,V>*);
    int size();
private:
    Node<K,V> * internal_search_element(K);
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);
    int get_random_level();
    Node<K,V> * create_node(K,V,int);
private:
    //�����������������
    int _max_level;

    //��ǰ����������
    int _skip_list_level;

    Node<K,V> * _header;
    //��,���ڳ־û����ļ���
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    //��ǰ�����к��е�����
    int _element_count;
};

template<typename K, typename V>
Node<K, V>* SkipList<K,V>::create_node(const K k,const V v,int level)
{
    Node<K, V>* back=new Node<K, V>(k,v,level);
    return back;
}

//�����еĲ���
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value)
{
    mtx.lock();
    Node<K,V>* current=this->_header;

    //����һ����������,�����������һ���������
    Node<K,V>* update[_max_level+1];
    memset(update,0,sizeof(Node<K,V>*)*(_max_level+1) );

    //����߲㿪ʼ�������Ԫ��Ӧ�ñ����뵽����
    for(int i=_skip_list_level;i>=0;i--){
        while(current->forward[i]!=nullptr && current->forward[i]->get_key()<key){
            current=current->forward[i];
        }
        update[i]=current;
    }

    //ָ��Ӧ�ò����λ��,0������Ϊ��0�㺬������Ԫ��
    current=current->forward[0];

    //˵��ԭ���������Ԫ��
    if(current!= nullptr && current->get_key()==key){
        std::cout<<"key: "<<key<<" exists"<<std::endl;
        mtx.unlock();
        return 1;
    }
    if(current== nullptr || current->get_key()!=key ){
        int random_level = get_random_level();
        //����ý�㱻�����λ����ԭ��Ҫ�ߵĲ�����λ��,����֮ǰû�г�ʼ��update�����_skip_list_level�����ⲿ��,��Ȼ��Ҫ����ָ��header
        //��Ϊ֮ǰû�н��
        if(random_level>_skip_list_level){
            for(int i=_skip_list_level+1;i<random_level+1;i++){
                update[i]=_header;
            }
            _skip_list_level=random_level;
        }
        Node<K,V>* inserted_node=create_node(key,value,random_level);
        //��ʼ����
        for(int i=0;i<=random_level;i++){
            inserted_node->forward[i]=update[i]->forward[i];
            update[i]->forward[i]=inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        _element_count ++;
    }
    mtx.unlock();
    return 0;
}

//��ȡ�����д洢��Ԫ�ظ���
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;
}

//չʾ�����еĽṹ
template<typename K, typename V> 
void SkipList<K,V>::display_list()
{
    std::cout<<"Now let's start showing the SkipList"<<std::endl;
    for(int i=0;i<=_skip_list_level;i++)
    {
        Node<K,V>* node=this->_header->forward[i];
        std::cout<<"level "<<i <<" : ";
        while(node!=nullptr){
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }

}

//���ڴ��е����ݳ־û����ļ���
template<typename K, typename V> 
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file-----------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0]; 

    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return ;
}

//���ļ��е����ݼ��ص��ڴ���
template<typename K, typename V> 
void SkipList<K, V>::load_file() {

    _file_reader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        // ������ת��Ϊint���ͣ�������һ�����⣬����
        insert_element(string_to_value<K>(*key), string_to_value<V>(*value) );
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    delete key;
    delete value;
    _file_reader.close();
}

template<typename K, typename V>
bool SkipList<K,V>::is_valid_string(const std::string &str)
{
    //�鿴�Ƿ�Ϊ���Լ��鿴�ܷ��ҵ��ָ���
    if(str.empty()||str.find(delimiter)==std::string::npos)
        return false;   
    return true;
}


template<typename K, typename V>
void SkipList<K,V>::get_key_value_from_string(const std::string& str,std::string* key,std::string * value)
{
    if(!is_valid_string(str))
        return ;
    *key=str.substr(0,str.find(delimiter) );
    *value=str.substr(str.find(delimiter)+1,str.length() );
}

//ɾ��Ԫ��
template<typename K, typename V> 
void SkipList<K, V>::delete_element(K key)
{
    mtx.lock();
    Node<K,V> * current =this->_header;
    Node<K,V> * update[_max_level+1];
    memset(update,0,sizeof(Node<K,V>*)*(_max_level+1) );

    // ����߲㿪ʼȥѰ��,ͨ������ͬ����ұ��ߣ���������
    for(int i=_skip_list_level;i>=0;i--)
    {
        while(current->forward[i]!=nullptr && current->forward[i]->get_key()<key){
            current=current->forward[i];
        }
        update[i]=current;
    }
    current=current->forward[0];
    if(current!=nullptr && current->get_key()==key){
        //˵�����ڸý�㣬��ô��ʼ����һ��ɾ�����Ĳ���
        for(int i=0;i<=_skip_list_level;i++){
            if(update[i]->forward[i]!=current)
                break;       //break��ԭ����˵��������Ĳ㶼û���������ˣ���Ϊ�ý��ֻ���������ǰi�������
            update[i]->forward[i]=current->forward[i]; 
        }
        while(_skip_list_level>0&&_header->forward[_skip_list_level]==0){
            _skip_list_level--;
        }
        std::cout << "Successfully deleted key "<< key << std::endl;
        delete current;
        _element_count--;
    }
    mtx.unlock();
    return ;
}

//�ڲ�����Ԫ���õ�
template<typename K, typename V> 
Node<K,V> * SkipList<K, V>::internal_search_element(K key)
{
    std::cout << "  internal_search_element-----------------" << std::endl;
    Node<K,V> * current=_header;
    //����߲�ȥ��,��ͬ�����ң�Ȼ������ȥ��
    for(int i=_skip_list_level;i>=0;i--)
    {
        while(current->forward[i] && current->forward[i]->get_key() < key){
            current=current->forward[i];
        }
    }

    //���ڵ�0��ض���������Ԫ�أ���Ȼ���ǽ���ָ���0��
    current=current->forward[0];

    if(current && current->get_key()==key){
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return current;
    }
    std::cout << "Not Found Key:" << key << std::endl;
    return nullptr;
}


//�ⲿ����Ԫ�أ������ж�Ԫ���Ƿ����
template<typename K, typename V> 
bool SkipList<K, V>::search_element(K key)
{
    std::cout << "search_element-----------------" << std::endl;
    Node<K,V> * current=_header;
    //����߲�ȥ��,��ͬ�����ң�Ȼ������ȥ��
    for(int i=_skip_list_level;i>=0;i--)
    {
        while(current->forward[i] && current->forward[i]->get_key() < key){
            current=current->forward[i];
        }
    }

    //���ڵ�0��ض���������Ԫ�أ���Ȼ���ǽ���ָ���0��
    current=current->forward[0];

    if(current && current->get_key()==key){
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }
    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}


//����һ������
template<typename K, typename V> 
SkipList<K, V>::SkipList(int max_level) {

    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // create header node and initialize key and value to null
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
};

template<typename K, typename V> 
SkipList<K, V>::~SkipList() 
{
    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    //ʹ��RAII˼��
    if(_header->forward[0]!=nullptr){
        clear(_header->forward[0]);
    }
    delete(_header);
}

//�ݹ������0�����һ��ɾ��
template <typename K, typename V>
void SkipList<K, V>::clear(Node<K,V> * cur)
{
    if(cur->forward[0]!=nullptr){
        clear(cur->forward[0]);
    }
    delete(cur);
}

//���һ��Ԫ�صĲ���,��Ҳ��Ϊʲô�ռ临�ӶȻ���o(NlogN)��ԭ��
template <typename K, typename V>
int SkipList<K, V>::get_random_level()
{
    int k=1;
    while(rand()%2){
        k++;
    }
    k=(k<_max_level) ? k : _max_level;
    return k;
}

template <typename K, typename V>
bool SkipList<K, V>::update(K key,V old_value,V new_value)
{
    delete_element(key);
    insert_element(key,new_value);
    return true;
}

template <typename K, typename V>
V& SkipList<K, V>::operator[](const K& key)
{
    //�Ȳ���һ���Ƿ�������key
    Node<K,V> *back =internal_search_element(key);
    if(back!=nullptr)
        return back->get_value();
    //�����Լ���������һ���µļ�ֵ
    insert_element(key,V{});
    return internal_search_element(key)->get_value();
}