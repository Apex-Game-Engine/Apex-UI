#include <iostream>
#include <cstdio>
#include <vector>
#include <cassert>

using s8 = signed char;
using s16 = signed short;
using s32 = signed int;
using s64 = signed long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long;

#define LOG_FUNC() printf("%s\n", __PRETTY_FUNCTION__)

#include "UiEvents.h"

struct Rect
{
    float x, y;
    float width, height;
};

class UiBase
{
public:
    virtual ~UiBase() = default;
    
    virtual void OnCreateBase() = 0;
    virtual void OnDestroyBase() = 0;
    virtual void OnAttachBase() = 0;
    virtual void OnDetachBase() = 0;

    virtual UiBase*& Parent() = 0;
    virtual UiBase*& NextSibling() = 0;
    virtual UiBase*& PrevSibling() = 0;
    virtual UiBase*& FirstChild() = 0;
    virtual UiBase*& LastChild() = 0;

    virtual void Add(UiBase*) = 0;    // Adds a node to this ui subtree. This call happens before Create.
    virtual void Remove(UiBase*) = 0; // Removes the node from this ui subtree. This call happens after Destroy.

	virtual UiBase* OnPreviewEventBase(UiEventBase*) = 0;
	virtual bool OnEventBase(UiEventBase*) = 0;

    bool IsInitialized() const { return m_flags.isInitialized; }
    bool IsAttached() const { return m_flags.isAttached; }
    u8 GetFlags() const { return m_flags.value; }

	//virtual void Invalidate() = 0;
	//virtual void Draw() = 0;

protected:
    union {
        struct {
            u8 isInitialized : 1;
            u8 isAttached : 1;
        };
        u8 value = 0;
    } m_flags;
};

template <typename Impl>
class UiElement : public UiBase
{
public:
    UiElement() = default;
    ~UiElement() = default;

    UiBase*& Parent() override { return m_parent; }
    UiBase*& NextSibling() override { return m_nextSibling; }
    UiBase*& PrevSibling() override { return m_prevSibling; }
    UiBase*& FirstChild() override { return m_firstChild; }
    UiBase*& LastChild() override { return m_lastChild; }

    void Add(UiBase* child) override
    {
		assert(!IsInitialized() && !IsAttached());
        if (m_lastChild)
        {
            m_lastChild->NextSibling() = child;
            m_lastChild = child;
        }
        else
        {
            assert(!m_firstChild);
            m_firstChild = child;
            m_lastChild = child;
        }
		child->Parent() = this;
    }

    void Remove(UiBase* child) override
    {
		assert(!IsInitialized() && !IsAttached() && child->Parent() == this);

		if (m_parent && m_parent->FirstChild() == this)
        {
            m_parent->FirstChild() = m_nextSibling;
        }
        if (m_parent && m_parent->LastChild() == this)
        {
            m_parent->LastChild() = m_prevSibling;
        }

        if (m_nextSibling)
        {
            m_nextSibling->PrevSibling() = m_prevSibling;
        }
        if (m_prevSibling)
        {
            m_prevSibling->NextSibling() = m_nextSibling;
        }
    }

protected:
#define IMPL (*static_cast<Impl*>(this))

    void OnCreateBase() override
    {
        assert(!IsInitialized());
        IMPL.OnCreate();

        for (UiBase* child = m_firstChild; child != nullptr; child = child->NextSibling())
        {
            child->OnCreateBase();
        }

        m_flags.isInitialized = true;
    }

    void OnDestroyBase() override
    {
        assert(IsInitialized());
        for (UiBase* child = m_firstChild; child != nullptr;)
        {
            UiBase* next = child->NextSibling();
            child->OnDestroyBase();
            delete child;
            child = next;
        }

        IMPL.OnDestroy();
        m_flags.isInitialized = false;
    }

    void OnAttachBase() override
    {
        assert(!IsAttached());
        IMPL.OnAttach();
		m_flags.isAttached = true;

        for (UiBase* child = m_firstChild; child != nullptr; child = child->NextSibling())
        {
            child->OnAttachBase();
        }
    }

    void OnDetachBase() override
    {
        assert(IsAttached());
        if (m_parent && m_parent->FirstChild() == this)
        {
            m_parent->FirstChild() = m_nextSibling;
        }
        if (m_parent && m_parent->LastChild() == this)
        {
            m_parent->LastChild() = m_prevSibling;
        }

        if (m_nextSibling)
        {
            m_nextSibling->PrevSibling() = m_prevSibling;
        }
        if (m_prevSibling)
        {
            m_prevSibling->NextSibling() = m_nextSibling;
        }

		IMPL.OnDetach();
		m_flags.isAttached = false;
    }

	UiBase* OnPreviewEventBase(UiEventBase* e) override
	{
		UiBase* handler = nullptr;
		if (IMPL.OnPreviewEvent(e))
			return this;
		else
		{
			for (UiBase* child = m_firstChild; child != nullptr; child = child->NextSibling())
			{
				if (!(handler = child->OnPreviewEventBase(e)))
					break;
			}
		}
		return handler;
	}

	bool OnEventBase(UiEventBase* e) override
	{
		if (!IMPL.OnEvent(e))
		{
			return Parent()->OnEventBase(e);
		}
		return true;
	}

private:
    Rect bounds;
    UiBase* m_parent = nullptr;
    UiBase* m_nextSibling = nullptr;
    UiBase* m_prevSibling = nullptr;
    UiBase* m_firstChild = nullptr;
    UiBase* m_lastChild = nullptr;
};

class Panel : public UiElement<Panel>
{
public:
    void OnCreate()
    {
        printf("Create Panel (%p)\n", this);
    }

    void OnDestroy()
    {
        printf("Destroy Panel (%p)\n", this);
    }

    void OnAttach()
	{
		printf("Attach Panel\n");
	}

    void OnDetach()
	{
		printf("Detach Panel\n");
	}

	bool OnPreviewEvent(UiEventBase* e)
	{
		if (e->EventType() == UiEventType::MouseButtonDown)
		{
			if (!FirstChild())
			{
				printf("Handler : (%p)\n", this);
				return true;
			}
		}
		return false;
	}

	bool OnEvent(UiEventBase* e)
	{
		//if (!Parent())
		{
			printf("Handled : (%p)\n", this);
			return true;
		}
		return false;
	}

private:

};

class UiFactory
{
public:
	static void Create(UiBase* root)
	{
		root->OnCreateBase();
	}

	static void Destroy(UiBase* root)
	{
		root->OnDestroyBase();
	}

	static void Attach(UiBase* root)
	{
		root->OnAttachBase();
	}

	static void Detach(UiBase* root)
	{
		root->OnDetachBase();
	}

	static void DispatchEvent(UiBase* root, UiEventBase* e)
	{
		printf(">>> Event (%d)\n", e->eventId);
		UiBase* handler = root->OnPreviewEventBase(e);
		if (handler)
			handler->OnEventBase(e);
	}
};

int main()
{
    UiBase* panel = new Panel();

	Panel* childPanel0 = new Panel();
	panel->Add(childPanel0);
	panel->Add(new Panel());
	panel->Add(new Panel());
    
	UiFactory::Create(panel);
	UiFactory::Attach(panel);


	MouseButtonDownEvent event;
	event.eventId = 1;
	UiFactory::DispatchEvent(panel, &event);


	UiFactory::Detach(panel);
	UiFactory::Destroy(panel);

	delete panel;

    return 0;
}

