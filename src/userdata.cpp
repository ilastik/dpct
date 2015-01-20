#include "userdata.h"
#include <stdexcept>

namespace dpct
{

NameData::NameData(const std::string& name):
	name_(name)
{}

std::ostream& operator<<(std::ostream& lhs, const UserData& rhs)
{
    lhs << rhs.toString();
    return lhs;
}

std::ostream& operator<<(std::ostream& lhs, UserDataPtr rhs)
{
    lhs << rhs->toString();
    return lhs;
}

UserDataHolder::UserDataHolder(UserDataPtr data):
    data_(data)
{}

} // namespace dpct
