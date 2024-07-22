#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include<sstream>
#include <type_traits>
#include <stdexcept>

//通用模板说明
template <typename T>
T string_to_value(const std::string & str);
//下面进行特化版本,目前只支持int,char,string,bool,long long类型
//整形int类型
template<>
int string_to_value(const std::string & str){
    return std::stoi(str);
}
// char类型
template<>
char string_to_value<char>(const std::string& str) {
    if(!str.size())
        return ' ';
    return str[0];
}
// 字符串类型
template<>
std::string string_to_value<std::string>(const std::string& str) {
    return str;
}
//bool 类型
template<>
bool string_to_value<bool>(const std::string& str) {
    return str == "true" || str == "1";
}
//long long 类型
template<>
long long string_to_value<long long>(const std::string& str) {
    return std::stoll(str);
}

std::mutex mtx;
std::string delimiter=":"; //分隔符

#define STORE_FILE "store/dumpFile"

template<typename K,typename V>
class Node{

public:
    //进行无参构造的时候需要将其置为空，否则如果在栈上开辟的node类型将会释放一个野指针，所以我们得初始化好
    Node():forward(nullptr),node_level(0){}

    Node(K k,V v,int );

    ~Node();

    K get_key() const ;

    V& get_value() ;

    void set_value(V);
    //这一个是一个二级指针，做到一个指向指针数组（数组中全是指针）的作用
    Node<K,V> ** forward;
    //表明结点当前层是哪一层
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
    //forward指向一个指针数组
    this->forward=new Node<K,V>*[level+1];
    //进行初始化
    memset(this->forward,0,sizeof(Node<K,V>*)*(level+1) );
}

template<typename K,typename V>
Node<K,V>::~Node()
{
    delete []forward; //释放掉这个指针,数组中的指针会通过跳表中的clear进行递归删除
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


//跳表部分
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
    //递归删除节点
    void clear(Node<K,V>*);
    int size();
private:
    Node<K,V> * internal_search_element(K);
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);
    int get_random_level();
    Node<K,V> * create_node(K,V,int);
private:
    //跳表所允许的最大层数
    int _max_level;

    //当前跳表最大层数
    int _skip_list_level;

    Node<K,V> * _header;
    //流,用于持久化到文件的
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    //当前跳表中含有的数量
    int _element_count;
};

template<typename K, typename V>
Node<K, V>* SkipList<K,V>::create_node(const K k,const V v,int level)
{
    Node<K, V>* back=new Node<K, V>(k,v,level);
    return back;
}

//跳表中的插入
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value)
{
    mtx.lock();
    Node<K,V>* current=this->_header;

    //创建一个更新数组,方便后续进行一个插入操作
    Node<K,V>* update[_max_level+1];
    memset(update,0,sizeof(Node<K,V>*)*(_max_level+1) );

    //从最高层开始查找这个元素应该被放入到哪里
    for(int i=_skip_list_level;i>=0;i--){
        while(current->forward[i]!=nullptr && current->forward[i]->get_key()<key){
            current=current->forward[i];
        }
        update[i]=current;
    }

    //指向本应该插入的位置,0层是因为第0层含有所有元素
    current=current->forward[0];

    //说明原本就有这个元素
    if(current!= nullptr && current->get_key()==key){
        std::cout<<"key: "<<key<<" exists"<<std::endl;
        mtx.unlock();
        return 1;
    }
    if(current== nullptr || current->get_key()!=key ){
        int random_level = get_random_level();
        //如果该结点被随机定位到比原先要高的层数的位置,由于之前没有初始化update上面的_skip_list_level往上这部分,固然需要让他指向header
        //因为之前没有结点
        if(random_level>_skip_list_level){
            for(int i=_skip_list_level+1;i<random_level+1;i++){
                update[i]=_header;
            }
            _skip_list_level=random_level;
        }
        Node<K,V>* inserted_node=create_node(key,value,random_level);
        //开始插入
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

//获取跳表中存储的元素个数
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;
}

//展示跳表中的结构
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

//将内存中的数据持久化到文件中
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

//将文件中的数据加载到内存中
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
        // 将类型转化为int类型，但是有一个问题，就是
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
    //查看是否为空以及查看能否找到分隔符
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

//删除元素
template<typename K, typename V> 
void SkipList<K, V>::delete_element(K key)
{
    mtx.lock();
    Node<K,V> * current =this->_header;
    Node<K,V> * update[_max_level+1];
    memset(update,0,sizeof(Node<K,V>*)*(_max_level+1) );

    // 从最高层开始去寻找,通过先向同层的右边走，再往下走
    for(int i=_skip_list_level;i>=0;i--)
    {
        while(current->forward[i]!=nullptr && current->forward[i]->get_key()<key){
            current=current->forward[i];
        }
        update[i]=current;
    }
    current=current->forward[0];
    if(current!=nullptr && current->get_key()==key){
        //说明存在该结点，那么开始进行一个删除结点的操作
        for(int i=0;i<=_skip_list_level;i++){
            if(update[i]->forward[i]!=current)
                break;       //break的原因是说明往上面的层都没有这个结点了，因为该结点只被随机到当前i的下面层
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

//内部查找元素用的
template<typename K, typename V> 
Node<K,V> * SkipList<K, V>::internal_search_element(K key)
{
    std::cout << "  internal_search_element-----------------" << std::endl;
    Node<K,V> * current=_header;
    //从最高层去找,先同层往右，然后向下去找
    for(int i=_skip_list_level;i>=0;i--)
    {
        while(current->forward[i] && current->forward[i]->get_key() < key){
            current=current->forward[i];
        }
    }

    //由于第0层必定含有所有元素，固然我们将其指向第0层
    current=current->forward[0];

    if(current && current->get_key()==key){
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return current;
    }
    std::cout << "Not Found Key:" << key << std::endl;
    return nullptr;
}


//外部查找元素，用于判断元素是否存在
template<typename K, typename V> 
bool SkipList<K, V>::search_element(K key)
{
    std::cout << "search_element-----------------" << std::endl;
    Node<K,V> * current=_header;
    //从最高层去找,先同层往右，然后向下去找
    for(int i=_skip_list_level;i>=0;i--)
    {
        while(current->forward[i] && current->forward[i]->get_key() < key){
            current=current->forward[i];
        }
    }

    //由于第0层必定含有所有元素，固然我们将其指向第0层
    current=current->forward[0];

    if(current && current->get_key()==key){
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }
    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}


//建造一个跳表
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
    //使用RAII思想
    if(_header->forward[0]!=nullptr){
        clear(_header->forward[0]);
    }
    delete(_header);
}

//递归遍历第0层进行一个删除
template <typename K, typename V>
void SkipList<K, V>::clear(Node<K,V> * cur)
{
    if(cur->forward[0]!=nullptr){
        clear(cur->forward[0]);
    }
    delete(cur);
}

//随机一个元素的层数,这也是为什么空间复杂度会是o(NlogN)的原因
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
    //先查找一下是否存在这个key
    Node<K,V> *back =internal_search_element(key);
    if(back!=nullptr)
        return back->get_value();
    //否则自己给它创建一个新的键值
    insert_element(key,V{});
    return internal_search_element(key)->get_value();
}