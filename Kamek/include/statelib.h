// This is no longer totally accurate to Nintendo's version.
// Should still work for Newer, though.

/******************************************************************************/
// dStateBase_c class
/******************************************************************************/

class dStateBase_c {
public:
	dStateBase_c(const char *name);

	virtual ~dStateBase_c();
	virtual bool isInvalid();
	virtual bool isEqualNotUsedForSomeReason(dStateBase_c *another);
	virtual bool isEqual(dStateBase_c *another);
	virtual bool isNotEqual(dStateBase_c *another);
	virtual bool isSameStateName(const char *name);
	virtual const char *getName();
	virtual int getID();
	virtual void doBegin(void *owner);
	virtual void doExecute(void *owner);
	virtual void doEnd(void *owner);


	static dStateBase_c mNoState;

private:
	const char *mName;
	int mID;

	static int mLastID;
};


/******************************************************************************/
// dState_c<TOwner> : dStateBase_c class
/******************************************************************************/

template <class TOwner>
class dState_c : public dStateBase_c {
public:
	typedef void (TOwner::*funcPtr)();

	dState_c(const char *name, funcPtr begin, funcPtr execute, funcPtr end) : dStateBase_c(name) {
		mBegin = begin;
		mExecute = execute;
		mEnd = end;
	}

	funcPtr mBegin;
	funcPtr mExecute;
	funcPtr mEnd;

	void *mBaseState;
};

/******************************************************************************/
// A Very Useless Class
/******************************************************************************/
class dStatePointless_c {
	public:
		virtual ~dStatePointless_c() { };
		virtual int returnsOne() { return 1; }
};

/******************************************************************************/
// Executor Interfaces
/******************************************************************************/
class dStateMethodExecutorBase_c {
	public:
		virtual ~dStateMethodExecutorBase_c() { }

		virtual void callBegin() = 0;
		virtual void callExecute() = 0;
		virtual void callEnd() = 0;
};

class dStateExecutorBase_c {
	public:
		virtual ~dStateExecutorBase_c() { }

		virtual dStateMethodExecutorBase_c *getMethodExecutorForState(dStateBase_c *state) = 0;
		virtual void clearMethodExecutor(dStateMethodExecutorBase_c **ptrToPtrToExecutor) = 0;
};

/******************************************************************************/
// Executor Implementations
/******************************************************************************/
template <class TOwner>
class dStateMethodExecutor_c : public dStateMethodExecutorBase_c {
	public:
		dStateMethodExecutor_c(TOwner *pOwner) : mOwningObject(pOwner) { }
		~dStateMethodExecutor_c() { }

		void callBegin();
		void callExecute();
		void callEnd();

		TOwner *mOwningObject;
		dState_c<TOwner> *mState;
};

template <class TOwner>
class dStateExecutor_c : public dStateExecutorBase_c {
	public:
		dStateExecutor_c(TOwner *pOwner) : mStockExecutor(pOwner) { }
		~dStateExecutor_c() { }

		dStateMethodExecutorBase_c *getMethodExecutorForState(dStateBase_c *state);
		void clearMethodExecutor(dStateMethodExecutorBase_c **ptrToPtrToExecutor);

	private:
		dStateMethodExecutor_c<TOwner> mStockExecutor;
};




template <class TOwner>
dStateMethodExecutorBase_c *dStateExecutor_c<TOwner>::getMethodExecutorForState(dStateBase_c *state) {
	if (!state->isInvalid()) {
		mStockExecutor.mState = static_cast<dState_c<TOwner>*>(state);
		return &mStockExecutor;
	} else {
		return 0;
	}
}

template <class TOwner>
void dStateExecutor_c<TOwner>::clearMethodExecutor(dStateMethodExecutorBase_c **ptrToPtrToExecutor) {
	*ptrToPtrToExecutor = 0;
}



template <class TOwner>
void dStateMethodExecutor_c<TOwner>::callBegin() { mState->doBegin(mOwningObject); }
template <class TOwner>
void dStateMethodExecutor_c<TOwner>::callExecute() { mState->doExecute(mOwningObject); }
template <class TOwner>
void dStateMethodExecutor_c<TOwner>::callEnd() { mState->doEnd(mOwningObject); }

/******************************************************************************/
// dStateMgrBase_c class
/******************************************************************************/
class dStateMgrBase_c {
public:
	dStateMgrBase_c(dStatePointless_c *pPointlessClass, dStateExecutorBase_c *pStateExecutor, dStateBase_c *pInitState);

	virtual ~dStateMgrBase_c();
	virtual void ensureStateHasBegun();
	virtual void execute();
	virtual void endCurrentState();
	virtual void setState(dStateBase_c *pNewState);

	virtual void executeNextStateThisTick() {
		mExecuteNextStateThisTick = true;
	}

	virtual dStateMethodExecutorBase_c *getCurrentStateMethodExecutor() {
		return mCurrentStateMethodExecutor;
	}

	virtual dStateBase_c *getNextState() {
		return mNextState;
	}

	virtual dStateBase_c *getCurrentState() {
		return mCurrentState;
	}

	virtual dStateBase_c *getPreviousState() {
		return mPreviousState;
	}

	virtual bool callBeginOnState() = 0;
	virtual void callExecuteOnState() = 0;
	virtual void callEndOnState() = 0;
	virtual void doSwitchToNextState(dStateBase_c *pState/*unused*/) = 0;

protected:
	dStatePointless_c *mPointlessClass;
	dStateExecutorBase_c *mStateExecutor;

	bool mIsStateBeginningOrEnding, mIsStateExecuting;
	bool mStateIsCurrentlyActive, mStateChangeHasOccurred;
	bool mExecuteNextStateThisTick;

	dStateBase_c *mNextState, *mPreviousState, *mCurrentState;
	dStateMethodExecutorBase_c *mCurrentStateMethodExecutor;
};

/******************************************************************************/
// dStateMgr_c : dStateMgrBase_c class
/******************************************************************************/
class dStateMgr_c : public dStateMgrBase_c {
public:
	dStateMgr_c(dStatePointless_c *pPointlessClass, dStateExecutorBase_c *pStateExecutor, dStateBase_c *pInitState);
	~dStateMgr_c() { }

	bool callBeginOnState();
	void callExecuteOnState();
	void callEndOnState();
	void doSwitchToNextState(dStateBase_c *pState/*unused*/);
};


