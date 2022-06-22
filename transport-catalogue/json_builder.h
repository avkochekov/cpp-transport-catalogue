#pragma once

#include <string>
#include <optional>
#include "json.h"

namespace json {


class Builder{
    Node root_node_;
    std::vector<Node*> nodes_stack_;

    template<typename Container>
    Builder& StartContainer(Node *node, Container &&container);

public:
    class DictItemContext;
    class DictValueContext;
    class ArrayItemContext;

    Builder();
    Builder& Key(const std::string& key);   // Задаёт строковое значение ключа для очередной пары ключ-значение. Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение с помощью метода Value или начинать его определение с помощью StartDict или StartArray.
    Builder& Value(Node::Value);            // Задаёт значение, соответствующее ключу при определении словаря, очередной элемент массива или, если вызвать сразу после конструктора json::Builder, всё содержимое конструируемого JSON-объекта. Может принимать как простой объект — число или строку — так и целый массив или словарь. Здесь Node::Value — это синоним для базового класса Node, шаблона variant с набором возможных типов-значений. Смотрите заготовку кода.
    DictItemContext& StartDict();           // Начинает определение сложного значения-словаря. Вызывается в тех же контекстах, что и Value. Следующим вызовом обязательно должен быть Key или EndDict.
    ArrayItemContext& StartArray();         // Начинает определение сложного значения-массива. Вызывается в тех же контекстах, что и Value. Следующим вызовом обязательно должен быть EndArray или любой, задающий новое значение: Value, StartDict или StartArray.
    Builder& EndDict();                     // Завершает определение сложного значения-словаря. Последним незавершённым вызовом Start* должен быть StartDict.
    Builder& EndArray();                    // Завершает определение сложного значения-массива. Последним незавершённым вызовом Start* должен быть StartArray.
    Node Build();                           // Возвращает объект json::Node, содержащий JSON, описанный предыдущими вызовами методов. К этому моменту для каждого Start* должен быть вызван соответствующий End*. При этом сам объект должен быть определён, то есть вызов json::Builder{}.Build() недопустим.

    class DictItemContext{
        Builder &builder;

    public:
        DictItemContext(Builder &);
        DictValueContext& Key(const std::string& key);
        Builder& EndDict();
    };

    class ArrayItemContext{
        Builder &builder;

    public:
        ArrayItemContext(Builder &);
        ArrayItemContext& Value(Node::Value);
        DictItemContext& StartDict();
        ArrayItemContext& StartArray();
        Builder& EndArray();
    };

    class DictValueContext{
        Builder &builder;

    public:
        DictValueContext(Builder &);
        DictItemContext& Value(Node::Value);
        DictItemContext& StartDict();
        ArrayItemContext& StartArray();
    };
};

template<typename Container>
inline Builder &Builder::StartContainer(Node *node, Container &&container)
{
    if (node->IsNull()) {
        *node = std::move(container);
    } else if (node->IsArray()) {
        node->AsArray().emplace_back(std::move(container));
        nodes_stack_.push_back(&node->AsArray().back());
    } else {
        throw std::logic_error("value insertion failed - invalid node type - not null|array");
    }
    return *this;
}


}
