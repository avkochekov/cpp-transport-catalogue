#include "json_builder.h"
#include <string>

json::Builder::Builder()
{
    root_node_ = nullptr;
    nodes_stack_.push_back(&root_node_);
}

json::Builder &json::Builder::Key(const std::string& key)
{
    if (nodes_stack_.empty())
        throw std::logic_error("key insertion failed - invalid node");

    Node *node = nodes_stack_.back();
    if (node->IsDict()){
        node->AsDict()[key] = Node();
        nodes_stack_.push_back(&node->AsDict().at(key));
    } else {
        throw std::logic_error("key insertion failed - invalid node type - not dict");
    }
    return *this;
}

json::Builder &json::Builder::Value(Node::Value value)
{
    if (nodes_stack_.empty())
        throw std::logic_error("value insertion failed - invalid node");

    Node *node = nodes_stack_.back();
    if (node->IsNull()){
        node->GetValue() = value;
        nodes_stack_.pop_back();
    } else if (node->IsArray()){
        Node n;
        n.GetValue() = value;
        node->AsArray().emplace_back(std::move(n));
    } else {
        throw std::logic_error("value insertion failed - invalid node type - not null|array");
    }
    return *this;
}

json::Builder::DictItemContext &json::Builder::StartDict()
{
    if (nodes_stack_.empty())
        throw std::logic_error("start dict failed - invalid node");

    StartContainer(nodes_stack_.back(), Dict());
    static DictItemContext context = DictItemContext(*this);
    return context;
}

json::Builder::ArrayItemContext &json::Builder::StartArray()
{
    if (nodes_stack_.empty())
        throw std::logic_error("start array failed - invalid node");

    StartContainer(nodes_stack_.back(), Array());
    static ArrayItemContext context = ArrayItemContext(*this);
    return context;
}

json::Builder &json::Builder::EndDict()
{
    if (nodes_stack_.empty())
        throw std::logic_error("end dict failed - invalid node");

    if (!nodes_stack_.back()->IsDict())
        throw std::logic_error("node is not dict");

    nodes_stack_.pop_back();
    return *this;
}

json::Builder &json::Builder::EndArray()
{
    if (nodes_stack_.empty())
        throw std::logic_error("end array failed - invalid node");

    if (!nodes_stack_.back()->IsArray())
        throw std::logic_error("node is not array");

    nodes_stack_.pop_back();
    return *this;
}

json::Node json::Builder::Build()
{
    if (!nodes_stack_.empty())
        throw std::logic_error("build node error - uncomplited node");

    return root_node_;
}

json::Builder::DictItemContext::DictItemContext(Builder &b) : builder{b} {
}

json::Builder::DictValueContext &json::Builder::DictItemContext::Key(const std::string &key)
{
    builder.Key(key);
    static DictValueContext context = DictValueContext(builder);
    return context;
}

json::Builder &json::Builder::DictItemContext::EndDict()
{
    return builder.EndDict();
}

json::Builder::ArrayItemContext::ArrayItemContext(Builder &b) : builder{b} {
}

json::Builder::ArrayItemContext &json::Builder::ArrayItemContext::Value(Node::Value value)
{
    builder.Value(value);
    static ArrayItemContext context = ArrayItemContext(builder);
    return context;
}

json::Builder::DictItemContext &json::Builder::ArrayItemContext::StartDict()
{
    return builder.StartDict();
}

json::Builder::ArrayItemContext &json::Builder::ArrayItemContext::StartArray()
{
    return builder.StartArray();
}

json::Builder &json::Builder::ArrayItemContext::EndArray()
{
    return builder.EndArray();
}

json::Builder::DictValueContext::DictValueContext(Builder &b) : builder{b} {
}

json::Builder::DictItemContext &json::Builder::DictValueContext::Value(Node::Value value)
{
    builder.Value(value);
    static DictItemContext context = DictItemContext(builder);
    return context;
}

json::Builder::DictItemContext &json::Builder::DictValueContext::StartDict()
{
    builder.StartDict();
    static DictItemContext context = DictItemContext(builder);
    return context;
}

json::Builder::ArrayItemContext &json::Builder::DictValueContext::StartArray()
{
    builder.StartArray();
    static ArrayItemContext context = ArrayItemContext(builder);
    return context;
}
