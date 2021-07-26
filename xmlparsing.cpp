#include "typestorage.h"
#include "rapidxml.hpp"

#include <cstring>
#include <sstream>
#include <iostream>

/* Forward declarations */
static AbstractType *find_type(rapidxml::xml_node<> *root, const char *type_id, AbstractType *context);
static void set_offset(ComplexType *cv);
static std::size_t get_count(rapidxml::xml_node<> *e);
static std::size_t find_min_max(rapidxml::xml_node<> *root, const char *type_id);
static std::string find_file_id(rapidxml::xml_node<> *root, const std::string &src);

static inline bool streq(const char *s1, const char *s2) noexcept
{
    return (std::strcmp(s1, s2) == 0);
}

static bool find_fields(rapidxml::xml_node<> *root, const char *type_id, ComplexType *context)
{
    rapidxml::xml_node<> *n = root->first_node("Field");
    while (n)
    {
        rapidxml::xml_attribute<> *context_attr = n->first_attribute("context");
        if (!context_attr)
        {
            std::cerr << "ther is no \"context\" attribute" << std::endl;
            return false;
        }

        if (streq(context_attr->value(), type_id))
        {
            // Здесь надо записать смещение
            // Если тип - сложный, то затем необходимо прибавлять его смещение к смещению простых типов

            rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
            if (!type_attr)
            {
                std::cerr << "no \"type\" attribute" << std::endl;
                return false;
            }

            rapidxml::xml_attribute<> *name_attr = n->first_attribute("name");
            if (!name_attr)
            {
                std::cerr << "no \"name\" attribute" << std::endl;
                return false;
            }


            AbstractType *v = find_type(root, type_attr->value(), context);
            if (!v)
            {
                return false;
            }

            v->name = name_attr->value();

            if (rapidxml::xml_attribute<> *offset_attr = n->first_attribute("offset"))
            {
                std::size_t offset = 0;
                std::stringstream(offset_attr->value()) >> offset;

                if (v->context)
                    offset += v->context->offset;
                v->offset = offset;

                if (ComplexType *cv = dynamic_cast<ComplexType*>(v))
                {
                    set_offset(cv);
                }
            }

            if (rapidxml::xml_attribute<> *bits_attr = n->first_attribute("bits"))
            {
                if (TrivialType *tv = dynamic_cast<TrivialType*>(v))
                {
                    std::size_t bits = 0;
                    std::stringstream(bits_attr->value()) >> bits;

                    tv->bits = bits;
                }
            }

            context->members.push_back(v);
        }

        n = n->next_sibling("Field");
    }

    return !context->members.empty();
}

static AbstractType *find_type(rapidxml::xml_node<> *root, const char *type_id, AbstractType *context)
{
    rapidxml::xml_node<> *n = root->first_node();
    while (n)
    {
        rapidxml::xml_attribute<> *id_attr = n->first_attribute("id");
        if (!id_attr)
        {
            std::cerr << "ther is no \"id\" attribute" << std::endl;
            return nullptr;
        }

        if (streq(id_attr->value(), type_id))
        {
            if (streq(n->name(), "CvQualifiedType"))
            {
                rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
                if (!type_attr)
                {
                    std::cerr << "ther is no \"type\" attribute" << std::endl;
                    return nullptr;
                }

                AbstractType *v = find_type(root, type_attr->value(), context);
                if (v)
                {
                    v->qualifiers.is_const = (n->first_attribute("const") == nullptr ? 0 : 1);
                    v->qualifiers.is_volatile = (n->first_attribute("volatile") == nullptr ? 0 : 1);
                }

                return v;
            }
            else if (streq(n->name(), "PointerType"))
            {
                rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
                if (!type_attr)
                {
                    std::cerr << "ther is no \"type\" attribute" << std::endl;
                    return nullptr;
                }

                AbstractType *v = find_type(root, type_attr->value(), context);
                if (v)
                {
                    v->qualifiers.is_pointer = true;
                }

                return v;
            }
            else if (streq(n->name(), "ArrayType"))
            {
                rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
                if (!type_attr)
                {
                    std::cerr << "ther is no \"type\" attribute" << std::endl;
                    return nullptr;
                }

                AbstractType *v = find_type(root, type_attr->value(), context);
                if (v)
                {
                    std::size_t max = get_count(n);
                    if (max > 0)
                    {
                        v->count = max;
                        v->qualifiers.is_array = true;
                    }
                    else
                    {
                        v->qualifiers.is_array = false;
                    }
                }

                return v;
            }
            else if (streq(n->name(), "Typedef"))
            {
                rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
                if (!type_attr)
                {
                    std::cerr << "ther is no \"type\" attribute" << std::endl;
                    return nullptr;
                }

                AbstractType *v = find_type(root, type_attr->value(), context);
                if (rapidxml::xml_attribute<> *name_attr = n->first_attribute("name"))
                {
                    if (v)
                    {
                        v->name = name_attr->value();
                    }
                }

                return v;
            }
            else if (streq(n->name(), "ElaboratedType"))
            {
                rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
                if (!type_attr)
                {
                    std::cerr << "ther is no \"type\" attribute" << std::endl;
                    return nullptr;
                }

                return find_type(root, type_attr->value(), context);
            }
            else if (streq(n->name(), "Struct") || streq(n->name(), "Union"))
            {
                rapidxml::xml_attribute<> *name_attr = n->first_attribute("name");
                if (!name_attr)
                {
                    std::cerr << "there is no \"name\" attribute" << std::endl;
                    return nullptr;
                }

                /* Защита от рекурсии */
                if (context)
                {
                    // Проверка на пустую строку нужна потому что за рекурсию может приняться анонимная структура без имени
                    if (strlen(name_attr->value()) > 0 && !context->name.empty())
                    {
                        if (streq(name_attr->value(), context->name.c_str()))
                        {
                            std::cerr << "recursion detected" << std::endl;
                            std::cerr << "\"" << name_attr->value() << "\" \"" << context->name << "\"" << std::endl;
                            return nullptr;
                        }
                    }
                }

                ComplexType *cv = new ComplexType(context);
                cv->name = name_attr->value();

                if (rapidxml::xml_attribute<> *size_attr = n->first_attribute("size"))
                {
                    std::size_t size = 0;
                    std::stringstream(size_attr->value()) >> size;

                    cv->size = size;
                }

                if (find_fields(root, id_attr->value(), cv))
                {
                    return cv;
                }
                else
                {
                    delete cv;
                    return nullptr;
                }
            }
            else if (streq(n->name(), "FundamentalType"))
            {
                TrivialType *tv = new TrivialType(context);

                if (rapidxml::xml_attribute<> *size_attr = n->first_attribute("size"))
                {
                    std::size_t size = 0;
                    std::stringstream(size_attr->value()) >> size;

                    tv->bits = tv->size = size;
                }

                if (rapidxml::xml_attribute<> *name_attr = n->first_attribute("name"))
                {
                    tv->type = name_attr->value();
                }

                return tv;
            }
            else
            {
                std::cerr << "unimplemented type" << std::endl;
                return nullptr;
            }

            break;
        }

        n = n->next_sibling();
    }

    std::cerr << "internal error" << std::endl;
    return nullptr;
}

static std::size_t find_min_max(rapidxml::xml_node<> *root, const char *type_id)
{
    std::size_t result = 0;
    rapidxml::xml_node<> *n = root->first_node();
    while (n)
    {
        rapidxml::xml_attribute<> *id_attr = n->first_attribute("id");
        if (!id_attr)
        {
            std::cerr << "there is no \"id\" attribute" << std::endl;
            return result;
        }

        if (streq(id_attr->value(), type_id))
        {
            if (streq(n->name(), "CvQualifiedType"))
            {
                rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
                if (!type_attr)
                {
                    std::cerr << "there is no \"type\" attribute" << std::endl;
                    return result;
                }

                return find_min_max(root, type_attr->value());
            }
            else if (streq(n->name(), "PointerType"))
            {
                rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
                if (!type_attr)
                {
                    std::cerr << "there is no \"type\" attribute" << std::endl;
                    return result;
                }

                return find_min_max(root, type_attr->value());
            }
            else if (streq(n->name(), "ArrayType"))
            {
                return get_count(n);
            }
            else if (streq(n->name(), "Typedef"))
            {
                rapidxml::xml_attribute<> *type_attr = n->first_attribute("type");
                if (!type_attr)
                {
                    std::cerr << "there is no \"type\" attribute" << std::endl;
                    return result;
                }

                return find_min_max(root, type_attr->value());
            }

            break;
        }

        n = n->next_sibling();
    }

    return result;
}

static void set_offset(ComplexType *cv)
{
    for (auto m : cv->members)
    {
        m->offset += cv->offset;
        if (ComplexType *c = dynamic_cast<ComplexType*>(m))
        {
            set_offset(c);
        }
    }
}

static std::size_t get_count(rapidxml::xml_node<> *e)
{
    std::size_t count = 0;
    if (e->first_attribute("min") && e->first_attribute("max"))
    {
        char *min_str = e->first_attribute("min")->value();
        char *max_str = e->first_attribute("max")->value();

        std::size_t min = 0;
        std::size_t max = 0;

        std::stringstream ss;
        ss << min_str << " " << max_str;
        ss >> min >> max;

        count = max - min + 1;
    }

    return count;
}

static std::string find_file_id(rapidxml::xml_node<> *root, const std::string &src)
{
    rapidxml::xml_node<> *n = root->first_node("File");
    while (n)
    {
        rapidxml::xml_attribute<> *id_attr = n->first_attribute("id");
        rapidxml::xml_attribute<> *name_attr = n->first_attribute("name");
        if (!id_attr || !name_attr)
        {
            std::cerr << "there is no \"id\" or \"name\" attribute" << std::endl;
            continue;
        }

        char *s = name_attr->value();
        std::size_t len = name_attr->value_size();

        while (len--)
        {
            if (s[len] == '\\' || s[len] == '/')
            {
                char *tmp = &s[len + 1];
                if (tmp == src)
                    return id_attr->value();
            }
        }

        n = n->next_sibling("File");
    }

    return std::string();
}

bool Storage::parse(char *xml, const std::string &source_filename)
{
    rapidxml::xml_document<char> doc;
    try
    {
        doc.parse<0>(xml);
    }
    catch (rapidxml::parse_error &e)
    {
        std::cerr << e.what() << std::endl;
        emit parse_error(e.what());
        return false;
    }
    catch (...)
    {
        std::cerr << "unexpected exception" << std::endl;
        emit parse_error(tr("Неожиданная ошибка"));
        return false;
    }

    rapidxml::xml_node<> *root = doc.first_node();
    if (!root)
    {
        std::cerr << "root == nullptr" << std::endl;
        emit parse_error(tr("Ошибка при разборе XML-документа"));
        return false;
    }

    // Нужно найти ID временного XML-файла, чтобы получать глобальные переменные, которые расположены только в нём, иначе вероятны всякие неприятности
    std::string file_id = find_file_id(root, source_filename);

    bool result = true;
    std::vector<std::pair<std::string, std::shared_ptr<AbstractType>>> tmp; // Временное хранилище чтобы не добавлять переменные по частям, а скопом и возможности отмены всей операции

    rapidxml::xml_node<> *node = root->first_node("Variable");
    while (node)
    {
        rapidxml::xml_attribute<> *type_id = node->first_attribute("type");
        rapidxml::xml_attribute<> *name = node->first_attribute("name");
        rapidxml::xml_attribute<> *file_attr = node->first_attribute("file");
        if (!type_id || !name || !file_attr)
        {
            std::cerr << "there is no \"type\", \"name\" or \"file\" attribute" << std::endl;
            emit parse_error(tr("Неправильная структура XML"));
            continue;
        }

        if (file_attr->value() != file_id)
            continue;

        std::string variable_name;
        std::shared_ptr<AbstractType> v(find_type(root, type_id->value(), nullptr));
        if (v)
        {
            if (TrivialType *tv = dynamic_cast<TrivialType*>(v.get()))
            {
                tv->name = name->value();
                variable_name = tv->name;
            }
            else if (ComplexType *cv = dynamic_cast<ComplexType*>(v.get()))
            {
                variable_name = name->value(); // Имя переменной
                cv->type = cv->name;
                cv->name = variable_name;
            }

            v->qualifiers.is_static = (node->first_attribute("static") == nullptr ? 0 : 1);
            v->qualifiers.is_extern = (node->first_attribute("extern") == nullptr ? 0 : 1);

            // Массив из созданных глобальных переменных
            if (v->qualifiers.is_array)
            {
                std::size_t max = find_min_max(root, type_id->value());
                if (max > 0)
                {
                    v->count = max;
                }
                else
                {
                    v->qualifiers.is_array = 0;
                    emit parse_error(tr("Ошибка получения количества элементов массива для параметра \"%1\"").arg(v->name.c_str()));
                }
            }

            // Дубликаты не нужны. Но и прерывать разбор из-за такого недоразумения не надо
            if (contains_variable(variable_name))
            {
                emit parse_error(tr("Переменная под именем \"%1\" уже существует").arg(variable_name.c_str()));
            }
            else
            {
                tmp.emplace_back(std::piecewise_construct, std::forward_as_tuple(variable_name), std::forward_as_tuple(v));
            }
        }
        else
        {
            emit parse_error(tr("Ошибка добавления \"%1\"").arg(name->value()));

            result = false;
            break;
        }

        node = node->next_sibling("Variable");
    }

    if (result)
    {
        std::move(tmp.begin(), tmp.end(), std::inserter(m_variables, m_variables.end()));
    }

    return result;
}
