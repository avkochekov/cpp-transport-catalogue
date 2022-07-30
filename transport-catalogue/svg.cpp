#include "svg.h"

#include <algorithm>
#include <unordered_map>

namespace svg {

using namespace std::literals;

static const std::unordered_map<char, std::string> text_replace_symbols = {
    {'\"', "&quot;"},
    {'\'', "&apos;"},
    {'<', "&lt;"},
    {'>', "&gt;"},
    {'&', "&amp;"}};

static const std::unordered_map<StrokeLineCap, std::string> map_StrokeLineCap = {
    {StrokeLineCap::BUTT, "butt"},
    {StrokeLineCap::ROUND, "round"},
    {StrokeLineCap::SQUARE, "square"},
};

static const std::unordered_map<StrokeLineJoin, std::string> map_StrokeLineJoin = {
    {StrokeLineJoin::ARCS, "arcs"},
    {StrokeLineJoin::BEVEL, "bevel"},
    {StrokeLineJoin::MITER, "miter"},
    {StrokeLineJoin::MITER_CLIP, "miter-clip"},
    {StrokeLineJoin::ROUND, "round"},
};

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline &Polyline::AddPoint(Point point)
{
    points.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext &context) const
{
    auto& out = context.out;
    out << "<polyline points=\""sv;
    size_t count = points.size();
    for (const Point &point : points){
        out << point.x << ',' << point.y;
        if (--count)
            out << ' ';
    }
    out << '\"';
    RenderAttrs(out);
    out << "/>"sv;
    }

Text &Text::SetPosition(Point pos)
{
    position = pos;
    return *this;
}

Text &Text::SetOffset(Point offset)
{
    this->offset = offset;
    return *this;
}

Text &Text::SetFontSize(uint32_t size)
{
    this->size = size;
    return *this;
}

Text &Text::SetFontFamily(std::string font_family)
{
    this->font_family = font_family;
    return *this;
}

Text &Text::SetFontWeight(std::string font_weight)
{
    this->font_weight = font_weight;
    return *this;
}

Text &Text::SetData(std::string data)
{
    auto first_symbol = data.find_first_not_of(' ');
    auto last_symbol = data.find_last_not_of(' ');

    data = data.substr(first_symbol, last_symbol - first_symbol + 1);

    size_t iter = data.size();
    while (iter > 0){
        --iter;
        if (text_replace_symbols.count(data[iter])) {
            char tmp = data[iter];
            data.erase(iter, 1);
            data.insert(iter, text_replace_symbols.at(tmp));
        }
    }
    this->data = data;
    return *this;
}

void Text::RenderObject(const RenderContext &context) const
{
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\"" << position.x << "\"";
    out << " y=\"" << position.y << "\"";
    out << " dx=\"" << offset.x << "\"";
    out << " dy=\"" << offset.y << "\"";
    if (size > 0)
        out << " font-size=\"" << size << "\"";
    if (!font_family.empty())
        out << " font-family=\"" << font_family << "\"";
    if (!font_weight.empty())
        out << " font-weight=\"" << font_weight << "\"";
    out << ">"sv;
    out << data;
    out << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object> &&obj)
{
    objects.push_back(std::move(obj));
}

void Document::Render(std::ostream &out) const
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for(auto &obj_ptr : objects){
        obj_ptr.get()->Render({out,2,2});
    }

    out << "</svg>"sv;

}

std::ostream &operator<<(std::ostream &out, const StrokeLineCap &val)
{
    out << map_StrokeLineCap.at(val);
    return out;
}

std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &val)
{
    out << map_StrokeLineJoin.at(val);
    return out;
}

struct ColorPrinter{
    std::ostream& out;
    void operator()(std::monostate){
        out << "none";
    }
    void operator()(const std::string& s){
        out << s;
    }
    void operator()(const Rgb& color){
        out << "rgb(" << color.red * 1. << ',' << color.green * 1. << ',' << color.blue * 1. << ')';
    }
    void operator()(const Rgba& color){
        out << "rgba(" << color.red * 1. << ',' << color.green * 1. << ',' << color.blue * 1. << ',' << color.opacity << ')';
    }

};

std::ostream &operator<<(std::ostream &out, const Color& color)
{
    std::visit(ColorPrinter{out}, color);
    return out;
}

Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue)
    : red{red}
    , green{green}
    , blue{blue} {
}

Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
    : Rgb{red, green, blue}
    , opacity{opacity} {
}

}  // namespace svg
