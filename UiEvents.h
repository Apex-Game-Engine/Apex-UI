#pragma once

enum class UiEventType
{
	MouseButtonDown,
	MouseButtonUp,

	KeyboardDown,
	KeyboardUp,
};

class UiEventBase
{
public:
    u32 eventId;

	virtual UiEventType EventType() = 0;
};

template <typename Event, UiEventType uiEventType>
class UiEvent : public UiEventBase
{
	static constexpr UiEventType kEventType = uiEventType;

public:
	static UiEventType StaticEventType() { return kEventType; }
	UiEventType EventType() override { return StaticEventType(); }
};

template <typename T>
class UiEventHandler
{
    bool OnEvent(T* e);
};


struct MouseButtonDownEvent : public UiEvent<MouseButtonDownEvent, UiEventType::MouseButtonDown>
{
	s32 x, y;
};

