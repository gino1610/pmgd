#include "Index.h"
#include "AvlTreeIndex.h"
#include "List.h"
#include "exception.h"

using namespace Jarvis;

typedef AvlTreeIndex<long long,List<Node *>> LongValueIndex;
typedef AvlTreeIndex<double,List<Node *>> FloatValueIndex;
typedef AvlTreeIndex<bool,List<Node *>> BoolValueIndex;

void Index::init(PropertyType ptype)
{
    switch(ptype) {
        case t_integer:
            static_cast<LongValueIndex *>(this)->init();
            break;
        case t_float:
            static_cast<FloatValueIndex *>(this)->init();
            break;
        case t_boolean:
            static_cast<BoolValueIndex *>(this)->init();
            break;
        case t_time:
        case t_string:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
    _ptype = ptype;
}

void Index::add(const Property &p, Node *n, Allocator &allocator)
{
    if (_ptype != p.type())
        throw Exception(property_type);

    // TODO: Tree is unnecessary and Node* needs better arrangement for
    // quick search and remove operations
    List<Node*> *dest = NULL;

    switch(_ptype) {
        case t_integer:
            dest = static_cast<LongValueIndex *>(this)->add(p.int_value(),
                                                            allocator);
            break;
        case t_float:
            dest = static_cast<FloatValueIndex *>(this)->add(p.float_value(),
                                                             allocator);
            break;
        case t_boolean:
            dest = static_cast<BoolValueIndex *>(this)->add(p.bool_value(),
                                                            allocator);
            break;
        case t_time:
        case t_string:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
    // dest will never be null since it gets allocated at the add time.
    if (dest->num_elems() == 0)
        dest->init();
    dest->add(n, allocator);
}

void Index::remove(const Property &p, Node *n, Allocator &allocator)
{
    if (_ptype != p.type())
        throw Exception(property_type);

    List<Node*> *dest;
    switch(_ptype) {
        case t_integer:
            {
                LongValueIndex *prop_idx = static_cast<LongValueIndex *>(this);
                dest = prop_idx->find(p.int_value());
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(p.int_value(), allocator);
                }
            }
            break;
        case t_float:
            {
                FloatValueIndex *prop_idx = static_cast<FloatValueIndex *>(this);
                dest = prop_idx->find(p.float_value());
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(p.float_value(), allocator);
                }
            }
            break;
        case t_boolean:
            {
                BoolValueIndex *prop_idx = static_cast<BoolValueIndex *>(this);
                dest = prop_idx->find(p.bool_value());
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(p.float_value(), allocator);
                }
            }
            break;
        case t_time:
        case t_string:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
}

namespace Jarvis {
    class NodeIndexIteratorImpl : public NodeIteratorImpl {
        const List<Node *>::ListType *_pos;
    public:
        NodeIndexIteratorImpl(List<Node *> *l)
            : _pos(l->begin())
        { }
        Node &operator*() const { return *_pos->value; }
        Node *operator->() const { return _pos->value; }
        Node &operator*() { return *_pos->value; }
        Node *operator->() { return _pos->value; }
        operator bool() const { return _pos != NULL; }
        bool next()
        {
            _pos = _pos->next; 
            return _pos != NULL;
        }
    };
}

NodeIterator Index::get_nodes(const PropertyPredicate &pp)
{
    // Since we will not have range support yet, only the equal relation
    // is valid
    if (pp.op != PropertyPredicate::eq)
        throw Exception(not_implemented);

    // TODO: Only handling eq case right now and the check has
    // already been done when this is called. Use value v1 from pp.
    const Property &p = pp.v1;
    List<Node *> *list = NULL;

    if (_ptype != p.type())
        throw Exception(property_type);
    switch(_ptype) {
        case t_integer:
            {
                LongValueIndex *prop_idx = static_cast<LongValueIndex *>(this);
                list = prop_idx->find(p.int_value());
            }
            break;
        case t_float:
            {
                FloatValueIndex *prop_idx = static_cast<FloatValueIndex *>(this);
                list = prop_idx->find(p.float_value());
            }
            break;
        case t_boolean:
            {
                BoolValueIndex *prop_idx = static_cast<BoolValueIndex *>(this);
                list = prop_idx->find(p.bool_value());
            }
            break;
        case t_time:
        case t_string:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
    if (list == NULL)
        return NodeIterator(NULL);
    else
        return NodeIterator(new NodeIndexIteratorImpl(list));
}