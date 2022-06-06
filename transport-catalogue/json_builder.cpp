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

json::DictItemContext &json::Builder::StartDict()
{
    if (nodes_stack_.empty())
        throw std::logic_error("start dict failed - invalid node");

    Node *node = nodes_stack_.back();
    if (node->IsNull()){
        *node = Dict();
    } else if (node->IsArray()){
        node->AsArray().emplace_back(Dict());
        nodes_stack_.push_back(&node->AsArray().back());
    } else {
        throw std::logic_error("value insertion failed - invalid node type - not null|array");
    }
    static DictItemContext context = DictItemContext(*this);
    return context;
}

json::ArrayItemContext &json::Builder::StartArray()
{
    if (nodes_stack_.empty())
        throw std::logic_error("start array failed - invalid node");

    Node *node = nodes_stack_.back();
    if (node->IsNull()){
        *node = Array();
    } else if (node->IsArray()){
        node->AsArray().emplace_back(Array());
        nodes_stack_.push_back(&node->AsArray().back());
    } else {
        throw std::logic_error("value insertion failed - invalid node type - not null|array");
    }
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

json::DictItemContext::DictItemContext(Builder &b) : builder{b} {
}

json::DictValueContext &json::DictItemContext::Key(const std::string &key)
{
    builder.Key(key);
    static DictValueContext context = DictValueContext(builder);
    return context;
}

json::Builder &json::DictItemContext::EndDict()
{
    return builder.EndDict();
}

json::ArrayItemContext::ArrayItemContext(Builder &b) : builder{b} {
}

json::ArrayItemContext &json::ArrayItemContext::Value(Node::Value value)
{
    builder.Value(value);
    static ArrayItemContext context = ArrayItemContext(builder);
    return context;
}

json::DictItemContext &json::ArrayItemContext::StartDict()
{
    return builder.StartDict();
}

json::ArrayItemContext &json::ArrayItemContext::StartArray()
{
    return builder.StartArray();
}

json::Builder &json::ArrayItemContext::EndArray()
{
    return builder.EndArray();
}

json::DictValueContext::DictValueContext(Builder &b) : builder{b} {
}

json::DictItemContext &json::DictValueContext::Value(Node::Value value)
{
    builder.Value(value);
    static DictItemContext context = DictItemContext(builder);
    return context;
}

json::DictItemContext &json::DictValueContext::StartDict()
{
    builder.StartDict();
    static DictItemContext context = DictItemContext(builder);
    return context;
}

json::ArrayItemContext &json::DictValueContext::StartArray()
{
    builder.StartArray();
    static ArrayItemContext context = ArrayItemContext(builder);
    return context;
}
