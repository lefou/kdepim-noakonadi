#include <RMM_Defines.h>
#include <RMM_ParameterList.h>
#include <RMM_Parameter.h>
#include <RMM_Token.h>

using namespace RMM;

RParameterList::RParameterList()
    :   RHeaderBody()
{
    // Empty.
}

RParameterList::RParameterList(const RParameterList & l)
    :   RHeaderBody(l),
        list_(l.list_)
{
    // Empty.
}

RParameterList::RParameterList(const QCString & s)
    :   RHeaderBody(s)
{
    // Empty.
}


RParameterList::~RParameterList()
{
    // Empty.
}

    RParameterList &
RParameterList::operator = (const RParameterList & l)
{
    if (this == &l) return *this;
    list_ = l.list_;
    RHeaderBody::operator = (l);
    return *this;
}

    RParameterList &
RParameterList::operator = (const QCString & s)
{
    strRep_ = s;
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RParameterList::operator == (RParameterList & l)
{
    parse();
    l.parse();

    return false; // XXX: Write this
}

    void
RParameterList::_parse()
{
    list_.clear();
    
    QStrList l;
    RTokenise(strRep_, ";", l, true, false);
    
    QStrListIterator it(l);
    
    for (; it.current(); ++it) {
        
        RParameter p(QCString(it.current()).stripWhiteSpace());
        p.parse();
        list_ << p;
    }
}

    void
RParameterList::_assemble()
{
    bool firstTime = true;
    
    QValueList<RParameter>::Iterator it;

    strRep_ = "";
    
    for (it = list_.begin(); it != list_.end(); ++it) {
        
        (*it).assemble();
        
        if (!firstTime) {
            strRep_ += QCString(";\n    ");
            firstTime = false;
        }

        strRep_ += (*it).asString();
    }
}

    void
RParameterList::createDefault()
{
    // STUB
}

    QValueList<RParameter>
RParameterList::list()
{
    parse();
    QValueList<RParameter>::Iterator it(list_.begin());

    for (; it != list_.end(); ++it) {
    rmmDebug("Parameter: `" + (*it).attribute() + "', `" +
                        (*it).value() + "'");
    }
    return list_;
}

    void
RParameterList::setList(QValueList<RParameter> & l)
{
    parse();
    list_ = l;
}

// vim:ts=4:sw=4:tw=78
