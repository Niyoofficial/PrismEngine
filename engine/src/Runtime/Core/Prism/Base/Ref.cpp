#include "pcpch.h"

namespace Prism
{
std::unordered_set<RefCounted*> ReferenceManager::s_references;
std::unordered_set<RefCounted*> ReferenceManager::s_constructionList;
}
