#pragma once

/*

Adds a nifty mechanism for events with multiple subscribers and bound instances for instance methods.

*/

template <typename TArg>
class EventHandler {
private:
	EventHandler<TArg>* Next = nullptr;
	EventHandler<TArg>* Previous = nullptr;

protected:
	virtual void OnInvoke(const TArg& arg) = 0;

public:
	void SetNext(EventHandler<TArg>* const next) {
		this->Next = next;
	}

	EventHandler<TArg>* GetNext() const {
		return this->Next;
	}

	void SetPrevious(EventHandler<TArg>* const previous) {
		this->Previous = previous;
	}

	EventHandler<TArg>* GetPrevious() const {
		return this->Previous;
	}

	EventHandler(EventHandler<TArg>* const previous) : Previous(previous) {

	}

	void Invoke(const TArg& arg) {
		this->OnInvoke(arg);
	}
};

template <typename TArg>
class StaticEventHandler : public EventHandler<TArg> {
public:
	typedef void(*CallbackFunc)(const TArg&);

private:
	CallbackFunc Callback = nullptr;

protected:
	virtual void OnInvoke(const TArg& arg) {
		Callback(arg);
	}

public:
	StaticEventHandler(CallbackFunc callback, EventHandler<TArg>* const previous) : EventHandler<TArg>(previous), Callback(callback) {

	}
};

template <typename TArg, typename TInstance>
class InstanceEventHandler : public EventHandler<TArg> {
public:
	typedef void(TInstance::*CallbackFunc)(const TArg&);

private:
	CallbackFunc Callback = nullptr;
	TInstance* Instance = nullptr;

protected:
	virtual void OnInvoke(const TArg& arg) {
		(Instance->*Callback)(arg);
	}

public:
	InstanceEventHandler(CallbackFunc callback, TInstance* const instance, EventHandler<TArg>* const previous) : EventHandler<TArg>(previous), Callback(callback), Instance(instance) {

	}
};

template <typename TArg>
class Event {
private:
	void AddHandler(EventHandler<TArg>* const handler) {
		if (this->First == nullptr) {
			this->First = handler;
		}
		else {
			this->Last->SetNext(handler);
		}
		handler->SetPrevious(this->Last);
		this->Last = handler;
	}

	EventHandler<TArg>* First = nullptr;
	EventHandler<TArg>* Last = nullptr;

public:
	EventHandler<TArg>* AddHandler(void(*callback)(const TArg&)) {
		EventHandler<TArg>* result = new StaticEventHandler<TArg>(callback, this->Last);
		AddHandler(result);
		return result;
	}

	template <typename TInstance>
	EventHandler<TArg>* AddHandler(void(TInstance::*callback)(const TArg&), TInstance* const instance) {
		EventHandler<TArg>* result = new InstanceEventHandler<TArg, TInstance>(callback, instance, this->Last);
		AddHandler(result);
		return result;
	}

	void Invoke(const TArg& arg) {
		EventHandler<TArg>* handler = this->First;
		while (handler) {
			handler->Invoke(arg);
			handler = handler->GetNext();
		}
	}

	void RemoveHandler(EventHandler<TArg>* const handler) {
		EventHandler<TArg>* prev = handler->GetPrevious();
		EventHandler<TArg>* next = handler->GetNext();

		if (prev == nullptr) {
			this->First = next;
		}
		else {
			prev->SetNext(next);
		}

		if (next == nullptr) {
			this->Last = prev;
		}
		else {
			next->SetPrevious(prev);
		}
		delete handler;
	}

	~Event() {
		EventHandler<TArg>* handler = this->First;
		while (handler) {
			EventHandler<TArg>* next = handler->GetNext();
			delete handler;
			handler = next;
		}
		this->First = nullptr;
		this->Last = nullptr;
	}
};