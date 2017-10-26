
#include <memory>
#include "kul/log.hpp"
#include "kul/time.hpp"

class Node;
class Tree{
  friend class Node;
  private:
    size_t m_count = 0;
    Node* m_nodes = nullptr;

  public:
    Tree(size_t size);
    ~Tree();

    Tree(const Tree& o) = delete;
    Tree(const Tree&& o) = delete;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(const Tree&&) = delete;

    Node& insert(const uint32_t& _v);
    Node& root();
    Node& node(size_t index);
    size_t size(){
      return m_count;
    }
};

class Node{
  private:
    Tree *m_tree = nullptr;
    size_t m_value = 0;    
    size_t i_index = 0;
    Node *m_left = nullptr, *m_right = nullptr, *m_parent = nullptr;
  public:
    Node(){}
    Node(Tree* tree) : m_tree(tree){}
    Node(Tree* tree, size_t _value, size_t _i) 
      : m_tree(tree), m_value(_value), i_index(_i){
    }
    ~Node(){}

    Node(const Node& o) = delete;
    Node(const Node&& o) = delete;
    Node& operator=(const Node&) = delete;
    Node& operator=(const Node&&) = delete;

    Node& fill(){
      m_left  = &m_tree->insert(1);
      m_right = &m_tree->insert(2);      
      m_left->m_tree  = m_tree;      
      m_right->m_tree = m_tree;
      m_left->m_parent  = this;
      m_right->m_parent = this;      
      m_value = 0;
      return *this;
    }

    size_t value() { return m_value; }
    Node* parent() { return m_parent; }
    Node* left()   { return m_left;  }
    Node* right()  { return m_right; }
};

Tree::Tree(size_t size){
  m_nodes = new Node[size]{{this, 2, 0}};
}
Tree::~Tree(){
  if(m_nodes) delete[] m_nodes;
}
Node& Tree::root(){
  return m_nodes[0];
}
Node& Tree::node(size_t index){
  return m_nodes[index];
}

Node& Tree::insert(const uint32_t& _v){
  return m_nodes[m_count++];
}

int main(int argc, char* argv[]){
  auto now = kul::Now::NANOS();
  Tree tree((654321 * 4) + 5);
  Node &root(tree.root()), *s_left = nullptr, *s_right = nullptr;
  root.fill();
  s_left  = root.left();
  s_right = root.right();
  for(size_t i = 0; i < 654321; i++){
    s_left  = s_left ->fill().left();
    s_right = s_right->fill().right();
  }
  while(s_left  != &root) s_left  = s_left->parent();
  while(s_right != &root) s_right = s_right->parent();

  KLOG(INF) << "time in millis : " << (kul::Now::NANOS() - now) / 1e6;
  KLOG(INF) << tree.size();


  return 0;
}
