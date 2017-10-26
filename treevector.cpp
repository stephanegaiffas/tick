
#include <memory>
#include <vector>

#include "kul/log.hpp"
#include "kul/time.hpp"

class Node;
class Tree{
  friend class Node;
  private:
    size_t m_count = 0;
    std::vector<Node> m_nodes;

    Tree(const Tree& o) = delete;
    Tree(const Tree&& o) = delete;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(const Tree&&) = delete;
  public:
    Tree();
    size_t insert(const uint32_t& _v);
    Node& root(){
      return m_nodes[0];
    }
    Node& node(size_t index){
      return m_nodes[index];
    }
    size_t get_next(){
      return m_count++;
    }
    size_t size(){
      return m_nodes.size();
    }
};
class Node{
  private:
    Tree &m_tree;
    size_t m_value = 0;
    size_t i_index = 0, i_left = 0, i_right = 0, i_parent;
  public:
    Node(Tree& tree) : m_tree(tree){}
    Node(Tree& tree, size_t _value, size_t _i) 
      : m_tree(tree), m_value(_value), i_index(_i){
    }
    Node(const Node& o) : m_tree(o.m_tree){
      m_value = o.m_value;
      i_left  = o.i_left;
      i_right = o.i_right;
      i_index = o.i_index;
    }
    Node(const Node&& o) : m_tree(o.m_tree){
      m_value = o.m_value;
      i_left  = o.i_left;
      i_right = o.i_right;
      i_index = o.i_index;
    }
    ~Node(){}
    Node& operator=(const Node&) = delete;
    Node& operator=(const Node&&) = delete;

    void fill(){
      size_t index = i_index;
      Tree* tree = &m_tree;
      tree->insert(1);
      tree->m_nodes[tree->m_count].i_parent = index;

      tree->insert(1);
      tree->m_nodes[tree->m_count].i_parent = index;

      tree->m_nodes[index].m_value = 0;
    }

    bool has_parent() { return i_index; }
    size_t left()     { return i_left;  }
    size_t right()    { return i_right; }
    size_t value()    { return m_value; }
    size_t parent()   { return i_parent; }
};

Tree::Tree(){
  m_nodes.emplace_back(*this, 1, m_count++);
}
size_t Tree::insert(const uint32_t& _v){
  m_nodes.emplace_back(*this, _v, m_count);
  return m_count++;
}

int main(int argc, char* argv[]){

  auto now = kul::Now::NANOS();

  Tree tree;
  Node &root(tree.root());
  root.fill();

  size_t i_left = root.left(), i_right = root.right();
  for(size_t i = 0; i < 654321; i++){
    tree.node(i_left).fill();
    i_left  = tree.node(i_left).left();
    tree.node(i_right).fill();
    i_right  = tree.node(i_right).left();
    
  }
  while(tree.node(i_left).has_parent())  i_left  = tree.node(i_left).parent();
  while(tree.node(i_right).has_parent()) i_right = tree.node(i_right).parent();

  KLOG(INF) << "time in millis : " << (kul::Now::NANOS() - now) / 1e6;
  KLOG(INF) << tree.size();

  return 0;
}
