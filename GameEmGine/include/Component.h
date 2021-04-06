#pragma once
#include <string>
#include <unordered_map>
class Component
{
public:
	//enum COMP_TYPE{UNKNOWN};
	typedef std::string COMP_TYPE;
	static bool m_exit;
private:
	std::vector<Component* >m_children;
	Component* m_parent;
protected:
	Component(Component* parent = nullptr);
	Component(COMP_TYPE type, Component* parent = nullptr);
	virtual ~Component();
	COMP_TYPE m_type;

public:
	virtual	COMP_TYPE getCompType()
	{
		return m_type;
	}
	static const std::unordered_map<COMP_TYPE, unsigned>& getComponentList();

	virtual void addChild(Component* child);
	virtual void removeChild(Component* child);
	virtual void removeChild(unsigned index);
	virtual void setParent(Component* parent);
	virtual void removeParent(Component* parent);

	virtual Component* getChild(unsigned int index);
	virtual Component* getParent();
	virtual std::vector<Component*>& getChildren();
};

