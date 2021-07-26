#ifndef TYPE_H
#define TYPE_H

#include <vector>
#include <memory>
#include <string>
#include <type_traits>

class AbstractType
{
public:
    AbstractType(AbstractType *context = nullptr) : context(context) {}
    virtual ~AbstractType() {}

    // Copy
    AbstractType(const AbstractType &other)
    {
        *this = other;
    }

    AbstractType &operator=(const AbstractType &other)
    {
        if (this != &other)
        {
            this->context = other.context;
            this->name = other.name;
            this->type = other.type;
            this->size = other.size;
            this->offset = other.offset;
            this->qualifiers = other.qualifiers;
        }

        return *this;
    }

    // Move
    AbstractType(AbstractType &&other)
    {
       *this = std::move(other);
    }

    AbstractType &operator=(AbstractType &&other)
    {
        this->context = other.context;
        this->name = std::move(other.name);
        this->type = std::move(other.type);
        this->size = other.size;
        this->offset = other.offset;
        this->qualifiers = other.qualifiers;

        other.context = nullptr;
        other.size = 0;
        other.offset = 0;
        other.qualifiers = {0, 0, 0, 0, 0, 0};

        return *this;
    }

    AbstractType *context = nullptr;

    std::string name;
    std::string type;

    std::size_t size = 0;
    std::size_t offset = 0;
    std::size_t count = 1;

    struct
    {
        uint8_t is_const:1;
        uint8_t is_pointer:1;
        uint8_t is_array:1;
        uint8_t is_static:1;
        uint8_t is_volatile:1;
        uint8_t is_extern:1;
        uint8_t :2; // Unused
    } qualifiers = {0, 0, 0, 0, 0, 0};

    std::string full_name() const
    {
        std::string n;
        if (qualifiers.is_extern)
            n += "extern ";
        if (qualifiers.is_static)
            n += "static ";
        if (qualifiers.is_const)
            n += "const ";
        if (qualifiers.is_volatile)
            n += "volatile ";
        if (qualifiers.is_pointer)
            n += "*";
        n += name;
        if (qualifiers.is_array)
            n += ("[" + std::to_string(count) + "]");
        return n;
    }
};

class TrivialType : public AbstractType
{
public:
    TrivialType(AbstractType *context) : AbstractType(context) {}
    ~TrivialType() {}

    // Copy
    TrivialType(const TrivialType &other) : AbstractType(other.context)
    {
        *this = other;
    }

    TrivialType &operator=(const TrivialType &other)
    {
        if (this != &other)
        {
            AbstractType::operator=(other);
            this->bits = other.bits;
        }

        return *this;
    }

    // Move
    TrivialType(TrivialType &&other)
    {
       *this = std::move(other);
    }

    TrivialType &operator=(TrivialType &&other)
    {
        AbstractType::operator=(other);
        this->bits = other.bits;

        other.bits = 0;

        return *this;
    }

    std::size_t bits = 0;
};

class ComplexType : public AbstractType
{
public:
    ComplexType(AbstractType *context) : AbstractType(context) {}
    ~ComplexType()
    {
        for (AbstractType *m : members)
        {
            delete m;
        }
    }

    // Copy
    ComplexType(const ComplexType &other) : AbstractType(other.context)
    {
        *this = other;
    }

    ComplexType &operator=(const ComplexType &other)
    {
        if (this != &other)
        {
            AbstractType::operator=(other);

            for (AbstractType *m : other.members)
            {
                AbstractType *a = new AbstractType(*m);
                this->members.push_back(a);
            }
        }

        return *this;
    }

    // Move
    ComplexType(ComplexType &&other)
    {
       *this = std::move(other);
    }

    ComplexType &operator=(ComplexType &&other)
    {
        AbstractType::operator=(other);
        this->members = std::move(other.members);

        return *this;
    }

    std::vector<AbstractType*> members;
};

#endif // TYPE_H
