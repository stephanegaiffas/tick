
#include <memory>
#include <unordered_map>

#include "kul/log.hpp"
#include "kul/time.hpp"

class Node;
class Tree{
  using MapType = std::unordered_map<uint32_t, Node>;
  private:
    uint32_t m_count = 0;
    std::unique_ptr<Node> m_root;
    std::unique_ptr<MapType> m_nodes;
  public:
    Tree();
    Node& insert(const uint32_t& _v);
    Node& root(){
      return *m_root.get();
    }
    size_t size(){
      return m_count;
    }
};
class Node{
  private:
    uint32_t m_value = 0;
    Tree *m_tree = nullptr;
    Node *m_left = nullptr, *m_right = nullptr, *m_parent = nullptr;
  public:
    Node(Tree* tree) : m_tree(tree){
    }
    Node(Tree* tree, uint32_t _value) : m_tree(tree), m_value(_value){
    }
    Node(const Node& o){
      m_value = o.m_value;
      m_tree  = o.m_tree;
      m_left  = o.m_left;
      m_right = o.m_right;
    }

    Node(const Node&& n) = delete;
    Node& operator=(const Node&) = delete;
    Node& operator=(const Node&&) = delete;
    ~Node(){}

    Node& fill(){
      m_left  = &m_tree->insert(1);
      m_left->m_parent = this;
      m_right = &m_tree->insert(2);
      m_right->m_parent = this;
      m_value = 0;
      return *this;
    }

    uint32_t value() { return m_value; }

    Tree* tree()     { return m_tree; }

    Node* parent()   { return m_parent; }
    Node* left()     { return m_left;  }
    Node* right()    { return m_right; }
};

Tree::Tree(){
  m_root.reset(new Node(this));
  m_nodes.reset(new MapType());
}
Node& Tree::insert(const uint32_t& _v){
  auto it(m_nodes->emplace(m_count++, Node(this, _v)).first);
  return it->second;
}

int main(int argc, char* argv[]){

  auto now = kul::Now::NANOS();

  Tree tree;
  Node &root(tree.root()), *s_left = nullptr, *s_right = nullptr;
  root.fill();
  s_left  = root.left();
  s_right = root.right();
  for(size_t i = 0; i < 654321; i++){
    s_left->fill();
    s_left  = s_left->left();
    s_right->fill();
    s_right = s_right->right();      
  }
  while(s_left->parent())  s_left  = s_left->parent();
  while(s_right->parent()) s_right = s_right->parent();

  KLOG(INF) << "time in millis : " << (kul::Now::NANOS() - now) / 1e6;
  KLOG(INF) << tree.size();

  return 0;
}
