#include "Component.h"
std::unordered_map<Component::COMP_TYPE, unsigned>
m_compList = std::unordered_map<Component::COMP_TYPE, unsigned>();
bool Component::m_exit = false;

Component::Component(Component* parent):m_parent(parent)
{
	m_type = "UNKNOWN";
	m_compList.find(m_type) != m_compList.end() ? m_compList[m_type]++ : 0;
}

Component::Component(COMP_TYPE type, Component* parent): m_parent(parent)
{
	m_type = type;
	m_compList.find(m_type) != m_compList.end() ? m_compList[m_type]++ : 0;/*this dose not matter but I did it anyways*/
}

Component::~Component()
{
	if(!m_exit)
		if(!(--m_compList[m_type]))
			m_compList.erase(m_type);
}


const std::unordered_map<Component::COMP_TYPE, unsigned>& Component::getComponentList()
{
	return m_compList;
}

void Component::addChild(Component* child)
{
	if(!child)return;

	if(std::find(m_children.begin(), m_children.end(), child) == m_children.end())
		m_children.push_back(child);
	child->m_parent = this;
}

void Component::removeChild(Component* child)
{
	if(!child)return;

	child->m_parent = nullptr;

	std::vector<Component*>::iterator ref;
	if((ref = std::find(m_children.begin(), m_children.end(), child)) != m_children.end())
		m_children.erase(ref);


}

void Component::removeChild(unsigned index)
{
	if(index >= m_children.size())return;

	m_children[index]->m_parent = nullptr;
	m_children.erase(m_children.begin() + index);

}

void Component::setParent(Component* parent)
{
	if(!parent)return;

	parent->addChild(this);

}

void Component::removeParent(Component* parent)
{
	if(!parent)return;

	parent->removeChild(this);
}

Component* Component::getChild(unsigned int index)
{
	return m_children[index];
}

Component* Component::getParent()
{
	return m_parent;
}

std::vector<Component*>& Component::getChildren()
{
	return m_children;
}

